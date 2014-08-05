#include "transportationlistwriter.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>

// 30 minutes
#define STANDAARD_OPHAALTIJD_IN_SECONDEN 1800

TransportationListWriter::TransportationListWriter(QWidget *parent) :
    QWidget(parent)
{
    m_markers.clear();
    empty_bags_of_kurk_needed = 0;
    empty_bags_of_kaarsresten_needed = 0;
    total_distance_in_meters = 0;
    total_time_on_the_road_in_seconds = 0;
    seconds_needed_to_complete_transport = 0;
    ready = false;

    nameTransportationListEdit = new QLineEdit();
    startTimeEdit = new QTimeEdit();
    expectedArrivalTimeEdit = new QTimeEdit();
    expectedArrivalTimeEdit->setDisabled(true);
    dateEdit = new QDateEdit( QDate::currentDate(), this ); // zie http://doc.qt.digia.com/3.3/qdateedit.html
    dateEdit->setDisplayFormat("dd MMM yyyy");
    dateEdit->setLocale(QLocale::Dutch);
    dateEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup

    empty_bags_of_kurk_neededEdit = new QSpinBox();
    empty_bags_of_kurk_neededEdit->setValue(empty_bags_of_kurk_needed);
    empty_bags_of_kaarsresten_neededEdit = new QSpinBox();
    empty_bags_of_kaarsresten_neededEdit->setValue(empty_bags_of_kaarsresten_needed);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                    | QDialogButtonBox::Cancel);
    QPushButton* resetButton = new QPushButton(tr("Reset"));
    buttonBox->addButton(resetButton,QDialogButtonBox::ResetRole);

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(setOriginalValues()));

    QFormLayout *formlayout = new QFormLayout();
    formlayout->addRow(tr("Bestandsnaam:"),nameTransportationListEdit);
    formlayout->addRow(tr("Datum ophaling:"),dateEdit);
    formlayout->addRow(tr("Start om:"),startTimeEdit);
    formlayout->addRow(tr("Terug om:"),expectedArrivalTimeEdit);
    //line
    formlayout->addRow(tr("Lege zakken voor kurk:"),empty_bags_of_kurk_neededEdit);
    formlayout->addRow(tr("Lege zakken voor kaarsresten:"),empty_bags_of_kaarsresten_neededEdit);
    formlayout->addRow(buttonBox);

    connect(startTimeEdit,SIGNAL(timeChanged(QTime)),this,SLOT(editExpectedArrivalTime(QTime)));
//    QVBoxLayout *layout = new QVBoxLayout();
//    layout->addLayout(formlayout);

    setLayout(formlayout);
}

TransportationListWriter::~TransportationListWriter()
{
    qDebug() << "start to deconstruct TransportationListWriter()";
    deleteTheMatrices();
    delete nameTransportationListEdit;
    delete dateEdit;
    delete startTimeEdit;
    delete expectedArrivalTimeEdit;
    delete empty_bags_of_kurk_neededEdit;
    delete empty_bags_of_kaarsresten_neededEdit;
 //   delete buttonBox; -> is this taken care of by ~QWidget?
 //   delete resetButton; -> is this taken care of by ~QWidget?
    qDebug() << "TransportationListWriter() deconstructed";
}

void TransportationListWriter::prepare(QList<SMarker *> _m_markers, int **_distance_matrix_in_meters, int **_distance_matrix_in_seconds)
{
    if(m_markers.length() > 0)
    {

        qDebug() << "TransportationListWriter has been called before, so we must delete the matrices!";
        deleteTheMatrices();

    }

    m_markers.clear();
    m_markers = _m_markers;
    int nr_of_cities = m_markers.length();

    distance_matrix_in_meters = new int*[nr_of_cities];
    distance_matrix_in_seconds = new int*[nr_of_cities];
    for(int i=0; i < nr_of_cities; i++)
     {
         distance_matrix_in_meters[i] = new int[nr_of_cities];
         distance_matrix_in_seconds[i] = new int[nr_of_cities];
     }

    for(int i=0; i < nr_of_cities; i++)
     {
         for(int j=0; j < nr_of_cities; j++)
         {
             distance_matrix_in_seconds[i][j] = _distance_matrix_in_seconds[i][j];
             distance_matrix_in_meters[i][j] = _distance_matrix_in_meters[i][j];
         }
     }

    ready = false;
    seconds_needed_to_complete_transport = 0;

    empty_bags_of_kurk_needed = 0;
    empty_bags_of_kaarsresten_needed = 0;
    total_distance_in_meters = 0;
    total_time_on_the_road_in_seconds = 0;

    int previous_distance_matrix_i = -1; // we do not need the information of the starting point in the Transportation List
    int counter = 1;

    // read each SMarker, in order shown, and add its information to the Transportation List
    for(int i = 0; i < m_markers.length(); i++)
    {
        qDebug() << "[TransportationListWriter::prepare()]" << "reading marker" << i << "of" << m_markers.length();
        int current_distance_matrix_i = m_markers[i]->distancematrixindex;
        if(previous_distance_matrix_i > -1)
        {
            populateWithSmarker(m_markers[i], previous_distance_matrix_i, current_distance_matrix_i);
        }
        previous_distance_matrix_i = current_distance_matrix_i;
        counter++;
    }

    if(m_markers.length() > 1)
    {
        qDebug() << "[TransportationListWriter::prepare()]" << "adding startpoint to end at" << previous_distance_matrix_i << 0;
        populateWithSmarker(m_markers[0],previous_distance_matrix_i, 0);
        qDebug() << "[TransportationListWriter::prepare()]" << "adding startpoint is done";

    }

    empty_bags_of_kurk_neededEdit->setValue(empty_bags_of_kurk_needed);
    empty_bags_of_kaarsresten_neededEdit->setValue(empty_bags_of_kaarsresten_needed);
    startTimeEdit->setTime(QTime(8,0));
    editExpectedArrivalTime(startTimeEdit->time());
    ready = true;

    qDebug() << "[TransportationListWriter::prepare()]" << "Done";

}

void TransportationListWriter::print()
{
    qDebug() << "vergeet niet ophaling ook op te slaan in DB!";
    if(!ready)
    {
        qDebug() << "[Vervoerslijst]" << "Not yet ready to print, you should prepare first!";
        return;
    }

    qDebug() << "vergeet niet ophaling ook op te slaan in DB!" << "als volgt: aanmeldingen.volgnr + aanmeldingen.datumophaling";
    // want maar één ophaling per dag max?

    ready = false;
    seconds_needed_to_complete_transport = 0;

    total_distance_in_meters = 0;
    total_time_on_the_road_in_seconds = 0;

    qDebug() << "\n\n\n";
    qDebug() << "<vvim> debuginfo, m_markers.length() = " << m_markers.length();
    qDebug() << "\n\n\n";

    qDebug() << "+--------------------+";
    qDebug() << "|   VERVOERSLIJST    |";
    qDebug() << "+--------------------+";
    qDebug() << "";
    qDebug() << "De Vlaspit vzw";
    qDebug() << "Basilieklaan 53, Scherpenheuvel";
    qDebug() << "BTW .....";
    qDebug() << "";
    qDebug() << "\t\tOpgenomen in het register voor erkende ............";
    qDebug() << "";
    qDebug() << "VERVOERSLIJST\t\tDatum:   " << QLocale().toString(dateEdit->date());

    qDebug() << "Meenemen:" << empty_bags_of_kurk_neededEdit->value() << "lege zakken kurk en" << empty_bags_of_kaarsresten_neededEdit->value() << "lege zakken kaarsresten";

    int previous_distance_matrix_i = -1; // we do not need the information of the starting point in the Transportation List
    int counter = 0;

    for(int i = 0; i < m_markers.length(); i++)
    {
        qDebug() << "[TransportationListWriter::print()]" << "write information of marker nr" << i << "of in total" << m_markers.length();
        int current_distance_matrix_i = m_markers[i]->distancematrixindex;
        if(previous_distance_matrix_i > -1)
        {
            qDebug() << "Locatie #" << counter;
            char kaart_nr = 'A' + counter;
            qDebug() << "Kaartnr" << QChar(kaart_nr);
            writeInformation(m_markers[i], previous_distance_matrix_i, current_distance_matrix_i);
            if(m_markers[i]->ophaling)
            {
                //ophaling in databank steken
                QSqlQuery query;
                query.prepare("UPDATE aanmelding SET ophaalronde_datum = :date , volgorde = :counter "
                              "WHERE id = :aanmeldings_id ");
                query.bindValue(":date",dateEdit->date());
                query.bindValue(":counter",counter);
                query.bindValue(":aanmeldings_id",m_markers[i]->ophaalpunt.aanmelding_id);

                if(!query.exec())
                {
                    qCritical(QString(tr("UPDATE aanmelding SET ophaalronde_datum = %1 , volgorde = %2 WHERE id = %3").arg(dateEdit->date().toString()).arg(counter).arg(m_markers[i]->ophaalpunt.aanmelding_id).append(query.lastError().text())).toStdString().c_str());
                    qDebug() << " !!! fout in wegschrijven aanmelding: " << QString(tr("UPDATE aanmelding SET ophaalronde_datum = %1 , volgorde = %2 WHERE id = %3").arg(dateEdit->date().toString()).arg(counter).arg(m_markers[i]->ophaalpunt.aanmelding_id).append(query.lastError().text()));
                }
                else
                    qDebug() << " <<< aanmelding is nu officieel in ophaalronde opgenomen :-) >>>";
            }
        }
        previous_distance_matrix_i = current_distance_matrix_i;
        counter++;
    }

    if(m_markers.length() > 1)
    {
        qDebug() << "[TransportationListWriter::print()]" << "adding startpoint to end at" << previous_distance_matrix_i << 0;
        writeInformation(m_markers[0],previous_distance_matrix_i, 0);
        qDebug() << "[TransportationListWriter::print()]" << "adding startpoint is done";

    }

    // TELKENS
    QString name_vervoerslijst = nameTransportationListEdit->text();
    name_vervoerslijst.replace( QRegExp("\\W"),QString("")); // escape for filenames, see http://comments.gmane.org/gmane.comp.lib.qt.general/20276
    if(name_vervoerslijst.simplified().length() < 1)
        name_vervoerslijst = QLocale().toString(QDate::currentDate()).replace(QRegExp("\\W"),QString(""));

    //vervoersLijst.write(QString("vervoerslijst-(%1).odt").arg(name_vervoerslijst));

    qDebug() << "[Vervoerslijst]" << "Lege zakken voor kurk:" << empty_bags_of_kurk_needed << ", lege zakken voor kaarsresten:" << empty_bags_of_kaarsresten_needed;

    //setTotalWeightTotalVolume();
    //qDebug() << "[Vervoerslijst]" << "Totaal lading:" << ui->totalWeightEdit->text() << "kg and " << ui->totalVolumeEdit->text() << "liter";


    qDebug() << "vergeet niet ophaling ook op te slaan in DB!" << "als volgt: aanmeldingen.volgnr + aanmeldingen.datumophaling";
    // want maar één ophaling per dag max?
}


void TransportationListWriter::writeInformation(SMarker* marker, int previous_distance_matrix_i, int current_distance_matrix_i)
{
    total_distance_in_meters += distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
    total_time_on_the_road_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
    seconds_needed_to_complete_transport += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];

    qDebug() << "Verwachtte aankomsttijd:" << QLocale().toString(startTimeEdit->time().addSecs(seconds_needed_to_complete_transport));
    // .trim(4) ?   // .truncate(4) ?

    if(marker->ophaling)
    {
        qDebug() << "Deze locatie is een" << "OPHALING";
        QSqlQuery query;
        query.prepare("SELECT naam, straat, nr, bus, postcode, plaats, land, openingsuren, contactpersoon, telefoonnummer1, extra_informatie"
                       " FROM   ophaalpunten WHERE id = :id ");
        query.bindValue(":id",marker->ophaalpunt.ophaalpunt_id);
        if((query.exec()) && (query.next()))
        {
            QString naam = query.value(0).toString();
            QString straat = query.value(1).toString();
            QString nr = query.value(2).toString();
            QString bus = query.value(3).toString();
            QString postcode = query.value(4).toString();
            QString gemeente = query.value(5).toString();
            QString land = query.value(6).toString();
            QString openingsuren = query.value(7).toString();
            QString contactpersoon = query.value(8).toString();
            QString telefoonnummer = query.value(9).toString();
            QString extra_informatie = query.value(10).toString(); // extra_info uit AANMELDING HALEN!

            qDebug() << "..Naam:" << naam;
            qDebug() << "..Adres:" << straat << nr << bus;
            qDebug() << "..Postcode:" << postcode;
            qDebug() << "..Gemeente:" << gemeente;
            qDebug() << "..Telefoon:" << telefoonnummer;
            qDebug() << "..Contactpersoon:" << contactpersoon;
            qDebug() << "..Openingsuren:" << openingsuren;
            qDebug() << "..Speciale opmerkingen:" << extra_informatie;
            qDebug() << "....................................";
            qDebug() << "..op te halen kurk:" << marker->ophaalpunt.kg_kurk << "kg" << marker->ophaalpunt.zakken_kurk << "zakken";
            qDebug() << "....werkelijk opgehaald: ___ kg , ___ zakken";
            qDebug() << "..op te halen kaars:" << marker->ophaalpunt.kg_kaarsresten << "kg" << marker->ophaalpunt.zakken_kaarsresten << "zakken";
            qDebug() << "....werkelijk opgehaald: ___ kg , ___ zakken";
            qDebug() << "..lege zakken afgegeven: ____";
        }
        else
        {
            qDebug() << "er ging iets mis met de databank bij het opzoeken van ophaalpunt" << marker->ophaalpunt.ophaalpunt_id << ":" << query.lastError();
            // opnemen in vervoerslijst als reden waarom info niet is ingevuld?
        }

        seconds_needed_to_complete_transport += STANDAARD_OPHAALTIJD_IN_SECONDEN;
        empty_bags_of_kurk_needed += marker->ophaalpunt.zakken_kurk;
        empty_bags_of_kaarsresten_needed += marker->ophaalpunt.zakken_kaarsresten;
    }
    if(marker->levering)
    {
        qDebug() << "Deze locatie is een" << "LEVERING";
        seconds_needed_to_complete_transport += marker->leveringspunt.minutes_needed;
    }
    if((!marker->ophaling)&&(!marker->levering))
    {
        qDebug() << "Deze locatie is een" << "gewoon ADRES";
    }
//            qDebug() << "[Vervoerslijst]" << "departure time:" << seconds_needed_to_complete_transport; // make human readable

}


void TransportationListWriter::editExpectedArrivalTime(QTime arrival)
{
    expectedArrivalTimeEdit->setTime(arrival.addSecs(seconds_needed_to_complete_transport));
}

void TransportationListWriter::populateWithSmarker(SMarker* marker, int previous_distance_matrix_i, int current_distance_matrix_i)
{
    qDebug() << "[TransportationListWriter::populateWithSmarker()]" << previous_distance_matrix_i << current_distance_matrix_i;

    total_distance_in_meters += distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
    total_time_on_the_road_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
    seconds_needed_to_complete_transport += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];

    if(marker->ophaling)
    {
        qDebug() << "[TransportationListWriter::populateWithSmarker()]" << "marker is een ophaling, ophaalpunt" << marker->ophaalpunt.ophaalpunt_id;
        // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SOphaalpunt) => marker->ophaalpunt
        seconds_needed_to_complete_transport += STANDAARD_OPHAALTIJD_IN_SECONDEN;
        empty_bags_of_kurk_needed += marker->ophaalpunt.zakken_kurk;
        empty_bags_of_kaarsresten_needed += marker->ophaalpunt.zakken_kaarsresten;
    }
    if(marker->levering)
    {
        qDebug() << "[TransportationListWriter::populateWithSmarker()]" << "marker is een levering";
        // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SLevering) => marker->levering
        seconds_needed_to_complete_transport += marker->leveringspunt.minutes_needed;
    }
    if((!marker->ophaling)&&(!marker->levering))
    {
        qDebug() << "[TransportationListWriter::populateWithSmarker()]" << "marker is een adres";
        // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SMarker) => marker
        ;
    }
//            qDebug() << "[Vervoerslijst]" << "departure time:" << seconds_needed_to_complete_transport; // make human readable

}

void TransportationListWriter::reject()
{
    setOriginalValues();
    ready = false;
    close();
}

void TransportationListWriter::accept()
{
    close();
    qDebug() << "printing time!";
    print();
}

void TransportationListWriter::setOriginalValues()
{
    dateEdit->setDate(QDate::currentDate());
    empty_bags_of_kurk_neededEdit->setValue(empty_bags_of_kurk_needed);
    empty_bags_of_kaarsresten_neededEdit->setValue(empty_bags_of_kaarsresten_needed);
    startTimeEdit->setTime(QTime(8,0));
    editExpectedArrivalTime(startTimeEdit->time());
}

void TransportationListWriter::deleteTheMatrices()
{
    qDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "start";
    int nr_of_cities = m_markers.length();
    qDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "markers length" << nr_of_cities;

    if(nr_of_cities > 0)
    {
        qDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "..starting for loop";
        for(int i = 0; i < nr_of_cities; i++) {
            delete [] distance_matrix_in_meters[i];
            qDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "....loop-a";
            delete [] distance_matrix_in_seconds[i];
            qDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "....loop-b";
        }
        qDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "..ending for loop";
        delete [] distance_matrix_in_meters;
        delete [] distance_matrix_in_seconds;
    }
    qDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "done";
}

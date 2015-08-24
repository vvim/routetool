#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QUuid>
#include <QMessageBox>
#include "transportationlistwriter.h"
#include "globalfunctions.h"

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

    translist_doc = new TransportationListDocumentWriter(QDate::currentDate(),0,0);



    startTimeEdit = new QTimeEdit();
    expectedArrivalTimeEdit = new QTimeEdit();
    expectedArrivalTimeEdit->setDisabled(true);

    // how to set QDateEdit to 'NULL', it cannot be done: http://www.qtcentre.org/threads/17295-How-to-put-empty-value-in-QDateEdit
    // dateEdit = new QDateEdit( QDate::currentDate(), this ); // zie http://doc.qt.digia.com/3.3/qdateedit.html
    dateEdit = new QDateEdit(QDate::currentDate().addDays(-1));
    dateEdit->setDisplayFormat("dd MMM yyyy");
    dateEdit->setLocale(QLocale::Dutch);
    dateEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup

    empty_bags_of_kurk_neededEdit = new QSpinBox();
    empty_bags_of_kurk_neededEdit->setMaximum(9999);
    empty_bags_of_kurk_neededEdit->setValue(empty_bags_of_kurk_needed);
    empty_bags_of_kaarsresten_neededEdit = new QSpinBox();
    empty_bags_of_kaarsresten_neededEdit->setMaximum(9999);
    empty_bags_of_kaarsresten_neededEdit->setValue(empty_bags_of_kaarsresten_needed);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                    | QDialogButtonBox::Cancel);
    QPushButton* resetButton = new QPushButton(tr("Reset"));
    buttonBox->addButton(resetButton,QDialogButtonBox::ResetRole);

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(setOriginalValues()));

    QFormLayout *formlayout = new QFormLayout();
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
    setWindowTitle("Aanmaken vervoerslijst");
}

TransportationListWriter::~TransportationListWriter()
{
    vvimDebug() << "start to deconstruct TransportationListWriter()";
    deleteTheMatrices();
    delete dateEdit;
    delete startTimeEdit;
    delete expectedArrivalTimeEdit;
    delete empty_bags_of_kurk_neededEdit;
    delete empty_bags_of_kaarsresten_neededEdit;
    delete translist_doc;
 //   delete buttonBox; -> is this taken care of by ~QWidget?
 //   delete resetButton; -> is this taken care of by ~QWidget?
    vvimDebug() << "TransportationListWriter() deconstructed";
}

void TransportationListWriter::prepare(QList<SMarker *> _m_markers, int **_distance_matrix_in_meters, int **_distance_matrix_in_seconds, QWidget *_mapwidget)
{
    setOriginalValues();
    if(m_markers.length() > 0)
    {

        vvimDebug() << "TransportationListWriter has been called before, so we must delete the matrices!";
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
        vvimDebug() << "[TransportationListWriter::prepare()]" << "reading marker" << i << "of" << m_markers.length();
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
        vvimDebug() << "[TransportationListWriter::prepare()]" << "adding startpoint to end at" << previous_distance_matrix_i << 0;
        populateWithSmarker(m_markers[0],previous_distance_matrix_i, 0);
        vvimDebug() << "[TransportationListWriter::prepare()]" << "adding startpoint is done";

    }

    empty_bags_of_kurk_neededEdit->setValue(empty_bags_of_kurk_needed);
    empty_bags_of_kaarsresten_neededEdit->setValue(empty_bags_of_kaarsresten_needed);
    startTimeEdit->setTime(QTime(8,0));
    editExpectedArrivalTime(startTimeEdit->time());

    mapwidget = _mapwidget;
    ready = true;

    vvimDebug() << "[TransportationListWriter::prepare()]" << "Done";

}

void TransportationListWriter::print()
{
    vvimDebug() << "vergeet niet ophaling ook op te slaan in DB!";
    if(!ready)
    {
        vvimDebug() << "[Vervoerslijst]" << "Not yet ready to print, you should prepare first!";
        return;
    }

    vvimDebug() << "vergeet niet ophaling ook op te slaan in DB!" << "als volgt: aanmeldingen.volgnr + aanmeldingen.datumophaling";
    // want maar één ophaling per dag max?

    ready = false;
    seconds_needed_to_complete_transport = 0;

    total_distance_in_meters = 0;
    total_time_on_the_road_in_seconds = 0;

    vvimDebug() << "\n\n\n";
    vvimDebug() << "<vvim> debuginfo, m_markers.length() = " << m_markers.length();
    vvimDebug() << "\n\n\n";

    vvimDebug() << "+--------------------+";
    vvimDebug() << "|   VERVOERSLIJST    |";
    vvimDebug() << "+--------------------+";
    vvimDebug() << "";
    vvimDebug() << "De Vlaspit vzw";
    vvimDebug() << "Basilieklaan 53, Scherpenheuvel";
    vvimDebug() << "BTW .....";
    vvimDebug() << "";
    vvimDebug() << "\t\tOpgenomen in het register voor erkende ............";
    vvimDebug() << "";
    vvimDebug() << "VERVOERSLIJST\t\tDatum:   " << QLocale().toString(dateEdit->date());

    vvimDebug() << "Meenemen:" << empty_bags_of_kurk_neededEdit->value() << "lege zakken kurk en" << empty_bags_of_kaarsresten_neededEdit->value() << "lege zakken kaarsresten";

    delete translist_doc;
    translist_doc = new TransportationListDocumentWriter(dateEdit->date(),empty_bags_of_kurk_neededEdit->value(),empty_bags_of_kaarsresten_neededEdit->value());

    int previous_distance_matrix_i = -1; // we do not need the information of the starting point in the Transportation List
    int counter = 0;

    for(int i = 0; i < m_markers.length(); i++)
    {
        vvimDebug() << "[TransportationListWriter::print()]" << "write information of marker nr" << i << "of in total" << m_markers.length();
        int current_distance_matrix_i = m_markers[i]->distancematrixindex;
        if(previous_distance_matrix_i > -1)
        {
            vvimDebug() << "Locatie #" << counter;
            char kaart_nr = 'A' + counter;
            vvimDebug() << "Kaartnr" << QChar(kaart_nr);
            writeInformation(m_markers[i], previous_distance_matrix_i, current_distance_matrix_i, counter, kaart_nr);
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
                    vvimDebug() << " !!! fout in wegschrijven aanmelding: " << QString(tr("UPDATE aanmelding SET ophaalronde_datum = %1 , volgorde = %2 WHERE id = %3").arg(dateEdit->date().toString()).arg(counter).arg(m_markers[i]->ophaalpunt.aanmelding_id).append(query.lastError().text()));
                }
                else
                    vvimDebug() << " <<< aanmelding is nu officieel in ophaalronde opgenomen :-) >>>";



                if(!query.exec())
                {
                    vvimDebug() << "something went wrong with query. Try to reconnect to DB" << query.lastError().text();
                    QString SQLquery = QString("UPDATE aanmelding SET ophaalronde_datum = %1 , volgorde = %2 WHERE id = %3").arg(dateEdit->date().toString()).arg(counter).arg(m_markers[i]->ophaalpunt.aanmelding_id);

                    if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
                    {
                        vvimDebug() << "unable to reconnect to DB: FAIL";
                        qCritical("Unable to reconnect to DB: FAIL");
                        vvimDebug() << " !!! fout in wegschrijven aanmelding: " << SQLquery << query.lastError().text();
                    }
                    else
                    {
                        if(!query.exec())
                        {
                            vvimDebug() << "query failed after reconnecting to DB" << SQLquery << query.lastError();
                            qCritical(SQLquery.append(query.lastError().text()).toStdString().c_str());
                            vvimDebug() << " !!! fout in wegschrijven aanmelding: " << SQLquery << query.lastError().text();
                        }
                    }
                }
            }
        }
        previous_distance_matrix_i = current_distance_matrix_i;
        counter++;
    }

    if(m_markers.length() > 1)
    {
        char kaart_nr = 'A' + counter;
        vvimDebug() << "[TransportationListWriter::print()]" << "adding startpoint to end at" << previous_distance_matrix_i << 0 << counter;
        writeInformation(m_markers[0],previous_distance_matrix_i, 0, counter, kaart_nr);
        vvimDebug() << "[TransportationListWriter::print()]" << "adding startpoint is done";

    }

    // escape for filenames, see http://comments.gmane.org/gmane.comp.lib.qt.general/20276 -> http://qt-project.org/doc/qt-5/qregexp.html#characters-and-abbreviations-for-sets-of-characters
    QString uniqueFeatureForFilenames = QLocale().toString(dateEdit->date()).append("-").append(QUuid::createUuid ()).replace(QRegExp("\\W"),QString(""));

    vvimDebug() << "[Vervoerslijst]" << "Lege zakken voor kurk:" << empty_bags_of_kurk_needed << ", lege zakken voor kaarsresten:" << empty_bags_of_kaarsresten_needed;

    //setTotalWeightTotalVolume();
    //vvimDebug() << "[Vervoerslijst]" << "Totaal lading:" << ui->totalWeightEdit->text() << "kg and " << ui->totalVolumeEdit->text() << "liter";


    //make screenshot of the QWidget "mapwidget" so that we have a screenshot of the route (built by Google Maps)
    //see also http://qt-project.org/doc/qt-4.8/desktop-screenshot.html
    //         http://stackoverflow.com/questions/2430877/how-to-save-a-qpixmap-object-to-a-file
    //     and http://qt-project.org/doc/qt-5/quuid.html#createUuid
    QPixmap pixmap = QPixmap::grabWidget(mapwidget);

    QString filename_map = QString(tr("kaart")).append(uniqueFeatureForFilenames).append(".png");
    QFile file(filename_map);
    file.open(QIODevice::WriteOnly);
    pixmap.save(&file, "PNG");

    QString name_vervoerslijst = QString(tr("vervoerslijst")).append(uniqueFeatureForFilenames);
    translist_doc->write(name_vervoerslijst, filename_map);

    vvimDebug() << "Vervoerslijst is aangemaakt en heet" << name_vervoerslijst << ".doc, de kaart heet" << filename_map;
    QMessageBox::information(this, tr("Vervoerslijst is aangemaakt"), tr("De vervoerslijst is aangemaakt, het bestand heet '%1.doc' en de kaart is opgeslagen in het bestand '%2'.").arg(name_vervoerslijst).arg(filename_map));
}


void TransportationListWriter::writeInformation(SMarker* marker, int previous_distance_matrix_i, int current_distance_matrix_i, int counter, char kaart_nr)
{
    total_distance_in_meters += distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
    total_time_on_the_road_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
    seconds_needed_to_complete_transport += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];

    vvimDebug() << "Verwachtte aankomsttijd:" << QLocale().toString(startTimeEdit->time().addSecs(seconds_needed_to_complete_transport));
    // .trim(4) ?   // .truncate(4) ?

    if(marker->ophaling)
    {
        vvimDebug() << "Deze locatie is een" << "OPHALING" << counter << kaart_nr;
        QSqlQuery query;
        query.prepare("SELECT naam, straat, nr, bus, postcode, plaats, land, openingsuren, contactpersoon, telefoonnummer1, extra_informatie"
                       " FROM   ophaalpunten WHERE id = :id ");
        query.bindValue(":id",marker->ophaalpunt.ophaalpunt_id);

        if(!query.exec())
        {
            vvimDebug() << "something went wrong with the query" << query.lastError().text() << "trying to reconnect to the DB";
            QString SQLquery = QString("SELECT naam, straat, nr, bus, postcode, plaats, land, openingsuren, contactpersoon, telefoonnummer1, extra_informatie"
                                       " FROM   ophaalpunten WHERE id = %1 ").arg(marker->ophaalpunt.ophaalpunt_id);
            if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
            {
                vvimDebug() << "unable to reconnect to DB: FAIL";
            }
            else
            {
                vvimDebug() << "reconnected to DB, try query again";
                if(!query.exec())
                {
                    vvimDebug() << "query failed after reconnecting to DB" << SQLquery << query.lastError();
                }
            }
        }

        if(query.next())
        {
            TransportationListDocumentWriter::Ophaalpunt doc_ophaalpunt;
            doc_ophaalpunt.counter = counter;
            doc_ophaalpunt.kaart_nr = kaart_nr;
            doc_ophaalpunt.arrivaltime = startTimeEdit->time().addSecs(seconds_needed_to_complete_transport);
            doc_ophaalpunt.naam = query.value(0).toString();
            doc_ophaalpunt.straat = query.value(1).toString();
            doc_ophaalpunt.nr = query.value(2).toString();
            doc_ophaalpunt.bus = query.value(3).toString();
            doc_ophaalpunt.postcode = query.value(4).toString();
            doc_ophaalpunt.gemeente = query.value(5).toString();
            doc_ophaalpunt.land = query.value(6).toString();
            doc_ophaalpunt.openingsuren = query.value(7).toString();
            doc_ophaalpunt.contactpersoon = query.value(8).toString();
            doc_ophaalpunt.telefoonnummer = query.value(9).toString();
            doc_ophaalpunt.opmerkingen = marker->ophaalpunt.opmerkingen;
            doc_ophaalpunt.kg_kurk = marker->ophaalpunt.kg_kurk;
            doc_ophaalpunt.kg_kaarsresten = marker->ophaalpunt.kg_kaarsresten;
            doc_ophaalpunt.zakken_kurk = marker->ophaalpunt.zakken_kurk;
            doc_ophaalpunt.zakken_kaarsresten = marker->ophaalpunt.zakken_kaarsresten;

            translist_doc->addOphaalpunt(doc_ophaalpunt);


            // <cut-here>
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
            QString opmerkingen = marker->ophaalpunt.opmerkingen;

            vvimDebug() << "..Naam:" << naam;
            vvimDebug() << "..Adres:" << straat << nr << bus;
            vvimDebug() << "..Postcode:" << postcode;
            if(land.left(4).toLower() != "belg")
            {
                vvimDebug() << "..Gemeente:" << gemeente << "("<<land<<")";
            }
            else
                vvimDebug() << "..Gemeente:" << gemeente;
            vvimDebug() << "..Telefoon:" << telefoonnummer;
            vvimDebug() << "..Contactpersoon:" << contactpersoon;
            vvimDebug() << "..Openingsuren:" << openingsuren;
            vvimDebug() << "..Speciale opmerkingen:" << opmerkingen;
            vvimDebug() << "....................................";
            vvimDebug() << "..op te halen kurk:" << marker->ophaalpunt.kg_kurk << "kg" << marker->ophaalpunt.zakken_kurk << "zakken";
            vvimDebug() << "....werkelijk opgehaald: ___ kg , ___ zakken";
            vvimDebug() << "..op te halen kaars:" << marker->ophaalpunt.kg_kaarsresten << "kg" << marker->ophaalpunt.zakken_kaarsresten << "zakken";
            vvimDebug() << "....werkelijk opgehaald: ___ kg , ___ zakken";
            vvimDebug() << "..lege zakken afgegeven: ____";
            // </cut-here>
        }
        else
        {
            vvimDebug() << "er ging iets mis met de databank bij het opzoeken van ophaalpunt" << marker->ophaalpunt.ophaalpunt_id << ":" << query.lastError();
            // opnemen in vervoerslijst als reden waarom info niet is ingevuld?
        }

        seconds_needed_to_complete_transport += STANDAARD_OPHAALTIJD_IN_SECONDEN;
        empty_bags_of_kurk_needed += marker->ophaalpunt.zakken_kurk;
        empty_bags_of_kaarsresten_needed += marker->ophaalpunt.zakken_kaarsresten;
    }
    if(marker->levering)
    {
        vvimDebug() << "Deze locatie is een" << "LEVERING" << counter << kaart_nr;
        seconds_needed_to_complete_transport += marker->leveringspunt.minutes_needed;

        TransportationListDocumentWriter::Levering doc_levering;
        doc_levering.counter = counter;
        doc_levering.kaart_nr = kaart_nr;
        doc_levering.arrivaltime = startTimeEdit->time().addSecs(seconds_needed_to_complete_transport);
        doc_levering.naam = marker->leveringspunt.name;
        doc_levering.straat = marker->leveringspunt.street;
        doc_levering.nr = marker->leveringspunt.housenr;
        doc_levering.bus = marker->leveringspunt.busnr;
        doc_levering.postcode = marker->leveringspunt.postalcode;
        doc_levering.gemeente = marker->leveringspunt.plaats;
        doc_levering.land = marker->leveringspunt.country;
        doc_levering.contactpersoon = marker->leveringspunt.contactperson;
        doc_levering.telefoonnummer = marker->leveringspunt.telephone;
        doc_levering.weight = marker->leveringspunt.weight;
        doc_levering.volume = marker->leveringspunt.volume;

        translist_doc->addLevering(doc_levering);

        QString naam = marker->leveringspunt.name;
        QString straat = marker->leveringspunt.street;
        QString nr = marker->leveringspunt.housenr;
        QString bus = marker->leveringspunt.busnr;
        QString postcode = marker->leveringspunt.postalcode;
        QString gemeente = marker->leveringspunt.plaats;
        QString land = marker->leveringspunt.country;
        QString contactpersoon = marker->leveringspunt.contactperson;
        QString telefoonnummer = marker->leveringspunt.telephone;
        double weight = marker->leveringspunt.weight;
        double volume = marker->leveringspunt.volume;

        vvimDebug() << "..Naam:" << naam;
        vvimDebug() << "..Adres:" << straat << nr << bus;
        vvimDebug() << "..Postcode:" << postcode;

        if(land.left(4).toLower() != "belg")
        {
            vvimDebug() << "..Gemeente:" << gemeente << "("<<land<<")";
        }
        else
            vvimDebug() << "..Gemeente:" << gemeente;

        vvimDebug() << "..Telefoon:" << telefoonnummer;
        vvimDebug() << "..Contactpersoon:" << contactpersoon;
        vvimDebug() << "....................................";
        vvimDebug() << "..af te leveren:" << weight << "kg" << volume << "liter";
        vvimDebug() << "....werkelijk afgeleverd: ___ kg , ___  liter";

        // ergens in databank opnemen? Aparte table voor LEVERINGEN ???
        seconds_needed_to_complete_transport += marker->leveringspunt.minutes_needed * 60;
    }
    if((!marker->ophaling)&&(!marker->levering))
    {
        TransportationListDocumentWriter::Adres doc_adres;
        doc_adres.counter = counter;
        doc_adres.kaart_nr = kaart_nr;
        doc_adres.arrivaltime = startTimeEdit->time().addSecs(seconds_needed_to_complete_transport);
        doc_adres.caption = marker->caption;

        translist_doc->addAdres(doc_adres);

        vvimDebug() << "Deze locatie is een" << "gewoon ADRES" << counter << kaart_nr;
        vvimDebug() << "..Adres:" << marker->caption;
    }
}


void TransportationListWriter::editExpectedArrivalTime(QTime arrival)
{
    expectedArrivalTimeEdit->setTime(arrival.addSecs(seconds_needed_to_complete_transport));
}

void TransportationListWriter::populateWithSmarker(SMarker* marker, int previous_distance_matrix_i, int current_distance_matrix_i)
{
    vvimDebug() << "[TransportationListWriter::populateWithSmarker()]" << previous_distance_matrix_i << current_distance_matrix_i;

    total_distance_in_meters += distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
    total_time_on_the_road_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
    seconds_needed_to_complete_transport += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];

    if(marker->ophaling)
    {
        vvimDebug() << "[TransportationListWriter::populateWithSmarker()]" << "marker is een ophaling, ophaalpunt" << marker->ophaalpunt.ophaalpunt_id;
        // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SOphaalpunt) => marker->ophaalpunt
        seconds_needed_to_complete_transport += STANDAARD_OPHAALTIJD_IN_SECONDEN;
        empty_bags_of_kurk_needed += marker->ophaalpunt.zakken_kurk;
        empty_bags_of_kaarsresten_needed += marker->ophaalpunt.zakken_kaarsresten;
    }
    if(marker->levering)
    {
        vvimDebug() << "[TransportationListWriter::populateWithSmarker()]" << "marker is een levering";
        // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SLevering) => marker->levering
        seconds_needed_to_complete_transport += marker->leveringspunt.minutes_needed * 60;
    }
    if((!marker->ophaling)&&(!marker->levering))
    {
        vvimDebug() << "[TransportationListWriter::populateWithSmarker()]" << "marker is een adres";
        // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SMarker) => marker
        ;
    }
//            vvimDebug() << "[Vervoerslijst]" << "departure time:" << seconds_needed_to_complete_transport; // make human readable

}

void TransportationListWriter::reject()
{
    setOriginalValues();
    ready = false;
    close();
}

void TransportationListWriter::accept()
{

    if(dateEdit->date() < QDate::currentDate())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Datum ligt in het verleden"),
                      tr("De opgegeven datum voor de vervoerslijst ligt in het verleden.\n\nWeet u zeker dat u de vervoerlijst wilt opstellen voor %1?").arg(QLocale().toString(dateEdit->date())),
                                QMessageBox::Yes|QMessageBox::No);

        if(reply == QMessageBox::No)
            return;
    }

    close();
    vvimDebug() << "printing time!";
    print();
}

void TransportationListWriter::setOriginalValues()
{
    // how to set QDateEdit to 'NULL', it cannot be done: http://www.qtcentre.org/threads/17295-How-to-put-empty-value-in-QDateEdit
    dateEdit->setDate(QDate::currentDate().addDays(-1));
    empty_bags_of_kurk_neededEdit->setValue(empty_bags_of_kurk_needed);
    empty_bags_of_kaarsresten_neededEdit->setValue(empty_bags_of_kaarsresten_needed);
    startTimeEdit->setTime(QTime(8,0));
    editExpectedArrivalTime(startTimeEdit->time());
}

void TransportationListWriter::deleteTheMatrices()
{
    vvimDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "start";
    int nr_of_cities = m_markers.length();
    vvimDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "markers length" << nr_of_cities;

    if(nr_of_cities > 0)
    {
        vvimDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "..starting for loop";
        for(int i = 0; i < nr_of_cities; i++) {
            delete [] distance_matrix_in_meters[i];
            vvimDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "....loop-a";
            delete [] distance_matrix_in_seconds[i];
            vvimDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "....loop-b";
        }
        vvimDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "..ending for loop";
        delete [] distance_matrix_in_meters;
        delete [] distance_matrix_in_seconds;
    }
    vvimDebug() << "[TransportationListWriter::deleteTheMatrices()]" << "done";
}

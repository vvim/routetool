#include "transportationlistwriter.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

// 30 minutes
#define STANDAARD_OPHAALTIJD_IN_SECONDEN 1800

TransportationListWriter::TransportationListWriter(QWidget *parent) :
    QWidget(parent)
{
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

    /*
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(setOriginalValues()));
    */

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

void TransportationListWriter::prepare(QList<SMarker *> m_markers, int **_distance_matrix_in_meters, int **_distance_matrix_in_seconds)
{
    qDebug() << "[TransportationListWriter::prepare()]" << "start";
    distance_matrix_in_meters = _distance_matrix_in_meters;
    distance_matrix_in_seconds = _distance_matrix_in_seconds;
    qDebug() << "[TransportationListWriter::prepare()]" << "matrices filled in";
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

/*
    if(!ready)
    {
        qDebug() << "[Vervoerslijst]" << "Not yet ready to print, you should prepare first!";
    }
        qDebug() << "[Vervoerslijst]" << "Vraag: verwachtte vertrektijd???" << startTimeEdit->time().toString();

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
    qDebug() << "VERVOERSLIJST\t\tDatum:   " << dateEdit->date().toString();

    qDebug() << "<vvim> test met Locale:" << QLocale().toString(dateEdit->date());

    int empty_bags_of_kurk_needed = 0;
    int empty_bags_of_kaarsresten_needed = 0;

    qDebug() << "Meenemen:" << d << "lege zakken kurk en" << d << "lege zakken kaarsresten";

    //QString currentdate = QDate::currentDate().toString("d MMMM yyyy"); // QLocale::setDefault(QLocale::Dutch);
    QString currentdate = QLocale().toString(QDateTime::currentDateTime(),"d MMMM yyyy - hh:mm:ss");
    DocumentWriter vervoersLijst("",
                                 "",
                                 currentdate);


    int previous_distance_matrix_i = -1; // we do not need the information of the starting point in the Transportation List
    int total_distance_in_meters = 0;
    int total_time_on_the_road_in_seconds = 0;
    int total_time_needed_in_seconds = 8 * 60 * 60; // 8 AM
    int STANDAARD_OPHAALTIJD_IN_SECONDEN = 30 * 60; // 30 minutes
    int counter = 1;
    int empty_bags_of_kurk_needed = 0;
    int empty_bags_of_kaarsresten_needed = 0;

    // 0. for debug only:
    for(int i = 0; i < m_markers.length(); i++)
    {
        qDebug() << "[Vervoerslijst]" << i << ":" << m_markers[i]->caption;
    }

    // 5. go to each SMarker, in order shown, and add its information to the Transportation List
    for(int i = 0; i < m_markers.length(); i++)
    {
        int current_distance_matrix_i = m_markers[i]->distancematrixindex;
        if(previous_distance_matrix_i > -1)
        {

            total_distance_in_meters += distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
            total_time_on_the_road_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
            total_time_needed_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];

            qDebug() << "[Vervoerslijst]" << "location:" << m_markers[i]->caption;
            qDebug() << "[Vervoerslijst]" << "arrival time:" << total_time_needed_in_seconds; // make human readable

            // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SMarker) => m_markers[i]
            DocumentWriter::VisitLocation location;
            location.order = counter;
            location.distance_seconds = distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
            location.distance_meters = distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
            location.Naam = m_markers[i]->caption;
            location.aankomsttijd = total_time_needed_in_seconds;
            if(m_markers[i]->ophaling)
            {
                // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SOphaalpunt) => m_markers[i]->ophaalpunt
                location.Naam = m_markers[i]->ophaalpunt.naam;
                location.Adres = m_markers[i]->ophaalpunt.adres;
                location.Postcode = "een PC";
                location.Gemeente = "een gemeente";
                location.Telefoon = "een telefoonnr";
                location.Contactpersoon = "een contactpersoon";
                location.Openingsuren = "ma-vr ...";
                location.Speciale_opmerkingen = "speciale opmerk...";
                location.Soort_vervoer = "Ophaling";
                location.Kaartnr = "kaartnr?";
                location.Kurk_op_te_halen_zakken = QString("%1 zakken, %2 kg").arg(m_markers[i]->ophaalpunt.zakken_kurk).arg(m_markers[i]->ophaalpunt.kg_kurk);
                //location.Kaars_op_te_halen_zakken = QString("%1 zakken, %2 kg").arg(m_markers[i]->ophaalpunt.zakken_kaarsresten).arg(m_markers[i]->ophaalpunt.kg_kaarsresten);
                vervoersLijst.addVisit(location);
                total_time_needed_in_seconds += STANDAARD_OPHAALTIJD_IN_SECONDEN;
                empty_bags_of_kurk_needed += m_markers[i]->ophaalpunt.zakken_kurk;
                empty_bags_of_kaarsresten_needed += m_markers[i]->ophaalpunt.zakken_kaarsresten;
            }
            if(m_markers[i]->levering)
            {
                // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SLevering) => m_markers[i]->levering
                location.Naam = m_markers[i]->leveringspunt.name;
                location.Adres = ""; location.Adres.append(m_markers[i]->leveringspunt.street).append(" ").append(m_markers[i]->leveringspunt.housenr);
                location.Postcode = m_markers[i]->leveringspunt.postalcode;
                location.Gemeente = m_markers[i]->leveringspunt.plaats;
                location.Telefoon = m_markers[i]->leveringspunt.telephone;
                location.Contactpersoon = m_markers[i]->leveringspunt.contactperson;
                location.Openingsuren = "ma-vr ...";
                location.Speciale_opmerkingen = "speciale opmerk...";
                location.Soort_vervoer = "Levering";
                location.Kaartnr = "kaartnr?";
                // GEWICHT, VOLUME + TIJD DIE NODIG IS
                vervoersLijst.addVisit(location);
                total_time_needed_in_seconds += m_markers[i]->leveringspunt.minutes_needed;
            }
            if((!m_markers[i]->ophaling)&&(!m_markers[i]->levering))
            {
                // enkel een adres?
                location.Soort_vervoer = "Adres";
                vervoersLijst.addVisit(location);
            }
            counter++;
            qDebug() << "[Vervoerslijst]" << "departure time:" << total_time_needed_in_seconds; // make human readable

        }
        previous_distance_matrix_i = current_distance_matrix_i;
    }

    if(m_markers.length() > 1)
    {
        int i = 0;
        qDebug() << "[Vervoerslijst]" << "We moeten de terugkeer naar het startpunt ook nog in de vervoerslijst steken";

        total_distance_in_meters += distance_matrix_in_meters[previous_distance_matrix_i][0];
        total_time_on_the_road_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][0];
        total_time_needed_in_seconds += distance_matrix_in_seconds[previous_distance_matrix_i][0];

        qDebug() << "[Vervoerslijst]" << "location:" << m_markers[i]->caption;
        qDebug() << "[Vervoerslijst]" << "arrival time:" << total_time_needed_in_seconds; // make human readable

        // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SMarker) => m_markers[i]
        DocumentWriter::VisitLocation location;
        location.order = counter;
        location.distance_seconds = distance_matrix_in_seconds[previous_distance_matrix_i][0];
        location.distance_meters = distance_matrix_in_meters[previous_distance_matrix_i][0];
        location.Naam = m_markers[i]->caption;
        location.aankomsttijd = total_time_needed_in_seconds;
        if(m_markers[i]->ophaling)
        {
            // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SOphaalpunt) => m_markers[i]->ophaalpunt
            location.Naam = m_markers[i]->ophaalpunt.naam;
            location.Adres = m_markers[i]->ophaalpunt.adres;
            location.Postcode = "een PC";
            location.Gemeente = "een gemeente";
            location.Telefoon = "een telefoonnr";
            location.Contactpersoon = "een contactpersoon";
            location.Openingsuren = "ma-vr ...";
            location.Speciale_opmerkingen = "speciale opmerk...";
            location.Soort_vervoer = "Ophaling";
            location.Kaartnr = "kaartnr?";
            location.Kurk_op_te_halen_zakken = QString("%1 zakken, %2 kg").arg(m_markers[i]->ophaalpunt.zakken_kurk).arg(m_markers[i]->ophaalpunt.kg_kurk);
            //location.Kaars_op_te_halen_zakken = QString("%1 zakken, %2 kg").arg(m_markers[i]->ophaalpunt.zakken_kaarsresten).arg(m_markers[i]->ophaalpunt.kg_kaarsresten);
            vervoersLijst.addVisit(location);
            total_time_needed_in_seconds += STANDAARD_OPHAALTIJD_IN_SECONDEN;
            empty_bags_of_kurk_needed += m_markers[i]->ophaalpunt.zakken_kurk;
            empty_bags_of_kaarsresten_needed += m_markers[i]->ophaalpunt.zakken_kaarsresten;
        }
        if(m_markers[i]->levering)
        {
            // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SLevering) => m_markers[i]->levering
            location.Naam = m_markers[i]->leveringspunt.name;
            location.Adres = ""; location.Adres.append(m_markers[i]->leveringspunt.street).append(" ").append(m_markers[i]->leveringspunt.housenr);
            location.Postcode = m_markers[i]->leveringspunt.postalcode;
            location.Gemeente = m_markers[i]->leveringspunt.plaats;
            location.Telefoon = m_markers[i]->leveringspunt.telephone;
            location.Contactpersoon = m_markers[i]->leveringspunt.contactperson;
            location.Openingsuren = "ma-vr ...";
            location.Speciale_opmerkingen = "speciale opmerk...";
            location.Soort_vervoer = "Levering";
            location.Kaartnr = "kaartnr?";
            // GEWICHT, VOLUME + TIJD DIE NODIG IS
            vervoersLijst.addVisit(location);
            total_time_needed_in_seconds += (m_markers[i]->leveringspunt.minutes_needed)* 60;
        }
        if((!m_markers[i]->ophaling)&&(!m_markers[i]->levering))
        {
            // enkel een adres?
            location.Soort_vervoer = "Adres";
            vervoersLijst.addVisit(location);
        }
        counter++;
        qDebug() << "[Vervoerslijst]" << "departure time:" << total_time_needed_in_seconds; // make human readable

    }

    // TELKENS

    vervoersLijst.write(QString("vervoerslijst-(%1).odt").arg(currentdate));
    qDebug() << "[Vervoerslijst]" << "Lege zakken voor kurk:" << empty_bags_of_kurk_needed << ", lege zakken voor kaarsresten:" << empty_bags_of_kaarsresten_needed;

    //setTotalWeightTotalVolume();
    //qDebug() << "[Vervoerslijst]" << "Totaal lading:" << ui->totalWeightEdit->text() << "kg and " << ui->totalVolumeEdit->text() << "liter";
*/
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

//            qDebug() << "[Vervoerslijst]" << "location:" << marker->caption;
//            qDebug() << "[Vervoerslijst]" << "arrival time:" << total_time_needed_in_seconds; // make human readable

//            // dit zou een aparte functie kunnen zijn binnen DocumentWriter(SMarker) => marker
//            DocumentWriter::VisitLocation location;
//            location.order = counter;
//            location.distance_seconds = distance_matrix_in_seconds[previous_distance_matrix_i][current_distance_matrix_i];
//            location.distance_meters = distance_matrix_in_meters[previous_distance_matrix_i][current_distance_matrix_i];
//            location.Naam = marker->caption;
//            location.aankomsttijd = total_time_needed_in_seconds;
    if(marker->ophaling)
    {
        qDebug() << "[TransportationListWriter::populateWithSmarker()]" << "marker is een ophaling";
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

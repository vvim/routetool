#include "listofophaalpuntentocontact.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QApplication>
#include <QVBoxLayout>

#define OPHAALPUNT_ID Qt::UserRole

ListOfOphaalpuntenToContact::ListOfOphaalpuntenToContact(QWidget *parent) :
    QWidget(parent)
{
    label = new QLabel();
    contactList = new QListWidget();
    info = new InfoOphaalpunt();
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addWidget(contactList);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setMinimumWidth(600);
    setWindowTitle(tr("Lijst van te contacteren ophaalpunten:"));

    connect(contactList,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(showOphaalpunt(QListWidgetItem*)));

    qDebug() << "<vvim> TODO: should we call UptodateAllOphaalpunten() everytime we initialise the contactList?";
    UpdateAllOphaalpunten();
}

ListOfOphaalpuntenToContact::~ListOfOphaalpuntenToContact()
{
    qDebug() << "start to deconstruct ListOfOphaalpuntenToContact()";
    delete info;
    delete contactList;
    delete buttonBox;
    delete label;
    qDebug() << "ListOfOphaalpuntenToContact() deconstructed";
}

void ListOfOphaalpuntenToContact::UpdateOphaalpunt(int ophaalpuntid)
{
    QSqlQuery query;
    query.prepare("SELECT ophalinghistoriek.ophalingsdatum, ophaalpunten.last_contact_date, ophaalpunten.contact_again_on FROM `ophalinghistoriek`, ophaalpunten where ophalinghistoriek.ophaalpunt = ophaalpunten.id AND ophalinghistoriek.ophaalpunt= :ophaal order by ophalinghistoriek.ophalingsdatum desc;");
    query.bindValue(":ophaal",ophaalpuntid);
    if(query.exec())
    {
        if (query.next())
        {
            QDate laatste_ophaling = query.value(0).toDate();
            qDebug() << "laatste ophaling voor punt" << ophaalpuntid << "was" << laatste_ophaling << "ofte" << laatste_ophaling.toString();

            QDate ophaalpunt_LastContactDate = query.value(1).toDate();
            QDate ophaalpunt_ContactAgainOn = query.value(2).toDate();

            int aantal_ophalingen = 0;
            int aantal_dagen_tussen_verschillende_ophalingen = 0;
            QDate temp_ophaling = laatste_ophaling;
            while(query.next())
            {
                if(aantal_ophalingen >= 5)
                    break;
                QDate oudere_ophaling = query.value(0).toDate();
                aantal_dagen_tussen_verschillende_ophalingen += oudere_ophaling.daysTo(temp_ophaling);
                qDebug() << "....ophaling" << aantal_ophalingen << "gebeurde op" << oudere_ophaling.toString() << ", totaal dagen:" << aantal_dagen_tussen_verschillende_ophalingen;
                aantal_ophalingen++;
                temp_ophaling = oudere_ophaling;
            }
            qDebug() << "..einde van de while-loop na" << aantal_ophalingen << "loops. Totaal: "<< aantal_dagen_tussen_verschillende_ophalingen;
            if(aantal_ophalingen > 0)
            {
                int gemiddelde = aantal_dagen_tussen_verschillende_ophalingen / aantal_ophalingen;
                if(gemiddelde > 365)
                {
                    qDebug() << "Gemiddelde blijkt groter dan een jaar: " << gemiddelde << "dagen, afronden naar 1 jaar.";
                    gemiddelde = 365;
                }
                qDebug() << "..gemiddeld:" << gemiddelde;
                //if CURRENTDATE > laatste_ophaling + GEMIDDELDE : contact == current_date (-1) ()
                qDebug() << "..dus ophaalpunt te contacteren rond:" << laatste_ophaling.addDays(gemiddelde).toString() << "(waarde van laatste_ophaling verandert ook?)" << laatste_ophaling.toString();

                QSqlQuery query_lastcontact_and_contact_on;

                //  << "** IF last_ophaling > current_value of field (oplossen door waarde op te vragen in eerste SELECT ?? )")
                //  << "** IF last_ophaling > TODAY
                QString q = QString("UPDATE ophaalpunten SET last_contact_date = %1 , contact_again_on = %2 WHERE id = %3;").arg(laatste_ophaling.toString()).arg(qMax(laatste_ophaling.addDays(gemiddelde), qMax(QDate().currentDate(),ophaalpunt_ContactAgainOn)).toString()).arg(ophaalpuntid);
                query_lastcontact_and_contact_on.prepare("UPDATE ophaalpunten SET last_contact_date = :last_ophaling , contact_again_on = :contact_again WHERE id = :id;");
                query_lastcontact_and_contact_on.bindValue(":last_ophaling",laatste_ophaling);

                    qDebug() << "...datum controle:";
                    qDebug() << ".....laatste_ophaling:" << laatste_ophaling.toString();
                    qDebug() << ".....huidige waarde in DB: ophaalpunt_LastContactDate:" << ophaalpunt_LastContactDate.toString();
                    qDebug() << "...Neem het maximum van de volgende drie als de nieuwe 'contact_again_date':";
                    qDebug() << ".....huidige waarde in DB: ophaalpunt_ContactAgainOn:" << ophaalpunt_ContactAgainOn.toString();
                    qDebug() << ".....vandaag:" << QDate().currentDate().toString();
                    qDebug() << ".....voorspelling:" << laatste_ophaling.addDays(gemiddelde).toString();
                    qDebug() << "...Maximum is:" << qMax(laatste_ophaling.addDays(gemiddelde), qMax(QDate().currentDate(),ophaalpunt_ContactAgainOn));
                    qDebug() << "<vvim> TODO: na 2 jaar gebruiken tabel AANMELDING gebruiken ipv OPHAALHISTORIEK?";

                query_lastcontact_and_contact_on.bindValue(":contact_again ", qMax(laatste_ophaling.addDays(gemiddelde), qMax(QDate().currentDate(),ophaalpunt_ContactAgainOn)) );
                query_lastcontact_and_contact_on.bindValue(":id",ophaalpuntid);

                if(query_lastcontact_and_contact_on.exec())
                    qDebug() << "..." << q << "done";
                else
                    qDebug() << "..." << q << "went WRONG:" << query_lastcontact_and_contact_on.lastError();
            }
            else
            {
                qDebug() << "..minder dan 2 ophalingen, het heeft dus geen zin om het gemiddelde te berekenen. We kunnen wel het laatste contact invullen:";
                QSqlQuery query_lastcontact_only;
                //  << "** IF last_ophaling > current_value of field (oplossen door waarde op te vragen in eerste SELECT ?? )")
                //  << "** IF last_ophaling > TODAY
                QString q = QString("UPDATE ophaalpunten SET last_contact_date = %1 WHERE id = %2;").arg(laatste_ophaling.toString()).arg(ophaalpuntid);
                query_lastcontact_only.prepare("UPDATE ophaalpunten SET last_contact_date = :last_ophaling WHERE id = :id;");
                query_lastcontact_only.bindValue(":last_ophaling",laatste_ophaling);
                query_lastcontact_only.bindValue(":id",ophaalpuntid);


                if(query_lastcontact_only.exec())
                    qDebug() << "..." << q << "done";
                else
                    qDebug() << "..." << q << "went WRONG:" << query_lastcontact_only.lastError();
            }
        }
    }
    else
        qDebug() << "something went wrong with checking for an existing aanmelding";
}



void ListOfOphaalpuntenToContact::UpdateAllOphaalpunten()
{
    QSqlQuery query;
    query.prepare("SELECT DISTINCT ophaalpunt FROM `ophalinghistoriek`");
    if(query.exec())
    {
        qDebug() << "\n\n ---------- START invullen voorspellingen:" << QDateTime().currentDateTime().toString();
        while (query.next())
        {
            qDebug() << "\nophaalpunt" << query.value(0).toInt();
            UpdateOphaalpunt(query.value(0).toInt());
        }
        qDebug() << "\n\n ---------- END invullen voorspellingen:" << QDateTime().currentDateTime().toString();
        qDebug() << "check with query SELECT id, `last_contact_date`, `contact_again_on` FROM `ophaalpunten` WHERE `last_contact_date` > 0";
    }

}

void ListOfOphaalpuntenToContact::initialise()
{

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    label->setText(tr("Hieronder de lijst met ophaalpunten die al een historiek hebben en gecontacteerd dienen te worden.\n"
                      "Dubbelklik op een ophaalpunt om de informatie te zien."));

    contactList->clear();
    contactList->setSortingEnabled(false); // sort it by database: contact_again_on ASC ?

    QSqlQuery query("SELECT id, naam, postcode, last_contact_date, contact_again_on "
                    "FROM ophaalpunten "
                    "WHERE contact_again_on < date( CURDATE() + INTERVAL 1 DAY)"
                    "ORDER BY postcode");

    if(query.exec())
    {
        while (query.next())
        {
            QListWidgetItem * item = new QListWidgetItem();

            int ophaalpunt_id = query.value(0).toInt();
            QString ophaalpunt_naam = query.value(1).toString();
            ophaalpunt_naam.replace("\n"," ");
            QString ophaalpunt_postcode = query.value(2).toString();
            QDate last_contact_date = query.value(3).toDate();
            QDate contact_again_on = query.value(4).toDate();

            QString item_name = "";
            item_name.append(QLocale().toString(contact_again_on,"d MMM yyyy")).append(" - ").append(ophaalpunt_naam).append(" (postcode: ").append(ophaalpunt_postcode).append(")");
            item->setData(Qt::DisplayRole,item_name);

            item->setData(OPHAALPUNT_ID,ophaalpunt_id);
            contactList->addItem(item);
        }
    }
    else
    {
        qDebug() << "FATAL:" << "Something went wrong, could not execute query: SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES, aanmelding.id from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_nr is NULL";
        qFatal("Something went wrong, could not execute query: SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES, aanmelding.id from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_nr is NULL");
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}

void ListOfOphaalpuntenToContact::showOphaalpunt(QListWidgetItem* item)
{
    info->setWindowTitle(tr("info over ophaalpunt"));
    info->showOphaalpunt(item->data(OPHAALPUNT_ID).toInt());
}

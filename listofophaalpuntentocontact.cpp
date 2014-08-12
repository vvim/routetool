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
    nieuweaanmeldingWidget = new NieuweAanmelding();
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addWidget(contactList);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setMinimumWidth(600);
    setWindowTitle(tr("Lijst van te contacteren ophaalpunten:"));

    connect(contactList,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(showOphaalpunt(QListWidgetItem*)));
    connect(buttonBox,SIGNAL(accepted()),this,SLOT(ok_button_pushed()));
    connect(info,SIGNAL(nieuweAanmelding(int)),nieuweaanmeldingWidget,SLOT(aanmeldingVoorOphaalpunt(int)));


    qDebug() << "<vvim> TODO: should we call UptodateAllOphaalpunten() everytime we initialise the contactList?";
    UpdateAllOphaalpunten();
}

ListOfOphaalpuntenToContact::~ListOfOphaalpuntenToContact()
{
    qDebug() << "start to deconstruct ListOfOphaalpuntenToContact()";
    delete info;
    delete nieuweaanmeldingWidget;
    delete contactList;
    delete buttonBox;
    delete label;
    qDebug() << "ListOfOphaalpuntenToContact() deconstructed";
}

void ListOfOphaalpuntenToContact::UpdateOphaalpunt(int ophaalpuntid)
{
    qDebug() << "[ListOfOphaalpuntenToContact::UpdateOphaalpunt(int ophaalpuntid)]" << "ophaalpuntid = " << ophaalpuntid;

    QSqlQuery query;
    query.prepare("SELECT ophalinghistoriek.ophalingsdatum, ophaalpunten.last_contact_date, ophaalpunten.contact_again_on, "
                        " ophaalpunten.last_ophaling_date,  ophaalpunten.forecast_new_ophaling_date "
                  " FROM `ophalinghistoriek`, ophaalpunten "
                  " WHERE ophalinghistoriek.ophaalpunt = ophaalpunten.id AND ophalinghistoriek.ophaalpunt= :ophaal "
                  " ORDER BY ophalinghistoriek.ophalingsdatum DESC;");
    query.bindValue(":ophaal",ophaalpuntid);
    if(query.exec())
    {
        if (query.next())
        {
            QDate laatste_ophaling_taken_from_Ophaalhistoriek = query.value(0).toDate();

            // not used: QDate ophaalpunt_LastContactDate_taken_from_Ophaalpunten = query.value(1).toDate();
            // not used: QDate ophaalpunt_ContactAgainOn_taken_from_Ophaalpunten = query.value(2).toDate();
            QDate ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten = query.value(3).toDate();
            QDate ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten = query.value(4).toDate();

            /**
              1. we moeten ervoor zorgen dat de OPHAALPUNTEN.last_ophaling_date altijd correct is ingevuld:
                    qMax(ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten, laatste_ophaling_taken_from_Ophaalhistoriek)

              2. de last_contact_date wordt bijgehouden door Geert

              3. opnieuw contacteren op wordt ook door Geert bijgehouden, maar mag nooit meer dan een jaar later zijn

              4. voorspelling 'forecast_new_ophaling_date' wordt in deze functie berekend, maar mag nooit meer dan een jaar later zijn
                    qMin(gemiddelde, 365)
            **/


            /// A] berekenen van GEMIDDELD aantal dagen tussen ophalingen (om voorspelling / ForeCast te berekenen)

            int aantal_ophalingen = 0;
            int aantal_dagen_tussen_verschillende_ophalingen = 0;
            QDate temp_ophaling = laatste_ophaling_taken_from_Ophaalhistoriek;
            while(query.next())
            {
                /// we kijken niet verder terug dan 5 ophalingen
                if(aantal_ophalingen >= 5)
                    break;
                QDate oudere_ophaling = query.value(0).toDate(); // from table OPHAAL_HISTORIEK
                aantal_dagen_tussen_verschillende_ophalingen += oudere_ophaling.daysTo(temp_ophaling);
                qDebug() << "....ophaling" << aantal_ophalingen << "gebeurde op" << oudere_ophaling.toString() << ", totaal dagen:" << aantal_dagen_tussen_verschillende_ophalingen;
                aantal_ophalingen++;
                temp_ophaling = oudere_ophaling;
            }
            qDebug() << "..einde van de while-loop na" << aantal_ophalingen << "loops. Totaal: "<< aantal_dagen_tussen_verschillende_ophalingen;

            if(aantal_ophalingen > 0)
            {
                /// B] hebben we meer dan 2 voorspellingen, dan kunnen we een gemiddelde berekenen
                qDebug() << "..Aantal ophalingen:" << aantal_ophalingen << "> 0 => we kunnen een gemiddelde berekenen";

                int gemiddelde = aantal_dagen_tussen_verschillende_ophalingen / aantal_ophalingen;

                /// we zetten een voorspelling / ForeCast nooit verder in de tijd dan 365 dagen
                if(gemiddelde > 365)
                {
                    qDebug() << "....Gemiddelde blijkt groter dan een jaar: " << gemiddelde << "dagen, afronden naar 1 jaar.";
                    gemiddelde = 365;
                }
                qDebug() << "..gemiddeld:" << gemiddelde;

                QDate max_of_laatste_ophaling = qMax(laatste_ophaling_taken_from_Ophaalhistoriek, ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten);

                QSqlQuery query_forecast;
                query_forecast.prepare(" UPDATE ophaalpunten "
                                       " SET last_ophaling_date = :last_ophaling, forecast_new_ophaling_date = :forecast "
                                       " WHERE id = :id ");
                query_forecast.bindValue(":last_ophaling",max_of_laatste_ophaling);
                query_forecast.bindValue(":forecast",qMax(ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten, max_of_laatste_ophaling.addDays(gemiddelde)));
                query_forecast.bindValue(":id",ophaalpuntid);

                qDebug() << "...datum controle:";
                qDebug() << ".....laatste_ophaling:" << laatste_ophaling_taken_from_Ophaalhistoriek.toString();
                qDebug() << ".....huidige waarde in DB: ophaalpunt_LastContactDate:" << ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten.toString();
                qDebug() << "....... max:" << max_of_laatste_ophaling.toString();
                qDebug() << "...voorspelling is dus:";
                qDebug() << ".....berekend:" << max_of_laatste_ophaling.addDays(gemiddelde);
                qDebug() << ".....in de DB:" << ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten;
                qDebug() << "....... max:" << qMax(ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten, max_of_laatste_ophaling.addDays(gemiddelde));

                QString query_forecast_in_string = QString(" UPDATE ophaalpunten "
                                                           " SET last_ophaling_date = %1, forecast_new_ophaling_date = %2 "
                                                           " WHERE id = %3 ")
                                                    .arg(max_of_laatste_ophaling.toString("dd MM yyyy"))
                                                    .arg(qMax(ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten, max_of_laatste_ophaling.addDays(gemiddelde)).toString("dd MM yyyy"))
                                                    .arg(ophaalpuntid);

                if(query_forecast.exec())
                    qDebug() << "..." << query_forecast_in_string << "done";
                else
                    qDebug() << "..." << query_forecast_in_string << "went WRONG:" << query_forecast.lastError();
            }
            else
            {
                /// C] zijn er nog geen 2 ophaalmomenten geweest, dan controleren we of de tabel OPHAALPUNTEN de juiste info heeft
                qDebug() << "..Aantal ophalingen:" << aantal_ophalingen << "== 0 => we kunnen geen gemiddelde berekenen";
                qDebug() << "...we kunnen wel testen of de tabel Ophaalpunten de juiste informatie heeft:";
                qDebug() << "....laatste ophaling volgens tabel Ophaalpunten:" << ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten;
                qDebug() << "....laatste ophaling volgens tabel OphaalHistoriek:" << laatste_ophaling_taken_from_Ophaalhistoriek;

                if(ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten < laatste_ophaling_taken_from_Ophaalhistoriek)
                {
                    /// C.1] effectief, de datum in tabel OPHAALPUNTEN < de datum in tabel OPHAAL_HISTORIEK
                    ///        => update tabel OPHAALPUNTEN

                    QSqlQuery query_laatste_ophaling;
                    query_laatste_ophaling.prepare(" UPDATE ophaalpunten "
                                           " SET last_ophaling_date = :last_ophaling "
                                           " WHERE id = :id ");
                    query_laatste_ophaling.bindValue(":last_ophaling",laatste_ophaling_taken_from_Ophaalhistoriek);
                    query_laatste_ophaling.bindValue(":id",ophaalpuntid);

                    qDebug() << "...datum in ophaalpunten moet dus aangepast worden!!";

                    QString query_laatste_ophaling_in_string = QString(" UPDATE ophaalpunten "
                                                               " SET last_ophaling_date = %1 "
                                                               " WHERE id = %2 ")
                                                        .arg(laatste_ophaling_taken_from_Ophaalhistoriek.toString("dd MM yyyy"))
                                                        .arg(ophaalpuntid);

                    if(query_laatste_ophaling.exec())
                        qDebug() << "..." << query_laatste_ophaling_in_string << "done";
                    else
                        qDebug() << "..." << query_laatste_ophaling_in_string << "went WRONG:" << query_laatste_ophaling.lastError();
                }
            }
        }
    }
    else
        qDebug() << "[ListOfOphaalpuntenToContact::UpdateOphaalpunt(int ophaalpuntid)]" << "ophaalpuntid = " << ophaalpuntid << "-" << "something went wrong with checking for an existing aanmelding" << query.lastError();
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

    QSqlQuery query("SELECT id, naam, postcode, last_contact_date, contact_again_on, last_ophaling_date, forecast_new_ophaling_date "
                    "FROM ophaalpunten "
                    "WHERE forecast_new_ophaling_date < date( CURDATE() + INTERVAL 1 DAY)"
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
    info->showAanmeldingButton(true);
    info->setWindowTitle(tr("info over ophaalpunt"));
    info->showOphaalpunt(item->data(OPHAALPUNT_ID).toInt());
}

void ListOfOphaalpuntenToContact::ok_button_pushed()
{
    this->close();
}

void ListOfOphaalpuntenToContact::show_never_contacted_ophaalpunten()
{
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    label->setText(tr("Hieronder de lijst met ophaalpunten die nog geen ophaalhistoriek hebben.\n"
                      "Dubbelklik op een ophaalpunt om de informatie te zien."));

    contactList->clear();
    contactList->setSortingEnabled(true); // sorting is okay

    QSqlQuery query;
    query.prepare("SELECT ophaalpunten.id, ophaalpunten.naam, ophaalpunten.postcode "
                  "FROM ophaalpunten WHERE not exists "
                      "(select null from ophalinghistoriek "
                       "where ophalinghistoriek.ophaalpunt = ophaalpunten.id);");

    if(query.exec())
    {
        while (query.next())
        {
            QListWidgetItem * item = new QListWidgetItem();

            int ophaalpunt_id = query.value(0).toInt();
            QString ophaalpunt_naam = query.value(1).toString();
            ophaalpunt_naam.replace("\n"," ");
            QString ophaalpunt_postcode = query.value(2).toString();

            QString item_name = "";
            item_name.append(ophaalpunt_postcode).append(": ").append(ophaalpunt_naam);

            QSqlQuery query2;
            query2.prepare("SELECT * FROM aanmelding WHERE ophaalpunt = :ophaal"); // and ophaalronde is NULL
            query2.bindValue(":ophaal",query.value(0).toInt());

            if(query2.exec())
            {
                if (query2.next())
                {
                    item_name.append(tr(" ( ** dit ophaalpunt heeft een aanmelding lopende)"));
                    item->setForeground(Qt::blue);
                }
            }
            else
                qDebug() << "something went wrong with checking for an existing aanmelding";

            qDebug() << "[show_never_contacted_ophaalpunten] ..adding ophaalpunt" << ophaalpunt_id << "to list:" << item_name;
            item->setData(Qt::DisplayRole,item_name);

            item->setData(OPHAALPUNT_ID,ophaalpunt_id);
            contactList->addItem(item);
        }
    }
    else
    {
        qDebug() << "FATAL:" << "Something went wrong, could not execute query: SELECT ophaalpunten.id, ophaalpunten.naam, ophaalpunten.postcode FROM ophaalpunten WHERE not exists (select null from ophalinghistoriek where ophalinghistoriek.ophaalpunt = ophaalpunten.id)";
        qFatal("Something went wrong, could not execute query: SELECT ophaalpunten.id, ophaalpunten.naam, ophaalpunten.postcode FROM ophaalpunten WHERE not exists (select null from ophalinghistoriek where ophalinghistoriek.ophaalpunt = ophaalpunten.id)");
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    this->show();
}

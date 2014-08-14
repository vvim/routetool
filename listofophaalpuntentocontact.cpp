#include "listofophaalpuntentocontact.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>

#define OPHAALPUNT_NAAM 0
#define OPHAALPUNT_ID 1
#define POSTCODE 2
#define LAST_CONTACT_DATE 3
#define LAST_OPHALING_DATE 4
#define FORECAST_NEW_OPHALING_DATE 5

ListOfOphaalpuntenToContact::ListOfOphaalpuntenToContact(QWidget *parent) :
    QWidget(parent)
{
    sortingascending = true;

    label = new QLabel();
    info = new InfoOphaalpunt();
    nieuweaanmeldingWidget = new NieuweAanmelding();
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    contactTree = new QTreeWidget();

    contactTree->setColumnCount(FORECAST_NEW_OPHALING_DATE + 1);
    contactTree->setRootIsDecorated(false);
    contactTree->setAlternatingRowColors(true);

    QStringList labels;
    labels << "Ophaalpunt" << "Ophaalpunt_id" << "Postcode" << "Laatste contact" << "Laatste ophaling" << "Voorspelde ophaling";
    contactTree->setHeaderLabels(labels);
    contactTree->setColumnHidden(OPHAALPUNT_ID,true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addWidget(contactTree);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setMinimumWidth(600);
    setWindowTitle(tr("Lijst van te contacteren ophaalpunten:"));

    connect(buttonBox,SIGNAL(accepted()),this,SLOT(ok_button_pushed()));
    connect(info,SIGNAL(nieuweAanmelding(int)),nieuweaanmeldingWidget,SLOT(aanmeldingVoorOphaalpunt(int)));
    connect(contactTree->header(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(sortTreeWidget(int)));
    connect(contactTree,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(showOphaalpunt(QTreeWidgetItem*)));

    qDebug() << "<vvim> TODO: should we call UptodateAllOphaalpunten() everytime we initialise the contactTree?";
    UpdateAllOphaalpunten();
}

ListOfOphaalpuntenToContact::~ListOfOphaalpuntenToContact()
{
    qDebug() << "start to deconstruct ListOfOphaalpuntenToContact()";
    delete info;
    delete nieuweaanmeldingWidget;
    delete contactTree;
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

    contactTree->clear();

    QSqlQuery query("SELECT id, naam, postcode, last_contact_date, contact_again_on, last_ophaling_date, forecast_new_ophaling_date "
                    "FROM ophaalpunten "
                    "WHERE forecast_new_ophaling_date < date( CURDATE() + INTERVAL 1 DAY)"
                    "ORDER BY postcode");

    if(query.exec())
    {
        while (query.next())
        {
            int ophaalpunt_id = query.value(0).toInt();
            QString ophaalpunt_naam = query.value(1).toString();
            ophaalpunt_naam.replace("\n"," ");
            QString ophaalpunt_postcode = query.value(2).toString();
            QDate last_contact_date = query.value(3).toDate();
            QDate contact_again_on = query.value(4).toDate();
            QDate last_ophaling_date = query.value(5).toDate();
            QDate forecast_ophaling_date = query.value(6).toDate();

            addToTreeWidget(ophaalpunt_naam, ophaalpunt_id, ophaalpunt_postcode, last_contact_date, last_ophaling_date, forecast_ophaling_date);
        }
    }
    else
    {
        qDebug() << "FATAL:" << "Something went wrong, could not execute query: SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES, aanmelding.id from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_nr is NULL";
        qFatal("Something went wrong, could not execute query: SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten, CONCAT_WS(' ', ophaalpunten.straat, ophaalpunten.nr,  ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land) AS ADRES, aanmelding.id from aanmelding, ophaalpunten where ophaalpunten.id = aanmelding.ophaalpunt AND aanmelding.ophaalronde_nr is NULL");
    }

    //contactTree->setColumnWidth(OPHAALPUNT_NAAM,100);

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}

void ListOfOphaalpuntenToContact::showOphaalpunt(QTreeWidgetItem* item)
{
    info->showAanmeldingButton(true);
    info->setWindowTitle(tr("info over ophaalpunt"));
    info->showOphaalpunt(item->text(OPHAALPUNT_ID).toInt());
}

void ListOfOphaalpuntenToContact::ok_button_pushed()
{
    this->close();
}

void ListOfOphaalpuntenToContact::show_one_year_ophaalpunten()
{
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    label->setText(tr("Hieronder de lijst met ophaalpunten die al minstens een jaar niet meer bezocht zijn.\n"
                      "De ophaalpunten die nog nooit bezocht zijn, werden NIET opgenomen in deze lijst.\n"
                      "Dubbelklik op een ophaalpunt om de informatie te zien."));

    contactTree->clear();

    QSqlQuery query;
    query.prepare("SELECT ophaalpunten.id, ophaalpunten.naam, ophaalpunten.postcode , ophaalpunten.last_contact_date, ophaalpunten.contact_again_on, ophaalpunten.last_ophaling_date, ophaalpunten.forecast_new_ophaling_date "
                  "FROM ophaalpunten WHERE ophaalpunten.last_ophaling_date < date( CURDATE() - INTERVAL 1 YEAR)");

    if(query.exec())
    {
        while (query.next())
        {
            bool aanmelding_running = false;

            int ophaalpunt_id = query.value(0).toInt();
            QString ophaalpunt_naam = query.value(1).toString();
            ophaalpunt_naam.replace("\n"," ");
            QString ophaalpunt_postcode = query.value(2).toString();
            QDate last_contact_date = query.value(3).toDate();
            QDate contact_again_on = query.value(4).toDate();
            QDate last_ophaling_date = query.value(5).toDate();
            QDate forecast_ophaling_date = query.value(6).toDate();

            QSqlQuery query2;
            query2.prepare("SELECT * FROM aanmelding WHERE ophaalpunt = :ophaal AND ophaalronde_datum is NULL"); // and ophaalronde is NULL
            query2.bindValue(":ophaal", ophaalpunt_id);

            if(query2.exec())
            {
                if (query2.next())
                {
                    aanmelding_running = true;
                }
            }
            else
                qDebug() << "something went wrong with checking for an existing aanmelding";

            addToTreeWidget(ophaalpunt_naam, ophaalpunt_id, ophaalpunt_postcode, last_contact_date, last_ophaling_date, forecast_ophaling_date, aanmelding_running);
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

    //contactTree->resizeColumnToContents(OPHAALPUNT_NAAM);

    this->show();
}

void ListOfOphaalpuntenToContact::show_never_contacted_ophaalpunten()
{
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    label->setText(tr("Hieronder de lijst met ophaalpunten die nog geen ophaalhistoriek hebben.\n"
                      "Dubbelklik op een ophaalpunt om de informatie te zien."));

    contactTree->clear();

    QSqlQuery query;
    query.prepare("SELECT ophaalpunten.id, ophaalpunten.naam, ophaalpunten.postcode , ophaalpunten.last_contact_date, ophaalpunten.contact_again_on, ophaalpunten.last_ophaling_date, ophaalpunten.forecast_new_ophaling_date "
                  "FROM ophaalpunten WHERE not exists "
                      "(select null from ophalinghistoriek "
                       "where ophalinghistoriek.ophaalpunt = ophaalpunten.id);");

    if(query.exec())
    {
        while (query.next())
        {
            bool aanmelding_running = false;

            int ophaalpunt_id = query.value(0).toInt();
            QString ophaalpunt_naam = query.value(1).toString();
            ophaalpunt_naam.replace("\n"," ");
            QString ophaalpunt_postcode = query.value(2).toString();
            QDate last_contact_date = query.value(3).toDate();
            QDate contact_again_on = query.value(4).toDate();
            QDate last_ophaling_date = query.value(5).toDate();
            QDate forecast_ophaling_date = query.value(6).toDate();

            QSqlQuery query2;
            query2.prepare("SELECT * FROM aanmelding WHERE ophaalpunt = :ophaal AND ophaalronde_datum is NULL"); // and ophaalronde is NULL
            query2.bindValue(":ophaal", ophaalpunt_id);

            if(query2.exec())
            {
                if (query2.next())
                {
                    aanmelding_running = true;
                }
            }
            else
                qDebug() << "something went wrong with checking for an existing aanmelding";

            addToTreeWidget(ophaalpunt_naam, ophaalpunt_id, ophaalpunt_postcode, last_contact_date, last_ophaling_date, forecast_ophaling_date, aanmelding_running);
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

    //contactTree->resizeColumnToContents(OPHAALPUNT_NAAM);

    this->show();
}
void ListOfOphaalpuntenToContact::sortTreeWidget(int column)
{
    qDebug() << "<vvim>" << "[ListOfOphaalpuntenToContact::sortTreeWidget]" << "goes wrong for sorting numbers, see" << "http://stackoverflow.com/questions/363200/is-it-possible-to-sort-numbers-in-a-qtreewidget-column";
    qDebug() << "<vvim>" << "read http://stackoverflow.com/questions/13075643/sorting-by-date-in-qtreewidget" << "The model you use for the tree view need to return QDate or QDateTime for the column in the data() function";
    // goes wrong for sorting numbers, see
    // http://stackoverflow.com/questions/363200/is-it-possible-to-sort-numbers-in-a-qtreewidget-column
    if(sortingascending)
        contactTree->sortByColumn(column,Qt::AscendingOrder);
    else
        contactTree->sortByColumn(column,Qt::DescendingOrder);
    sortingascending = !sortingascending;
}

void ListOfOphaalpuntenToContact::addToTreeWidget(QString NaamOphaalpunt, int OphaalpuntId, QString Postcode,
                                                  QDate LastContactDate, QDate LastOphalingDate, QDate ForecastNewOphalingDate,
                                                  bool color_item)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(contactTree);
    item->setText(OPHAALPUNT_NAAM, NaamOphaalpunt);
    item->setText(OPHAALPUNT_ID, QString().number(OphaalpuntId));
    item->setText(POSTCODE, Postcode);
    item->setText(LAST_CONTACT_DATE, QLocale().toString(LastContactDate,"dd MMM yyyy"));
    item->setText(LAST_OPHALING_DATE, QLocale().toString(LastOphalingDate,"dd MMM yyyy"));
    item->setText(FORECAST_NEW_OPHALING_DATE, QLocale().toString(ForecastNewOphalingDate,"dd MMM yyyy"));

    //item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
    //item->setCheckState(0,Qt::Unchecked);

    if(color_item)
    {
        for (int i = 0 ; i < contactTree->columnCount(); i++)
        item->setForeground(i,Qt::blue);
    }
}

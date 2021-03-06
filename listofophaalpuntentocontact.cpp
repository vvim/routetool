#include "listofophaalpuntentocontact.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include "globalfunctions.h"

ListOfOphaalpuntenToContact::ListOfOphaalpuntenToContact(QWidget *parent) :
    QWidget(parent)
{
    label = new QLabel();
    info = new InfoOphaalpunt();
    nieuweaanmeldingWidget = new NieuweAanmelding();
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    contactTreeView = new QTreeView();
    contactTreeView->setRootIsDecorated(false);
    contactTreeView->setAlternatingRowColors(true);
    contactTreeView->setSortingEnabled(true);
    contactTreeView->sortByColumn(1, Qt::AscendingOrder);
    contactTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    model = NULL;
    listToContactModel = NULL;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addWidget(contactTreeView);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setMinimumWidth(600);
    setWindowTitle(tr("Lijst van te contacteren ophaalpunten:"));

    connect(buttonBox,SIGNAL(accepted()),this,SLOT(ok_button_pushed()));
    connect(info,SIGNAL(nieuweAanmelding(int)),nieuweaanmeldingWidget,SLOT(aanmeldingVoorOphaalpunt(int)));
    connect(contactTreeView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(showOphaalpunt(QModelIndex)));

    vvimDebug() << "<vvim> TODO: should we call UptodateAllOphaalpunten() everytime we initialise the contactTree?";
    UpdateAllOphaalpunten();

    connect(info, SIGNAL(infoChanged()), this, SLOT(databaseBeenUpdated()));
}

ListOfOphaalpuntenToContact::~ListOfOphaalpuntenToContact()
{
    vvimDebug() << "start to deconstruct ListOfOphaalpuntenToContact()";
    delete info;
    delete nieuweaanmeldingWidget;
    delete contactTreeView;
    delete model;
    delete listToContactModel;
    delete buttonBox;
    delete label;
    vvimDebug() << "ListOfOphaalpuntenToContact() deconstructed";
}

void ListOfOphaalpuntenToContact::UpdateOphaalpunt(int ophaalpuntid)
{
    vvimDebug() << "ophaalpuntid = " << ophaalpuntid;

    QString SQLquery = QString("SELECT ophalinghistoriek.ophalingsdatum, ophaalpunten.last_contact_date, ophaalpunten.contact_again_on, "
                        " ophaalpunten.last_ophaling_date,  ophaalpunten.forecast_new_ophaling_date "
                          " FROM `ophalinghistoriek`, ophaalpunten "
                          " WHERE ophalinghistoriek.ophaalpunt = ophaalpunten.id AND ophalinghistoriek.ophaalpunt= %1 "
                        " ORDER BY ophalinghistoriek.ophalingsdatum DESC;").arg(ophaalpuntid);
    QSqlQuery query;
    query.prepare("SELECT ophalinghistoriek.ophalingsdatum, ophaalpunten.last_contact_date, ophaalpunten.contact_again_on, "
                        " ophaalpunten.last_ophaling_date,  ophaalpunten.forecast_new_ophaling_date "
                  " FROM `ophalinghistoriek`, ophaalpunten "
                  " WHERE ophalinghistoriek.ophaalpunt = ophaalpunten.id AND ophalinghistoriek.ophaalpunt= :ophaal "
                  " ORDER BY ophalinghistoriek.ophalingsdatum DESC;");
    query.bindValue(":ophaal",ophaalpuntid);

    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, halting";
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
        vvimDebug() << "reconnected to DB, will try query again";
        QSqlQuery query2;
        query2.prepare("SELECT ophalinghistoriek.ophalingsdatum, ophaalpunten.last_contact_date, ophaalpunten.contact_again_on, "
                            " ophaalpunten.last_ophaling_date,  ophaalpunten.forecast_new_ophaling_date "
                      " FROM `ophalinghistoriek`, ophaalpunten "
                      " WHERE ophalinghistoriek.ophaalpunt = ophaalpunten.id AND ophalinghistoriek.ophaalpunt= :ophaal "
                      " ORDER BY ophalinghistoriek.ophalingsdatum DESC;");
        query = query2;
        query.bindValue(":ophaal",ophaalpuntid);
        if(!query.exec())
        {
            vvimDebug() << "ophaalpuntid = " << ophaalpuntid << "-" << "something went wrong with checking for an existing aanmelding" << query.lastError();
            QMessageBox::information(this, tr("Fout bij verbinding met heruitvoeren query ").arg(Q_FUNC_INFO), tr("De query kon niet uitgevoerd worden na reconnectie met databank, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
    }

    vvimDebug() << "<vvim> TODO: testen -> geeft 'query.next()' al meteen de tweede oplossing, of de eerste? En wat met 'query.exec()'' gevolgd door 'query.next()' ?";
    vvimDebug() << "<vvim> TODO: TEST mis ik hier het allereerste ophaalpunt om te contacteren???";

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
            vvimDebug() << "....ophaling" << aantal_ophalingen << "gebeurde op" << oudere_ophaling.toString() << ", totaal dagen:" << aantal_dagen_tussen_verschillende_ophalingen;
            aantal_ophalingen++;
            temp_ophaling = oudere_ophaling;
        }
        vvimDebug() << "..einde van de while-loop na" << aantal_ophalingen << "loops. Totaal: "<< aantal_dagen_tussen_verschillende_ophalingen;

        if(aantal_ophalingen > 0)
        {
            /// B] hebben we meer dan 2 voorspellingen, dan kunnen we een gemiddelde berekenen
            vvimDebug() << "..Aantal ophalingen:" << aantal_ophalingen << "> 0 => we kunnen een gemiddelde berekenen";

            int gemiddelde = aantal_dagen_tussen_verschillende_ophalingen / aantal_ophalingen;

            /// we zetten een voorspelling / ForeCast nooit verder in de tijd dan 365 dagen
            if(gemiddelde > 365)
            {
                vvimDebug() << "....Gemiddelde blijkt groter dan een jaar: " << gemiddelde << "dagen, afronden naar 1 jaar.";
                gemiddelde = 365;
            }
            vvimDebug() << "..gemiddeld:" << gemiddelde;

            QDate max_of_laatste_ophaling = qMax(laatste_ophaling_taken_from_Ophaalhistoriek, ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten);

            QSqlQuery query_forecast;
            query_forecast.prepare(" UPDATE ophaalpunten "
                                   " SET last_ophaling_date = :last_ophaling, forecast_new_ophaling_date = :forecast "
                                   " WHERE id = :id ");
            query_forecast.bindValue(":last_ophaling",max_of_laatste_ophaling);
            query_forecast.bindValue(":forecast",qMax(ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten, max_of_laatste_ophaling.addDays(gemiddelde)));
            query_forecast.bindValue(":id",ophaalpuntid);

            vvimDebug() << "...datum controle:";
            vvimDebug() << ".....laatste_ophaling:" << laatste_ophaling_taken_from_Ophaalhistoriek.toString();
            vvimDebug() << ".....huidige waarde in DB: ophaalpunt_LastContactDate:" << ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten.toString();
            vvimDebug() << "....... max:" << max_of_laatste_ophaling.toString();
            vvimDebug() << "...voorspelling is dus:";
            vvimDebug() << ".....berekend:" << max_of_laatste_ophaling.addDays(gemiddelde);
            vvimDebug() << ".....in de DB:" << ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten;
            vvimDebug() << "....... max:" << qMax(ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten, max_of_laatste_ophaling.addDays(gemiddelde));

            QString query_forecast_in_string = QString(" UPDATE ophaalpunten "
                                                       " SET last_ophaling_date = %1, forecast_new_ophaling_date = %2 "
                                                       " WHERE id = %3 ")
                                                .arg(max_of_laatste_ophaling.toString("dd MM yyyy"))
                                                .arg(qMax(ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten, max_of_laatste_ophaling.addDays(gemiddelde)).toString("dd MM yyyy"))
                                                .arg(ophaalpuntid);

            if(!query_forecast.exec())
            {
                /** Not showing any MessageBoxes here as this function is called 'en masse' at the start of the program. **/

                if(!reConnectToDatabase(query_forecast.lastError(), query_forecast_in_string, QString("[%1]").arg(Q_FUNC_INFO)))
                {
                    vvimDebug()<< "..." << "unable to reconnect to DB " << query_forecast_in_string;
                }
                else
                {
                    QSqlQuery query_forecast2;
                    query_forecast2.prepare(" UPDATE ophaalpunten "
                                           " SET last_ophaling_date = :last_ophaling, forecast_new_ophaling_date = :forecast "
                                           " WHERE id = :id ");
                    query_forecast = query_forecast2;
                    query_forecast.bindValue(":last_ophaling",max_of_laatste_ophaling);
                    query_forecast.bindValue(":forecast",qMax(ophaalpunt_ForecastNewOphalingDate_taken_from_Ophaalpunten, max_of_laatste_ophaling.addDays(gemiddelde)));
                    query_forecast.bindValue(":id",ophaalpuntid);
                    if(!query_forecast.exec())
                    {
                        vvimDebug() << "..." << "reconnection to DB succesful, but " << query_forecast_in_string << "went WRONG:" << query_forecast.lastError();
                    }
                    else
                    {
                        vvimDebug() << "..." << "needed to reconnect to DB and query done: " << query_forecast_in_string;
                    }
                }
            }
            else
            {
                vvimDebug() << "..." << query_forecast_in_string << "done";
            }
        }
        else
        {
            /// C] zijn er nog geen 2 ophaalmomenten geweest, dan controleren we of de tabel OPHAALPUNTEN de juiste info heeft
            vvimDebug() << "..Aantal ophalingen:" << aantal_ophalingen << "== 0 => we kunnen geen gemiddelde berekenen";
            vvimDebug() << "...we kunnen wel testen of de tabel Ophaalpunten de juiste informatie heeft:";
            vvimDebug() << "....laatste ophaling volgens tabel Ophaalpunten:" << ophaalpunt_LastOphalingDate_taken_from_Ophaalpunten;
            vvimDebug() << "....laatste ophaling volgens tabel OphaalHistoriek:" << laatste_ophaling_taken_from_Ophaalhistoriek;

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

                vvimDebug() << "...datum in ophaalpunten moet dus aangepast worden!!";

                QString query_laatste_ophaling_in_string = QString(" UPDATE ophaalpunten "
                                                           " SET last_ophaling_date = %1 "
                                                           " WHERE id = %2 ")
                                                    .arg(laatste_ophaling_taken_from_Ophaalhistoriek.toString("dd MM yyyy"))
                                                    .arg(ophaalpuntid);

                if(query_laatste_ophaling.exec())
                    vvimDebug() << "..." << query_laatste_ophaling_in_string << "done";
                else
                    vvimDebug() << "..." << query_laatste_ophaling_in_string << "went WRONG:" << query_laatste_ophaling.lastError();


                if(!query_laatste_ophaling.exec())
                {
                    /** Not showing any MessageBoxes here as this function is called 'en masse' at the start of the program. **/

                    if(!reConnectToDatabase(query_laatste_ophaling.lastError(), query_laatste_ophaling_in_string, QString("[%1]").arg(Q_FUNC_INFO)))
                    {
                        vvimDebug()<< "..." << "unable to reconnect to DB, halting" << query_laatste_ophaling_in_string;
                    }
                    else
                    {
                        QSqlQuery query_laatste_ophaling2;
                        query_laatste_ophaling2.prepare(" UPDATE ophaalpunten "
                                                        " SET last_ophaling_date = :last_ophaling "
                                                        " WHERE id = :id ");
                        query_laatste_ophaling = query_laatste_ophaling2;
                        query_laatste_ophaling.bindValue(":last_ophaling",laatste_ophaling_taken_from_Ophaalhistoriek);
                        query_laatste_ophaling.bindValue(":id",ophaalpuntid);

                        if(!query_laatste_ophaling.exec())
                        {
                            vvimDebug() << "..." << "reconnection to DB succesful, but " << query_laatste_ophaling_in_string << "went WRONG:" << query_laatste_ophaling.lastError();
                        }
                        else
                        {
                            vvimDebug() << "..." << "needed to reconnect to DB and query done: " << query_laatste_ophaling_in_string;
                        }
                    }
                }
                else
                {
                    vvimDebug() << "..." << query_laatste_ophaling_in_string << "done";
                }

            }
        }
    }
}



void ListOfOphaalpuntenToContact::UpdateAllOphaalpunten()
{
    QString SQLquery = "SELECT DISTINCT ophaalpunt FROM `ophalinghistoriek`";

    QSqlQuery query(SQLquery);

    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, returning" << SQLquery;
            return;
        }
        query = QSqlQuery(SQLquery);
        if(!query.exec())
        {
            vvimDebug() << "query failed after reconnecting to DB, halting" << SQLquery;
            return;
        }
    }

    vvimDebug() << "\n\n ---------- START invullen voorspellingen:" << QDateTime().currentDateTime().toString();
    while (query.next())
    {
        vvimDebug() << "\nophaalpunt" << query.value(0).toInt();
        UpdateOphaalpunt(query.value(0).toInt());
    }
    vvimDebug() << "\n\n ---------- END invullen voorspellingen:" << QDateTime().currentDateTime().toString();
    vvimDebug() << "check with query SELECT id, `last_contact_date`, `contact_again_on` FROM `ophaalpunten` WHERE `last_contact_date` > 0";
}

void ListOfOphaalpuntenToContact::initModel()
{
    vvimDebug() << "[ListOfOphaalpuntenToContact::initModel]" << "start";
    delete model;
    delete listToContactModel;

    QStringList labels;
    labels << "Ophaalpunt" << "Ophaalpunt_id" << "Postcode" << "Laatste contact" << "Laatste ophaling" << "Voorspelde ophaling" << "Aanmelding bekend";

    model = new QStandardItemModel(0, labels.count());

    listToContactModel = new ListOfOphaalpuntenToContactSortFilterProxyModel(this);
    listToContactModel->setDynamicSortFilter(true);
    listToContactModel->setSourceModel(model);

    for(int i = 0; i < labels.count(); i++)
    {
       model->setHeaderData(i,Qt::Horizontal, /* QObject::tr( */ labels[i] /*)*/ );  // why does 'tr()' not work? -> QString& instead of QString...
    }

    contactTreeView->setModel(listToContactModel);

    contactTreeView->hideColumn(LIST_OPHAALPUNT_ID);
    contactTreeView->hideColumn(LIST_AANMELDING_PRESENT);
}


void ListOfOphaalpuntenToContact::initialise()
{

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    label->setText(tr("Hieronder de lijst met ophaalpunten die al een historiek hebben en gecontacteerd dienen te worden.\n"
                      "Dubbelklik op een ophaalpunt om de informatie te zien.\n"
                      "Dubbleklik op een kolom om te sorteren."));

    initModel();

    QString SQLquery = "SELECT id, naam, postcode, last_contact_date, contact_again_on, last_ophaling_date, forecast_new_ophaling_date "
                    "FROM ophaalpunten WHERE kurk > 0 or parafine > 0 "
                    "ORDER BY postcode";

    QSqlQuery query(SQLquery);


    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, halting";
            QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, het programma zal zich nu afsluiten.\n\nProbeer later opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return;
        }
        query = QSqlQuery(SQLquery);
        if(!query.exec())
        {
            vvimDebug() << "FATAL:" << "Something went wrong, could not execute query:" << SQLquery;
            qFatal(QString("Something went wrong, could not execute query: %1").arg(SQLquery).toStdString().c_str());
            return;
        }
    }

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


       // it seems useless to me to recheck for a DB-connection as this query is right after the previous DB-connection-check
       // also, if the connection would fail for this query, the worst thing that can happen, is that the TreeView is incorrect (no big deal)
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
           vvimDebug() << "something went wrong with checking for an existing aanmelding";

       addToTreeView(ophaalpunt_naam, ophaalpunt_id, ophaalpunt_postcode, last_contact_date, last_ophaling_date, forecast_ophaling_date, aanmelding_running);
   }
   vvimDebug() << "TOTAL of ophaalpunten loaded in \"Bellijst\" :" << model->rowCount();


    //contactTree->setColumnWidth(LIST_OPHAALPUNT_NAAM,100);

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}

void ListOfOphaalpuntenToContact::showOphaalpunt(QModelIndex index)
{
    int row = index.row();
    int ophaalpunt_id = listToContactModel->data(listToContactModel->index(row, LIST_OPHAALPUNT_ID)).toInt();
    vvimDebug() << "get ophaalpunt id from row " << row << "is" << ophaalpunt_id;
    info->showAanmeldingAndHistoriekButton(true);
    info->setWindowTitle(tr("info over ophaalpunt"));
    info->showOphaalpunt(ophaalpunt_id);
}

void ListOfOphaalpuntenToContact::ok_button_pushed()
{
    this->close();
}

void ListOfOphaalpuntenToContact::addToTreeView(QString NaamOphaalpunt, int OphaalpuntId, QString Postcode,
                                                  QDate LastContactDate, QDate LastOphalingDate, QDate ForecastNewOphalingDate,
                                                  bool color_item)
{
    model->insertRow(0);
    model->setData(model->index(0,LIST_OPHAALPUNT_NAAM), NaamOphaalpunt);
    model->setData(model->index(0,LIST_OPHAALPUNT_ID), OphaalpuntId);
    model->setData(model->index(0,LIST_POSTCODE), Postcode);
    model->setData(model->index(0,LIST_LAST_CONTACT_DATE), LastContactDate);
    model->setData(model->index(0,LIST_LAST_OPHALING_DATE), LastOphalingDate);
    model->setData(model->index(0,LIST_FORECAST_NEW_OPHALING_DATE), ForecastNewOphalingDate);
    model->setData(model->index(0,LIST_AANMELDING_PRESENT), color_item);
}


void ListOfOphaalpuntenToContact::databaseBeenUpdated()
{
    emit contentsOfDatabaseChanged();
}

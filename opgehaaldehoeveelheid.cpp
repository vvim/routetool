#include "opgehaaldehoeveelheid.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <QMessageBox>
#include <QtGui>
#include "globalfunctions.h"

OpgehaaldeHoeveelheid::OpgehaaldeHoeveelheid(QDate ophaalronde_datum, QWidget *parent) :
    QWidget(parent)
{
    vvimDebug() << "<vvim> [TODO] if window OpgehaaldeHoeveelheid gets closed: NOTHING HAPPENS! how to delete from memory???";

    // cfr SimpleWidgetMapper example

   /**
     "ophalingDateEdit" and "chauffeurEdit" are not part of the model
     as we assume that during a route, the date or chauffeur don't change
   **/
   vvimDebug() << "'ophalingDateEdit' and 'chauffeurEdit' are not part of the model"
               << "as we assume that during a route, the date or chauffeur do not change";
   ophalingDateLabel = new QLabel(tr("&Datum ophaling:"));
   ophalingDateEdit = new QDateEdit(ophaalronde_datum, this);
   ophalingDateEdit->setDisplayFormat("dd MMM yyyy");
   ophalingDateEdit->setLocale(QLocale::Dutch);
   ophalingDateEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup
   ophalingDateLabel->setBuddy(ophalingDateEdit);

   chauffeurLabel = new QLabel(tr("&Chauffeur:"));
   chauffeurEdit = new QLineEdit();
   chauffeurLabel->setBuddy(chauffeurEdit);

   setupModel(ophaalronde_datum);
   vvimDebug() << "model is set";

   ophaalpuntLabel = new QLabel(tr("Ophaal&punt:"));
   ophaalpuntEdit = new QLineEdit();
   ophaalpuntEdit->setEnabled(false);   // naam ophaalpunt kan niet bewerkt worden.
   // <vvim> TODO: in de toekomst een Checkbox of een ophaalpunt al dan niet echt bediend is geweest???
   opmerkingenLabel = new QLabel(tr("&Opmerkingen:"));
   opmerkingenEdit = new QPlainTextEdit();

   kgKurkLabel = new QLabel(tr("Kurk (kg)"));
   zakkenKurkLabel = new QLabel(tr("Kurk (zakken)"));
   kgKaarsrestenLabel = new QLabel(tr("Kaarsresten (kg)"));
   zakkenKaarsrestenLabel = new QLabel(tr("Kaarsresten (zakken)"));

   kgKurkSpinBox = new QSpinBox();
   kgKurkSpinBox->setMaximum(9999);
   zakkenKurkSpinBox = new QSpinBox();
   zakkenKurkSpinBox->setMaximum(9999);
   kgKaarsrestenSpinBox = new QSpinBox();
   kgKaarsrestenSpinBox->setMaximum(9999);
   zakkenKaarsrestenSpinBox = new QSpinBox();
   zakkenKaarsrestenSpinBox->setMaximum(9999);

   locationShouldBeSkippedCheckBox = new QCheckBox(tr("Dit ophaalpunt is tijdens de route niet bezocht."));

   nextButton = new QPushButton(tr("&Volgende"));
   previousButton = new QPushButton(tr("V&orige"));

   ophaalpuntLabel->setBuddy(ophaalpuntEdit);
   opmerkingenLabel->setBuddy(opmerkingenEdit);
   kgKurkLabel->setBuddy(kgKurkSpinBox);
   zakkenKurkLabel->setBuddy(zakkenKurkSpinBox);
   kgKaarsrestenLabel->setBuddy(kgKaarsrestenSpinBox);
   zakkenKaarsrestenLabel->setBuddy(zakkenKaarsrestenSpinBox);
   //! [Set up widgets]

   vvimDebug() << "set up mapper";
//! [Set up the mapper]
   mapper = new QDataWidgetMapper(this);
   mapper->setModel(model);
   vvimDebug() << "--ophaalpunt";
   mapper->addMapping(ophaalpuntEdit, 0);
   vvimDebug() << "--opmerkingen";
   mapper->addMapping(opmerkingenEdit, 1);
   vvimDebug() << "--kurkkg";
   mapper->addMapping(kgKurkSpinBox, 3);
   vvimDebug() << "--kurkzakk";
   mapper->addMapping(zakkenKurkSpinBox, 4);
   vvimDebug() << "--kaarskg";
   mapper->addMapping(kgKaarsrestenSpinBox, 5);
   vvimDebug() << "--kaarszakk";
   mapper->addMapping(zakkenKaarsrestenSpinBox, 6);
   vvimDebug() << "--location-has-not-been-visited-Checkbox";
   mapper->addMapping(locationShouldBeSkippedCheckBox, 8);


   connect(previousButton, SIGNAL(clicked()),
           mapper, SLOT(toPrevious()));
   connect(nextButton, SIGNAL(clicked()),
           mapper, SLOT(toNext()));
   connect(mapper, SIGNAL(currentIndexChanged(int)),
           this, SLOT(updateButtons(int)));
   connect(zakkenKurkSpinBox, SIGNAL(editingFinished()), this, SLOT(zakkenKurkTest()));
   connect(zakkenKaarsrestenSpinBox, SIGNAL(editingFinished()), this, SLOT(zakkenKaarsrestenTest()));

   connect(locationShouldBeSkippedCheckBox, SIGNAL(toggled(bool)),this, SLOT(checkBoxToggled(bool)));

//! [Set up the mapper]


   vvimDebug() << "set up QDialogButtonBox";
   buttonBox = new QDialogButtonBox();
   buttonBox->addButton(tr("Bevestig hoeveelheden van ALLE locaties"),
                QDialogButtonBox::AcceptRole);
   buttonBox->addButton(tr("Route nog niet bevestigen"),
                        QDialogButtonBox::RejectRole);
   buttonBox->setEnabled(false);

   /// could be interesting to only enable the cancel button and switch the 'accept' role
   /// don't get it to work, though
   //  vvimDebug() << "set up cancelbutton";
   //  buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
   /// or we can make a 'counter' in the upperright corner saying "locatin <nr> of <total>"


   connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
   connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));

   vvimDebug() << "set up QGridLayout";
//! [Set up the layout]
   QGridLayout *layout = new QGridLayout();
   layout->addWidget(ophalingDateLabel, 0, 0, 1, 1);
   layout->addWidget(ophalingDateEdit, 0, 1, 1, 3);
   layout->addWidget(previousButton, 0, 4, 1, 1);
   layout->addWidget(chauffeurLabel, 1, 0, 1, 1);
   layout->addWidget(chauffeurEdit, 1, 1, 1, 3);
   layout->addWidget(nextButton, 1, 4, 1, 1);

   int row_ui = 2;
   layout->addWidget(ophaalpuntLabel, row_ui, 0, 1, 1);
   layout->addWidget(ophaalpuntEdit, row_ui, 1, 1, 3);
   row_ui++;
   layout->addWidget(opmerkingenLabel, row_ui, 0, 1, 1);
   layout->addWidget(opmerkingenEdit, row_ui, 1, 1, 3);
   row_ui++;
   layout->addWidget(kgKurkLabel, row_ui, 0, 1, 1);
   layout->addWidget(kgKurkSpinBox, row_ui, 1, 1, 1);
   layout->addWidget(zakkenKurkLabel, row_ui, 2, 1, 1);
   layout->addWidget(zakkenKurkSpinBox, row_ui, 3, 1, 1);
   row_ui++;
   layout->addWidget(kgKaarsrestenLabel, row_ui, 0, 1, 1);
   layout->addWidget(kgKaarsrestenSpinBox, row_ui, 1, 1, 1);
   layout->addWidget(zakkenKaarsrestenLabel, row_ui, 2, 1, 1);
   layout->addWidget(zakkenKaarsrestenSpinBox, row_ui, 3, 1, 1);
   row_ui++;
   spaceritem_beforelocationshouldbeskipped = new QSpacerItem(this->width(),10);
   layout->addItem(spaceritem_beforelocationshouldbeskipped,row_ui,0,1,5);
   row_ui++;
   layout->addWidget(locationShouldBeSkippedCheckBox,row_ui,0,1,5);
   row_ui++;
   spaceritem_beforebuttonbox = new QSpacerItem(this->width(),40);
   layout->addItem(spaceritem_beforebuttonbox,row_ui,0,1,5);
   row_ui++;
   layout->addWidget(buttonBox,row_ui,0,1,5);

   setLayout(layout);

   setWindowTitle(tr("Bevestig opgehaalde hoeveelheden"));
   mapper->toFirst();
   vvimDebug() << "DONE";
}
//! [Set up the layout]

//! [Set up the model]
void OpgehaaldeHoeveelheid::setupModel(QDate ophaalronde_datum)
{

    QSqlQuery query;

    query.prepare("SELECT aanmelding.ophaalronde_datum, aanmelding.volgorde, ophaalpunten.naam, aanmelding.opmerkingen, "
                         "ophaalpunten.id, aanmelding.kg_kurk, aanmelding.zakken_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kaarsresten, aanmelding.datum, aanmelding.id "
                  "FROM aanmelding, ophaalpunten "
                  "WHERE aanmelding.ophaalronde_datum = :ophaalrondedatum AND aanmelding.ophaalpunt = ophaalpunten.id "
                  "ORDER BY aanmelding.volgorde");
    query.bindValue(":ophaalrondedatum",ophaalronde_datum);

    if(!query.exec())
    {

        QString SQLquery = QString("SELECT aanmelding.ophaalronde_datum, aanmelding.volgorde, ophaalpunten.naam, aanmelding.opmerkingen, "
                "ophaalpunten.id, aanmelding.kg_kurk, aanmelding.zakken_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kaarsresten, aanmelding.datum, aanmelding.id "
         "FROM aanmelding, ophaalpunten "
         "WHERE aanmelding.ophaalronde_datum = %1 AND aanmelding.ophaalpunt = ophaalpunten.id "
         "ORDER BY aanmelding.volgorde").arg(ophaalronde_datum.toString());

        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            qCritical("Reconnection to DB failed");
            vvimDebug() << "Reconnection to DB failed";
            vvimDebug() << "<vvim> TODO we should cancel the building of the model??? maybe give a return value to the function to alert the constructor???";
            return; // errorboodschap tonen???
        }

        /** retry **/
        QSqlQuery query2;
        query2.prepare("SELECT aanmelding.ophaalronde_datum, aanmelding.volgorde, ophaalpunten.naam, aanmelding.opmerkingen, "
                             "ophaalpunten.id, aanmelding.kg_kurk, aanmelding.zakken_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kaarsresten, aanmelding.datum, aanmelding.id "
                      "FROM aanmelding, ophaalpunten "
                      "WHERE aanmelding.ophaalronde_datum = :ophaalrondedatum AND aanmelding.ophaalpunt = ophaalpunten.id "
                      "ORDER BY aanmelding.volgorde");
        query = query2;
        query.bindValue(":ophaalrondedatum",ophaalronde_datum);

        if(!query.exec())
        {
            qCritical(tr("Reconnection to DB worked, but executing query %1 still failed: %2.").arg(SQLquery).arg(query.lastError().text()).toStdString().c_str());
            vvimDebug() << tr("Reconnection to DB worked, but executing query %1 still failed: %2.").arg(SQLquery).arg(query.lastError().text());
            vvimDebug() << "<vvim> TODO we should cancel the building of the model??? maybe give a return value to the function to alert the constructor???";
            return; // errorboodschap tonen???
        }
    }

    vvimDebug() << "start";
    int query_results = 0;
    QStringList opmerkingen;
    QStringList ophaalpunten_string;

    vvimDebug() << "# of results:" << query.size();
    // int* ophaalpunten_id = new int[query.size()];
    QStringList ophaalpunten_id;
    QStringList kg_kurk;
    QStringList zakken_kurk;
    QStringList kg_kaarsresten;
    QStringList zakken_kaarsresten;
    QList<QDate> aanmeldingsdata;
    QStringList aanmeldings_ids;

    while(query.next())
    {
       vvimDebug() << query_results << query.value(0).toDate().toString() << "volgorde" << query.value(1).toInt() << "ophaalpunt:" << query.value(2).toString();

       vvimDebug() << "names";
       ophaalpunten_string << query.value(2).toString();

       vvimDebug() << "opmerkingen";
       opmerkingen << query.value(3).toString();

       //ophaalpunten_id[query_results] = query.value(4).toInt();
       ophaalpunten_id << query.value(4).toString();
       kg_kurk << query.value(5).toString();
       zakken_kurk << query.value(6).toString();
       kg_kaarsresten << query.value(7).toString();
       zakken_kaarsresten << query.value(8).toString();

       vvimDebug() << "aanmeldingsdatum:" <<  query.value(9).toDate().toString();
       aanmeldingsdata.append(query.value(9).toDate());

       vvimDebug() << "aanmeldings_id:" <<  query.value(10).toString();
       aanmeldings_ids << query.value(10).toString();

       query_results++;

    } // end of Query Results

    vvimDebug() << "Debugging the way of putting aanmeldingsdatum and _ids in the model. Size of list is" << aanmeldingsdata.size() << ". Writing out all members:";
    for (int i = 0; i < aanmeldingsdata.size(); ++i)
    {
        vvimDebug() << ".." << aanmeldingsdata.at(i).toString() << "id:" << aanmeldings_ids.at(i);
    }

    vvimDebug() << "initializing the model";
    model = new QStandardItemModel(query_results, 9, this);
    for (int row = 0; row < query_results; ++row) {
      QStandardItem *item = new QStandardItem(ophaalpunten_string[row]);
      model->setItem(row, 0, item);
      item = new QStandardItem(opmerkingen[row]);
      model->setItem(row, 1, item);
      item = new QStandardItem(ophaalpunten_id[row]);
      vvimDebug() << "ophaalpunt_id" << ophaalpunten_id[row];
      model->setItem(row, 2, item);
      item = new QStandardItem(kg_kurk[row]);
      model->setItem(row, 3, item);
      item = new QStandardItem(zakken_kurk[row]);
      model->setItem(row, 4, item);
      item = new QStandardItem(kg_kaarsresten[row]);
      model->setItem(row, 5, item);
      item = new QStandardItem(zakken_kaarsresten[row]);
      model->setItem(row, 6, item);
      item = new QStandardItem();
      item->setData(aanmeldingsdata.at(row),Qt::UserRole);
      model->setItem(row, 7, item);
      item = new QStandardItem();
      item->setData(false,Qt::UserRole); // all checkboxes are set on 'false'
      item->setData(aanmeldings_ids[row],Qt::UserRole+1); // set aanmelding_id together with the checkbox-value
      item->setCheckable(true);
      item->setCheckState(Qt::Unchecked);
      model->setItem(row, 8, item);
    }
    vvimDebug() << "DONE";

    // delete ophaalpunten_id; ???
    vvimDebug() << "TODO: delete ophaalpunten_id";
}
//! [Set up the model]

//! [Slot for updating the buttons]
void OpgehaaldeHoeveelheid::updateButtons(int row)
{
   previousButton->setEnabled(row > 0);
   nextButton->setEnabled(row < model->rowCount() - 1);
   buttonBox->setEnabled(row == model->rowCount() - 1);
}
//! [Slot for updating the buttons]


OpgehaaldeHoeveelheid::~OpgehaaldeHoeveelheid()
{
    vvimDebug() << "start to deconstruct OpgehaaldeHoeveelheid()";
/*
    delete ophalingDateLabel;
    delete chauffeurLabel;
    delete ophaalpuntLabel;
    delete opmerkingenLabel;
    delete kgKurkLabel;
    delete zakkenKurkLabel;
    delete kgKaarsrestenLabel;
    delete zakkenKaarsrestenLabel;
    delete ophalingDateEdit;
    delete chauffeurEdit;
    delete ophaalpuntEdit;
    delete opmerkingenEdit;
    delete kgKurkSpinBox;
    delete zakkenKurkSpinBox;
    delete kgKaarsrestenSpinBox;
    delete zakkenKaarsrestenSpinBox;
    delete nextButton;
    delete previousButton;
    delete locationShouldBeSkippedCheckBox;

    vvimDebug() << "deleting buttonbox";
    delete buttonBox;
    delete spaceritem_beforebuttonbox;
    delete spaceritem_beforelocationshouldbeskipped;

    vvimDebug() << "deleting model";
    delete model;
    vvimDebug() << "deleting mapper";
    delete mapper;
*/
    vvimDebug() << "OpgehaaldeHoeveelheid() deconstructed";
}

void OpgehaaldeHoeveelheid::reject()
{
    vvimDebug() << "Cancel confirmation of collected quantities of route";
    close();
    vvimDebug() << "before deleting self";
/* <vvim> TODO: does this one result in a crash? */    delete this;
    vvimDebug() << "after deleting self (shouldn't show though :-) )";
}

void OpgehaaldeHoeveelheid::accept()
{
    /**
        Regarding [ locationShouldBeSkippedCheckBox ] :
           It annoys me that I can't use ->checkState, but it works when I check the ->text() for the string "true"
           (see below)

           What is going wrong?

           For now it works, but it would be cleaner if we can simply check the state of the locationShouldBeSkippedCheckBox ?

           In initiating the mapper, maybe the following can be used:
            mapper->addMapping(locationShouldBeSkippedCheckBox, 8, "checked");

           Or is it because of Flag-settings? Look at [ listofophaalpuntentocontact.cpp ] or [ kiesophaalpunten.cpp ] ?
            item->setFlags(item->flags|Qt::ItemIsUserCheckable);
           and what is the difference with [ item->SetCheckable() ] ???
    **/


    vvimDebug() << "CONFIRM collected quantities of route";
    bool all_queries_worked_fine = true;
    QDate ophaalrondedatum = ophalingDateEdit->date();
    QString chauffeur = chauffeurEdit->text();

    int row;
    for(row=0; row < model->rowCount(); ++row )
    {
        int aanmelding_id = model->item(row,8)->data(Qt::UserRole+1).toInt();

        /** extra debugging information
        //bool checkbox_state = model->item(row,8)->data(Qt::UserRole).toBool();
        vvimDebug() << " ... A: " << model->item(row,8)->data(Qt::UserRole).toBool();  /// FALSE
        vvimDebug() << " ... B: " << model->item(row,8)->text();                       /// ""
        vvimDebug() << " ... C: " << model->item(row,8)->checkState() << Qt::Checked << Qt::Unchecked; /// 0 => Qt::UnChecked
        vvimDebug() << " ... D: " << model->item(row,8)->isCheckable();                /// FALSE
        vvimDebug() << " ... E: " << model->item(row,8)->isEnabled();                   /// TRUE
        vvimDebug() << " ... F: " << model->item(row,8)->flags();                      /// qbitearray?
        vvimDebug() << " ... G: " << aanmelding_id;
        **/

        /** it annoys me that I can't use ->checkState, but it works when I check the ->text() for the string "true" **/
        //if(model->item(row,8)->checkState() == Qt::Checked)
        if(model->item(row,8)->text() == "true")
        {
            vvimDebug() << "row" << row << ": to be IGNORED per request of user, aanmelding_id from t_aanmelding:" << aanmelding_id;
            QSqlQuery query;
            query.prepare("UPDATE aanmelding SET ophaalronde_datum = NULL, volgorde = NULL WHERE id = :id");
            query.bindValue(":id",aanmelding_id);
            if(!query.exec())
            {
                QString SQLquery = QString("UPDATE aanmelding SET ophaalronde_datum = NULL, volgorde = NULL WHERE id = %1").arg(aanmelding_id);

                vvimDebug() << ".. something went wrong with the query, we try to reconnect to DB. Query error: " << query.lastError().text();

                if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
                {
                    vvimDebug() << ".. unable to reconnect to DB, FAIL";
                    QMessageBox::information(this, tr("Fout bij verbinding met de databank ").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
                    return;
                }
                else
                {
                    vvimDebug() << ".. reconnected to DB, try query again:";

                    QSqlQuery query2;
                    query2.prepare("UPDATE aanmelding SET ophaalronde_datum = NULL, volgorde = NULL WHERE id = :id");
                    query = query2;
                    query.bindValue(":id",aanmelding_id);

                    if(!query.exec())
                    {
                        qCritical(QString("FAIL: Location was not visited and therefore supposed to be ignored. Couldn't return the location from route to 'aanmelding'. Row #%1 of the model, something went wrong with the UPDATE-query: "
                                          "UPDATE aanmelding SET ophaalronde_datum = NULL, volgorde = NULL WHERE id = %2"
                                          ).arg(row).arg(aanmelding_id).toStdString().c_str()   );
                        vvimDebug() << ".. FAIL: Location was not visited and therefore supposed to be ignored. Couldn't return the location from route to 'aanmelding'. Row #" << row << " of the model, something went wrong with the UPDATE-query: "
                                       "UPDATE aanmelding SET ophaalronde_datum = NULL, volgorde = NULL WHERE id = " << aanmelding_id;
                        vvimDebug() << ".. query error:" << query.lastError().text();
                        vvimDebug() << ".. should I break the FOR-loop or continue? For now we choose to CONTINUE";
                        QMessageBox::information(this, tr("Fout bij heruitvoeren query ").arg(Q_FUNC_INFO), tr("De query kon niet uitgevoerd worden na reconnectie met databank, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
                    }
                    else
                        vvimDebug() << ".. succesfully returned row to t_aanmeldingen, erased the content of column 'ophaalronde_datum' and 'volgorde'.";
                }

            }
            else
                vvimDebug() << ".. succesfully returned row to t_aanmeldingen, erased the content of column 'ophaalronde_datum' and 'volgorde'.";
        }
        else
        {
            /// --start----- adding [row] to ophalinghistoriek
            vvimDebug() << "row" << row << ": to be added to t_ophalinghistoriek";
            QString opmerkingen = model->item(row,1)->text();
            int ophaalpunt_id = model->item(row,2)->text().toInt();
            int kg_kurk = model->item(row,3)->text().toInt();
            int zakken_kurk = model->item(row,4)->text().toInt();
            int kg_kaarsresten = model->item(row,5)->text().toInt();
            int zakken_kaarsresten = model->item(row,6)->text().toInt();
            QDate aanmeldingsdatum = model->item(row,7)->data(Qt::UserRole).toDate();

            QSqlQuery query;
            query.prepare("INSERT ophalinghistoriek (id,   timestamp,         ophalingsdatum,    chauffeur, ophaalpunt, zakken_kurk, kg_kurk, zakken_kaarsresten, kg_kaarsresten, opmerkingen, aanmeldingsdatum) "
                          "                  VALUES (NULL, CURRENT_TIMESTAMP, :ophaalrondedatum, :chauffeur, :ophaalpunt, :zakkenkurk, :kgkurk, :zakkenkaarsresten, :kgkaarsresten, :opmerkingen, :aanmeldingsdatum)");
            query.bindValue(":ophaalrondedatum",ophaalrondedatum);
            query.bindValue(":chauffeur",chauffeur);
            query.bindValue(":ophaalpunt",ophaalpunt_id);
            query.bindValue(":zakkenkurk",zakken_kurk);
            query.bindValue(":kgkurk",kg_kurk);
            query.bindValue(":zakkenkaarsresten",zakken_kaarsresten);
            query.bindValue(":kgkaarsresten",kg_kaarsresten);
            query.bindValue(":opmerkingen",opmerkingen);
            query.bindValue(":aanmeldingsdatum",aanmeldingsdatum);

            QString SQLquery = QString("INSERT ophalinghistoriek (id,   timestamp,         ophalingsdatum,    chauffeur, ophaalpunt, zakken_kurk, kg_kurk, zakken_kaarsresten, kg_kaarsresten, opmerkingen) "
                                                                    "                  VALUES (NULL, CURRENT_TIMESTAMP, %1, %2, %3, ..., %4, %5)").arg(ophaalrondedatum.toString()).arg(chauffeur).arg(ophaalpunt_id).arg(opmerkingen).arg(aanmeldingsdatum.toString());

            if(!query.exec())
            {
                qCritical(QString("FAIL: Couldn't put row #%1 of the model in the database, something went wrong with the INSERT-query: ").toStdString().c_str());
                all_queries_worked_fine = false;
                row = model->rowCount(); // how do I abort a for-loop -> 'break' ?
                break;




                vvimDebug() << ".. something went wrong with the query, we try to reconnect to DB. Query error: " << query.lastError().text();

                if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
                {
                    vvimDebug() << ".. unable to reconnect to DB, FAIL";
                    vvimDebug() << ".. should I break the FOR-loop or continue? For now we choose to BREAK";
                    vvimDebug() << ".. abort for-loop";
                    all_queries_worked_fine = false;
                    row = model->rowCount(); // how do I abort a for-loop -> 'break' ?
                    break; // how do I abort a for-loop -> 'break' ?
                }
                else
                {
                    vvimDebug() << ".. reconnected to DB, try query again:";

                    QSqlQuery query2;
                    query2.prepare("INSERT ophalinghistoriek (id,   timestamp,         ophalingsdatum,    chauffeur, ophaalpunt, zakken_kurk, kg_kurk, zakken_kaarsresten, kg_kaarsresten, opmerkingen, aanmeldingsdatum) "
                                  "                  VALUES (NULL, CURRENT_TIMESTAMP, :ophaalrondedatum, :chauffeur, :ophaalpunt, :zakkenkurk, :kgkurk, :zakkenkaarsresten, :kgkaarsresten, :opmerkingen, :aanmeldingsdatum)");
                    query = query2;
                    query.bindValue(":ophaalrondedatum",ophaalrondedatum);
                    query.bindValue(":chauffeur",chauffeur);
                    query.bindValue(":ophaalpunt",ophaalpunt_id);
                    query.bindValue(":zakkenkurk",zakken_kurk);
                    query.bindValue(":kgkurk",kg_kurk);
                    query.bindValue(":zakkenkaarsresten",zakken_kaarsresten);
                    query.bindValue(":kgkaarsresten",kg_kaarsresten);
                    query.bindValue(":opmerkingen",opmerkingen);
                    query.bindValue(":aanmeldingsdatum",aanmeldingsdatum);

                    if(!query.exec())
                    {
                        qCritical(QString("FAIL: Couldn't put row #%1 of the model in the database, something went wrong with the INSERT-query: ").arg(row).append(query.lastError().text()).toStdString().c_str());
                        vvimDebug() << ".. FAIL: Couldn't put row" << row << "of the model in the database, something went wrong with the INSERT-query: " << query.lastError();
                        vvimDebug() << ".. should I break the FOR-loop or continue? For now we choose to BREAK";
                        vvimDebug() << ".. abort for-loop";
                        all_queries_worked_fine = false;
                        row = model->rowCount(); // how do I abort a for-loop -> 'break' ?
                        break; // how do I abort a for-loop -> 'break' ?
                    }
                    else
                        vvimDebug() << ".. succesfully returned row" << row << "to t_aanmeldingen, erased the content of column 'ophaalronde_datum' and 'volgorde'.";
                }

            }
            else
                vvimDebug() << ".. succesfully returned row to t_aanmeldingen, erased the content of column 'ophaalronde_datum' and 'volgorde'.";

            /// --end----- adding [row] to ophalinghistoriek
        }
    }

    if (!all_queries_worked_fine)
    {
        qCritical(QString("something went wrong from row %1, aborting").arg(row).toStdString().c_str());
        vvimDebug() << "<vvim> [TODO] [URGENT] What to do if part of the confirmations have been executed? then we might get doubles in the table OPHALINGHISTORIEK. [URGENT]";
    }
    else
    {
        vvimDebug() << "everything went fine, we just need to delete the unconfirmed copies from table AANMELDINGEN";
        QSqlQuery query;
        query.prepare("DELETE FROM aanmelding WHERE ophaalronde_datum = :ophaalrondedatum");
        query.bindValue(":ophaalrondedatum",ophaalrondedatum);

        if(!query.exec())
        {
            vvimDebug() << ".. something went wrong with the query, we try to reconnect to DB. Query error: " << query.lastError().text();

            QString SQLquery = QString("DELETE FROM aanmelding WHERE ophaalronde_datum = %1").arg(ophaalrondedatum.toString());

            if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
            {
                vvimDebug() << ".. unable to reconnect to DB, FAIL";
                vvimDebug() << ".. We now have duplicates in table OPHALINGHISTORIEK and table AANMELDINGEN for ophaaldatum" << ophaalrondedatum.toString();
                qCritical(QString("FAILED to reconnect to DB.   DELETE FROM aanmelding WHERE ophaalronde_datum = %1 FAILED. We now have duplicates in table OPHALINGHISTORIEK and table AANMELDINGEN.").arg(ophaalrondedatum.toString()).toStdString().c_str());
            }
            else
            {
                vvimDebug() << ".. reconnected to DB, try query again:";
                QSqlQuery query2;
                query2.prepare("DELETE FROM aanmelding WHERE ophaalronde_datum = :ophaalrondedatum");
                query = query2;
                query.bindValue(":ophaalrondedatum",ophaalrondedatum);

                if(!query.exec())
                {
                    vvimDebug() << ".. unable to run query" << query.lastError().text();
                    vvimDebug() << ".. We now have duplicates in table OPHALINGHISTORIEK and table AANMELDINGEN for ophaaldatum" << ophaalrondedatum.toString();
                    qCritical(QString("Query FAILED :   DELETE FROM aanmelding WHERE ophaalronde_datum = %1 FAILED. We now have duplicates in table OPHALINGHISTORIEK and table AANMELDINGEN.").arg(ophaalrondedatum.toString()).append(query.lastError().text()).toStdString().c_str());
                }
                else
                    vvimDebug() << ".. copies deleted, everything went fine";
            }
        }
        else
            vvimDebug() << ".. copies deleted, everything went fine";



    }

    reject();
}

void OpgehaaldeHoeveelheid::zakkenKurkTest()
{
    int zakkenKurk = zakkenKurkSpinBox->value();
    int kgKurk_input = kgKurkSpinBox->value();
    int kgKurk_expected = zakkenKurk * settings.value("zak_kurk_naar_kg").toInt();

    vvimDebug() << "zakken kurk:" << zakkenKurk << "ingegeven kg:" << kgKurk_input << "verwachtte kg:" << kgKurk_expected;

    if(kgKurk_input != kgKurk_expected)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Gewicht aan kurk aanpassen?"),
                                      tr("%1 zakken kurk wordt geschat op %2 kilogram. Wilt u dit geschat gewicht nemen in plaats van de ingegeven %3 kilogram?").arg(zakkenKurk).arg(kgKurk_expected).arg(kgKurk_input),
                                      QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::Yes)
        {
            kgKurkSpinBox->setValue(kgKurk_expected);
            kgKurkSpinBox->setFocus(); // else the model does not save the valuechange. Other way possible?
            vvimDebug() << "kg kurk changed to:" << kgKurk_expected;
        }
    }
}

void OpgehaaldeHoeveelheid::zakkenKaarsrestenTest()
{
    int zakkenKaars = zakkenKaarsrestenSpinBox->value();
    int kgKaars_input = kgKaarsrestenSpinBox->value();

    int kgKaars_expected = zakkenKaars * settings.value("zak_kaarsresten_naar_kg").toInt();

    vvimDebug() << "zakken kaars:" << zakkenKaars << "ingegeven kg:" << kgKaars_input << "verwachtte kg:" << kgKaars_expected;

    if(kgKaars_input != kgKaars_expected)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Gewicht aan kaarsresten aanpassen?"),
                                      tr("%1 zakken kaarsresten wordt geschat op %2 kilogram. Wilt u dit geschat gewicht nemen in plaats van de ingegeven %3 kilogram?").arg(zakkenKaars).arg(kgKaars_expected).arg(kgKaars_input),
                                      QMessageBox::Yes|QMessageBox::No);
        if(reply == QMessageBox::Yes)
        {
            kgKaarsrestenSpinBox->setValue(kgKaars_expected);
            kgKaarsrestenSpinBox->setFocus(); // else the model does not save the valuechange. Other way possible?
            vvimDebug() << "kg kurk changed to:" << kgKaars_expected;
        }
    }
}

void OpgehaaldeHoeveelheid::checkBoxToggled(bool checked)
{
    vvimDebug() << "CheckBox toggled: this location has (not) been visited => enable/disable all Edits";
    ophaalpuntEdit->setDisabled(checked);
    opmerkingenEdit->setDisabled(checked);
    kgKurkSpinBox->setDisabled(checked);
    zakkenKurkSpinBox->setDisabled(checked);
    kgKaarsrestenSpinBox->setDisabled(checked);
    zakkenKaarsrestenSpinBox->setDisabled(checked);
}

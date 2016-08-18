#include "routetool.h"
#include "ui_routetool.h"
#include <QDebug>
#include <QApplication>
#include "form.h"

#include <QMessageBox>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

#define EndOfLine "\r\n"

#include "kiesgedaneophaling.h"


RouteTool::RouteTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RouteTool)
{
    ui->setupUi(this);

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);

    /***
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->nieuweAanmeldingMenuButton, SIGNAL(triggered()), this, SLOT(showNieuweAanmelding()));
    connect(ui->nieuwOphaalpuntMenuButton, SIGNAL(triggered()), this, SLOT(showNieuwOphaalpunt()));
    connect(ui->configurationMenuButton, SIGNAL(triggered()), this, SLOT(showConfiguration()));
    connect(ui->kiesOphaalpuntenMenuButton, SIGNAL(triggered()), this, SLOT(showKiesOphaalpunten()));
    connect(ui->nieuweLeveringMenuButton, SIGNAL(triggered()), this, SLOT(showNieuweLevering()));
    connect(ui->bellijstMenuButton, SIGNAL(triggered()), this, SLOT(showBellijst()));
    connect(ui->toonOphaalpuntenMenuButton, SIGNAL(triggered()), this, SLOT(showOphaalpuntenWidget()));
    connect(ui->effectiefOpgehaaldeHoeveelhedenMenuButton, SIGNAL(triggered()), this, SLOT(showEffectiefOpgehaaldeHoeveelheden()));
    connect(ui->actionAnnuleer_ingegeven_ophaalronde, SIGNAL(triggered()), this, SLOT(showAnnuleerIngegevenOphaalronde()));
    connect(ui->actionExporteer_Historiek, SIGNAL(triggered()), this, SLOT(showExportCollectionHistory()));
    connect(ui->actionExporteer_ophaalpunten, SIGNAL(triggered()), this, SLOT(showExportGegevensOphaalpunten()));
    connect(ui->actionOphaalronde_aanpassen, SIGNAL(triggered()), this, SLOT(showOphaalrondeAanpassen()));


    connect(&kiesOphaalpuntenWidget, SIGNAL(aanmelding_for_route(QList<SOphaalpunt> *)), m_pForm, SLOT(add_aanmeldingen(QList<SOphaalpunt>*)));
    connect(this, SIGNAL(aanmelding_for_route(QList<SOphaalpunt> *)), m_pForm, SLOT(add_aanmeldingen(QList<SOphaalpunt>*)));
    connect(&leveringWidget, SIGNAL(levering_for_route(SLevering)), m_pForm, SLOT(add_levering(SLevering)));
    connect(&configurationWidget, SIGNAL(configurationChanged()), m_pForm, SLOT(setTotalWeightTotalVolume()));

    // when table OPHAALPUNTEN is changed, we should reload all the completers. Yes, _all_ of them...
    /// 1] for the mainform
    connect(&ophaalpuntenWidget, SIGNAL(contentsOfDatabaseChanged()), m_pForm, SLOT(reloadCompleter()));
    connect(&contactListWidget, SIGNAL(contentsOfDatabaseChanged()), m_pForm, SLOT(reloadCompleter()));
    connect(&nieuwOphaalpuntWidget, SIGNAL(infoChanged()), m_pForm, SLOT(reloadCompleter()));
    /// 2] for the nieuweAanmeldingWidget
    connect(&ophaalpuntenWidget, SIGNAL(contentsOfDatabaseChanged()), &nieuweAanmeldingWidget, SLOT(loadOphaalpunten()));
    connect(&contactListWidget, SIGNAL(contentsOfDatabaseChanged()), &nieuweAanmeldingWidget, SLOT(loadOphaalpunten()));
    connect(&nieuwOphaalpuntWidget, SIGNAL(infoChanged()), &nieuweAanmeldingWidget, SLOT(loadOphaalpunten()));
    /// 3] for the ophaalpuntenWidget
    connect(&ophaalpuntenWidget, SIGNAL(contentsOfDatabaseChanged()), &ophaalpuntenWidget, SLOT(loadOphaalpunten()));
    connect(&contactListWidget, SIGNAL(contentsOfDatabaseChanged()), &ophaalpuntenWidget, SLOT(loadOphaalpunten()));
    connect(&nieuwOphaalpuntWidget, SIGNAL(infoChanged()), &ophaalpuntenWidget, SLOT(loadOphaalpunten()));
    /// 3] for the leveringWidget
    connect(&ophaalpuntenWidget, SIGNAL(contentsOfDatabaseChanged()), &leveringWidget, SLOT(loadOphaalpunten()));
    connect(&contactListWidget, SIGNAL(contentsOfDatabaseChanged()), &leveringWidget, SLOT(loadOphaalpunten()));
    connect(&nieuwOphaalpuntWidget, SIGNAL(infoChanged()), &leveringWidget, SLOT(loadOphaalpunten()));

    connect(m_pForm, SIGNAL(showOphaalpuntInfo(int)), this, SLOT(showOphaalpuntInfo(int)));
    connect(&nieuwOphaalpuntWidget,SIGNAL(nieuweAanmelding(int)),&nieuweAanmeldingWidget,SLOT(aanmeldingVoorOphaalpunt(int)));

    /// call cleanMarkersAndOpenOldRoute() when TransportationListWriter::print() is done and Boolean is set commit 89741718d8520aab7ddb21a1a9bc2b90b7f5ec2c https://github.com/vvim/routetool/commit/89741718d8520aab7ddb21a1a9bc2b90b7f5ec2c
    connect(m_pForm, SIGNAL(signalCleanMarkersAndOpenOldRoute()), this, SLOT(cleanMarkersAndOpenOldRoute()));
    ***/
}

RouteTool::~RouteTool()
{
    vvimDebug() << "start to deconstruct RouteTool()";
    delete m_pForm;
    delete ui;
    vvimDebug() << "RouteTool() deconstructed";
}

/*
void RouteTool::showNieuweAanmelding()
{
    nieuweAanmeldingWidget.resetValues();
    nieuweAanmeldingWidget.show();
}

void RouteTool::showNieuwOphaalpunt()
{
    nieuwOphaalpuntWidget.showAanmeldingAndHistoriekButton(false);
    nieuwOphaalpuntWidget.createNewOphaalpunt();
    // this functions triggers "show()" AND resets the information in the dialogbox at the same time
}

void RouteTool::showConfiguration()
{
    configurationWidget.setOriginalValues();
    configurationWidget.show();
    // here the information stays the same, so it shouldn't matter
}



void RouteTool::showKiesOphaalpunten()
{
    kiesOphaalpuntenWidget.initialise();
    kiesOphaalpuntenWidget.show();
    // always re-initialise the list of 'aanmeldingen' as the user might have given new input since last time
}

void RouteTool::showNieuweLevering()
{
    leveringWidget.resetValues();
    leveringWidget.show();
}

void RouteTool::showBellijst()
{
    contactListWidget.initialise();
    contactListWidget.show();
}

void RouteTool::showOphaalpuntenWidget()
{
    ophaalpuntenWidget.show();
}

void RouteTool::showEffectiefOpgehaaldeHoeveelheden()
{
    vvimDebug() << "user clicked on showEffectiefOpgehaaldeHoeveelheden()";
    KiesGedaneOphaling *kgo = new KiesGedaneOphaling(Confirming);

    switch(kgo->initialise())
    {
        case -1 :
            QMessageBox::critical(this, tr("Databank error"), tr("We konden de onbevestigde ophaalrondes niet opzoeken in de databank."));
            return;
            break;
        case 0 :
            QMessageBox::information(this, tr("Geen onbevestigde ophaalrondes gevonden"), tr("Er werden geen onbevestigde ophaalrondes gevonden."));
            return;
            break;
    }
    kgo->show();

}

void RouteTool::showAnnuleerIngegevenOphaalronde()
{
    // inherit KiesGedaneOphaling , override "accept" met UPDATE aanmelding SET ophalings_datum = NULL, volgorde = NULL WHERE ophalings_datum = :ophalingsdatum;
    vvimDebug() << "user clicked on showAnnuleerIngegevenOphaalronde()";
    KiesGedaneOphaling *kgo = new KiesGedaneOphaling(Deleting);

    switch(kgo->initialise())
    {
        case -1 :
            QMessageBox::critical(this, tr("Databank error"), tr("We konden de onbevestigde ophaalrondes niet opzoeken in de databank."));
            return;
            break;
        case 0 :
            QMessageBox::information(this, tr("Geen onbevestigde ophaalrondes gevonden"), tr("Er werden geen onbevestigde ophaalrondes gevonden."));
            return;
            break;
    }
    kgo->show();
}

void RouteTool::showOphaalrondeAanpassen()
{
    vvimDebug() << "user clicked on showOphaalrondeAanpassen()";
    vvimDebug() << "[0] FIRST, CLEAN CURRENT ROUTE. Save it ???";

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Huidige route opslaan?"));
    msgBox.setText(tr("Voor je de ingegeven route opent, wil je de huidige opslaan?"));
    QAbstractButton *myYesButton = msgBox.addButton(tr("Huidige route opslaan"), QMessageBox::YesRole);
    QAbstractButton *myNoButton = msgBox.addButton(tr("Huidige route wissen"), QMessageBox::NoRole);
    QAbstractButton *myCancelButton = msgBox.addButton(tr("Annuleren"), QMessageBox::RejectRole);

    msgBox.setIcon(QMessageBox::Question);
    msgBox.exec();

    if(msgBox.clickedButton() == myCancelButton)
    {
        // works also when the user presses ESC or simply closes the QMessageBox
        vvimDebug() << "user pressed CANCEL or ESC";
        return;
    }

    if(msgBox.clickedButton() == myYesButton)
    {
        // open "save route"
        vvimDebug() << "user pressed YES" << "save the current route before continuing";
        // EXTRA BOOLEAN VOOR FORM.CPP, zie on_pbTransportationList. AFTER_VERVOERSLIJST_CONTINUE_TO_OPEN
        m_pForm->afterTransportationListCleanMarkersAndOpenRoute();
        m_pForm->on_pbTransportationList_clicked();
        return;
    }

    if(msgBox.clickedButton() == myNoButton)
    {
        // do not save previous work, just open old route
        vvimDebug() << "user pressed NO" << "do not save the current route, just open old route";
        cleanMarkersAndOpenOldRoute();
    }
}

void RouteTool::cleanMarkersAndOpenOldRoute()
{
    vvimDebug() << "now we erase the current route and show the KGO to choose a previously saved route";
    m_pForm->removeAllMarkers();

    // inherit KiesGedaneOphaling
    KiesGedaneOphaling *kgo = new KiesGedaneOphaling(Editing);

    switch(kgo->initialise())
    {
        case -1 :
            QMessageBox::critical(this, tr("Databank error"), tr("We konden de onbevestigde ophaalrondes niet opzoeken in de databank."));
            return;
            break;
        case 0 :
            QMessageBox::information(this, tr("Geen onbevestigde ophaalrondes gevonden"), tr("Er werden geen onbevestigde ophaalrondes gevonden."));
            return;
            break;
    }
    kgo->show();
    connect(kgo, SIGNAL(showPlannedRoute(QList<SOphaalpunt> *, QDate)), m_pForm, SLOT(showPlannedRoute(QList<SOphaalpunt>*, QDate)));
}

void RouteTool::showExportCollectionHistory()
{
    ExportCollectionHistory *dialogboxToExportHistoryOfCollectedQuantities = new ExportCollectionHistory();
    dialogboxToExportHistoryOfCollectedQuantities->show();
}

void RouteTool::showOphaalpuntInfo(int ophaalpunt_id)
{

    if(ophaalpuntenWidget.OphaalpuntHasAanmeldingPresent(ophaalpunt_id))
    {
        // YES, ophaalpunt has an Aanmelding present. But which one? => query database

        QString SQLquery = QString("SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten,"
                " aanmelding.id, ophaalpunten.id, aanmelding.opmerkingen,"
                " ophaalpunten.straat, ophaalpunten.nr, ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land, aanmelding.datum "
                " FROM aanmelding, ophaalpunten"
                " WHERE ophaalpunten.id = aanmelding.ophaalpunt AND ophaalpunten.id = %1 AND aanmelding.ophaalronde_datum is NULL").arg(ophaalpunt_id);

        QSqlQuery query;
        query.prepare("SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten,"
                      " aanmelding.id, ophaalpunten.id, aanmelding.opmerkingen,"
                      " ophaalpunten.straat, ophaalpunten.nr, ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land, aanmelding.datum "
                      " FROM aanmelding, ophaalpunten"
                      " WHERE ophaalpunten.id = aanmelding.ophaalpunt AND ophaalpunten.id = :ophaalpuntid AND aanmelding.ophaalronde_datum is NULL");
        query.bindValue(":ophaalpuntid",ophaalpunt_id);

        if(!query.exec())
        {
            if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
            {
                vvimDebug() << "unable to reconnect to DB, halting";
                QMessageBox::information(this, tr("Fout bij verbinding met de databank in functie %1").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
                return;
            }
            vvimDebug() << "reconnected to DB, will try query again";

            QSqlQuery query2;
            query2.prepare("SELECT ophaalpunten.naam, aanmelding.kg_kurk, aanmelding.kg_kaarsresten, aanmelding.zakken_kurk, aanmelding.zakken_kaarsresten,"
                          " aanmelding.id, ophaalpunten.id, aanmelding.opmerkingen,"
                          " ophaalpunten.straat, ophaalpunten.nr, ophaalpunten.bus, ophaalpunten.postcode, ophaalpunten.plaats, ophaalpunten.land, aanmelding.datum "
                          " FROM aanmelding, ophaalpunten"
                          " WHERE ophaalpunten.id = aanmelding.ophaalpunt AND ophaalpunten.id = :ophaalpuntid AND aanmelding.ophaalronde_datum is NULL");

            query = query2;
            query.bindValue(":ophaalpuntid",ophaalpunt_id);

            if(!query.exec())
            {
                vvimDebug() << "ophaalpuntid = " << ophaalpunt_id << "-" << "something went wrong with checking for an existing aanmelding" << query.lastError();
                QMessageBox::information(this, tr("Fout bij verbinding met heruitvoeren query in functie %1").arg(Q_FUNC_INFO), tr("De query kon niet uitgevoerd worden na reconnectie met databank, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
                return;
            }
        }

        if(query.size() != 1)
        	vvimDebug() << "[ERROR] size of result set should be 1 but is " << query.size() << "how come? There should only be 1 aanmelding???";

        if (query.next())
        {
            QString ophaalpunt_naam = query.value(0).toString();

            double kg_kurk = query.value(1).toDouble();
            double kg_kaars = query.value(2).toDouble();
            double zakken_kurk = query.value(3).toDouble();
            double zakken_kaars = query.value(4).toDouble();
            int aanmelding_id = query.value(5).toInt();

            QString message_for_messagebox = tr("Ophaalpunt %5 heeft %1 kg kurk aangemeld (%2 zakken) en %3 kg kaarsresten (%4 zakken).\n\nWilt u dit ophaalpunt toevoegen aan de huidige route of het informatiescherm van dit ophaalpunt zien?").arg(kg_kurk).arg(zakken_kurk).arg(kg_kaars).arg(zakken_kaars).arg(ophaalpunt_naam);
            vvimDebug() << "aanmelding:" << message_for_messagebox;


            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Dit ophaalpunt heeft een aanmelding"));
            msgBox.setText(message_for_messagebox);
            QAbstractButton *myYesButton = msgBox.addButton(tr("Toevoegen aan route"), QMessageBox::YesRole);
            QAbstractButton *myNoButton = msgBox.addButton(tr("Toon info ophaalpunt"), QMessageBox::NoRole);
            QAbstractButton *myCancelButton = msgBox.addButton(tr("Annuleren"), QMessageBox::RejectRole);
            // msgBox.setStandardButtons(QMessageBox::Cancel); -> how to translate buttons to Dutch?
            msgBox.setIcon(QMessageBox::Question);
            msgBox.exec();

            if(msgBox.clickedButton() == myYesButton)
            {
                vvimDebug() << "voeg ophaalpunt" << ophaalpunt_id << "toe aan route";

                QString opmerkingen = query.value(7).toString();  // opmerkingen (uit table aanmelding)
                QString ophaalpunt_straat = query.value(8).toString();
                QString huisnr = query.value(9).toString();
                QString busnr = query.value(10).toString();
                QString postcode = query.value(11).toString();
                QString plaats = query.value(12).toString();
                QString land = query.value(13).toString();

                QList<SOphaalpunt> *listOfAanmeldingen = new QList<SOphaalpunt>();
                SOphaalpunt _ophaalpunt(
                                ophaalpunt_naam,
                                ophaalpunt_straat,
                                huisnr,
                                busnr,
                                postcode,
                                plaats,
                                land,
                                kg_kurk,
                                kg_kaars,
                                zakken_kurk,
                                zakken_kaars,
                                aanmelding_id,
                                ophaalpunt_id,
                                opmerkingen
                            );
                listOfAanmeldingen->append(_ophaalpunt);
                emit aanmelding_for_route(listOfAanmeldingen);
                return;
            }
            if(msgBox.clickedButton() == myCancelButton)
            {
                vvimDebug() << "user canceled action";
                return;
            }
        }
        else
        {
            vvimDebug() << "ERROR, something went wrong with querying for the aanmelding??";
            vvimDebug() << "will just show info on Ophaalpunt";
        }

    }

    vvimDebug() << "user agreed, or there is no Messagebox (because the ophaalpunt has no aanmelding_present, or maybe a DB-error occured in the final request?";

    // so we arrive at this point in 2 instances: or there is no AanmeldingPresent, or there is but the user clicked on "ShowOphaalpuntInfo" in during the messagebox
    nieuwOphaalpuntWidget.showOphaalpunt(ophaalpunt_id);
    nieuwOphaalpuntWidget.showAanmeldingAndHistoriekButton(true);
    nieuwOphaalpuntWidget.setWindowTitle("info over ophaalpunt");
}

void RouteTool::showExportGegevensOphaalpunten()
{
    vvimDebug() << "export data of all locations";

    vvimDebug() << "[1] show QFileDialog";

    QString filters("CSVbestanden (*.csv);;Tekstbestanden (*.txt);;Microsoft Excel (*.xls *.xlsx);;All files (*.*)");
    QString defaultFilter("Microsoft Excel (*.xls *.xlsx)");
    QString filename = QFileDialog::getSaveFileName(0, tr("Exporteer lijst ophaalpunten naar..."), QDir::currentPath(), filters, &defaultFilter);

    if(filename.count() < 1)
    {
        vvimDebug() << "... no filename given, the user must have pressed 'cancel' or 'close' in the FileDialogBox. Let's return to the Export Collection History Dialog Box like nothing happened...";
        // no need to show QMessageBox
        return; // RETURN FALSE
    }

    // would be better with "QFileDialog.SetDefaultSuffix()", but I simply don't get it: http://stackoverflow.com/questions/1953631/qfiledialog-adding-extension-automatically-when-saving-file
    // so, a little hack:
    if((filename.right(4) == ".xls") || (filename.right(5) == ".xlsx") || (filename.right(4) == ".txt") || (filename.right(4) == ".csv") )
        vvimDebug() << "...file-extension is alright:" << filename.right(4);
    else
    {
        vvimDebug() << "... no valid extension given, so we will add '.csv'";
        filename.append(".csv");
    }

    vvimDebug() << "...we will save it as" << filename;

    /// 2. now that we finally have this filename-stuff settled, let's go down to business!

    vvimDebug() << "[2] open file";
    QFile f( filename );

    if(!f.open(QFile::WriteOnly | QFile::Truncate)) // 'truncate' == overwrite
    {
        vvimDebug() << "... FAILED: We could not opened file" << filename << "show messagebox to user and return to Export Collection History Dialog Box";
        QMessageBox::information(this, tr("Kan bestand niet openen"), tr("Bestand %1 kan niet geopend worden, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder.").arg(filename));
        return; // RETURN FALSE
    }

    QTextStream data( &f );
    //data.setCodec("Latin-1");
    data.setCodec("UTF-8");
    data.setGenerateByteOrderMark(true);

    QSqlQuery query;
    QString SQLquery = "SELECT * FROM ophaalpunten ORDER BY postcode";

    query.prepare(SQLquery);
    data << tr("\"Export van de ophaalpunten uit de databank\"")+EndOfLine;
    data << EndOfLine;


    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "... FAILED: We could not execute the query " << SQLquery;
            vvimDebug() << "... show messagebox to user and cancel function";
            QMessageBox::information(this, tr("Fout bij verbinding met de databank").arg(Q_FUNC_INFO), tr("De databank kon niet geraadpleegd worden, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
            return; // RETURN FALSE
        }
        else
        {

            query.prepare(SQLquery);
            if(!query.exec())
            {
                vvimDebug() << "FAILED: We could not execute the query " << SQLquery;
                vvimDebug() << "show messagebox to user and return to Export Collection History Dialog Box";
                QMessageBox::information(this, tr("Fout bij verbinding met heruitvoeren query ").arg(Q_FUNC_INFO), tr("De query kon niet uitgevoerd worden na reconnectie met databank, probeer opnieuw. Als deze fout zich blijft voordoen, stuur het logbestand naar Wim of neem contact op met de systeembeheerder."));
                return; // RETURN FALSE
            }
        }
    }

    QStringList strList;

    int records = 0;

    int columns_in_queryresult = query.record().count();

    vvimDebug() << "[3] get all possibilities for contactpreference, attestation, language, ... and put them in QMaps";

    QMap <int,QString> contact_preference = getQMapFromSQLTable("SELECT id, medium FROM contacteren");
    QMap <int,QString> attest_frequencies = getQMapFromSQLTable("SELECT id, frequentie FROM frequentie");
    QMap <int,QString> location_type = getQMapFromSQLTable("SELECT code, soort FROM soort_ophaalpunt");
    QMap <int,QString> intercommunales = getQMapFromSQLTable("SELECT id, naam_intercommunale FROM intercommunales");
    QMap <int,QString> languages = getQMapFromSQLTable("SELECT id, taal FROM talen");

    vvimDebug() << "[4] fill in columnheaders";
    vvimDebug() << "... dit mag 'dynamischer', nu is alles hardcoded, niet flexibel";

    strList << tr("\"Ophaalpunt\"");
    strList << tr("\"Voor kurk\"");
    strList << tr("\"Voor parafine\"");
    strList << tr("\"Soort ophaalpunt\""); // "code"
    strList << tr("\"Intercommunale?\""); // "code intercommunale"
    strList << tr("\"Straat\"");
    strList << tr("\"Nr\"");
    strList << tr("\"Bus\"");
    strList << tr("\"Postcode\"");
    strList << tr("\"Plaats\"");
    strList << tr("\"Land\"");
    strList << tr("\"Openingsuren\"");
    strList << tr("\"Contactpersoon\"");
    strList << tr("\"Telefoon1\"");
    strList << tr("\"Telefoon2\"");
    strList << tr("\"Email1\"");
    strList << tr("\"Email2\"");
    strList << tr("\"Taalvoorkeur\""); // "taalvoorkeur"
    strList << tr("\"Liefst contacteren per\""); // "preferred_contact"
    strList << tr("\"Attest nodig?\"");
    strList << tr("\"Attestfrequentie\"");
    strList << tr("\"Extra informatie\"");
    strList << tr("\"Laatste contactmoment\"");
//    strList << tr("\"Contacteer tegen\""); -> we do not need to export this column, see (c == 25 ) while going through the results
    strList << tr("\"Laatste ophaalmoment\"");
    strList << tr("\"Voorspelde nieuwe ophaling\"");
    strList << tr("\"Latitude\"");
    strList << tr("\"Longitude\"");

    data << strList.join( ";" )+EndOfLine;

    vvimDebug() << "[5] fill in the content";

    while (query.next())
    {
        strList.clear();
        for( int c = 2; c < columns_in_queryresult; ++c ) // first two columns, namely "ophaalpunten.id" and "ophaalpunten.timestamp", do not need to be exported
        {
/// MAKE THIS A SWITCH STATEMENT!!!
            switch(c)
            {
                case 25:
                    // the column "contact_again_on" is obsolete, but still exist in the database
                    break;

                // Boolean values:
                case 3: // Kurk?
                case 4: // Parafine?
                case 21: // Attest nodig?
                    if(query.value(c).toBool())
                        strList <<  tr("\"ja\"");
                    else
                        strList <<  tr("\"nee\"");
                    break;
                case 5: // Soort ophaalpunt
                    strList <<  "\""+location_type.value(query.value(c).toInt()) +"\"";
                    break;
                case 6: // Intercommunale?
                    strList <<  "\""+intercommunales.value(query.value(c).toInt()) +"\"";
                    break;
                case 19: // Taalvoorkeur?
                    strList <<  "\""+languages.value(query.value(c).toInt()) +"\"";
                    break;
                case 20: // Liefst contacteren per?
                    strList <<  "\""+contact_preference.value(query.value(c).toInt()) +"\"";
                    break;
                case 22: // Attestfrequentie
                    strList <<  "\""+attest_frequencies.value(query.value(c).toInt()) +"\"";
                    break;
                default:
                    strList <<  "\""+query.value(c).toString().toUtf8() +"\"";
                    break;
            }
        }

        data << strList.join( ";" ).replace("\n"," ")+EndOfLine;

        records++;
        data.flush();
    }

    if(records < 1)
    {
        vvimDebug() << "...no results from this query";
        data << tr("Geen ophaalpunten gevonden in de databank.")+";\n";
    }
    else
    {
        vvimDebug() << "..." << records << "locations found and recorded";
    }
    QMessageBox::information(this, tr("Informatie ophaalpunten geëxporteerd"), tr("Er werden %1 ophaalpunten gevonden en geëxporteerd naar bestand %2.").arg(records).arg(filename));

    f.close();

    // RETURN TRUE
}
*/

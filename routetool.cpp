#include "routetool.h"
#include "ui_routetool.h"
#include <QDebug>
#include <QApplication>
#include "form.h"


#include <QMessageBox>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"






#include "kiesgedaneophaling.h"


RouteTool::RouteTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RouteTool)
{
    ui->setupUi(this);

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
    connect(ui->actionOphaalronde_aanpassen, SIGNAL(triggered()), this, SLOT(showOphaalrondeAanpassen()));

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);

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

}

RouteTool::~RouteTool()
{
    vvimDebug() << "start to deconstruct RouteTool()";
    delete m_pForm;
    delete ui;
    vvimDebug() << "RouteTool() deconstructed";
}

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

    /**
      2. nieuw venster met daarin de QSqlWidgetMapper om elke locatie apart te bevestigen, in volgorde!

                => select * from aanmelding where ophaalronde_datum = GEKOZEN_OPHAALRONDE_DATUM

      3. dubbelen verwijderen? => zie select in 1.

      ** insert into TABLE OPHAALHISTORIEK
      ging dat ok? =>
      ** delete from TABLE AANMELDING

      ??? wat als we een bevestigde ophaling toch nog willen wijzigen ???

      ??? wat als we een geplande ophaling willen annuleren ???
        --> zie showAnnuleerIngegevenOphaalronde()
    **/
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
    // inherit KiesGedaneOphaling
    vvimDebug() << "user clicked on showOphaalrondeAanpassen()";
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
    connect(kgo, SIGNAL(aanmelding_for_route(QList<SOphaalpunt> *)), m_pForm, SLOT(add_aanmeldingen(QList<SOphaalpunt>*)));
}

void RouteTool::showExportCollectionHistory()
{
    ExportCollectionHistory *dialogboxToExportHistoryOfCollectedQuantities = new ExportCollectionHistory();
    dialogboxToExportHistoryOfCollectedQuantities->show();
}

void RouteTool::showOphaalpuntInfo(int ophaalpunt_id)
{
    /** signal gets emited from Form::askMainProgramToShowOphaalpuntInfo(int ophaalpunt_id)
        reason for being: when the user clicks on "on_showOphaalpunten_clicked()", the map
        displays all known locations on the map (except those who were already in the route).

        The locations with a known 'aanmelding' will be colored blue, the other ones are yellow.
        Instead of making the distinction Blue/Yellow in the Javascript of "on_showOphaalpunten_clicked()",
        we will make the distinction here. It is easier to query OphaalpuntenWidget than to write
        an extensive Javascript.
    **/

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
                QDate datum = query.value(14).toDate();

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

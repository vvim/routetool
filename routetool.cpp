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

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);

    connect(&kiesOphaalpuntenWidget, SIGNAL(aanmelding_for_route(QList<SOphaalpunt> *)), m_pForm, SLOT(add_aanmeldingen(QList<SOphaalpunt>*)));
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
    ophaalpuntenWidget.initialise();
    ophaalpuntenWidget.show();
}

void RouteTool::showEffectiefOpgehaaldeHoeveelheden()
{
    vvimDebug() << "user clicked on showEffectiefOpgehaaldeHoeveelheden()";
    KiesGedaneOphaling *kgo = new KiesGedaneOphaling();

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
    KiesGedaneOphaling *kgo = new KiesGedaneOphaling(false);

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

void RouteTool::showExportCollectionHistory()
{
    ExportCollectionHistory *dialogboxToExportHistoryOfCollectedQuantities = new ExportCollectionHistory();
    dialogboxToExportHistoryOfCollectedQuantities->show();
}

void RouteTool::showOphaalpuntInfo(int ophaalpunt_id)
{
    nieuwOphaalpuntWidget.showAanmeldingAndHistoriekButton(true);
    nieuwOphaalpuntWidget.setWindowTitle("info over ophaalpunt");
    nieuwOphaalpuntWidget.showOphaalpunt(ophaalpunt_id);
}

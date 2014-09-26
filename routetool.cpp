#include "routetool.h"
#include "ui_routetool.h"
#include <QDebug>
#include <QApplication>
#include "form.h"

#include <QSqlQuery>
#include <QSqlError>

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
    connect(ui->wieNogNooitMenuButton, SIGNAL(triggered()), this, SLOT(showBijWieNogNooitOpgehaald()));
    connect(ui->ouderDanEenJaarMenuButton, SIGNAL(triggered()), this, SLOT(showBijWieOuderDanJaarWidget()));
    connect(ui->voorspellingMenuButton, SIGNAL(triggered()), this, SLOT(showVoorspelling()));
    connect(ui->toonOphaalpuntenMenuButton, SIGNAL(triggered()), this, SLOT(showOphaalpuntenWidget()));
    connect(ui->effectiefOpgehaaldeHoeveelhedenMenuButton, SIGNAL(triggered()), this, SLOT(showEffectiefOpgehaaldeHoeveelheden()));

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

void RouteTool::showBijWieNogNooitOpgehaald()
{
    contactListWidget.show_never_contacted_ophaalpunten(); // does 'init' & 'show' in one go.
}

void RouteTool::showBijWieOuderDanJaarWidget()
{
    contactListWidget.show_one_year_ophaalpunten(); // does 'init' & 'show' in one go.
}

void RouteTool::showVoorspelling()
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
    qDebug() << "Here we go!";
    KiesGedaneOphaling *kgo = new KiesGedaneOphaling();

    /**
      1. geef een QComboBox met de verschillende data waarvan een ophaling nog niet bevestigd is geweest
                dit zijn aanmeldingspunten in de table AANMELDING die een ophaalronde_datum en een volgorde hebben gekregen
                (zie code regel 219 in transportationlistwriter.cpp)

                => select distinct ophaalronde_datum from aanmelding where volgorde is not null

                ???=[3.]=> select distinct aanmelding.ophaalronde_datum from aanmelding where volgorde is not null
                                            AND not exists
                            (select null from ophalinghistoriek
                             wehere ophalinghistoriek.ophaalpunt = aanmelding.ophaalpunt and ophalinghistoriek.datum = ophaalronde_datum)
                    zie regel 361 van void ListOfOphaalpuntenToContact::show_never_contacted_ophaalpunten()



      2. nieuw venster met daarin de QSqlWidgetMapper om elke locatie apart te bevestigen, in volgorde!

                => select * from aanmelding where ophaalronde_datum = GEKOZEN_OPHAALRONDE_DATUM

      3. dubbelen verwijderen? => zie select in 1.

      ** insert into TABLE OPHAALHISTORIEK
      ging dat ok? =>
      ** delete from TABLE AANMELDING

      ??? wat als we een bevestigde ophaling toch nog willen wijzigen ???

      ??? wat als we een geplande ophaling willen annuleren ???
        --> ophalings_datum en volgorde terug op NULL zetten
    **/
}

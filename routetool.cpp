#include "routetool.h"
#include "ui_routetool.h"
#include <QDebug>
#include <QApplication>
#include "form.h"

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
    connect(ui->voorspellingMenuButton, SIGNAL(triggered()), this, SLOT(showVoorspelling()));

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);

    connect(&kiesOphaalpuntenWidget, SIGNAL(aanmelding_for_route(QList<SOphaalpunt> *)), m_pForm, SLOT(add_aanmeldingen(QList<SOphaalpunt>*)));
    connect(&leveringWidget, SIGNAL(levering_for_route(SLevering)), m_pForm, SLOT(add_levering(SLevering)));
    connect(&configurationWidget, SIGNAL(configurationChanged()), m_pForm, SLOT(setTotalWeightTotalVolume()));
}

RouteTool::~RouteTool()
{
    qDebug() << "start to deconstruct RouteTool()";
    delete m_pForm;
    delete ui;
    qDebug() << "RouteTool() deconstructed";
}

void RouteTool::showNieuweAanmelding()
{
    nieuweAanmeldingWidget.resetValues();
    nieuweAanmeldingWidget.show();
}

void RouteTool::showNieuwOphaalpunt()
{
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
    qDebug() << "Toon bij welke ophaalpunten we nog nooit iets zijn gaan ophalen";
    /** waarschijnlijk gebaseerd op kiesOphaalpuntenWidget
        kiesOphaalpuntenWidget.initialise();
        kiesOphaalpuntenWidget.show();
    **/
}

void RouteTool::showVoorspelling()
{
    qDebug() << "Toon welke ophaalpunten nu waarschijnlijk een voorraad zullen hebben (berekend uit de historiek van ophalingen)";
    /** waarschijnlijk gebaseerd op kiesOphaalpuntenWidget
        kiesOphaalpuntenWidget.initialise();
        kiesOphaalpuntenWidget.show();
    **/
}

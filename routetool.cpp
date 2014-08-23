#include "routetool.h"
#include "ui_routetool.h"
#include <QDebug>
#include <QApplication>
#include "form.h"

#include <QSqlQuery>
#include <QSqlError>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

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

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);

    connect(&kiesOphaalpuntenWidget, SIGNAL(aanmelding_for_route(QList<SOphaalpunt> *)), m_pForm, SLOT(add_aanmeldingen(QList<SOphaalpunt>*)));
    connect(&leveringWidget, SIGNAL(levering_for_route(SLevering)), m_pForm, SLOT(add_levering(SLevering)));
    connect(&configurationWidget, SIGNAL(configurationChanged()), m_pForm, SLOT(setTotalWeightTotalVolume()));
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

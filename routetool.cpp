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

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);

}

RouteTool::~RouteTool()
{
    delete m_pForm;
    delete ui;
}

void RouteTool::showNieuweAanmelding()
{
    nieuweAanmeldingWidget.resetValues();
    nieuweAanmeldingWidget.show();
/**
    NieuweAanmelding *na = new NieuweAanmelding();
    na->show();
    // are these pointers automatically destroyed after closing the program, or does this create memory leaks?
    // might use "nieuweAanmeldingWidget.show()" , but then will have to clear the information every time I call it?
**/
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

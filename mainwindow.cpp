#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    /**

      make variables of all possible windows

      0) see last meeting for complete list, but what I can think of now is:

      1) RouteTool

      2) NieuweAanmelding

      3) InfoOphaalpunt (to create a new ophaalpunt)

      4) Configuration

      each need a QPushButton to be activated, maybe a nice logo?

    **/

    routetool = new RouteTool();
    nieuweaanmelding = new NieuweAanmelding();
    nieuwophaalpunt = new InfoOphaalpunt();
    configuration = new Configuration();

    startRouteToolButton = new QPushButton(tr("Start Route Tool"));
    startNieuweaanmeldingButton = new QPushButton(tr("Nieuwe aanmelding ingeven"));
    startNieuwophaalpuntButton = new QPushButton(tr("Nieuw ophaalpunt ingeven"));
    startConfigurationButton = new QPushButton(tr("Configuratie aanpassen"));
    exitButton = new QPushButton(tr("Exit"));

    connect(startRouteToolButton, SIGNAL(clicked()), this, SLOT(startRouteTool()));
    connect(startNieuweaanmeldingButton, SIGNAL(clicked()), this, SLOT(startNieuweaanmelding()));
    connect(startNieuwophaalpuntButton, SIGNAL(clicked()), this, SLOT(startNieuwophaalpunt()));
    connect(startConfigurationButton, SIGNAL(clicked()), this, SLOT(startConfiguration()));
    connect(exitButton, SIGNAL(clicked()), this, SLOT(exitProgram()));

    QGridLayout *layout = new QGridLayout();

        // ROW,  COLUMN,  rowspan columnspan

    layout->addWidget(startRouteToolButton,0,0,1,1);
    layout->addWidget(startNieuweaanmeldingButton,1,0,1,1);
    layout->addWidget(startNieuwophaalpuntButton,2,0,1,1);
    layout->addWidget(startConfigurationButton,3,0,1,1);
    layout->addWidget(exitButton,4,0,1,1);

    centralButtons = new QWidget();
    centralButtons->setLayout(layout);
    setCentralWidget(centralButtons);
    setWindowTitle(tr("Bow for your central buttons overlord!"));
    setMinimumWidth(850);
    setMinimumHeight(700);
}


void MainWindow::startRouteTool()
{
    setCentralWidget(routetool);
    //routetool->show();
}


void MainWindow::startNieuweaanmelding()
{
    setCentralWidget(nieuweaanmelding);
    //nieuweaanmelding->show();
}


void MainWindow::startNieuwophaalpunt()
{
    setCentralWidget(nieuwophaalpunt);
    //nieuwophaalpunt->show();
}


void MainWindow::startConfiguration()
{
    setCentralWidget(configuration);
    //configuration->show();
}


void MainWindow::exitProgram()
{
    delete routetool;
    delete nieuweaanmelding;
    delete nieuwophaalpunt;
    delete configuration;
    this->close();
}




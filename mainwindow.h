#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "routetool.h"
#include "nieuweaanmelding.h"
#include "infoophaalpunt.h"
#include "configuration.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    
signals:
    
public slots:

private:
    RouteTool *routetool;
    NieuweAanmelding *nieuweaanmelding;
    InfoOphaalpunt *nieuwophaalpunt;
    Configuration *configuration;
    
    QPushButton *startRouteToolButton;
    QPushButton *startNieuweaanmeldingButton;
    QPushButton *startNieuwophaalpuntButton;
    QPushButton *startConfigurationButton;
    QPushButton *exitButton;

    QWidget *centralButtons;

private slots:
    void startRouteTool();
    void startNieuweaanmelding();
    void startNieuwophaalpunt();
    void startConfiguration();
    void exitProgram();
};

#endif // MAINWINDOW_H

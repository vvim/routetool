#ifndef ROUTETOOL_H
#define ROUTETOOL_H

#include <QMainWindow>
#include <QSettings>
#include "nieuweaanmelding.h"
#include "infoophaalpunt.h"
#include "configuration.h"
#include "kiesophaalpunten.h"
#include "levering.h"
#include "listofophaalpuntentocontact.h"
#include "ophaalpuntenwidget.h"

extern QSettings settings;

class Form;

namespace Ui {
    class RouteTool;
}

class RouteTool : public QMainWindow
{
    Q_OBJECT

public:
    explicit RouteTool(QWidget *parent = 0);
    ~RouteTool();


private:
    Ui::RouteTool *ui;
    Form* m_pForm;
    NieuweAanmelding nieuweAanmeldingWidget;
    InfoOphaalpunt nieuwOphaalpuntWidget;
    Configuration configurationWidget;
    KiesOphaalpunten kiesOphaalpuntenWidget;
    Levering leveringWidget;
    ListOfOphaalpuntenToContact contactListWidget;
    OphaalpuntenWidget ophaalpuntenWidget;

private slots:
    void showNieuweAanmelding();
    void showNieuwOphaalpunt();
    void showConfiguration();
    void showKiesOphaalpunten();
    void showNieuweLevering();
    void showBijWieNogNooitOpgehaald();
    void showVoorspelling();
    void showOphaalpuntenWidget();
};

#endif // ROUTETOOL_H

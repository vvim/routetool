#ifndef ROUTETOOL_H
#define ROUTETOOL_H

#include <QMainWindow>
#include <QSettings>
#include "nieuweaanmelding.h"
#include "infoophaalpunt.h"
#include "configuration.h"
#include "kiesophaalpunten.h"

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

private slots:
    void showNieuweAanmelding();
    void showNieuwOphaalpunt();
    void showConfiguration();
    void showKiesOphaalpunten();
    void showNieuweLevering();
};

#endif // ROUTETOOL_H

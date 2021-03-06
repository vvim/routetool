#ifndef ROUTETOOL_H
#define ROUTETOOL_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include <QSqlRecord>
#include "nieuweaanmelding.h"
#include "infoophaalpunt.h"
#include "configuration.h"
#include "kiesophaalpunten.h"
#include "levering.h"
#include "listofophaalpuntentocontact.h"
#include "ophaalpuntenwidget.h"
#include "exportcollectionhistory.h"

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

signals:
    void aanmelding_for_route(QList<SOphaalpunt> *);

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
    void showBellijst();
    void showOphaalpuntenWidget();
    void showEffectiefOpgehaaldeHoeveelheden();
    void showAnnuleerIngegevenOphaalronde();
    void showExportCollectionHistory();
    void showExportGegevensOphaalpunten();
    void showOphaalpuntInfo(int ophaalpunt_id);
    void showOphaalrondeAanpassen();
    void cleanMarkersAndOpenOldRoute();
};

#endif // ROUTETOOL_H

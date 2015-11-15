#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "geocode_data_manager.h"
#include "distancematrix.h"
#include "smarker.h"
#include <QCompleter>
#include "mylineedit.h" // not my personal code, where is it from? (forum on QCompleter)
#include <QSettings>
#include <QList>
#include <QWebPage>
#include "sophaalpunt.h"
#include "transportationlistwriter.h"

extern QSettings settings;

namespace Ui {
    class Form;
}

class myWebPage : public QWebPage
{
    virtual QString userAgentForUrl(const QUrl& url) const {
        // return "Chrome/1.0"; // see https://wiki.qt.io/How_to_set_user_agent_in_Qt_application
        return "Mozilla/5.0 (X11; Linux i686; rv:40.0) Gecko/20100101 Firefox/40.0";
    }
};


class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

signals:
    void showOphaalpuntInfo(int ophaalpunt_id);

public slots:
    void askMainProgramToShowOphaalpuntInfo(int ophaalpunt_id);
    void resetTotalDistanceAndTotalTime();

private slots:
    void goClicked();
    void showCoordinates(double east, double north, QString markername, bool saveMarker = true);
    void showOphaalpunt(double east, double north, SOphaalpunt ophaalpunt, bool saveMarker = true);
    void showLevering(double east, double north, SLevering levering, bool saveMarker = true);
    void putCoordinatesInDatabase(double east, double north, int ophaalpunt_id);
    //set marker to map and save marker in markers list
    void setMarker(double east, double north, QString caption);
    void setMarker(double east, double north, SOphaalpunt ophaalpunt);
    void setMarker(double east, double north, SLevering levering);
    void errorOccured(const QString&);
    void process_result_distancematrix(QList<int> *);
    void reload_distancematrix(int**, int**);
    void add_aanmeldingen(QList<SOphaalpunt> *);
    void add_levering(SLevering);
    void keyPressEvent( QKeyEvent *k );
    void on_lwMarkers_currentRowChanged(int currentRow);
    void on_pbRemoveMarker_clicked();
    void on_zoomSpinBox_valueChanged(int arg1);
    void on_pbOptimizeRoute_clicked();
    void on_pbTransportationList_clicked();
    void drawRoute();
    void on_pbRouteOmdraaien_clicked();
    void reorderMarkers();
    void setTotalWeightTotalVolume();
    void reloadCompleter();

    void on_showOphaalpunten_clicked();
    void populateJavaScriptWindowObject();

private:
    int **distance_matrix_in_meters;
    int **distance_matrix_in_seconds;
    int matrix_dimensions;
    bool matrices_up_to_date;
    bool after_calculating_distance_matrix_continue_to_tsp;
    bool after_calculating_distance_matrix_continue_to_transportationlist;

    QMap<QString, SMarker*> link_lwMarkers_mmarkers;
    QPalette *warning, *normal;

    Ui::Form *ui;
    GeocodeDataManager m_geocodeDataManager;
    DistanceMatrix m_distanceMatrix;
    TransportationListWriter transportationlistWriter;
    //markers list
    QList <SMarker*> m_markers;

    MyCompleter *completer;

    QSet<int> *getOphaalpuntIdFromRoute();

    void buildTransportationList();
    void logOutputMarkers();
    void logOutputLwMarkers();
    void setTotalDistanceAndTotalTime(int total_distance_in_meters, int total_time_on_the_road_in_seconds);

};

#endif // FORM_H

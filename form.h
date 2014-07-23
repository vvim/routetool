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

extern QSettings settings;

namespace Ui {
    class Form;
}



class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

private slots:
    void goClicked();
    void showCoordinates(double east, double north, QString markername, bool saveMarker = true);
    //set marker to map and save marker in markers list
    void setMarker(double east, double north, QString caption);
    void errorOccured(const QString&);
    void adapt_order_smarkers(QList<int> *, int**, int**);
    void add_aanmeldingen(QList<QString> *);
    void keyPressEvent( QKeyEvent *k );
    void on_lwMarkers_currentRowChanged(int currentRow);
    void on_pbRemoveMarker_clicked();
    void on_zoomSpinBox_valueChanged(int arg1);
    void on_pbDistanceMatrix_clicked();
    void drawRoute();
    void on_pbRouteOmdraaien_clicked();
    void reorderMarkers();

private:
    /**    I believe this member is a copy/paste accident, must try out.
        void getCoordinates(const QString& address);
    **/
    int **distance_matrix_in_meters;
    int **distance_matrix_in_seconds;
    int matrix_dimensions;

    QMap<QString, SMarker*> link_lwMarkers_mmarkers;


private:
    Ui::Form *ui;
    GeocodeDataManager m_geocodeDataManager;
    DistanceMatrix m_distanceMatrix;
    //markers list
    QList <SMarker*> m_markers;

    MyCompleter *completer;

    /**    I believe this member is a copy/paste accident, must try out. Was already included in version 20140519

    QAbstractItemModel *modelFromFile(const QString& fileName);

    **/

    void logOutputMarkers();
};

#endif // FORM_H

#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "geocode_data_manager.h"
#include "distancematrix.h"
#include "smarker.h"
#include <QCompleter>
#include "mylineedit.h" // not my personal code, where is it from? (forum on QCompleter)
#include <QSettings>

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

    void showCoordinates(double east, double north, bool saveMarker = true);
    //set marker to map and save marker in markers list
    void setMarker(double east, double north, QString caption);
    void errorOccured(const QString&);
    void adapt_order_smarkers(QList<int> *);

    void keyPressEvent( QKeyEvent *k );

    void on_lwMarkers_currentRowChanged(int currentRow);

    void on_pbRemoveMarker_clicked();

    void on_zoomSpinBox_valueChanged(int arg1);

    void on_pbDistanceMatrix_clicked();

private:
    void getCoordinates(const QString& address);


private:
    Ui::Form *ui;
    GeocodeDataManager m_geocodeDataManager;
    DistanceMatrix m_distanceMatrix;
    //markers list
    QList <SMarker*> m_markers;

    QAbstractItemModel *modelFromFile(const QString& fileName);

    MyCompleter *completer;
};

#endif // FORM_H

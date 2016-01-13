#ifndef TRANSPORTATIONLISTWRITER_H
#define TRANSPORTATIONLISTWRITER_H

#include <QWidget>
#include <QLineEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include "smarker.h"
#include "transportationlistdocumentwriter.h"

#include <QSettings>

extern QSettings settings;


class TransportationListWriter : public QWidget
{
    Q_OBJECT
public:
    explicit TransportationListWriter(QWidget *parent = 0);
    ~TransportationListWriter();
    
    void prepare(QList <SMarker*> m_markers, int **_distance_matrix_in_meters, int **_distance_matrix_in_seconds, QWidget *_mapwidget);
    void print();
    inline int getTotalMetersOfRoute() { return total_distance_in_meters; }
    inline int getTotalSecondsOfRoute() { return seconds_needed_to_complete_transport; }
    inline void afterTransportationListCleanMarkersAndOpenRoute() { after_transportationlist_cleanmarkersandopenroute = true; }
    inline bool checkIfWeShouldRemoveAllMarkersAndOpenRouteAfterBuildingTransportationList() { return after_transportationlist_cleanmarkersandopenroute; }

signals:
    
private slots:
    void editExpectedArrivalTime(QTime);
    void reject();
    void accept();
    void setOriginalValues();

private:
    QDateEdit* dateEdit;
    QTimeEdit* startTimeEdit;
    QTimeEdit* expectedArrivalTimeEdit;

    QList<SMarker *> m_markers;

    QSpinBox* empty_bags_of_kurk_neededEdit;
    QSpinBox* empty_bags_of_kaarsresten_neededEdit;
    bool ready, after_transportationlist_cleanmarkersandopenroute;

    int empty_bags_of_kurk_needed;
    int empty_bags_of_kaarsresten_needed;

    int total_distance_in_meters;
    int total_time_on_the_road_in_seconds;
    int seconds_needed_to_complete_transport;

    int **distance_matrix_in_meters;
    int **distance_matrix_in_seconds;
    QWidget *mapwidget;

    TransportationListDocumentWriter * translist_doc;

    void populateWithSmarker(SMarker* marker, int previous_distance_matrix_i, int current_distance_matrix_i);
    void writeInformation(SMarker* marker, int previous_distance_matrix_i, int current_distance_matrix_i, int counter = 0, char kaart_nr = 0);
    void deleteTheMatrices();
};

#endif // TRANSPORTATIONLISTWRITER_H

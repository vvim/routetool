#ifndef TRANSPORTATIONLISTWRITER_H
#define TRANSPORTATIONLISTWRITER_H

#include <QWidget>
#include <QLineEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include "smarker.h"
#include "documentwriter.h"

#include <QSettings>

extern QSettings settings;


class TransportationListWriter : public QWidget
{
    Q_OBJECT
public:
    explicit TransportationListWriter(QWidget *parent = 0);
    ~TransportationListWriter();
    
    void prepare(QList <SMarker*> m_markers, int **distance_matrix_in_meters, int **distance_matrix_in_seconds);
    void print();
signals:
    
private slots:
    void editExpectedArrivalTime(QTime);
    
private:
    QLineEdit* nameTransportationListEdit;
    QDateEdit* dateEdit;
    QTimeEdit* startTimeEdit;
    QTimeEdit* expectedArrivalTimeEdit;

    QSpinBox* empty_bags_of_kurk_neededEdit;
    QSpinBox* empty_bags_of_kaarsresten_neededEdit;
    int minutes_needed;
    bool ready;
};

#endif // TRANSPORTATIONLISTWRITER_H

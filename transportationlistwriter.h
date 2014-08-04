#ifndef TRANSPORTATIONLISTWRITER_H
#define TRANSPORTATIONLISTWRITER_H

#include <QWidget>
#include <QLineEdit>
#include <QTimeEdit>
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
    
signals:
    
public slots:
    
private:
    QLineEdit* nameTransportationListEdit;
    QTimeEdit* startTimeEdit;
    QTimeEdit* expectedArrivalTimeEdit;
};

#endif // TRANSPORTATIONLISTWRITER_H

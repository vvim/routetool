#ifndef TRANSPORTATIONLISTWRITER_H
#define TRANSPORTATIONLISTWRITER_H

#include <QWidget>
#include <QSettings>

#include "smarker.h"
#include "documentwriter.h"

extern QSettings settings;


class TransportationListWriter : public QWidget
{
    Q_OBJECT
public:
    explicit TransportationListWriter(QWidget *parent = 0);
    ~TransportationListWriter();
    
signals:
    
public slots:
    
};

#endif // TRANSPORTATIONLISTWRITER_H

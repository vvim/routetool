#ifndef OPGEHAALDEHOEVEELHEID_H
#define OPGEHAALDEHOEVEELHEID_H

#include <QWidget>
#include <QDate>

class OpgehaaldeHoeveelheid : public QWidget
{
    Q_OBJECT
public:
    explicit OpgehaaldeHoeveelheid(QDate ophaalronde_datum, QWidget *parent = 0);
    
signals:
    
public slots:
    
};

#endif // OPGEHAALDEHOEVEELHEID_H

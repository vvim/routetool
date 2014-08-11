#ifndef OPHAALPUNTENWIDGET_H
#define OPHAALPUNTENWIDGET_H

#include <QWidget>
#include <QApplication>
#include "mylineedit.h"
#include "infoophaalpunt.h"

class OphaalpuntenWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OphaalpuntenWidget(QWidget *parent = 0);
    ~OphaalpuntenWidget();
    
signals:
    
private slots:
    void ophaalpuntChanged();
    void toonOphaalpuntInformatie();

private:
    void loadOphaalpunten();

    QLabel *ophaalpuntLabel;
    MyLineEdit *ophaalpuntEdit;
    InfoOphaalpunt *info;
    MyCompleter *completer;
    QMap<QString, int> ophaalpunten;
    QPushButton* toonOphaalpunt;
};

#endif // OPHAALPUNTENWIDGET_H

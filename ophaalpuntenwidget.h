#ifndef OPHAALPUNTENWIDGET_H
#define OPHAALPUNTENWIDGET_H

#include <QWidget>
#include <QApplication>
#include "mylineedit.h"
#include "infoophaalpunt.h"
#include "nieuweaanmelding.h"

class OphaalpuntenWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OphaalpuntenWidget(QWidget *parent = 0);
    ~OphaalpuntenWidget();

    void initialise();
    
signals:
    void contentsOfDatabaseChanged();
    
private slots:
    void ophaalpuntTextChanged();
    void toonOphaalpuntInformatie();
    void databaseBeenUpdated();
    void loadOphaalpunten();

private:
    QLabel *ophaalpuntLabel;
    MyLineEdit *ophaalpuntEdit;
    InfoOphaalpunt *info;
    MyCompleter *completer;
    QPushButton* toonOphaalpunt;
    NieuweAanmelding* nieuweaanmeldingWidget;

    QMap<QString, int> ophaalpunten;
};

#endif // OPHAALPUNTENWIDGET_H

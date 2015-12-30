#ifndef OPHAALPUNTENWIDGET_H
#define OPHAALPUNTENWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QTreeView>
#include "mylineedit.h"
#include "infoophaalpunt.h"
#include "nieuweaanmelding.h"
#include "ophaalpuntenwidgetsortfilterproxymodel.h"

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
    void toonOphaalpuntInformatie(QModelIndex index);

private:
    QLabel *ophaalpuntLabel;
    MyLineEdit *ophaalpuntEdit;
    InfoOphaalpunt *info;
    MyCompleter *completer;
    QPushButton* toonOphaalpunt;
    NieuweAanmelding* nieuweaanmeldingWidget;

    QStandardItemModel *model;
    OphaalpuntenWidgetSortFilterProxyModel *listOfLocationsModel;
    QTreeView *contactTreeView;

    QMap<QString, int> ophaalpunten;

    void addToTreeView(int ophaalpuntId, QString naamOphaalpunt, QString straat, QString huisnummer, QString busnummer, QString postcode, QString plaats, QString land, bool color_item = false);
    void initModel();
};

#endif // OPHAALPUNTENWIDGET_H

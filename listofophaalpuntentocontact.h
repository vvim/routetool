#ifndef LISTOFOPHAALPUNTENTOCONTACT_H
#define LISTOFOPHAALPUNTENTOCONTACT_H

#include "infoophaalpunt.h"
#include "nieuweaanmelding.h"
#include "listofophaalpuntentocontactsortfilterproxymodel.h"
#include <QTreeWidget>
#include <QTreeView>

extern QSettings settings;

class ListOfOphaalpuntenToContact : public QWidget
{
    Q_OBJECT

public:
    explicit ListOfOphaalpuntenToContact(QWidget *parent = 0);
    ~ListOfOphaalpuntenToContact();

    void initialise();

signals:
    void contentsOfDatabaseChanged();

private slots:
    void showOphaalpunt(QTreeWidgetItem*);
    void showOphaalpunt(QModelIndex);
    void ok_button_pushed();
    void sortTreeWidget(int column);
    void databaseBeenUpdated();

private:
    InfoOphaalpunt *info;
    NieuweAanmelding *nieuweaanmeldingWidget;
    QTreeWidget *contactTree;
    QDialogButtonBox *buttonBox;
    QLabel *label;

    QStandardItemModel *model;
    ListOfOphaalpuntenToContactSortFilterProxyModel *listToContactModel;
    QTreeView *contactTreeView;


    bool sortingascending;

    void UpdateOphaalpunt(int ophaalpuntid);
    void UpdateAllOphaalpunten();
    void addToTreeWidget(QString NaamOphaalpunt, int OphaalpuntId, QString Postcode, QDate LastContactDate, QDate LastOphalingDate, QDate ForecastNewOphalingDate, bool color_item = false);
    void initModel();
};

#endif // LISTOFOPHAALPUNTENTOCONTACT_H

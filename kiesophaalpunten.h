#ifndef KIESOPHAALPUNTEN_H
#define KIESOPHAALPUNTEN_H

#include <QWidget>
#include <QLabel>
#include <QTreeView>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSettings>
#include <QStandardItemModel>
#include "sophaalpunt.h"
#include "mysortfilterproxymodel.h"
#include "globalfunctions.h"


class KiesOphaalpunten : public QWidget
{
    Q_OBJECT
public:
    explicit KiesOphaalpunten(QWidget *parent = 0);
    ~KiesOphaalpunten();

    void initialise();

    
signals:
    void aanmelding_for_route(QList<SOphaalpunt> *);
    
public slots:
private:
    QTreeView *legeAanmeldingenTreeView;
    MySortFilterProxyModel *legeAanmeldingenModel;
    QStandardItemModel *model;
    QLabel *legeAanmeldingenLabel, *totalWeightLabel, *totalVolumeLabel;
    QLineEdit *totalWeightEdit, *totalVolumeEdit;
    QDialogButtonBox *buttonBox;
    QPushButton *resetButton, *allButton, *deleteButton;
    QPalette *warning, *normal;
    double total_weight, total_volume;
    double maximum_weight, maximum_volume;
    bool sortingascending;

    void populateLegeAanmeldingen();

    void addToTreeModel(QString NaamOphaalpunt, double WeightKurk, double WeightKaars, double ZakKurk, double ZakKaars, int AanmeldingId, int OphaalpuntId, QString Opmerkingen, QString Straat, QString HuisNr, QString BusNr, QString Postcode, QString Plaats, QString Land, QDate Aanmeldingsdatum);
    double getWeightOfRow(const int row);
    double getVolumeOfRow(const int row);
    void initModel();

private slots:
    void checkAll();
    void uncheckAll();
    void deleteSelected();
    void accept();
    void reject();
    void setTotalWeightTotalVolume();
};

#endif // KIESOPHAALPUNTEN_H

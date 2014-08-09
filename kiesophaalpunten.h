#ifndef KIESOPHAALPUNTEN_H
#define KIESOPHAALPUNTEN_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSettings>
#include "sophaalpunt.h"

extern QSettings settings;

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
    //use QTableWidget to work with different columns: http://qt-project.org/wiki/How_to_Use_QTableWidget
    QTreeWidget *legeAanmeldingenTree;
    QLabel *legeAanmeldingenLabel, *totalWeightLabel, *totalVolumeLabel;
    QLineEdit *totalWeightEdit, *totalVolumeEdit;
    QDialogButtonBox *buttonBox;
    QPushButton *resetButton, *allButton;
    QPalette *warning, *normal;
    double total_weight, total_volume;
    double maximum_weight, maximum_volume;
    bool sortingascending;

    void populateLegeAanmeldingen();

    void addToTreeWidget(QString NaamOphaalpunt, double WeightKurk, double WeightKaars, double ZakKurk, double ZakKaars, int AanmeldingId, int OphaalpuntId, QString Opmerkingen, QString Straat, QString HuisNr, QString BusNr, QString Postcode, QString Plaats, QString Land);
    double weightColumnToDouble(QString kg);
    double getWeightOfItem(QTreeWidgetItem* item);

private slots:
    void checkAll();
    void uncheckAll();
    void accept();
    void reject();
    void sortTreeWidget(int column);
    void setTotalWeightTotalVolume();
};

#endif // KIESOPHAALPUNTEN_H

#ifndef KIESOPHAALPUNTEN_H
#define KIESOPHAALPUNTEN_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
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
    QListWidget *legeAanmeldingenList;
    QLabel *legeAanmeldingenLabel, *totalWeightLabel, *totalVolumeLabel;
    QLineEdit *totalWeightEdit, *totalVolumeEdit;
    QDialogButtonBox *buttonBox;
    QPushButton *resetButton, *allButton;
    QPalette *warning, *normal;
    double total_weight, total_volume;
    double maximum_weight, maximum_volume;

    void populateLegeAanmeldingen();
    void setTotalWeightTotalVolume();

private slots:
    void itemSelected(QListWidgetItem* aanmelding);
    void itemSelected();
    void checkAll();
    void uncheckAll();
    void accept();
    void reject();

};

#endif // KIESOPHAALPUNTEN_H

#ifndef OPHAALHISTORIEKDIALOG_H
#define OPHAALHISTORIEKDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
//#include "ophaalhistoriekproxymodel.h"

#define HIST_HISTORIEK_ID 0
#define HIST_OPHALINGSDATUM 1
#define HIST_CHAUFFEUR 2
#define HIST_OPHAALPUNT_ID 3
#define HIST_WEIGHT_KURK 4
#define HIST_WEIGHT_KAARS 5
#define HIST_ZAK_KURK 6
#define HIST_ZAK_KAARS 7
#define HIST_OPMERKINGEN 8

namespace Ui {
class OphaalHistoriekDialog;
}

class OphaalHistoriekDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit OphaalHistoriekDialog(int ophaalpunt_id, QWidget *parent = 0);
    ~OphaalHistoriekDialog();
    
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::OphaalHistoriekDialog *ui;
    QStandardItemModel *model;
    void addToTreeModel(int _historiek_id, QDate _ophalingsdatum, QString _chauffeur, int _ophaalpunt_id, double _kg_kurk, double _kg_kaarsresten, double _zakken_kurk, double _zakken_kaarsresten, QString _opmerkingen);
};

#endif // OPHAALHISTORIEKDIALOG_H

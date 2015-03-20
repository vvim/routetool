#ifndef OPHAALHISTORIEKDIALOG_H
#define OPHAALHISTORIEKDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "ophaalhistoriekdialogsortfilterproxymodel.h"

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
    OphaalHistoriekDialogSortFilterProxyModel *ophalingHistoriekModel;
    void addToTreeModel(int _historiek_id, QDate _ophalingsdatum, QString _chauffeur, int _ophaalpunt_id, double _kg_kurk, double _kg_kaarsresten, double _zakken_kurk, double _zakken_kaarsresten, QString _opmerkingen);
};

#endif // OPHAALHISTORIEKDIALOG_H

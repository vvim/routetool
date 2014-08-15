#ifndef OPHAALHISTORIEKDIALOG_H
#define OPHAALHISTORIEKDIALOG_H

#include <QDialog>
#include "sophalinghistoriek.h"
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
};

#endif // OPHAALHISTORIEKDIALOG_H

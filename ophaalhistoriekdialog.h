#ifndef OPHAALHISTORIEKDIALOG_H
#define OPHAALHISTORIEKDIALOG_H

#include <QDialog>

namespace Ui {
class OphaalHistoriekDialog;
}

class OphaalHistoriekDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit OphaalHistoriekDialog(QWidget *parent = 0);
    ~OphaalHistoriekDialog();
    
private:
    Ui::OphaalHistoriekDialog *ui;
};

#endif // OPHAALHISTORIEKDIALOG_H

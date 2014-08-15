#include "ophaalhistoriekdialog.h"
#include "ui_ophaalhistoriekdialog.h"

OphaalHistoriekDialog::OphaalHistoriekDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OphaalHistoriekDialog)
{
    ui->setupUi(this);
}

OphaalHistoriekDialog::~OphaalHistoriekDialog()
{
    delete ui;
}

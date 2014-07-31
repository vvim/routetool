#ifndef LEVERING_H
#define LEVERING_H

#include <QWidget>
#include <QApplication>
#include <QDialogButtonBox>

#include <QDataWidgetMapper>
#include <QStandardItemModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTextEdit>
#include <QDateEdit>
#include <QGridLayout>
#include <QDialogButtonBox>

#include <QSettings>

#include "mylineedit.h"
#include "infoophaalpunt.h"

extern QSettings settings;

class Levering : public QWidget
{
    Q_OBJECT

public:
    explicit Levering(QWidget *parent = 0);
    ~Levering();
    void resetValues();

private:
    QLabel *locationLabel;
    MyLineEdit *locationEdit;
    QPushButton *toonOphaalpunt;
    QPushButton *vulOphaalpuntIn;
    QDialogButtonBox *buttonBox;
    QLineEdit *nameEdit;
    QLineEdit *streetEdit;
    QLineEdit *houseNrEdit;
    QLineEdit *busNrEdit;
    QLineEdit *postalCodeEdit;
    QLineEdit *plaatsEdit;
    QLineEdit *countryEdit;
    QLineEdit *contactPersonEdit;
    QLineEdit *telephoneEdit;
    QSpinBox *weightSpinBox;
    QSpinBox *volumeSpinBox;
    QSpinBox *timeNeededSpinBox;

    QMap<QString, int> ophaalpunten;

    MyCompleter *completer;
    InfoOphaalpunt *info;

private slots:
    void accept();
    void reject();
    void ophaalpuntChanged();
    void loadOphaalpunten();
    void toonOphaalpuntInformatie();
    void vulInformatieOphaalpuntIn();
};

#endif // LEVERING_H

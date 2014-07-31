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
    QPushButton *toonOphaalpunt, *nieuwOphaalpunt;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *dateLabel;
    QDateEdit *dateEdit;

    QLabel *zakkenkurkLabel;
    QSpinBox *zakkenkurkSpinBox;
    QLabel *kgkurkLabel;
    QSpinBox *kgkurkSpinBox;

    QLabel *zakkenkaarsenLabel;
    QSpinBox *zakkenkaarsenSpinBox;
    QLabel *kgkaarsenLabel;
    QSpinBox *kgkaarsenSpinBox;

    QLabel *opmerkingenLabel;
    QTextEdit *opmerkingenEdit;

    QDialogButtonBox *buttonBox;

    QStandardItemModel *model;
    QDataWidgetMapper *mapper;

    MyCompleter *completer;

    QMap<QString, int> ophaalpunten;

    InfoOphaalpunt *info;

private slots:
    void accept();
    void reject();
    void ophaalpuntChanged();
    void toonOphaalpuntInformatie();
    void loadOphaalpunten();
    void createNewOphaalpunt();
};

#endif // LEVERING_H

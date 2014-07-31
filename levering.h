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
    QDialogButtonBox *buttonBox;

    QMap<QString, int> ophaalpunten;

    MyCompleter *completer;
    InfoOphaalpunt *info;

private slots:
    void accept();
    void reject();
    void ophaalpuntChanged();
    void loadOphaalpunten();
    void toonOphaalpuntInformatie();
};

#endif // LEVERING_H

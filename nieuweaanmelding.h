#ifndef NIEUWEAANMELDING_H
#define NIEUWEAANMELDING_H

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

class NieuweAanmelding : public QWidget
{
    Q_OBJECT
    
public:
    explicit NieuweAanmelding(QWidget *parent = 0);
    ~NieuweAanmelding();
    void resetValues();

public slots:
    void aanmeldingVoorOphaalpunt(int ophaalpunt_id);

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

#endif // NIEUWEAANMELDING_H

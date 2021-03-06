#ifndef OPGEHAALDEHOEVEELHEID_H
#define OPGEHAALDEHOEVEELHEID_H

#include <QWidget>
#include <QDate>
#include <QSettings>

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
class QLabel;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStandardItemModel;
class QPlainTextEdit;
class QDateEdit;
class QDialogButtonBox;
class QSpacerItem;
QT_END_NAMESPACE

extern QSettings settings;

//! [OpgehaaldeHoeveelheid definition]
class OpgehaaldeHoeveelheid : public QWidget
{
    Q_OBJECT

public:
    OpgehaaldeHoeveelheid(QDate ophaalronde_datum, QWidget *parent = 0);
    ~OpgehaaldeHoeveelheid();

private slots:
    void updateButtons(int row);
    void reject();
    void accept();
    void zakkenKurkTest();
    void zakkenKaarsrestenTest();
    void checkBoxToggled(bool);

private:
    void setupModel(QDate ophaalronde_datum);

    QLabel *ophalingDateLabel;
    QLabel *chauffeurLabel;
    QLabel *ophaalpuntLabel;
    QLabel *opmerkingenLabel;
    QLabel *kgKurkLabel;
    QLabel *zakkenKurkLabel;
    QLabel *kgKaarsrestenLabel;
    QLabel *zakkenKaarsrestenLabel;
    QDateEdit *ophalingDateEdit;
    QLineEdit *chauffeurEdit;
    QLineEdit *ophaalpuntEdit;
    QPlainTextEdit *opmerkingenEdit;
    QSpinBox *kgKurkSpinBox;
    QSpinBox *zakkenKurkSpinBox;
    QSpinBox *kgKaarsrestenSpinBox;
    QSpinBox *zakkenKaarsrestenSpinBox;
    QPushButton *nextButton;
    QPushButton *previousButton;
    QCheckBox *locationShouldBeSkippedCheckBox;

    QDialogButtonBox *buttonBox;
    QSpacerItem *spaceritem_beforebuttonbox;
    QSpacerItem *spaceritem_beforelocationshouldbeskipped;

    QStandardItemModel *model;
    QDataWidgetMapper *mapper;
};
//! [OpgehaaldeHoeveelheid definition]

#endif // OPGEHAALDEHOEVEELHEID_H

#ifndef INFOOPHAALPUNT_H
#define INFOOPHAALPUNT_H

#include <QWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QSqlRelationalTableModel>

extern QSettings settings;

class InfoOphaalpunt : public QWidget
{
    Q_OBJECT
    
public:
    explicit InfoOphaalpunt(QWidget *parent = 0);
    ~InfoOphaalpunt();

    void showOphaalpunt(int ophaalpunt_id);
    void createNewOphaalpunt();

private:
    QSqlRelationalTableModel *codemodel;
    int codeIndex;
    QDialogButtonBox *buttonBox;
    QPushButton *resetButton;

    int	id;
    QLabel *ophaalpuntLabel;
    QLineEdit *ophaalpuntEdit;
    //QLabel *kurkLabel;
    QCheckBox *kurkCheckBox;
    //QLabel *parafineLabel;
    QCheckBox *parafineCheckBox;
    QLabel *codeLabel;
    QComboBox *codeComboBox; // soort ophaalpunt
    QLabel *code_intercommunaleLabel;
    QComboBox *code_intercommunaleComboBox; // indien code = intercommunale => welke intercommunale?
    QLabel *straatLabel;
    QLineEdit *straatEdit;
    QLabel *nrLabel;
    QLineEdit *nrEdit;
    QLabel *busLabel;
    QLineEdit *busEdit;
    QLabel *postcodeLabel;
    QLineEdit *postcodeEdit;
    QLabel *plaatsLabel;
    QLineEdit *plaatsEdit;
    QLabel *landLabel;
    QLineEdit *landEdit;
    QLabel *openingsurenLabel;
    QTextEdit *openingsurenEdit;

    QLabel *contactpersoonLabel;
    QLineEdit *contactpersoonEdit;
    //maak hier een lijst  van met toe te voegen en te verwijderen telefoonnrs
    QLabel *telefoonnummer1Label;
    QLineEdit *telefoonnummer1Edit;
    QLabel *telefoonnummer2Label;
    QLineEdit *telefoonnummer2Edit;

    //maak hier een lijst  van met toe te voegen en te verwijderen emails
    QLabel *email1Label;
    QLineEdit *email1Edit;
    QLabel *email2Label;
    QLineEdit *email2Edit;

    QLabel *taalvoorkeurLabel;
    QComboBox *taalvoorkeurComboBox;
    QLabel *preferred_contactLabel;
    QComboBox *preferred_contactComboBox;
    //QLabel *attest_nodigLabel;
    QCheckBox *attest_nodigCheckBox;
    QLabel *frequentie_attestLabel;
    QComboBox *frequentie_attestComboBox;
    QLabel *extra_informatieLabel;
    QTextEdit *extra_informatieEdit;

private slots:
    void accept();
    void reject();
    void reset();
    void toggleIntercommunale(int);
    void toggleFrequentie(int);

signals:
    void infoChanged();
};

#endif // INFOOPHAALPUNT_H

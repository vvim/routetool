#ifndef LISTOFOPHAALPUNTENTOCONTACT_H
#define LISTOFOPHAALPUNTENTOCONTACT_H

#include "infoophaalpunt.h"
#include <QListWidget>

extern QSettings settings;

class ListOfOphaalpuntenToContact : public QWidget
{
    Q_OBJECT

public:
    explicit ListOfOphaalpuntenToContact(QWidget *parent = 0);
    ~ListOfOphaalpuntenToContact();

    void initialise();
    void show_never_contacted_ophaalpunten();

private slots:
    void showOphaalpunt(QListWidgetItem*);
    void ok_button_pushed();

private:
    InfoOphaalpunt *info;
    QListWidget *contactList;
    QDialogButtonBox *buttonBox;
    QLabel *label;

    void UpdateOphaalpunt(int ophaalpuntid);
    void UpdateAllOphaalpunten();
};

#endif // LISTOFOPHAALPUNTENTOCONTACT_H

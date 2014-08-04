#ifndef LISTOFOPHAALPUNTENTOCONTACT_H
#define LISTOFOPHAALPUNTENTOCONTACT_H

#include "infoophaalpunt.h"

extern QSettings settings;

class ListOfOphaalpuntenToContact
{
public:
    ListOfOphaalpuntenToContact();
    ~ListOfOphaalpuntenToContact();

private:
    InfoOphaalpunt *info;

    void UpdateOphaalpunt(int ophaalpuntid);
public:
    void UptodateAllOphaalpunten();
};

#endif // LISTOFOPHAALPUNTENTOCONTACT_H

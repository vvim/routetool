#include "infoophaalpunt.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <math.h>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include "nieuweaanmelding.h"

#define CODE_INTERCOMMUNALE 1

InfoOphaalpunt::InfoOphaalpunt(QWidget *parent) :
    QWidget(parent)
{
    ophaalHistoriekDialog = NULL;
    ophaalpuntLabel = new QLabel(tr("Ophaalpunt:"));
    ophaalpuntEdit = new QLineEdit();
    kurkCheckBox = new QCheckBox(tr("Ophaalpunt voor kurk"));
    parafineCheckBox = new QCheckBox(tr("Ophaalpunt voor kaarsresten"));
    codeLabel = new QLabel(tr("Soort ophaalpunt:"));

    codeComboBox = new QComboBox();

    // insert items to ComboBox
    codeComboBox->insertItem(0,"");
    QSqlQuery query;
    query.prepare("SELECT * FROM soort_ophaalpunt ORDER by code");
    if(!query.exec())
        qCritical(QString(tr("SELECT for soort_ophaalpunt FAILED!").append(query.lastError().text())).toStdString().c_str());

    while(query.next())
    {
        // query.value(0) == code; query.value(1) == soort
        codeComboBox->insertItem(query.value(0).toInt(),query.value(1).toString());
    }

    code_intercommunaleLabel = new QLabel(tr(""));
    code_intercommunaleComboBox = new QComboBox();
    code_intercommunaleComboBox->setEnabled(false);

    // insert items to ComboBox
    code_intercommunaleComboBox->insertItem(0,"");

    query.prepare("SELECT * FROM intercommunales ORDER by id");
    if(!query.exec())
        qDebug() << "SELECT for intercommunales FAILED!" << query.lastError();

    while(query.next())
    {
        // query.value(0) == id; query.value(1) == naam_intercommunale
        code_intercommunaleComboBox->insertItem(query.value(0).toInt(),query.value(1).toString());
    }


    straatLabel = new QLabel(tr("Straat:"));
    straatEdit = new QLineEdit();
    nrLabel = new QLabel(tr("Nr:"));
    nrEdit = new QLineEdit();
    busLabel = new QLabel(tr("Bus:"));
    busEdit = new QLineEdit();

    postcodeLabel = new QLabel(tr("Postcode:"));
    postcodeEdit = new QLineEdit();
    plaatsLabel = new QLabel(tr("Plaats:"));
    plaatsEdit = new QLineEdit();
    landLabel = new QLabel(tr("Land:"));
    landEdit = new QLineEdit();
    openingsurenLabel = new QLabel(tr("Openingsuren:"));
    openingsurenEdit = new QTextEdit();
    contactpersoonLabel = new QLabel(tr("Contactpersoon:"));
    contactpersoonEdit = new QLineEdit();
    telefoonnummer1Label = new QLabel(tr("Telefoon 1:"));
    telefoonnummer1Edit = new QLineEdit();
    telefoonnummer2Label = new QLabel(tr("Telefoon 2:"));
    telefoonnummer2Edit = new QLineEdit();
    email1Label = new QLabel(tr("Email 1:"));
    email1Edit = new QLineEdit();
    email2Label = new QLabel(tr("Email 2:"));
    email2Edit = new QLineEdit();
    taalvoorkeurLabel = new QLabel(tr("Taalvoorkeur:"));

    taalvoorkeurComboBox = new QComboBox();

    // insert items to ComboBox
    taalvoorkeurComboBox->insertItem(0,"");

    query.prepare("SELECT * FROM talen ORDER by id");
    if(!query.exec())
        qCritical(QString(tr("SELECT for talen FAILED!").append(query.lastError().text())).toStdString().c_str());


    while(query.next())
    {
        // query.value(0) == id; query.value(1) == taal
        taalvoorkeurComboBox->insertItem(query.value(0).toInt(),query.value(1).toString());
    }


    preferred_contactLabel = new QLabel(tr("Contactvoorkeur:"));

    preferred_contactComboBox = new QComboBox();

    // insert items to ComboBox
    preferred_contactComboBox->insertItem(0,"");

    query.prepare("SELECT * FROM contacteren ORDER by id");
    if(!query.exec())
        qCritical(QString(tr("SELECT for contacteren FAILED!").append(query.lastError().text())).toStdString().c_str());

    while(query.next())
    {
        // query.value(0) == id; query.value(1) == medium
        preferred_contactComboBox->insertItem(query.value(0).toInt(),query.value(1).toString());
    }


    attest_nodigCheckBox = new QCheckBox(tr("Attest nodig"));
    frequentie_attestLabel = new QLabel(tr("frequentie:"));

    frequentie_attestComboBox = new QComboBox();

    // insert items to ComboBox
    frequentie_attestComboBox->insertItem(0,"");

    query.prepare("SELECT * FROM frequentie ORDER by id");
    if(!query.exec())
        qCritical(QString(tr("SELECT for frequentie FAILED!").append(query.lastError().text())).toStdString().c_str());


    while(query.next())
    {
        // query.value(0) == id; query.value(1) == frequentie
        frequentie_attestComboBox->insertItem(query.value(0).toInt(),query.value(1).toString());
    }


    extra_informatieLabel = new QLabel(tr("Extra informatie:"));
    extra_informatieEdit = new QTextEdit();

    lastContactDateEdit = new QDateEdit();
    lastContactDateEdit->setDisplayFormat("dd MMM yyyy");
    lastContactDateEdit->setLocale(QLocale::Dutch);
    lastContactDateEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup


    contactAgainOnEdit = new QDateEdit();
    contactAgainOnEdit->setDisplayFormat("dd MMM yyyy");
    contactAgainOnEdit->setLocale(QLocale::Dutch);
    contactAgainOnEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup

    contactAgainOnEdit->setEnabled(false);

    lastOphalingEdit = new QDateEdit();
    lastOphalingEdit->setDisplayFormat("dd MMM yyyy");
    lastOphalingEdit->setLocale(QLocale::Dutch);
    lastOphalingEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup

    lastOphalingEdit->setEnabled(false);

    forecastNewOphalingEdit = new QDateEdit();
    forecastNewOphalingEdit->setDisplayFormat("dd MMM yyyy");
    forecastNewOphalingEdit->setLocale(QLocale::Dutch);
    forecastNewOphalingEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup

    forecastNewOphalingEdit->setEnabled(false);

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(tr("Wijzigingen opslaan"),
                 QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Niet opslaan"),
                         QDialogButtonBox::RejectRole);
    resetButton = new QPushButton(tr("Reset"));
    buttonBox->addButton(resetButton,QDialogButtonBox::ResetRole);

    aanmeldingButton = new QPushButton(tr("Nieuwe Aanmelding"));
    buttonBox->addButton(aanmeldingButton,QDialogButtonBox::ActionRole);
    aanmeldingButton->setHidden(true);
    // ?? aanmeldingButton->setVisible(false);

    showHistoriekButton = new QPushButton(tr("Ophalingshistoriek"));
    buttonBox->addButton(showHistoriekButton,QDialogButtonBox::ActionRole);
    //hide ???

    connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(reset()));
    connect(codeComboBox,SIGNAL(currentIndexChanged(int)), this, SLOT(toggleIntercommunale(int)));
    connect(attest_nodigCheckBox,SIGNAL(stateChanged(int)), this, SLOT(toggleFrequentie(int)));
    connect(aanmeldingButton, SIGNAL(pressed()), this, SLOT(nieuweAanmeldingButtonPressed()));
    connect(showHistoriekButton, SIGNAL(clicked()), this, SLOT(showHistoriekButtonPressed()));

    everContactedBeforeCheckBox = new QCheckBox(tr("ja, laatste contact bekend"));
    everContactedBeforeCheckBox->setChecked(false);
    everContactedBeforeCheckBoxToggled(everContactedBeforeCheckBox->checkState() == Qt::Checked);
    connect(everContactedBeforeCheckBox, SIGNAL(toggled(bool)), this, SLOT(everContactedBeforeCheckBoxToggled(bool)));


    //connect(__elke_Edit,SIGNAL(TextChanged), this, SLOT (setFlagInfoChanged());

    QHBoxLayout *adresLayout_rule1 = new QHBoxLayout();
    adresLayout_rule1->addWidget(straatEdit);
    adresLayout_rule1->addWidget(nrLabel);
    adresLayout_rule1->addWidget(nrEdit);
    adresLayout_rule1->addWidget(busLabel);
    adresLayout_rule1->addWidget(busEdit);

    QHBoxLayout *adresLayout_rule2 = new QHBoxLayout();
    adresLayout_rule2->addWidget(postcodeEdit);
    adresLayout_rule2->addWidget(plaatsLabel);
    adresLayout_rule2->addWidget(plaatsEdit);

    // Grid Layout? FormLayout?
    // http://doc.qt.digia.com/qq/qq25-formlayout.html
    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Ophaalpunt:"), ophaalpuntEdit);
    layout->addRow(straatLabel,adresLayout_rule1);
    layout->addRow(postcodeLabel,adresLayout_rule2);
/*
    layout->addRow(tr("Straat:"), straatEdit);
    layout->addRow(tr("Nummer:"), nrEdit);
    layout->addRow(tr("Bus:"), busEdit);
    layout->addRow(tr("Postcode:"), postcodeEdit);
    layout->addRow(tr("Plaats:"), plaatsEdit);
*/
    layout->addRow(tr("Land:"), landEdit);
    // horizontal line
    layout->addRow(tr("Materiaal:"), kurkCheckBox);
    layout->addRow(tr(""), parafineCheckBox);
    layout->addRow(tr("Soort ophaalpunt:"), codeComboBox);
    layout->addRow(tr("Indien intercommunale, welke?"), code_intercommunaleComboBox);
    layout->addRow(tr("Attesten:"), attest_nodigCheckBox);
    layout->addRow(tr("Indien attest nodig, welke frequentie?"), frequentie_attestComboBox);
    // horizontal line
    layout->addRow(tr("Openingsuren:"), openingsurenEdit);
    layout->addRow(tr("Contactpersoon:"), contactpersoonEdit);
    layout->addRow(tr("Taalvoorkeur:"), taalvoorkeurComboBox);
    layout->addRow(tr("Contactvoorkeur:"), preferred_contactComboBox);
    // maak hiervan een lijst waarin naar wens telefoonnummers aan kunnen worden toegevoegd -> nieuwe Widget
    layout->addRow(tr("Telefoon:"), telefoonnummer1Edit);
    layout->addRow(tr(""), telefoonnummer2Edit);
    // maak hiervan een lijst waarin naar wens emails aan kunnen worden toegevoegd -> nieuwe Widget
    layout->addRow(tr("Email:"), email1Edit);
    layout->addRow(tr(""), email2Edit);
    // horizontal line
    layout->addRow(tr("Laatste ophaling:"), lastOphalingEdit);
    layout->addRow(tr("Voorspelling nieuwe ophaling:"), forecastNewOphalingEdit);
    // horizontal line
    layout->addRow(tr("Extra informatie:"), extra_informatieEdit);
    layout->addRow(tr("Al ooit gecontacteerd?"), everContactedBeforeCheckBox);
    layout->addRow(tr("Laatste contact:"), lastContactDateEdit);

    // this seems obsolete to me, nothing filled in database, maybe the user wants this functionality in the future?
    //layout->addRow(tr("Contacteer op:"), contactAgainOnEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setMinimumWidth(600);
    setWindowTitle(tr("Nieuw ophaalpunt"));
}

InfoOphaalpunt::~InfoOphaalpunt()
{
    qDebug() << "start to deconstruct InfoOphaalpunt()";
    delete  resetButton;
    delete  aanmeldingButton;
    delete  showHistoriekButton;
    delete  buttonBox;
    delete  ophaalpuntLabel;
    delete  ophaalpuntEdit;
    delete  kurkCheckBox;
    delete  parafineCheckBox;
    delete  codeLabel;
    delete  codeComboBox;
    delete  code_intercommunaleLabel;
    delete  code_intercommunaleComboBox;
    delete  straatLabel;
    delete  straatEdit;
    delete  nrLabel;
    delete  nrEdit;
    delete  busLabel;
    delete  busEdit;
    delete  postcodeLabel;
    delete  postcodeEdit;
    delete  plaatsLabel;
    delete  plaatsEdit;
    delete  landLabel;
    delete  landEdit;
    delete  openingsurenLabel;
    delete  openingsurenEdit;
    delete  contactpersoonLabel;
    delete  contactpersoonEdit;
    delete  telefoonnummer1Label;
    delete  telefoonnummer1Edit;
    delete  telefoonnummer2Label;
    delete  telefoonnummer2Edit;
    delete  email1Label;
    delete  email1Edit;
    delete  email2Label;
    delete  email2Edit;
    delete  taalvoorkeurLabel;
    delete  taalvoorkeurComboBox;
    delete  preferred_contactLabel;
    delete  preferred_contactComboBox;
    delete  attest_nodigCheckBox;
    delete  frequentie_attestLabel;
    delete  frequentie_attestComboBox;
    delete  extra_informatieLabel;
    delete  extra_informatieEdit;
    delete  lastContactDateEdit;
    delete  contactAgainOnEdit;
    delete  everContactedBeforeCheckBox;
    delete  lastOphalingEdit;
    delete  forecastNewOphalingEdit;
    if(ophaalHistoriekDialog)
        delete ophaalHistoriekDialog;
    qDebug() << "InfoOphaalpunt() deconstructed";
}

void InfoOphaalpunt::accept()
{
    if(id > 0)
    {
        //changing existing ophaalpunt
        /** update db **/
        QSqlQuery query;

        //ESCAPING QUERY: http://stackoverflow.com/questions/19045281/insert-strings-that-contain-or-to-the-database-table-qt and http://qt-project.org/doc/qt-5/qsqlquery.html#prepare
        query.prepare("UPDATE ophaalpunten SET naam = :naam, kurk = :kurk, parafine = :parafine, code = :code, code_intercommunale = :code_intercommunale, straat = :straat, nr = :nr, bus = :bus, postcode = :postcode, plaats = :plaats, land = :land, openingsuren = :openingsuren, contactpersoon = :contactpersoon, telefoonnummer1 = :telefoonnummer1, telefoonnummer2 = :telefoonnummer2, email1 = :email1, email2 = :email2, taalvoorkeur = :taalvoorkeur, preferred_contact = :preferred_contact, attest_nodig = :attest_nodig, frequentie_attest = :frequentie_attest, extra_informatie = :extra_informatie, last_contact_date = :last_contact_date WHERE id = :id");
        query.bindValue(":naam",ophaalpuntEdit->text());
        query.bindValue(":kurk",kurkCheckBox->isChecked());
        query.bindValue(":parafine",parafineCheckBox->isChecked());
        query.bindValue(":code",codeComboBox->currentIndex());
        query.bindValue(":code_intercommunale",code_intercommunaleComboBox->currentIndex());
        query.bindValue(":straat",straatEdit->text());
        query.bindValue(":nr",nrEdit->text());
        query.bindValue(":bus",busEdit->text());
        query.bindValue(":postcode",postcodeEdit->text());
        query.bindValue(":plaats",plaatsEdit->text());
        query.bindValue(":land",landEdit->text());
        query.bindValue(":openingsuren",openingsurenEdit->toPlainText());
        query.bindValue(":contactpersoon",contactpersoonEdit->text());
        query.bindValue(":telefoonnummer1",telefoonnummer1Edit->text());
        query.bindValue(":telefoonnummer2",telefoonnummer2Edit->text());
        query.bindValue(":email1",email1Edit->text());
        query.bindValue(":email2",email2Edit->text());
        query.bindValue(":taalvoorkeur",taalvoorkeurComboBox->currentIndex());
        query.bindValue(":preferred_contact",preferred_contactComboBox->currentIndex());
        query.bindValue(":attest_nodig",attest_nodigCheckBox->isChecked());
        query.bindValue(":frequentie_attest",frequentie_attestComboBox->currentIndex());
        query.bindValue(":extra_informatie",extra_informatieEdit->toPlainText());
        query.bindValue(":id",id);

        if(everContactedBeforeCheckBox->checkState() == Qt::Checked)
        {
            query.bindValue(":last_contact_date",lastContactDateEdit->date());
        }
        else
        {
            // results in "0000-00-00" instead of a real NULL. What can I do?
            qDebug() << "[InfoOphaalpunt::accept()]" << "results in '0000-00-00'' instead of a real NULL. What can I do?";
            query.bindValue(":last_contact_date","NULL");
        }

        /*  // update  "contact_again_on", "lastOphalingEdit", "forecastNewOphalingEdit" ?
                for now: no:    A] "contact_again_on" is probably obsolete
                                B] "lastOphalingEdit" is set by ListOfOphaalpuntenToContact
                                C] "forecastNewOphalingEdit" is calculated by ListOfOphaalpuntenToContact

            + working on code of class MetaDateEdit()
        */

        if(!query.exec())
        {
            QMessageBox::critical(this, tr("UPDATE informatie voor ophaalpunt %1 FAALT!").arg(ophaalpuntEdit->text()),
                        query.lastError().text().append(tr("\n\nHerstel de fout en probeer opnieuw.")), QMessageBox::Cancel);
            qCritical(QString(tr("UPDATE informatie voor ophaalpunt %1 FAALT!").arg(ophaalpuntEdit->text()).append(query.lastError().text())).toStdString().c_str());
        }
        else
        {
            this->close();
            emit infoChanged();
        }

    }
    else
    {
        //adding new ophaalpunt
        /** insert into db **/
        QSqlQuery query;

        query.prepare("INSERT INTO ophaalpunten (id,timestamp,naam, kurk, parafine, code, code_intercommunale, straat, nr, bus, postcode, plaats, land, openingsuren, contactpersoon, telefoonnummer1, telefoonnummer2, email1, email2, taalvoorkeur, preferred_contact, attest_nodig, frequentie_attest, extra_informatie, last_contact_date, contact_again_on)"
                      "                  VALUES (NULL, NULL, :naam, :kurk, :parafine, :code, :code_intercommunale, :straat, :nr, :bus, :postcode, :plaats, :land, :openingsuren, :contactpersoon, :telefoonnummer1, :telefoonnummer2, :email1, :email2, :taalvoorkeur, :preferred_contact, :attest_nodig, :frequentie_attest, :extra_informatie, :last_contact_date, NULL)");

        query.bindValue(":naam",ophaalpuntEdit->text());
        query.bindValue(":kurk",kurkCheckBox->isChecked());
        query.bindValue(":parafine",parafineCheckBox->isChecked());
        query.bindValue(":code",codeComboBox->currentIndex());
        query.bindValue(":code_intercommunale",code_intercommunaleComboBox->currentIndex());
        query.bindValue(":straat",straatEdit->text());
        query.bindValue(":nr",nrEdit->text());
        query.bindValue(":bus",busEdit->text());
        query.bindValue(":postcode",postcodeEdit->text());
        query.bindValue(":plaats",plaatsEdit->text());
        query.bindValue(":land",landEdit->text());
        query.bindValue(":openingsuren",openingsurenEdit->toPlainText());
        query.bindValue(":contactpersoon",contactpersoonEdit->text());
        query.bindValue(":telefoonnummer1",telefoonnummer1Edit->text());
        query.bindValue(":telefoonnummer2",telefoonnummer2Edit->text());
        query.bindValue(":email1",email1Edit->text());
        query.bindValue(":email2",email2Edit->text());
        query.bindValue(":taalvoorkeur",taalvoorkeurComboBox->currentIndex());
        query.bindValue(":preferred_contact",preferred_contactComboBox->currentIndex());
        query.bindValue(":attest_nodig",attest_nodigCheckBox->isChecked());
        query.bindValue(":frequentie_attest",frequentie_attestComboBox->currentIndex());
        query.bindValue(":extra_informatie",extra_informatieEdit->toPlainText());

        if(everContactedBeforeCheckBox->checkState() == Qt::Checked)
        {
            query.bindValue(":last_contact_date",lastContactDateEdit->date());
        }
        else
            query.bindValue(":last_contact_date","NULL");


        if(!query.exec())
        {
            QMessageBox::critical(this, tr("AANMAKEN nieuw ophaalpunt %1 FAALT!").arg(ophaalpuntEdit->text()),
                        query.lastError().text().append(tr("\n\nHerstel de fout en probeer opnieuw.")), QMessageBox::Cancel);
            qCritical(QString(tr("AANMAKEN ophaalpunt %1 FAALT!").arg(ophaalpuntEdit->text()).append(query.lastError().text())).toStdString().c_str());
        }
        else
        {
            this->close();
            emit infoChanged();
        }
    }
}

void InfoOphaalpunt::reject()
{
    this->close();
}

void InfoOphaalpunt::showAanmeldingButton(bool show_button)
{
    qDebug() << "[InfoOphaalpunt::showAanmeldingButton(bool show_button)]" << "show_button" << show_button << "id" << id;
    // show_button is passed to InfoOphaalpunt before id is set, so id will probably == 0 ...
//    if (id > 0)
        aanmeldingButton->setHidden(!show_button);
        //   ???     aanmeldingButton->setVisible(true);
//    else
//        aanmeldingButton->setHidden(true);
}

void InfoOphaalpunt::showOphaalpunt(int ophaalpunt_id)
{
    id = ophaalpunt_id;
    this->reset();
    this->show();
    // TODO    connect(NIEUWEWIDGETtoonInfoOphaalpunten, SIGNAL(close()), this, SLOT(loadOphaalpunten()));
}

void InfoOphaalpunt::createNewOphaalpunt()
{
    showOphaalpunt(0);
}

void InfoOphaalpunt::reset()
{
    if (id > 0)
    {
        // widget shows EXISTING ophaalpunt, reset info to information found in DB
        qDebug() << "haal uit databank informatie id" << id;

        QSqlQuery query;
        query.prepare("SELECT * FROM ophaalpunten WHERE id = :id");
        query.bindValue(":id", id);
        if(!query.exec())
            qDebug() << "SELECT FAILED!" << query.lastError();

        if (query.next())
        {
            // query.value(0) == id; query.value(1) == timestamp
            ophaalpuntEdit->setText(query.value(2).toString());
            kurkCheckBox->setChecked(query.value(3).toBool());
            parafineCheckBox->setChecked(query.value(4).toBool());
            codeComboBox->setCurrentIndex(query.value(5).toInt());
            code_intercommunaleComboBox->setCurrentIndex(query.value(6).toInt());
                // enable/disable wordt al automatisch opgevangen door signal/slot
            straatEdit->setText(query.value(7).toString());
            nrEdit->setText(query.value(8).toString());
            busEdit->setText(query.value(9).toString());
            postcodeEdit->setText(query.value(10).toString());
            plaatsEdit->setText(query.value(11).toString());
            landEdit->setText(query.value(12).toString());
            openingsurenEdit->setText(query.value(13).toString());
            contactpersoonEdit->setText(query.value(14).toString());
            telefoonnummer1Edit->setText(query.value(15).toString());
            telefoonnummer2Edit->setText(query.value(16).toString());
            email1Edit->setText(query.value(17).toString());
            email2Edit->setText(query.value(18).toString());
            taalvoorkeurComboBox->setCurrentIndex(query.value(19).toInt());
            preferred_contactComboBox->setCurrentIndex(query.value(20).toInt());
            bool attest = query.value(21).toBool();
            attest_nodigCheckBox->setChecked(attest);
            frequentie_attestComboBox->setCurrentIndex(query.value(22).toInt());
                // enable/disable wordt NIET automatisch opgevangen door signal/slot => toggleFrequentie()
                if (attest)
                    toggleFrequentie(Qt::Checked);
                else
                    toggleFrequentie(Qt::Unchecked);
            extra_informatieEdit->setText(query.value(23).toString());
            if(!query.value(24).isNull())
            {
                everContactedBeforeCheckBox->setChecked(true);
                lastContactDateEdit->setDate(query.value(24).toDate());
            }
            else
            {
                everContactedBeforeCheckBox->setChecked(false);
                lastContactDateEdit->setDate(QDate().currentDate());
            }

            if(!query.value(25).isNull())
            {
                contactAgainOnEdit->setDate(query.value(25).toDate());
                contactAgainOnEdit->setVisible(true);
            }
            else
            {
                contactAgainOnEdit->setDate(QDate());
                contactAgainOnEdit->setVisible(false);
            }

            if(!query.value(26).isNull())
            {
                lastOphalingEdit->setDate(query.value(26).toDate());
                lastOphalingEdit->setVisible(true);
            }
            else
            {
                lastOphalingEdit->setDate(QDate());
                lastOphalingEdit->setVisible(false);
            }
            if(!query.value(27).isNull())
            {
                forecastNewOphalingEdit->setDate(query.value(27).toDate());
                forecastNewOphalingEdit->setVisible(true);
            }
            else
            {
                forecastNewOphalingEdit->setDate(QDate());
                forecastNewOphalingEdit->setVisible(false);
            }
        }
    }
    else // clear all
    {
        // widget is called to CREATE a NEW ophaalpunt, empty all information in the form
        ophaalpuntEdit->clear();
        kurkCheckBox->setChecked(false);
        parafineCheckBox->setChecked(false);
        codeComboBox->setCurrentIndex(0);
        code_intercommunaleComboBox->setCurrentIndex(0);
        straatEdit->clear();
        nrEdit->clear();
        busEdit->clear();
        postcodeEdit->clear();
        plaatsEdit->clear();
        landEdit->clear();
        openingsurenEdit->clear();
        contactpersoonEdit->clear();
        telefoonnummer1Edit->clear();
        telefoonnummer2Edit->clear();
        email1Edit->clear();
        email2Edit->clear();
        taalvoorkeurComboBox->setCurrentIndex(0);
        preferred_contactComboBox->setCurrentIndex(0);
        attest_nodigCheckBox->setChecked(false);
        frequentie_attestComboBox->setCurrentIndex(0);
        toggleFrequentie(Qt::Unchecked);
        extra_informatieEdit->clear();

        everContactedBeforeCheckBox->setChecked(false);

        lastContactDateEdit->setDate(QDate().currentDate());

        contactAgainOnEdit->setDate(QDate());
        contactAgainOnEdit->setVisible(false);

        lastOphalingEdit->setDate(QDate());
        lastOphalingEdit->setVisible(false);

        forecastNewOphalingEdit->setDate(QDate());
        forecastNewOphalingEdit->setVisible(false);
    }

    everContactedBeforeCheckBoxToggled(everContactedBeforeCheckBox->checkState() == Qt::Checked);

}

void InfoOphaalpunt::toggleIntercommunale(int soort)
{
    if (soort == CODE_INTERCOMMUNALE)
        code_intercommunaleComboBox->setEnabled(true);
    else
        code_intercommunaleComboBox->setEnabled(false);
}

void InfoOphaalpunt::toggleFrequentie(int attest)
{
    if(attest == Qt::Checked)
        frequentie_attestComboBox->setEnabled(true);
    else
        frequentie_attestComboBox->setEnabled(false);

}

void InfoOphaalpunt::nieuweAanmeldingButtonPressed()
{
    qDebug() << "[InfoOphaalpunt::nieuweAanmelding()]" << "button 'Nieuwe Aanmelding' pressed";
    qDebug() << "[InfoOphaalpunt::nieuweAanmelding()]" << "nieuwe aanmelding, ophaalpunt_id" << id;
    if(id > 0)
        emit nieuweAanmelding(id);
}

void InfoOphaalpunt::showHistoriekButtonPressed()
{
    qDebug() << "[InfoOphaalpunt::showHistoriek()]" << "button 'Show Historiek' pressed";
    qDebug() << "[InfoOphaalpunt::showHistoriek()]" << "show historiek, ophaalpunt_id" << id;

    if(id > 0) // else it has no use of course :-)
    {
        if(ophaalHistoriekDialog)
            delete ophaalHistoriekDialog;
        ophaalHistoriekDialog = new OphaalHistoriekDialog(id); // id;
        ophaalHistoriekDialog->show();
    }
}

void InfoOphaalpunt::everContactedBeforeCheckBoxToggled(bool visible)
{
    lastContactDateEdit->setVisible(visible);
}

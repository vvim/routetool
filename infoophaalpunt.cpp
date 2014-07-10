#include "infoophaalpunt.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <math.h>
#include <QFormLayout>

#define CODE_INTERCOMMUNALE 1

InfoOphaalpunt::InfoOphaalpunt(QWidget *parent) :
    QWidget(parent)
{

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
        qDebug() << "SELECT for soort_ophaalpunt FAILED!" << query.lastError();

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
        qDebug() << "SELECT for talen FAILED!" << query.lastError();

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
        qDebug() << "SELECT for contacteren FAILED!" << query.lastError();

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
        qDebug() << "SELECT for frequentie FAILED!" << query.lastError();

    while(query.next())
    {
        // query.value(0) == id; query.value(1) == frequentie
        frequentie_attestComboBox->insertItem(query.value(0).toInt(),query.value(1).toString());
    }


    extra_informatieLabel = new QLabel(tr("Extra informatie:"));
    extra_informatieEdit = new QTextEdit();


    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(tr("Wijzigingen opslaan"),
                 QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Niet opslaan"),
                         QDialogButtonBox::RejectRole);
    resetButton = new QPushButton(tr("Reset"));
    buttonBox->addButton(resetButton,QDialogButtonBox::ResetRole);

    connect(buttonBox, SIGNAL(accepted()),this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),this, SLOT(reject()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(reset()));
    connect(codeComboBox,SIGNAL(currentIndexChanged(int)), this, SLOT(toggleIntercommunale(int)));
    connect(attest_nodigCheckBox,SIGNAL(stateChanged(int)), this, SLOT(toggleFrequentie(int)));



    //connect(__elke_Edit,SIGNAL(TextChanged), this, SLOT (setFlagInfoChanged());


    // Grid Layout? FormLayout?
    // http://doc.qt.digia.com/qq/qq25-formlayout.html
    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Ophaalpunt:"), ophaalpuntEdit);
    layout->addRow(tr("Straat:"), straatEdit);
    layout->addRow(tr("Nummer:"), nrEdit);
    layout->addRow(tr("Bus:"), busEdit);
    layout->addRow(tr("Postcode:"), postcodeEdit);
    layout->addRow(tr("Plaats:"), plaatsEdit);
    layout->addRow(tr("Land:"), landEdit);
    // horizontal line
    layout->addRow(tr("Ophaalpunt:"), kurkCheckBox);
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
    layout->addRow(tr("Extra informatie"), extra_informatieEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setMinimumWidth(600);
}

InfoOphaalpunt::~InfoOphaalpunt()
{
}

void InfoOphaalpunt::accept()
{

    if(id > 0)
    {
        //changing existing ophaalpunt
        /** update db **/
        QSqlQuery query;

        //ESCAPING QUERY: http://stackoverflow.com/questions/19045281/insert-strings-that-contain-or-to-the-database-table-qt and http://qt-project.org/doc/qt-5/qsqlquery.html#prepare
        query.prepare("UPDATE ophaalpunten SET naam = :naam, kurk = :kurk, parafine = :parafine, code = :code, code_intercommunale = :code_intercommunale, straat = :straat, nr = :nr, bus = :bus, postcode = :postcode, plaats = :plaats, land = :land, openingsuren = :openingsuren, contactpersoon = :contactpersoon, telefoonnummer1 = :telefoonnummer1, telefoonnummer2 = :telefoonnummer2, email1 = :email1, email2 = :email2, taalvoorkeur = :taalvoorkeur, preferred_contact = :preferred_contact, attest_nodig = :attest_nodig, frequentie_attest = :frequentie_attest, extra_informatie = :extra_informatie WHERE id = :id");
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


        if(!query.exec())
            qDebug() << "UPDATE ophaalpunten FAILED!" << query.lastError();
    }
    else
    {
        //adding new ophaalpunt
        /** insert into db **/
        QSqlQuery query;

        query.prepare("INSERT INTO ophaalpunten (id,timestamp,naam, kurk, parafine, code, code_intercommunale, straat, nr, bus, postcode, plaats, land, openingsuren, contactpersoon, telefoonnummer1, telefoonnummer2, email1, email2, taalvoorkeur, preferred_contact, attest_nodig, frequentie_attest, extra_informatie)"
                      "                  VALUES (NULL, NULL, :naam, :kurk, :parafine, :code, :code_intercommunale, :straat, :nr, :bus, :postcode, :plaats, :land, :openingsuren, :contactpersoon, :telefoonnummer1, :telefoonnummer2, :email1, :email2, :taalvoorkeur, :preferred_contact, :attest_nodig, :frequentie_attest, :extra_informatie)");

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


        if(!query.exec())
            qDebug() << "INSERT INTO ophaalpunten FAILED!" << query.lastError();
    }

    this->close();
    emit infoChanged();
}

void InfoOphaalpunt::reject()
{
    this->close();
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
    }
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

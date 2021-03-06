#include "configuration.h"
#include <QDoubleValidator>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

Configuration::Configuration(QWidget *parent) :
    QWidget(parent)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                    | QDialogButtonBox::Cancel);
    resetButton = new QPushButton(tr("Reset"));
    buttonBox->addButton(resetButton,QDialogButtonBox::ResetRole);

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(resetButton, SIGNAL(pressed()), this, SLOT(setOriginalValues()));

    // bekijk? http://stackoverflow.com/questions/13828657/qspaceritem-in-qformlayout-vertical-expand
    apiKeyEdit = new QLineEdit();
    db_databasenameEdit = new QLineEdit();
    db_hostEdit = new QLineEdit();
    db_passwordEdit = new QLineEdit();
    db_usernameEdit = new QLineEdit();
    startpuntEdit = new QLineEdit();
    zak_kaarsresten_naar_kgEdit= new QLineEdit();
    zak_kaarsresten_naar_kgEdit->setValidator( new QDoubleValidator(0, 1000, 2, this) );
    zak_kaarsresten_volumeEdit= new QLineEdit();
    zak_kaarsresten_volumeEdit->setValidator( new QDoubleValidator(0, 1000, 2, this) );
    zak_kurk_naar_kgEdit= new QLineEdit();
    zak_kurk_naar_kgEdit->setValidator( new QDoubleValidator(0, 1000, 2, this) );
    zak_kurk_volumeEdit= new QLineEdit();
    zak_kurk_volumeEdit->setValidator( new QDoubleValidator(0, 1000, 2, this) );
    max_gewicht_vrachtwagenEdit = new QLineEdit();
    max_gewicht_vrachtwagenEdit->setValidator( new QDoubleValidator(0, 100000, 2, this) );
    max_volume_vrachtwagenEdit = new QLineEdit();
    max_volume_vrachtwagenEdit->setValidator( new QDoubleValidator(0, 100000, 2, this) );
    setOriginalValues();


    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Google ApiKey:"), apiKeyEdit);
    // line: Database
    layout->addRow(tr("Database - name:"), db_databasenameEdit);
    layout->addRow(tr("Database - host:"), db_hostEdit);
    layout->addRow(tr("Database - user:"), db_usernameEdit);
    layout->addRow(tr("Database - password:"), db_passwordEdit);
    // line: startpunt
    layout->addRow(tr("Startpunt:"), startpuntEdit);
    // line: producten
    layout->addRow(tr("Gewicht zak kaarsresten in kg:"), zak_kaarsresten_naar_kgEdit);
    layout->addRow(tr("Volume zak kaarsresten in liter:"), zak_kaarsresten_volumeEdit);
    layout->addRow(tr("Gewicht zak kurk in kg:"), zak_kurk_naar_kgEdit);
    layout->addRow(tr("Volume zak kurk in liter:"), zak_kurk_volumeEdit);
    // line: capaciteit vrachtwagen
    layout->addRow(tr("Maximum capaciteit vrachtwagen in kg:"), max_gewicht_vrachtwagenEdit);
    layout->addRow(tr("Maximum capaciteit vrachtwagen in liter:"), max_volume_vrachtwagenEdit);



    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    //w->setMinimumHeight(600);
    setMinimumWidth(600);
    setWindowTitle("Instellingen");
}

void Configuration::accept()
{
     settings.setValue("apiKey", apiKeyEdit->text());
     settings.setValue("db/host", db_hostEdit->text());
     settings.setValue("db/username", db_usernameEdit->text());
     settings.setValue("db/password", db_passwordEdit->text());
     settings.setValue("db/databasename", db_databasenameEdit->text());
     settings.setValue("startpunt", startpuntEdit->text());
     settings.setValue("zak_kaarsresten_naar_kg", zak_kaarsresten_naar_kgEdit->text().toDouble());
     settings.setValue("zak_kurk_naar_kg", zak_kurk_naar_kgEdit->text().toDouble());
     settings.setValue("zak_kaarsresten_volume", zak_kaarsresten_volumeEdit->text().toDouble()); // in liter
     settings.setValue("zak_kurk_volume", zak_kurk_volumeEdit->text().toDouble()); // in liter
     settings.setValue("max_gewicht_vrachtwagen", max_gewicht_vrachtwagenEdit->text().toDouble()); // in liter
     settings.setValue("max_volume_vrachtwagen", max_volume_vrachtwagenEdit->text().toDouble()); // in liter
     emit configurationChanged();

     this->close();
}

void Configuration::reject()
{
    this->close();
}

void Configuration::setOriginalValues()
{
    apiKeyEdit->setText(settings.value("apiKey").toString());
    db_databasenameEdit->setText(settings.value("db/databasename").toString());
    db_hostEdit->setText(settings.value("db/host").toString());
    db_passwordEdit->setText(settings.value("db/password").toString());
    db_usernameEdit->setText(settings.value("db/username").toString());
    startpuntEdit->setText(settings.value("startpunt").toString());
    zak_kaarsresten_naar_kgEdit->setText(settings.value("zak_kaarsresten_naar_kg").toString());
    zak_kaarsresten_volumeEdit->setText(settings.value("zak_kaarsresten_volume").toString());
    zak_kurk_naar_kgEdit->setText(settings.value("zak_kurk_naar_kg").toString());
    zak_kurk_volumeEdit->setText(settings.value("zak_kurk_volume").toString());
    max_gewicht_vrachtwagenEdit->setText(settings.value("max_gewicht_vrachtwagen").toString());
    max_volume_vrachtwagenEdit->setText(settings.value("max_volume_vrachtwagen").toString());
}

Configuration::~Configuration()
{
    vvimDebug() << "start to deconstruct Configuration()";
    delete apiKeyEdit;
    delete db_databasenameEdit;
    delete db_hostEdit;
    delete db_passwordEdit;
    delete db_usernameEdit;
    delete zak_kaarsresten_naar_kgEdit;
    delete zak_kaarsresten_volumeEdit;
    delete zak_kurk_naar_kgEdit;
    delete zak_kurk_volumeEdit;
    delete max_gewicht_vrachtwagenEdit;
    delete max_volume_vrachtwagenEdit;
    delete startpuntEdit;
    delete resetButton;
    vvimDebug() << "Configuration() deconstructed";
}

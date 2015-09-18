#include "nieuweaanmelding.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <math.h>
#include <QMessageBox>
#include "globalfunctions.h"

NieuweAanmelding::NieuweAanmelding(QWidget *parent) :
    QWidget(parent)
{
    /**
        * ophaalpunt dat zich heeft aangemled (kan je uit een keuzelijst kiezen)
        * datum melding
        * naam contactpersoon
        * # zakken kurk    | echte # zakken  (aparte knop "melding ingeven" , "ophaalronde bevestigen"
        * # kg kurk        | echte # kg
        * # zakken kaars   | echte # zakken
        * # kg kaars       | echte # kg
        * eventueel opmerkingen
    **/

    completer = NULL;

    info = new InfoOphaalpunt();

    locationLabel = new QLabel(tr("Ophaalpunt:")); //wordt een keuzelijst uit de databank!
    locationEdit = new MyLineEdit(); //wordt een keuzelijst uit de databank!
    toonOphaalpunt  = new QPushButton(tr("Toon info"));
    toonOphaalpunt->setEnabled(false);
    nieuwOphaalpunt = new QPushButton(tr("&Nieuw ophaalpunt aanmaken"));
    nieuwOphaalpunt->setEnabled(true);

    nameLabel = new QLabel(tr("Contactpersoon:"));
    nameEdit = new QLineEdit();
    dateLabel = new QLabel(tr("Datum aanmelding:"));
    dateEdit = new QDateEdit( QDate::currentDate(), this ); // zie http://doc.qt.digia.com/3.3/qdateedit.html
    dateEdit->setDisplayFormat("dd MMM yyyy");
    dateEdit->setLocale(QLocale::Dutch);
    dateEdit->setCalendarPopup(true);  //zie http://stackoverflow.com/questions/7031962/qdateedit-calendar-popup

    zakkenkurkLabel = new QLabel(tr("Zakken kurk:"));
    zakkenkurkSpinBox = new QSpinBox();
    zakkenkurkSpinBox->setMaximum(9999);
    kgkurkLabel = new QLabel(tr("Kg kurk:"));
    kgkurkSpinBox = new QSpinBox();
    kgkurkSpinBox->setMaximum(9999);

    zakkenkaarsenLabel = new QLabel(tr("Zakken kaarsen:"));
    zakkenkaarsenSpinBox = new QSpinBox();
    zakkenkaarsenSpinBox->setMaximum(9999);
    kgkaarsenLabel = new QLabel(tr("Kg kaarsen:"));
    kgkaarsenSpinBox = new QSpinBox();
    kgkaarsenSpinBox->setMaximum(9999);

    opmerkingenLabel = new QLabel(tr("Opmerkingen:"));
    opmerkingenEdit = new QTextEdit();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                          | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    // zie mapper zoals custom widget mapper om ID te linken ??
    connect(toonOphaalpunt, SIGNAL(clicked()), this, SLOT(toonOphaalpuntInformatie()));
    connect(nieuwOphaalpunt, SIGNAL(clicked()), this, SLOT(createNewOphaalpunt()));
    connect(locationEdit, SIGNAL(textChanged(QString)), this, SLOT(ophaalpuntTextChanged()));

    connect(info, SIGNAL(infoChanged()), this, SLOT(loadOphaalpunten()));

    locationLabel->setBuddy(locationEdit);

    loadOphaalpunten();


    nameLabel->setBuddy(nameEdit);
    dateLabel->setBuddy(dateEdit);

    zakkenkurkLabel->setBuddy(zakkenkurkSpinBox);
    kgkurkLabel->setBuddy(kgkurkSpinBox);

    zakkenkaarsenLabel->setBuddy(zakkenkaarsenSpinBox);
    kgkaarsenLabel->setBuddy(kgkaarsenSpinBox);

    opmerkingenLabel->setBuddy(opmerkingenEdit);

    QGridLayout *layout = new QGridLayout();

        // ROW,  COLUMN,  rowspan columnspan
    layout->addWidget(locationLabel,0,0,1,1);
    layout->addWidget(locationEdit,0,1,1,5);
    layout->addWidget(toonOphaalpunt,0,6,1,1);
    layout->addWidget(nameLabel,1,0,1,1);
    layout->addWidget(nameEdit,1,1,1,5);
    layout->addWidget(dateLabel,2,0,1,1);
    layout->addWidget(dateEdit,2,1,1,2);

    layout->addWidget(zakkenkurkLabel,3,0,1,1);
    layout->addWidget(zakkenkurkSpinBox,3,1,1,1);
    layout->addWidget(kgkurkLabel,3,2,1,1);
    layout->addWidget(kgkurkSpinBox,3,3,1,1);

    layout->addWidget(zakkenkaarsenLabel,4,0,1,1);
    layout->addWidget(zakkenkaarsenSpinBox,4,1,1,1);
    layout->addWidget(kgkaarsenLabel,4,2,1,1);
    layout->addWidget(kgkaarsenSpinBox,4,3,1,1);

    layout->addWidget(opmerkingenLabel,5,0,1,1);
    layout->addWidget(opmerkingenEdit,5,1,3,6);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    //mainLayout->addWidget(nieuwOphaalpunt); // not needed in my opinion
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);


    locationEdit->setFocus(); // always setFocus() as last action: http://stackoverflow.com/questions/526761/set-qlineedit-focus-in-qt

    setWindowTitle(tr("Nieuwe aanmelding op te halen hoeveelheid"));
    setMinimumWidth(600);

    /**
            mapper niet nodig voor "Aanmeldingen" , wel voor "bevestigen van ophaling",
            de ophaalpunten zijn dan een "dropdownbox" en de selectie daar bepaald
            welke "row" er getoond wordt




            naam van de ophaling
             -> adres laten zien
             -> telefoonnumer
             -> email
             -> ...

             Beter een knop aanmaken "geef fiche met ophaalpunt weer" (mogelijkheid om gegevens te wijzigen, nieuw ophalapunt aanmaken)
    **/
}

void NieuweAanmelding::resetValues()
{
    locationEdit->setText("");
    nameEdit->setText("");
    dateEdit->setDate(QDate::currentDate());

    zakkenkurkSpinBox->setValue(0);
    kgkurkSpinBox->setValue(0);

    zakkenkaarsenSpinBox->setValue(0);
    kgkaarsenSpinBox->setValue(0);

    opmerkingenEdit->setText("");

    locationEdit->setFocus(); // always setFocus() as last action: http://stackoverflow.com/questions/526761/set-qlineedit-focus-in-qt

    toonOphaalpunt->setEnabled(false);

    //setMinimumWidth(600);
}

void NieuweAanmelding::aanmeldingVoorOphaalpunt(int ophaalpunt_id)
{
    resetValues();
    QString SQLquery = QString("SELECT naam, contactpersoon FROM ophaalpunten WHERE id = %1").arg(ophaalpunt_id);
    QSqlQuery query;
    query.prepare("SELECT naam, contactpersoon FROM ophaalpunten WHERE id = :id");
    query.bindValue(":id", ophaalpunt_id);


    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB, halting";
            exit(-1);
        }

        QSqlQuery query2;
        query2.prepare("SELECT naam, contactpersoon FROM ophaalpunten WHERE id = :id");
        query = query2;
        query.bindValue(":id", ophaalpunt_id);

        if(!query.exec())
        {
            vvimDebug() << "query failed after reconnecting to DB, halting" << SQLquery << query.lastError();
            exit(-1);
        }
    }

    vvimDebug() << "<vvim> TODO: testen -> geeft 'query.next()' al meteen de tweede oplossing, of de eerste? En wat met 'query.exec()'' gevolgd door 'query.next()' ?";
    vvimDebug() << "<vvim> TODO: TEST mis ik hier de allereerste aanmelding???";

    if (query.next())
    {
        locationEdit->setText(query.value(0).toString());
        nameEdit->setText(query.value(1).toString());
    }

    this->show();
}

NieuweAanmelding::~NieuweAanmelding()
{
    vvimDebug() << "start to deconstruct NieuweAanmelding()";
    delete info;
    delete locationLabel;
    delete locationEdit;
    delete toonOphaalpunt;
    delete nieuwOphaalpunt;
    delete nameLabel;
    delete nameEdit;
    delete dateLabel;
    delete dateEdit;
    delete zakkenkurkLabel;
    delete zakkenkurkSpinBox;
    delete kgkurkLabel;
    delete kgkurkSpinBox;
    delete kgkaarsenLabel;
    delete kgkaarsenSpinBox;
    delete opmerkingenLabel;
    delete opmerkingenEdit;
    delete buttonBox;
    if(completer)
        delete completer;
    vvimDebug() << "NieuweAanmelding() deconstructed";
}

void NieuweAanmelding::accept()
{
    if(ophaalpunten[locationEdit->text()] < 1)
    {
        vvimDebug() << "no valid ophaalpunt, just ignore";
        return;
    }


    /** calculate bags to kgs (if needed) **/
    if (zakkenkaarsenSpinBox->value() == 0)
        zakkenkaarsenSpinBox->setValue(ceil(   kgkaarsenSpinBox->value() / settings.value("zak_kaarsresten_naar_kg").toDouble()  ) );
    else if (kgkaarsenSpinBox->value() == 0)
        kgkaarsenSpinBox->setValue(zakkenkaarsenSpinBox->value() * settings.value("zak_kaarsresten_naar_kg").toInt()  );

    if (zakkenkurkSpinBox->value() == 0)
        zakkenkurkSpinBox->setValue(ceil(   kgkurkSpinBox->value() / settings.value("zak_kurk_naar_kg").toDouble()  ) );
    else if (kgkurkSpinBox->value() == 0)
        kgkurkSpinBox->setValue(zakkenkurkSpinBox->value() * settings.value("zak_kurk_naar_kg").toInt()  );


    /** insert into db **/
    QSqlQuery query;

    //ESCAPING QUERY: http://stackoverflow.com/questions/19045281/insert-strings-that-contain-or-to-the-database-table-qt and http://qt-project.org/doc/qt-5/qsqlquery.html#prepare
    query.prepare("INSERT INTO aanmelding (id,   timestamp, ophaalpunt,  contactpersoon,  datum,  zakken_kurk,  kg_kurk,  zakken_kaarsresten,  kg_kaarsresten,  opmerkingen, ophaalronde_datum, volgorde) "
                                  "VALUES (NULL, NULL,     :ophaalpunt, :contactpersoon, :datum, :zakken_kurk, :kg_kurk, :zakken_kaarsresten, :kg_kaarsresten, :opmerkingen, NULL,           NULL) ");
    // id wordt ingevuld via AUTO_INCREMENT en is primary key
    // timestamp wordt ingevuld met default value 'current_timestamp'
    query.bindValue(":ophaalpunt", ophaalpunten[locationEdit->text()]);  // VERANDER NAARophaalpunt_id
    query.bindValue(":contactpersoon", nameEdit->text());
    query.bindValue(":datum", dateEdit->date().toString("yyyy-MM-dd"));
    query.bindValue(":zakken_kurk", zakkenkurkSpinBox->value());
    query.bindValue(":kg_kurk", kgkurkSpinBox->value());
    query.bindValue(":zakken_kaarsresten", zakkenkaarsenSpinBox->value());
    query.bindValue(":kg_kaarsresten", kgkaarsenSpinBox->value());
    query.bindValue(":opmerkingen", opmerkingenEdit->toPlainText());
    // ophaalronde_datum wordt pas ingevuld als aanmelding is geselecteerd voor een ophaalronde
    // volgorde wordt pas ingevuld als aanmelding is geselecteerd voor een ophaalronde

    QString SQLqueryMessage = tr("INSERT aanmelding voor ophaalpunt %1 FAALT!").arg(locationEdit->text());

    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLqueryMessage, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            QMessageBox::critical(this, tr("Databankverbinding verloren"),
                        tr("\n\nHerstel de fout en probeer opnieuw."), QMessageBox::Cancel);
            qCritical(SQLqueryMessage.append(" - reconnection to DB failed on first try").toStdString().c_str());
        }
        else
        {
            QSqlQuery query2;
            query2.prepare("INSERT INTO aanmelding (id,   timestamp, ophaalpunt,  contactpersoon,  datum,  zakken_kurk,  kg_kurk,  zakken_kaarsresten,  kg_kaarsresten,  opmerkingen, ophaalronde_datum, volgorde) "
                           "VALUES (NULL, NULL,     :ophaalpunt, :contactpersoon, :datum, :zakken_kurk, :kg_kurk, :zakken_kaarsresten, :kg_kaarsresten, :opmerkingen, NULL,           NULL) ");
            query2.bindValue(":ophaalpunt", ophaalpunten[locationEdit->text()]);  // VERANDER NAARophaalpunt_id
            query2.bindValue(":contactpersoon", nameEdit->text());
            query2.bindValue(":datum", dateEdit->date().toString("yyyy-MM-dd"));
            query2.bindValue(":zakken_kurk", zakkenkurkSpinBox->value());
            query2.bindValue(":kg_kurk", kgkurkSpinBox->value());
            query2.bindValue(":zakken_kaarsresten", zakkenkaarsenSpinBox->value());
            query2.bindValue(":kg_kaarsresten", kgkaarsenSpinBox->value());
            query2.bindValue(":opmerkingen", opmerkingenEdit->toPlainText());

            query = query2;

            if(!query.exec())
            {
                QMessageBox::critical(this, SQLqueryMessage,
                            query.lastError().text().append(tr("\n\nDatabankverbinding succesvol herstart, toch ging de query fout.\n\nHerstel de fout en probeer opnieuw.")), QMessageBox::Cancel);
                qCritical(QString(tr("INSERT aanmelding voor ophaalpunt %1 FAALT!").arg(locationEdit->text()).append(query.lastError().text())).toStdString().c_str());
                vvimDebug() << "Reconnection to DB was succesful but query still failed" << SQLqueryMessage << query.lastError();
            }
            else
            {
                vvimDebug() << "successfully added ophaalpunt" << locationEdit->text() << "( id" << ophaalpunten[locationEdit->text()] << ") to table AANMELDING";
                this->close();
            }
        }
    }
    else
    {
        vvimDebug() << "successfully added ophaalpunt" << locationEdit->text() << "( id" << ophaalpunten[locationEdit->text()] << ") to table AANMELDING";
        this->close();
    }
}

void NieuweAanmelding::reject()
{
    this->close();
}

void NieuweAanmelding::ophaalpuntTextChanged()
{
    if (ophaalpunten[locationEdit->text()] > 0)
    {
        toonOphaalpunt->setEnabled(true);
    }
    else
        toonOphaalpunt->setEnabled(false);
}

void NieuweAanmelding::toonOphaalpuntInformatie()
{
    info->setWindowTitle("info over ophaalpunt");
    info->showOphaalpunt(ophaalpunten[locationEdit->text()]);
}

void NieuweAanmelding::loadOphaalpunten()
{
    vvimDebug() << "database has been changed, so we should reload the Completer";
    // autocompletion for locationEdit:
    // telkens aanroepen na aanmaken / wijzigen van een ophaalpunt?
    QStringList words; // "don't come easy, to me, la la la laaa la la"

    vvimDebug() << "empty QMap 'ophaalpunten'";
    ophaalpunten.clear();

    #ifndef QT_NO_CURSOR
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    #endif

    QString SQLquery = "SELECT id, naam FROM ophaalpunten WHERE kurk > 0 OR parafine > 0";

    QSqlQuery query(SQLquery);

    if(!query.exec())
    {
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB";
        }
        query = QSqlQuery(SQLquery);
        if(!query.exec())
        {
            vvimDebug() << "query failed after reconnecting to DB" << SQLquery << query.lastError();
        }
    }


    while (query.next()) {
        int id = query.value(0).toInt();
        QString naam	= query.value(1).toString();
        words << naam;

        ophaalpunten[naam] = id;
    }

    vvimDebug() << "TOTAL of ophaalpunten loaded in completer from the \"create a new collection request\" window : " << words.length();

    #ifndef QT_NO_CURSOR
        QApplication::restoreOverrideCursor();
    #endif



    if(completer)
        delete completer;

    completer = new MyCompleter(words, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    locationEdit->setCompleter(completer);
    vvimDebug() << "done, completer (re)loaded.";
}

void NieuweAanmelding::createNewOphaalpunt()
{
    info->setWindowTitle("Nieuw ophaalpunt");
    info->createNewOphaalpunt();
}

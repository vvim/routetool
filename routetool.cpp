#include "routetool.h"
#include "ui_routetool.h"
#include <QDebug>
#include <QApplication>
#include "form.h"

#include <QSqlQuery>
#include <QSqlError>

RouteTool::RouteTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RouteTool)
{
    ui->setupUi(this);

    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->nieuweAanmeldingMenuButton, SIGNAL(triggered()), this, SLOT(showNieuweAanmelding()));
    connect(ui->nieuwOphaalpuntMenuButton, SIGNAL(triggered()), this, SLOT(showNieuwOphaalpunt()));
    connect(ui->configurationMenuButton, SIGNAL(triggered()), this, SLOT(showConfiguration()));
    connect(ui->kiesOphaalpuntenMenuButton, SIGNAL(triggered()), this, SLOT(showKiesOphaalpunten()));
    connect(ui->nieuweLeveringMenuButton, SIGNAL(triggered()), this, SLOT(showNieuweLevering()));
    connect(ui->wieNogNooitMenuButton, SIGNAL(triggered()), this, SLOT(showBijWieNogNooitOpgehaald()));
    connect(ui->voorspellingMenuButton, SIGNAL(triggered()), this, SLOT(showVoorspelling()));

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);

    connect(&kiesOphaalpuntenWidget, SIGNAL(aanmelding_for_route(QList<SOphaalpunt> *)), m_pForm, SLOT(add_aanmeldingen(QList<SOphaalpunt>*)));
    connect(&leveringWidget, SIGNAL(levering_for_route(SLevering)), m_pForm, SLOT(add_levering(SLevering)));
    connect(&configurationWidget, SIGNAL(configurationChanged()), m_pForm, SLOT(setTotalWeightTotalVolume()));
}

RouteTool::~RouteTool()
{
    qDebug() << "start to deconstruct RouteTool()";
    delete m_pForm;
    delete ui;
    qDebug() << "RouteTool() deconstructed";
}

void RouteTool::showNieuweAanmelding()
{
    nieuweAanmeldingWidget.resetValues();
    nieuweAanmeldingWidget.show();
}

void RouteTool::showNieuwOphaalpunt()
{
    nieuwOphaalpuntWidget.createNewOphaalpunt();
    // this functions triggers "show()" AND resets the information in the dialogbox at the same time
}

void RouteTool::showConfiguration()
{
    configurationWidget.setOriginalValues();
    configurationWidget.show();
    // here the information stays the same, so it shouldn't matter
}



void RouteTool::showKiesOphaalpunten()
{
    kiesOphaalpuntenWidget.initialise();
    kiesOphaalpuntenWidget.show();
    // always re-initialise the list of 'aanmeldingen' as the user might have given new input since last time
}

void RouteTool::showNieuweLevering()
{
    leveringWidget.resetValues();
    leveringWidget.show();
}

void RouteTool::showBijWieNogNooitOpgehaald()
{
    qDebug() << "Toon bij welke ophaalpunten we nog nooit iets zijn gaan ophalen";
    /** waarschijnlijk gebaseerd op kiesOphaalpuntenWidget
        kiesOphaalpuntenWidget.initialise();
        kiesOphaalpuntenWidget.show();
    **/

    QSqlQuery query;
    query.prepare("select ophaalpunten.id, ophaalpunten.naam "
                  "from ophaalpunten where not exists "
                      "(select null from ophalinghistoriek "
                       "where ophalinghistoriek.ophaalpunt = ophaalpunten.id);");
    if(!query.exec())
        qDebug() << "SELECT FAILED!" << query.lastError();
    else
        qDebug() << "Deze ophaalpunten hebben geen historiek: (misschien hebben ze wel een aanmelding gedaan!)";

    int i = 0;
    while (query.next())
    {
        qDebug() << "#" << query.value(0).toInt() << ":" << query.value(1).toString();
        i++;

        QSqlQuery query2;
        query2.prepare("Select * from aanmelding where ophaalpunt = :ophaal");
        query2.bindValue(":ophaal",query.value(0).toInt());

        if(query2.exec())
        {
            if (query2.next())
                qDebug() << "... dit ophaalpunt heeft een aanmelding lopende.";
        }
        else
            qDebug() << "something went wrong with checking for an existing aanmelding";

    }
    qDebug() << i << "ophaalpunten in totaal";
}

void RouteTool::showVoorspelling()
{
    qDebug() << "Toon welke ophaalpunten nu waarschijnlijk een voorraad zullen hebben (berekend uit de historiek van ophalingen)";
    /** waarschijnlijk gebaseerd op kiesOphaalpuntenWidget
        kiesOphaalpuntenWidget.initialise();
        kiesOphaalpuntenWidget.show();
    **/

    QSqlQuery query;
    query.prepare("SELECT DISTINCT ophaalpunt FROM `ophalinghistoriek`");
    if(query.exec())
    {
        qDebug() << "\n\n ---------- START invullen voorspellingen:" << QDateTime().currentDateTime().toString();
        while (query.next())
        {
            qDebug() << "\nophaalpunt" << query.value(0).toInt();
            vvimTestingPurposesOnly(query.value(0).toInt());
        }
        qDebug() << "\n\n ---------- END invullen voorspellingen:" << QDateTime().currentDateTime().toString();
        qDebug() << "check with query SELECT id, `last_contact_date`, `contact_again_on` FROM `ophaalpunten` WHERE `last_contact_date` > 0";
    }
}

void RouteTool::vvimTestingPurposesOnly(int ophaalpuntid)
{
    QSqlQuery query;
    query.prepare("SELECT ophalinghistoriek.ophalingsdatum, ophaalpunten.last_contact_date, ophaalpunten.contact_again_on FROM `ophalinghistoriek`, ophaalpunten where ophalinghistoriek.ophaalpunt = ophaalpunten.id AND ophalinghistoriek.ophaalpunt= :ophaal order by ophalinghistoriek.ophalingsdatum desc;");
    query.bindValue(":ophaal",ophaalpuntid);
    if(query.exec())
    {
        if (query.next())
        {
            QDate laatste_ophaling = query.value(0).toDate();
            qDebug() << "laatste ophaling voor punt" << ophaalpuntid << "was" << laatste_ophaling << "ofte" << laatste_ophaling.toString();

            QDate ophaalpunt_LastContactDate = query.value(1).toDate();
            QDate ophaalpunt_ContactAgainOn = query.value(2).toDate();

            int aantal_ophalingen = 0;
            int aantal_dagen_tussen_verschillende_ophalingen = 0;
            QDate temp_ophaling = laatste_ophaling;
            while(query.next())
            {
                if(aantal_ophalingen >= 5)
                    break;
                QDate oudere_ophaling = query.value(0).toDate();
                aantal_dagen_tussen_verschillende_ophalingen += oudere_ophaling.daysTo(temp_ophaling);
                qDebug() << "....ophaling" << aantal_ophalingen << "gebeurde op" << oudere_ophaling.toString() << ", totaal dagen:" << aantal_dagen_tussen_verschillende_ophalingen;
                aantal_ophalingen++;
                temp_ophaling = oudere_ophaling;
            }
            qDebug() << "..einde van de while-loop na" << aantal_ophalingen << "loops. Totaal: "<< aantal_dagen_tussen_verschillende_ophalingen;
            if(aantal_ophalingen > 0)
            {
                int gemiddelde = aantal_dagen_tussen_verschillende_ophalingen / aantal_ophalingen;
                qDebug() << "..gemiddeld:" << gemiddelde;
                //if CURRENTDATE > laatste_ophaling + GEMIDDELDE : contact == current_date (-1) ()
                qDebug() << "..dus ophaalpunt te contacteren rond:" << laatste_ophaling.addDays(gemiddelde).toString() << "(waarde van laatste_ophaling verandert ook?)" << laatste_ophaling.toString();

                QSqlQuery query_lastcontact_and_contact_on;

                //  << "** IF last_ophaling > current_value of field (oplossen door waarde op te vragen in eerste SELECT ?? )")
                //  << "** IF last_ophaling > TODAY
                QString q = QString("UPDATE ophaalpunten SET last_contact_date = %1 , contact_again_on = %2 WHERE id = %3;").arg(laatste_ophaling.toString()).arg(laatste_ophaling.addDays(gemiddelde).toString()).arg(ophaalpuntid);
                query_lastcontact_and_contact_on.prepare("UPDATE ophaalpunten SET last_contact_date = :last_ophaling , contact_again_on = :contact_again WHERE id = :id;");
                query_lastcontact_and_contact_on.bindValue(":last_ophaling",laatste_ophaling);

                    qDebug() << "...datum controle:";
                    qDebug() << ".....laatste_ophaling:" << laatste_ophaling.toString();
                    qDebug() << ".....huidige waarde in DB: ophaalpunt_LastContactDate:" << ophaalpunt_LastContactDate.toString();
                    qDebug() << "...Neem het maximum van de volgende drie als de nieuwe 'contact_again_date':";
                    qDebug() << ".....huidige waarde in DB: ophaalpunt_ContactAgainOn:" << ophaalpunt_ContactAgainOn.toString();
                    qDebug() << ".....vandaag:" << QDate().currentDate().toString();
                    qDebug() << ".....voorspelling:" << laatste_ophaling.addDays(gemiddelde).toString();
                    qDebug() << "...Maximum is:" << qMax(laatste_ophaling.addDays(gemiddelde), qMax(QDate().currentDate(),ophaalpunt_ContactAgainOn));

                query_lastcontact_and_contact_on.bindValue(":contact_again ", laatste_ophaling.addDays(gemiddelde) );
                query_lastcontact_and_contact_on.bindValue(":id",ophaalpuntid);

                if(query_lastcontact_and_contact_on.exec())
                    qDebug() << "..." << q << "done";
                else
                    qDebug() << "..." << q << "went WRONG:" << query_lastcontact_and_contact_on.lastError();
            }
            else
            {
                qDebug() << "..minder dan 2 ophalingen, het heeft dus geen zin om het gemiddelde te berekenen. We kunnen wel het laatste contact invullen:";
                QSqlQuery query_lastcontact_only;
                //  << "** IF last_ophaling > current_value of field (oplossen door waarde op te vragen in eerste SELECT ?? )")
                //  << "** IF last_ophaling > TODAY
                QString q = QString("UPDATE ophaalpunten SET last_contact_date = %1 WHERE id = %2;").arg(laatste_ophaling.toString()).arg(ophaalpuntid);
                query_lastcontact_only.prepare("UPDATE ophaalpunten SET last_contact_date = :last_ophaling WHERE id = :id;");
                query_lastcontact_only.bindValue(":last_ophaling",laatste_ophaling);
                query_lastcontact_only.bindValue(":id",ophaalpuntid);


                if(query_lastcontact_only.exec())
                    qDebug() << "..." << q << "done";
                else
                    qDebug() << "..." << q << "went WRONG:" << query_lastcontact_only.lastError();
            }
        }
    }
    else
        qDebug() << "something went wrong with checking for an existing aanmelding";
}

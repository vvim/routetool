#include <QDebug>
#include <QSqlQuery>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "ophaalpuntenwidget.h"
#include "globalfunctions.h"

OphaalpuntenWidget::OphaalpuntenWidget(QWidget *parent) :
    QWidget(parent)
{
    completer = NULL;
    info = new InfoOphaalpunt();
    info->showAanmeldingAndHistoriekButton(true);
    info->setWindowTitle(tr("info over ophaalpunt"));

    nieuweaanmeldingWidget = new NieuweAanmelding();

    ophaalpuntLabel = new QLabel(tr("Ophaalpunt:")); //wordt een keuzelijst uit de databank!
    ophaalpuntEdit = new MyLineEdit(); //wordt een keuzelijst uit de databank!
    toonOphaalpunt  = new QPushButton(tr("Toon info"));
    toonOphaalpunt->setEnabled(false);

    connect(toonOphaalpunt, SIGNAL(clicked()), this, SLOT(toonOphaalpuntInformatie()));
    connect(ophaalpuntEdit, SIGNAL(textChanged(QString)), this, SLOT(ophaalpuntTextChanged()));
    connect(info,SIGNAL(nieuweAanmelding(int)),nieuweaanmeldingWidget,SLOT(aanmeldingVoorOphaalpunt(int)));
    connect(info, SIGNAL(infoChanged()), this, SLOT(databaseBeenUpdated()));

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(ophaalpuntLabel);
    layout->addWidget(ophaalpuntEdit);
    layout->addWidget(toonOphaalpunt);

    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->addLayout(layout);

    // add list of all ophaalpunten:
    contactTreeView = new QTreeView();
    contactTreeView->setRootIsDecorated(false);
    contactTreeView->setAlternatingRowColors(true);
    contactTreeView->setSortingEnabled(true);
    contactTreeView->sortByColumn(1, Qt::AscendingOrder);
    contactTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    model = NULL;
    listOfLocationsModel = NULL;
    initialise();

    connect(contactTreeView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(toonOphaalpuntInformatie(QModelIndex)));

    mainlayout->addWidget(contactTreeView);

    setLayout(mainlayout);

    ophaalpuntEdit->setFocus(); // always setFocus() as last action: http://stackoverflow.com/questions/526761/set-qlineedit-focus-in-qt

    setWindowTitle(tr("Bekijk informatie bestaande ophaalpunten"));
    setMinimumSize(600, 300);
}

OphaalpuntenWidget::~OphaalpuntenWidget()
{
    vvimDebug() << "start to deconstruct OphaalpuntenWidget()";
    delete ophaalpuntLabel;
    delete ophaalpuntEdit;
    delete info;
    delete contactTreeView;
    delete model;
    delete listOfLocationsModel;
    delete nieuweaanmeldingWidget;
    delete toonOphaalpunt;
    if(completer)
    {
        vvimDebug() << "completer _not_ NULL";
        delete completer;
    }
    else
        vvimDebug() << "completer == NULL";
    vvimDebug() << "OphaalpuntenWidget() deconstructed";
}

void OphaalpuntenWidget::initModel()
{
    vvimDebug() << "[ListOfOphaalpuntenToContact::initModel]" << "start";
    delete model;
    delete listOfLocationsModel;

    QStringList labels;
    labels << tr("id") << tr("Ophaalpunt") << tr("Straat") << tr("Huisnummer") << tr("Busnummer") << tr("Postcode") << tr("Plaats") << tr("Land") << tr("Aanmelding_present");

    model = new QStandardItemModel(0, labels.count());

    listOfLocationsModel = new OphaalpuntenWidgetSortFilterProxyModel(this);
    listOfLocationsModel->setDynamicSortFilter(true);
    listOfLocationsModel->setSourceModel(model);

    for(int i = 0; i < labels.count(); i++)
    {
       model->setHeaderData(i,Qt::Horizontal, labels[i]);  // why does 'tr()' not work? -> QString& instead of QString...
    }

    contactTreeView->setModel(listOfLocationsModel);

    contactTreeView->hideColumn(OPHAALPUNTQTREEVIEW_OPHAALPUNT_ID);
    contactTreeView->hideColumn(OPHAALPUNTQTREEVIEW_AANMELDING_PRESENT);

    // possible enhancement: add checkbox "show street / housenumber / busnumber"
    contactTreeView->hideColumn(OPHAALPUNTQTREEVIEW_OPHAALPUNT_HUISNR);
    contactTreeView->hideColumn(OPHAALPUNTQTREEVIEW_OPHAALPUNT_BUSNR);
}


void OphaalpuntenWidget::loadOphaalpunten()
{
    vvimDebug() << "database has been changed, so we should reload the Completer";
    // reinit TreeWidget
    initModel();

    // autocompletion for locationEdit:
    // telkens aanroepen na aanmaken / wijzigen van een ophaalpunt?
    QStringList words; // "don't come easy, to me, la la la laaa la la"

    vvimDebug() << "empty QMap 'ophaalpunten'";
    ophaalpunten.clear();

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

    QString SQLquery = "SELECT naam, straat, nr, bus, postcode, plaats, land, id FROM ophaalpunten ORDER by postcode, naam";
    QSqlQuery query(SQLquery);

    vvimDebug() << "<vvim> TODO: testen -> geeft 'query.next()' al meteen de tweede oplossing, of de eerste? En wat met 'query.exec()'' gevolgd door 'query.next()' ?";
    vvimDebug() << "<vvim> TODO: TEST mis ik hier het allereerste ophaalpunt???";

    if(!query.exec())
    {
        vvimDebug() << "something went wrong with the query" << query.lastError().text() << "trying to reconnect to the DB";
        if(!reConnectToDatabase(query.lastError(), SQLquery, QString("[%1]").arg(Q_FUNC_INFO)))
        {
            vvimDebug() << "unable to reconnect to DB: FAIL";
        }
        else
        {
            query = QSqlQuery(SQLquery);
            vvimDebug() << "reconnected to DB";
            if(!query.exec())
            {
                vvimDebug() << "query failed after reconnecting to DB" << SQLquery << query.lastError();
            }
        }
    }


    while (query.next()) {
        bool aanmelding_running = false;
        QString naam	= query.value(0).toString();
        QString straat	= query.value(1).toString();
        QString nr	    = query.value(2).toString();
        QString bus	    = query.value(3).toString();
        QString postcode	= query.value(4).toString();
        QString plaats	= query.value(5).toString();
        QString land	= query.value(6).toString();

        // 1. add ophaalpunt to QTextEdit Completer
        int ophaalpunt_id = query.value(7).toInt();

        QString ophaalpunt = naam;
        ophaalpunt.append(QString(", %1 %2, %3 %4, %5").arg(straat).arg(nr).arg(postcode).arg(plaats).arg(land));

        ophaalpunten[ophaalpunt] = ophaalpunt_id;

        words << ophaalpunt;

        // 2. [A] to add ophaalpunt to QTreeView, we first have to check if it has a running Aanmelding or not

        // it seems useless to me to recheck for a DB-connection as this query is right after the previous DB-connection-check
        // also, if the connection would fail for this query, the worst thing that can happen, is that the TreeView is incorrect (no big deal)
        QSqlQuery query2;
        query2.prepare("SELECT * FROM aanmelding WHERE ophaalpunt = :ophaal AND ophaalronde_datum is NULL"); // and ophaalronde is NULL
        query2.bindValue(":ophaal", ophaalpunt_id);

        if(query2.exec())
        {
            if (query2.next())
            {
                aanmelding_running = true;
            }
        }
        else
            vvimDebug() << "something went wrong with checking for an existing aanmelding";

        // 2. [B] add ophaalpunt to QTreeView
        addToTreeView(ophaalpunt_id, naam, straat, nr, bus, postcode, plaats, land, aanmelding_running);
    }

    vvimDebug() << "TOTAL of ophaalpunten loaded in \"OphaalpuntQTreeView\" :" << model->rowCount();

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    if(completer)
        delete completer;

    completer = new MyCompleter(words, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    ophaalpuntEdit->setCompleter(completer);
    vvimDebug() << "done, completer (re)loaded.";
}

void OphaalpuntenWidget::addToTreeView(int ophaalpuntId, QString naamOphaalpunt, QString straat, QString huisnummer, QString busnummer, QString postcode, QString plaats, QString land, bool color_item)
{
    model->insertRow(0);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_OPHAALPUNT_ID), ophaalpuntId);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_OPHAALPUNT_NAAM), naamOphaalpunt);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_OPHAALPUNT_STRAAT), straat);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_OPHAALPUNT_HUISNR), huisnummer);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_OPHAALPUNT_BUSNR), busnummer);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_OPHAALPUNT_POSTCODE), postcode);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_OPHAALPUNT_PLAATS), plaats);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_OPHAALPUNT_LAND), land);
    model->setData(model->index(0,OPHAALPUNTQTREEVIEW_AANMELDING_PRESENT), color_item);
}

void OphaalpuntenWidget::toonOphaalpuntInformatie()
{
    info->setWindowTitle("info over ophaalpunt");
    info->showAanmeldingAndHistoriekButton(true);
    info->showOphaalpunt(ophaalpunten[ophaalpuntEdit->text()]);
}

void OphaalpuntenWidget::toonOphaalpuntInformatie(QModelIndex index)
{
    int row = index.row();
    int ophaalpunt_id = listOfLocationsModel->data(listOfLocationsModel->index(row, OPHAALPUNTQTREEVIEW_OPHAALPUNT_ID)).toInt();
    vvimDebug() << "get ophaalpunt id from row " << row << "is" << ophaalpunt_id;
    info->showOphaalpunt(ophaalpunt_id);
    info->showAanmeldingAndHistoriekButton(true);
    info->setWindowTitle(tr("info over ophaalpunt"));
}


void OphaalpuntenWidget::ophaalpuntTextChanged()
{
    if (ophaalpunten[ophaalpuntEdit->text()] > 0)
    {
        toonOphaalpunt->setEnabled(true);
    }
    else
        toonOphaalpunt->setEnabled(false);

}

void OphaalpuntenWidget::initialise()
{
    loadOphaalpunten();
    ophaalpuntEdit->setText("");
}

void OphaalpuntenWidget::databaseBeenUpdated()
{
    emit contentsOfDatabaseChanged();
}

bool OphaalpuntenWidget::OphaalpuntHasAanmeldingPresent(int ophaalpunt_id)
{
    // get the information whether there is OPHAALPUNTQTREEVIEW_AANMELDING_PRESENT or not.

    // FIRST CHECK IF MODEL IS INITIALISED!!!
    vvimDebug() << "model is NULL?" <<     (model == NULL) ;
    if(model == NULL)
    {
        vvimDebug() << "model is NULL! we should initialise and fill the model";
        initialise();
    }

    int row = -1;
    int i = 0;

    // 1. search for row that contains ophaalpunt #ophaalpunt_id
    /** is this the correct way to iterate through listOfLocationsModel? Better ask at StackExchange, maybe an Iterator is better? But can't find any for StandardItemModel?
        form.cpp:   QSet<SOphaalpunt*>::Iterator it = markers_met_aanmelding.begin();
        form.cpp:    while(it != markers_met_aanmelding.end()) **/
    for(i = 0; i < listOfLocationsModel->rowCount(); i++)
    {
        if(listOfLocationsModel->data(listOfLocationsModel->index(i, OPHAALPUNTQTREEVIEW_OPHAALPUNT_ID)).toInt() == ophaalpunt_id)
        {
            row = i;
            break;
        }
    }

    // what if Row is not found?
    if(row < 0)
    {
        vvimDebug() << "ophaalpunt" << ophaalpunt_id << "was not found, even after iterating through " << i << listOfLocationsModel->rowCount() << "rows of the model. Return false";
        vvimDebug() << "total of rows in model:" << listOfLocationsModel->rowCount();
        return false;
    }

    vvimDebug() << "ophaalpunt" << ophaalpunt_id << "was found at row" << row << "total of rows:" << listOfLocationsModel->rowCount() << "name:" << listOfLocationsModel->data(listOfLocationsModel->index(row, OPHAALPUNTQTREEVIEW_OPHAALPUNT_NAAM)).toString();
    // 2. return the column OPHAALPUNTQTREEVIEW_AANMELDING_PRESENT

    return listOfLocationsModel->data(listOfLocationsModel->index(row, OPHAALPUNTQTREEVIEW_AANMELDING_PRESENT)).toBool();
}

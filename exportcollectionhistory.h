#ifndef EXPORTCOLLECTIONHISTORY_H
#define EXPORTCOLLECTIONHISTORY_H

#include <QWidget>
#include "mylineedit.h" // not my personal code, where is it from? (forum on QCompleter)

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QDateEdit;
class QDialogButtonBox;
class QFileDialog;
QT_END_NAMESPACE

class ExportCollectionHistory : public QWidget
{
    Q_OBJECT
public:
    explicit ExportCollectionHistory(QWidget *parent = 0);
    ~ExportCollectionHistory();
    
signals:
    
private slots:
    void accept();
    void reject();

private:
    MyCompleter *completer;
    QDialogButtonBox *buttonBox;
    QLabel *ophaalpuntLabel, *timeperiod_startLabel, *timeperiod_endLabel, *timeperiodLabel;
    QLineEdit *ophaalpuntEdit;
    QDateEdit *timeperiod_startEdit, *timeperiod_endEdit;
    QFileDialog* exportToFile;
};

#endif // EXPORTCOLLECTIONHISTORY_H

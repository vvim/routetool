#ifndef KIESGEDANEOPHALING_H
#define KIESGEDANEOPHALING_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDate>

enum KGOPurpose { Confirming, Deleting, Editing };

class KiesGedaneOphaling : public QWidget
{
    Q_OBJECT
public:
    explicit KiesGedaneOphaling(KGOPurpose purpose = Confirming, QWidget *parent = 0);
    ~KiesGedaneOphaling();
    int initialise(); // return: -1 is error, 0 is nothing found, +1 is all ok

signals:
    
public slots:
    
private:
    QLabel* ophalingenLabel;
    QComboBox* ophalingenComboBox;
    QDialogButtonBox *buttonBox;
    QMap<int,QDate> ophalingenMap; //QMap die linkt tussen ophalingenComboBox-> index en QDate
    KGOPurpose m_confirm_or_cancel;

    void confirmRoute(QDate ophaalronde_datum);
    void cancelRoute(QDate ophaalronde_datum);

private slots:
    void reject();
    void accept();
};

#endif // KIESGEDANEOPHALING_H

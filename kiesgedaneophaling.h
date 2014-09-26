#ifndef KIESGEDANEOPHALING_H
#define KIESGEDANEOPHALING_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDate>

class KiesGedaneOphaling : public QWidget
{
    Q_OBJECT
public:
    explicit KiesGedaneOphaling(QWidget *parent = 0);
    ~KiesGedaneOphaling();
    
signals:
    
public slots:
    
private:
    QLabel* ophalingenLabel;
    QComboBox* ophalingenComboBox;
    QDialogButtonBox *buttonBox;
    QMap<int,QDate> ophalingenMap; //QMap die linkt tussen ophalingenComboBox-> index en QDate

private slots:
    void reject();
    void accept();
};

#endif // KIESGEDANEOPHALING_H

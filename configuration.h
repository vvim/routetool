#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QWidget>
#include <QDebug>
#include <QSettings>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

extern QSettings settings;

class Configuration : public QWidget
{
    Q_OBJECT
public:
    explicit Configuration(QWidget *parent = 0);

public slots:
    void setOriginalValues();
    
private:
    QLineEdit *apiKeyEdit;
    QLineEdit *db_databasenameEdit;
    QLineEdit *db_hostEdit;
    QLineEdit *db_passwordEdit;
    QLineEdit *db_usernameEdit;
    QLineEdit *zak_kaarsresten_naar_kgEdit;
    QLineEdit *zak_kaarsresten_volumeEdit;
    QLineEdit *zak_kurk_naar_kgEdit;
    QLineEdit *zak_kurk_volumeEdit;
    QLineEdit *startpuntEdit;
    QPushButton *resetButton;

private slots:
    void accept();
    void reject();
};

#endif // CONFIGURATION_H

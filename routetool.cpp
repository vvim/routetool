#include "routetool.h"
#include "ui_routetool.h"
#include "form.h"

RouteTool::RouteTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RouteTool)
{
    ui->setupUi(this);

    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);

}

RouteTool::~RouteTool()
{
    delete ui;
}

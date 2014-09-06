#include "mylineedit.h"
#include <QDebug>

#define vvimDebug()\
    qDebug() << "[" << Q_FUNC_INFO << "]"

MyLineEdit::MyLineEdit(QWidget *parent)
: QLineEdit(parent), c(0)
{
}

MyLineEdit::~MyLineEdit()
{
}

void MyLineEdit::setCompleter(MyCompleter *completer)
{
    vvimDebug() << "a) test for if(c)";
if (c)
{
    vvimDebug() << "b-1) yes, c is true";
//QObject::disconnect(c, 0, this, 0); // seriously, is this useful?? It definitely is part of the crash issue 1 (see http://stackoverflow.com/questions/25698644/concerning-qlineedit-can-i-change-completers-at-runtime )
    vvimDebug() << "b-2) c is now disconnected";
    vvimDebug() << "b-3) the above statement is NOT true, we no longer disconnect c as this appears to sometimes create a bug that is difficult to reproduce";
}

vvimDebug() << "c) make c the completer";
c = completer;

if (!c)
{
    vvimDebug() << "d) c is FALSE, ABORT!";
    return;
}

vvimDebug() << "d) c is not false, we don't need to abort";
c->setWidget(this);
vvimDebug() << "e) connect";
connect(completer, SIGNAL(activated(const QString&)), this, SLOT(insertCompletion(const QString&)));
vvimDebug() << "f) DONE";

}

MyCompleter *MyLineEdit::completer() const
{
return c;
}

void MyLineEdit::insertCompletion(const QString& completion)
{
setText(completion);
selectAll();
}


void MyLineEdit::keyPressEvent(QKeyEvent *e)
{
if (c && c->popup()->isVisible())
{
// The following keys are forwarded by the completer to the widget
switch (e->key())
{
case Qt::Key_Enter:
case Qt::Key_Return:
case Qt::Key_Escape:
case Qt::Key_Tab:
case Qt::Key_Backtab:
e->ignore();
return; // Let the completer do default behavior
}
}

bool isShortcut = (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E;
if (!isShortcut)
QLineEdit::keyPressEvent(e); // Don't send the shortcut (CTRL-E) to the text edit.

if (!c)
return;

bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
if (!isShortcut && !ctrlOrShift && e->modifiers() != Qt::NoModifier)
{
c->popup()->hide();
return;
}

c->update(text());
c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
}

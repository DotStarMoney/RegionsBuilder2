#include "mainwin.h"
#include "ui_mainwin.h"
#include "regionedit.h"
#include <QGLFunctions>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <QDateTime>
#include <QFile>

mainwin::mainwin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mainwin)
{
    QDate       date;
    int         intDay;
    QFile       qotdFile;
    QTextStream fileRead;
    QString     quote;
    RegionEdit* tab;

    ui->setupUi(this);


    date = QDate::currentDate();
    intDay = date.dayOfYear() % 365;
    intDay -= 95;
    if(intDay < 0) intDay += 365;

    qotdFile.setFileName(":/misc/Qotd.txt");
    qotdFile.open(QIODevice::ReadOnly);
    fileRead.setDevice(&qotdFile);
    while(intDay >= 0 && !fileRead.atEnd()){
        quote = fileRead.readLine();
        intDay--;
    }

    ui->statusbar->showMessage(quote, 5000);

    tab = new RegionEdit(this);
    pages.push_back(tab);
    connect(tab, SIGNAL(setMessage(QString,int)), this, SLOT(receiveMessage(QString,int)));
    ui->EditorTabs->insertTab(0, pages[0], "Region-Untitled");
    ui->EditorTabs->removeTab(1);

    this->setGeometry(100, 50, this->width() * 1.5, this->height());

}

mainwin::~mainwin()
{
    delete ui;
}

void mainwin::receiveMessage(QString msg, int delay){

}

void mainwin::on_actionWhats_my_OpenGL_Version_triggered()
{
    QMessageBox alert;
    QString msg;
    msg = "Regions Builder detects OpenGL Version: ";
    msg += ((char*) glGetString(GL_VERSION));
    msg += "\nShader Language Version: ";
    msg += ((char*) glGetString(GL_SHADING_LANGUAGE_VERSION));
    alert.setText(msg);
    alert.setStandardButtons(QMessageBox::Ok);
    alert.setIcon(QMessageBox::Information);
    alert.setWindowTitle("OpenGL Version");
    alert.setWindowIcon(QIcon(":/misc/gfx/tree.png"));
    alert.exec();
}

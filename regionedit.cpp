#include "regionedit.h"
#include "ui_regionedit.h"
#include "qwidget_slicesheet.h"
#include "snapping_dialog.h"
#include <QDebug>
#include <QFile>
#include <QString>


RegionEdit::RegionEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegionEdit)
{

    ui->setupUi(this);
    connect(this, SIGNAL(setSnapParameters(float, float)), ui->widget_mainWidget, SLOT(getSnapParameters(float,float)));
    setSnapParameters(32.0, 32.0);
}


RegionEdit::~RegionEdit()
{
    delete ui;
}

void RegionEdit::receiveSnapParameters(float xs, float ys){
    setSnapParameters(xs, ys);
}

void RegionEdit::sliceButtonPushed()
{
    qwidget_sliceSheet sliceDialog( this );
    sliceDialog.setWindowTitle("Auto-Slice");

    connect(&sliceDialog, SIGNAL(sendPalette(QString)), ui->widget_mainWidget, SLOT(loadPalette(QString)));

    sliceDialog.exec();

}

void RegionEdit::on_pushButton_clicked()
{

}

void RegionEdit::on_toolButton_18_clicked()
{
    snapping_dialog snapDialog(this);
    snapDialog.setWindowTitle("Snap Settings");
    connect(&snapDialog, SIGNAL(setSnapParameters(float, float)), this, SLOT(receiveSnapParameters(float,float)));

    snapDialog.exec();
}

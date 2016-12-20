#include "qwidget_slicesheet.h"
#include "ui_qwidget_slicesheet.h"
#include <QString>
#include <QFileDialog>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

qwidget_sliceSheet::qwidget_sliceSheet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qwidget_sliceSheet)
{
    ui->setupUi(this);
    connect(ui->widget_woge, SIGNAL(endWaiting()), this, SLOT(preparePreview()));
    connect(&asyncProcessImage_watch, SIGNAL(finished()), this, SLOT(asyncProcessImage_end()));
    connect(&sliceBot, SIGNAL(reportProgress(int)), this, SLOT(update_progress(int)));
    ui->pushButton_3->setEnabled(false);
    working = false;
}

qwidget_sliceSheet::~qwidget_sliceSheet()
{
    delete ui;
}

void qwidget_sliceSheet::update_progress(int x){
    ui->progressBar_sliceActionProg->setValue(x);
}

void qwidget_sliceSheet::on_pushButton_clicked()
{
    openOutputFileDialog();
}


void qwidget_sliceSheet::preparePreview(){
    ui->pushButton_3->setEnabled(true);
}

void qwidget_sliceSheet::openOutputFileDialog(){
    QString filename;

    filename = QFileDialog::getOpenFileName(this, tr("Image to slice"), "", tr("Images (*.png *.bmp *.jpg)"));
    if(filename != ""){
        ui->lineEdit_2->setText(filename);
        ui->pushButton_3->setEnabled(false);
        ui->widget_woge->setNewImage(filename);
    }
}

void qwidget_sliceSheet::on_lineEdit_textChanged(const QString &arg1)
{
    QString slashReg;
    QString path;
    int slashLoc;
    slashReg = "/";
    path = ui->lineEdit_3->text();
    if((slashLoc = path.lastIndexOf(slashReg)) >= 0){
        path = path.left(slashLoc) + "/";
    }
    else{
        path = "";
    }
    ui->lineEdit_3->setText(path + arg1 + "_pal.rgn");
}

void qwidget_sliceSheet::on_pushButton_2_clicked()
{
    QString foldername;
    QString oldText;
    QString slashReg;
    int slashLoc;
    foldername = QFileDialog::getExistingDirectory(this, tr("Output directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    slashReg = "/";
    oldText = ui->lineEdit_3->text();
    if((slashLoc = oldText.lastIndexOf(slashReg)) < 0){
        foldername = foldername + "/";
    }
    oldText = oldText.right(oldText.length() - slashLoc);
    ui->lineEdit_3->setText(foldername + oldText);
}

void qwidget_sliceSheet::on_toolButton_clicked()
{
    float z;
    if(ui->widget_woge->getImageMode() == 2){
        z = ui->widget_woge->getZoom();
        z = qMax(1.0, z - 1.0);
        if(ui->widget_woge->getZoom() != z){
            ui->pushButton_3->setEnabled(false);
            ui->widget_woge->setParams(ui->widget_woge->getX(), ui->widget_woge->getY(), z);
        }
    }
}

void qwidget_sliceSheet::on_toolButton_2_clicked()
{
    float z;
    if(ui->widget_woge->getImageMode() == 2){
        z = ui->widget_woge->getZoom();
        z = qMin(10.0, z + 1.0);
        if(ui->widget_woge->getZoom() != z){
            ui->pushButton_3->setEnabled(false);
            ui->widget_woge->setParams(ui->widget_woge->getX(), ui->widget_woge->getY(), z);
        }
    }
}

void qwidget_sliceSheet::on_checkBox_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked){
        ui->widget_woge->setDrawMode(true, (ui->checkBox_2->checkState() == Qt::Checked) ? true : false);
    }
    else{
        ui->widget_woge->setDrawMode(false, (ui->checkBox_2->checkState() == Qt::Checked) ? true : false);
    }
}

void qwidget_sliceSheet::on_checkBox_2_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked){
        ui->widget_woge->setDrawMode((ui->checkBox->checkState() == Qt::Checked) ? true : false, true);
    }
    else{
        ui->widget_woge->setDrawMode((ui->checkBox->checkState() == Qt::Checked) ? true : false, false);
    }
}

void qwidget_sliceSheet::on_doubleSpinBox_valueChanged(double arg1)
{
    ui->widget_woge->setEdgeAlpga(arg1);
}

void qwidget_sliceSheet::on_pushButton_3_clicked()
{
    if(!working) sliceSheet();
}

void qwidget_sliceSheet::sliceSheet(){
    ui->pushButton_3->setEnabled(false);
    working = true;
    sliceBot.reset();
    ui->progressBar_sliceActionProg->setValue(0);
    asyncProcessImage_future = QtConcurrent::run(this, &qwidget_sliceSheet::asyncProcessImage);
    asyncProcessImage_watch.setFuture(asyncProcessImage_future);
}
void qwidget_sliceSheet::asyncProcessImage(){
    sliceBot.buildPalette(*(ui->widget_woge->fullImgPtr()),
                             ui->lineEdit_3->text(),
                             ui->doubleSpinBox->value(),
                             ui->doubleSpinBox_2->value(),
                             (ui->checkBox_4->checkState() == Qt::Checked) ? true : false,
                             (ui->checkBox_5->checkState() == Qt::Checked) ? false : true,
                             ui->spinBox->value());
}

void qwidget_sliceSheet::asyncProcessImage_end(){
    ui->pushButton_3->setEnabled(true);
    working = false;
    sendPalette(ui->lineEdit_3->text());
    if(ui->checkBox_3->checkState() == Qt::Unchecked) close();
}






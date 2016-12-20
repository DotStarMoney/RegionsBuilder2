#ifndef QWIDGET_SLICESHEET_H
#define QWIDGET_SLICESHEET_H

#include <QDialog>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include "autoslicer.h"

namespace Ui {
class qwidget_sliceSheet;
}

class qwidget_sliceSheet : public QDialog
{
    Q_OBJECT

public:
    explicit qwidget_sliceSheet(QWidget *parent = 0);
    ~qwidget_sliceSheet();

private:
    Ui::qwidget_sliceSheet *ui;
    void openOutputFileDialog();
    void sliceSheet();
    bool working;

    float zoomLevel;
    float zoomBounds[2];
    float zoomTick[2];

    QFutureWatcher<void> asyncProcessImage_watch;
    QFuture<void> asyncProcessImage_future;
    void asyncProcessImage();
    Autoslicer sliceBot;

private slots:
    void on_pushButton_clicked();
    void on_lineEdit_textChanged(const QString &arg1);
    void on_pushButton_2_clicked();
    void on_toolButton_clicked();
    void on_toolButton_2_clicked();
    void on_checkBox_stateChanged(int arg1);
    void on_checkBox_2_stateChanged(int arg1);
    void on_doubleSpinBox_valueChanged(double arg1);
    void on_pushButton_3_clicked();
    void update_progress(int x);
    void asyncProcessImage_end();

public slots:
    void preparePreview();

signals:
    void sendPalette(const QString &x);

};

#endif // QWIDGET_SLICESHEET_H

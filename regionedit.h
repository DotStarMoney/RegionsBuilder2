#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

namespace Ui {
class RegionEdit;
}

class RegionEdit : public QWidget
{
    Q_OBJECT

public:
    explicit RegionEdit(QWidget *parent = 0);
    ~RegionEdit();

private slots:
    void sliceButtonPushed();

    void on_pushButton_clicked();

    void on_toolButton_18_clicked();

public slots:
    void receiveSnapParameters(float xs, float ys);

private:
    Ui::RegionEdit *ui;

signals:
    void setMessage(QString msg, int delay);
    void setSnapParameters(float xs, float ys);


};

#endif // MAINWINDOW_H

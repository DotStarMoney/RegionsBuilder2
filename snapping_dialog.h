#ifndef SNAPPING_DIALOG_H
#define SNAPPING_DIALOG_H

#include <QDialog>

namespace Ui {
class snapping_dialog;
}

class snapping_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit snapping_dialog(QWidget *parent = 0);
    ~snapping_dialog();

private slots:
    void on_pushButton_clicked();

signals:
    void setSnapParameters(float xs, float ys);

private:
    Ui::snapping_dialog *ui;
};

#endif // SNAPPING_DIALOG_H

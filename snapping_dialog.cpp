#include "snapping_dialog.h"
#include "ui_snapping_dialog.h"

snapping_dialog::snapping_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::snapping_dialog)
{
    ui->setupUi(this);
}

snapping_dialog::~snapping_dialog()
{
    delete ui;
}

void snapping_dialog::on_pushButton_clicked()
{
    setSnapParameters(ui->doubleSpinBox->value(), ui->doubleSpinBox_2->value());
    this->close();
}

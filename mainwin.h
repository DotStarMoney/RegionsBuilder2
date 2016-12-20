#ifndef MAINWIN_H
#define MAINWIN_H

#include <QMainWindow>
#include <vector>
#include <QWidget>

namespace Ui {
class mainwin;
}

class mainwin : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainwin(QWidget *parent = 0);
    ~mainwin();

private slots:
    void on_actionWhats_my_OpenGL_Version_triggered();

public slots:
    void receiveMessage(QString msg, int delay);

private:
    Ui::mainwin *ui;
    std::vector<QWidget*> pages;
};

#endif // MAINWIN_H

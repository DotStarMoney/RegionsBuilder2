#ifndef ERRLOG_H
#define ERRLOG_H

#include <QVector>
#include <QString>

namespace ErrLog{

    enum errstatus_t{STATUS_WARNING, STATUS_ERROR, STATUS_OK};

    class Log
    {
    public:
        Log();
        errstatus_t status();
        QString lastMsg();
        QString getMsg(int i);
        int size();
    protected:
        void warning(QString w);
        void error(QString e);
    private:
        errstatus_t status_field;
        QVector<QString> mlog;
    };

}
#endif // ERRLOG_H

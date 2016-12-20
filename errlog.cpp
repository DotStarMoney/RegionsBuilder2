#include "errlog.h"

using namespace ErrLog;

Log::Log()
{
    status_field = STATUS_OK;
}

void Log::warning(QString w){
    mlog.push_back("WARNING: " + w);
    if(status_field != STATUS_ERROR) status_field = STATUS_WARNING;
}

void Log::error(QString e){
    mlog.push_back("ERROR: " + e);
    status_field = STATUS_ERROR;
}

ErrLog::errstatus_t Log::status(){
    return status_field;
}

QString Log::lastMsg(){
    if(mlog.size() > 0)
        return mlog[mlog.size() - 1];
    else
        return "NO ERROR.";
}

QString Log::getMsg(int i){
    if((mlog.size() > 0) && (i < mlog.size())){
        return mlog[i];
    }
    else{
        if(mlog.size() == 0) return "NO ERROR.";
        return "NO MESSAGE AT INDEX " + QString::number(i);
    }
}

int Log::size(){
    return mlog.size();
}

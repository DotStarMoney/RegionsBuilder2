#ifndef KEYBANK_H
#define KEYBANK_H

#include "errlog.h"
#include <QHash>

template<typename T>
class KeyBank<T> : public ErrLog
{
    public:
        KeyBank();
        T acquire(){
            int* addr;
            addr = new int;
            s.insert(addr, addr);
            return ((T) addr);
        }

        void relinquish(T key){
            int* k;
            k = (int*) key;
            if(s.contains(k)){
                s.remove(k);
            }
            else{
                warning("key does not exist");
            }
        }

    private:
        QHash<int*, int*> s;
};

#endif // KEYBANK_H

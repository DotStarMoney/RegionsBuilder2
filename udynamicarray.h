#ifndef UDYNAMICARRAY_H
#define UDYNAMICARRAY_H

#define COARSE_MEM_SCALE_START 5
#define COARSE_MEM_SCALE 20

#include <QDebug>

template<typename T>
class uDynamicArray
{
public:
    uDynamicArray(int c_scale = COARSE_MEM_SCALE_START,
                  int c_start = COARSE_MEM_SCALE){
        sz = 0;
        cap = c_start;
        m_data = (T*) malloc(sizeof(T) * cap);
        cM = c_scale;
    }
    ~uDynamicArray(){
        delete[] m_data;
    }
    void reset(int c){
        sz = 0;
        cap = c;
        m_data = (T*) realloc((void*) m_data, sizeof(T) * cap);
    }
    void reserve(int r){
        sz += r;
        if(sz > cap){
            while(cap < sz) cap += cM;
            m_data = (T*) realloc((void*) m_data, sizeof(T) * cap);
        }
    }
    void clear(){
        sz = 0;
    }

    void flush(int c_scale = COARSE_MEM_SCALE_START,
               int c_start = COARSE_MEM_SCALE){
        sz = 0;
        cap = c_start;
        m_data = (T*) malloc(sizeof(T) * cap);
        cM = c_scale;
    }

    int size(){
        return sz;
    }
    int capacity(){
        return cap;
    }
    T* data(){
        return m_data;
    }
private:
    T* m_data;
    int sz;
    int cap;
    int cM;
};

#endif // UDYNAMICARRAY_H

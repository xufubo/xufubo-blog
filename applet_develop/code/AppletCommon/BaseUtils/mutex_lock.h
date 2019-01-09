/*********************************************************************************
 FileName: 
 Author: xufubo
 Date:  2018-12-26
 Description: 
 Version:
 History:
     <author>        <time>          <version>          <desc>
 ********************************************************************************/

#ifndef __HYDRA_MUTEX_LOCK_H__
#define __HYDRA_MUTEX_LOCK_H__ 

#include <pthread.h>
#include "base_define.h"

namespace base_utils
{

class CMutex 
{
    friend class condition;

public:
    inline CMutex (void) 
    {
        ::pthread_mutex_init (&_mutex, 0);
    }

    inline void lock (void) 
    {
        ::pthread_mutex_lock (&_mutex);
    }

    inline void unlock (void) 
    {
        ::pthread_mutex_unlock (&_mutex);
    }

    inline ~CMutex (void) 
    {
        ::pthread_mutex_destroy (&_mutex);
    }

private:
    CMutex (const CMutex& m);
    CMutex& operator= (const CMutex &m);

private:
    pthread_mutex_t _mutex;
};


class CScopedLock 
{
    friend class condition;

public:
    inline CScopedLock (CMutex& mutex) : _mutex (mutex) 
    {
        _mutex.lock ();
    }

    inline ~CScopedLock (void) 
    {
        _mutex.unlock ();
    }

private:
    CMutex& _mutex;
};

}

#endif
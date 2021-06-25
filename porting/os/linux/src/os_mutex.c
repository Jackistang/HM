#include "os_type.h"
#include <assert.h>
#include <time.h>

int os_mutex_init(struct os_mutex *mutex)
{
    assert(mutex);

    int err;
    pthread_mutexattr_t mu_attr;
    
    if ((err = pthread_mutexattr_settype(&mu_attr, PTHREAD_MUTEX_NORMAL)))
        return err;

    if ((err = pthread_mutex_init(&mutex->lock, &mu_attr)))
        return err;

    return 0;
}

int os_mutex_take(struct os_mutex *mutex, os_time_t timeout)
{
    assert(mutex);

    int err;
    struct timespec wait;

    if ((err = clock_gettime(CLOCK_REALTIME, &wait)))
        return err;

    wait.tv_nsec += (timeout % 1000) * 1000000;
    wait.tv_sec  += timeout / 1000 + wait.tv_nsec / 1000000000;
    wait.tv_nsec %= 1000000000;

    if (timeout == OS_WAIT_FOREVER)
        err = pthread_mutex_lock(&mutex->lock);
    else
        err = pthread_mutex_timedlock(&mutex->lock, &wait);

    return err;
}

int os_mutex_release(struct os_mutex *mutex)
{
    assert(mutex);

    return pthread_mutex_unlock(&mutex->lock);
}
#include "os_type.h"
#include <assert.h>
#include <time.h>

int os_sem_init(struct os_sem *sem, uint16_t tokens)
{
    assert(sem);

    return sem_init(&sem->sem, 0, tokens);
}

int os_sem_take(struct os_sem *sem, os_time_t timeout)
{
    assert(sem);

    int err;
    struct timespec wait;

    if ((err = clock_gettime(CLOCK_REALTIME, &wait)))
        return err;
    
    wait.tv_nsec += (timeout % 1000) * 1000000;
    wait.tv_sec  += timeout / 1000 + wait.tv_nsec / 1000000000;
    wait.tv_nsec %= 1000000000;

    if (timeout == OS_WAIT_FOREVER)
        err = sem_wait(&sem->sem);
    else 
        err = sem_timedwait(&sem->sem, &wait);

    return err;
}

int os_sem_release(struct os_sem *sem)
{
    assert(sem);

    return sem_post(&sem->sem);
}
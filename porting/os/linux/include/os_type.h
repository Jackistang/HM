#ifndef HCI_MIDDLEWARE_OS_TYPES_H
#define HCI_MIDDLEWARE_OS_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_WAIT_FOREVER (-1)

typedef uint32_t os_time_t;
typedef int os_stack_t;

typedef void *(*os_task_func_t)(void *);


struct os_sem {
    sem_t                   sem;
};

struct os_mutex {
    pthread_mutex_t         lock;
};

struct os_task {
    pthread_t               handle;
    const char*             name;
};

#ifdef __cplusplus
}
#endif

#endif /* HCI_MIDDLEWARE_OS_TYPES_H */
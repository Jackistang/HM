#include "os_type.h"
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define UNUSED(n)   ((void)(sizeof(n)))

// A simple implement.
int os_task_create(struct os_task *t, const char *name, os_task_func_t func,
		 void *arg, uint8_t prio, os_time_t sanity_itvl, os_stack_t *stack_bottom, uint16_t stack_size)
{
    UNUSED(prio);
    UNUSED(sanity_itvl);
    UNUSED(stack_bottom);
    UNUSED(stack_size);

    assert(t);
    assert(func);

    int err;
    t->name = name;
    err = pthread_create(&t->handle, NULL, func, arg);

    return err;
}

int os_task_remove(struct os_task *t)
{
    return pthread_cancel(t->handle);
}


void os_sleep(int ms)
{
    sleep(ms);
}
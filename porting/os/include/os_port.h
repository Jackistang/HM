#ifndef HCI_MIDDLEWARE_OS_PORT_H
#define HCI_MIDDLEWARE_OS_PORT_H

#include "os_type.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief This function create a task, and run it.
 * 
 * @param t             The task to create.
 * @param name          Task name.
 * @param func          Task function entry.
 * @param arg           Task function arguments.
 * @param prio          Task priority.
 * @param stack_bottom  Task stack start address.
 * @param stack_size    Task stack size.
 * 
 * @return int
 * @retval  0        Success
 * @retval  -1       Fail
 * @retval  -EINVAL  Invalid Paramters
*/
extern int os_task_create(struct os_task *t, const char *name, os_task_func_t func,
		 void *arg, uint8_t prio, os_time_t sanity_itvl, os_stack_t *stack_bottom, uint16_t stack_size);

/**
 * @brief This function remove a task.
 * 
 * @param t The task handler.
 * 
 * @return int
 * @retval  0   Success
 * @retval  -1  Fail
*/
extern int os_task_remove(struct os_task *t);

/*
 * Semaphores
 */

/**
 * @brief Initial a semaphore.
 * 
 * @param sem       Semaphore handler.
 * @param tokens    Initial semaphore counts.
 * 
 * @return int
 * @retval  0   Success
 * @retval  -1  Fail
 * 
*/
extern int os_sem_init(struct os_sem *sem, uint16_t tokens);

/**
 * @brief   Take a semaphore.
 * 
 * @param sem       Semaphore handler.
 * @param timeout   Wait time (ms), or wait forever with `OS_WAIT_FOREVER`.
 * 
 * @return int
 * @retval  0           Success
 * @retval  -1          Fail
 * @retval  -ETIMEDOUT  Wait timeout.  
*/
extern int os_sem_take(struct os_sem *sem, os_time_t timeout);


/**
 * @brief   Release a semaphore.
 * 
 * @param sem   Semaphore handler.
 * 
 * @return int
 * @retval  0           Success
 * @retval  -1          Fail
*/
extern int os_sem_release(struct os_sem *sem);


/*
 * Mutexes
 */

/**
 * @brief Initial a mutex.
 * 
 * @param sem       Mutex handler.
 * 
 * @return int
 * @retval  0   Success
 * @retval  -1  Fail
 * 
*/
extern int os_mutex_init(struct os_mutex *mutex);

/**
 * @brief   Take a mutex.
 * 
 * @param sem       Mutex handler.
 * @param timeout   Wait time (ms), or wait forever with `OS_WAIT_FOREVER`.
 * 
 * @return int
 * @retval  0           Success
 * @retval  -1          Fail
 * @retval  -ETIMEDOUT  Wait timeout.  
*/
extern int os_mutex_take(struct os_mutex *mutex, os_time_t timeout);

/**
 * @brief   Release a mutex.
 * 
 * @param sem   Mutex handler.
 * 
 * @return int
 * @retval  0           Success
 * @retval  -1          Fail
*/
extern int os_mutex_release(struct os_mutex *mutex);


#ifdef __cplusplus
}
#endif

#endif /* HCI_MIDDLEWARE_OS_PORT_H */
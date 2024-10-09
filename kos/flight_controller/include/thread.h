/**
 * @file
 * @brief KasperskyOS Thread interface.
 *
 * @copyright
 *   Copyright 2018 AO Kaspersky Lab. All Rights Reserved.
 *   Registered trademarks and service marks are the property of their
 *   respective owners.
 */
#ifndef KOS_THREAD_H
#define KOS_THREAD_H
#pragma once

#include <rtl/compiler.h>
#include <rtl/list.h>
#include <rtl/atomic.h>

#include <coresrv/time/time_api.h>
#include <coresrv/thread/thread_api.h>

__RTL_BEGIN_DECLS

#define KOS_THREAD_ONCE_INIT 0

/**
 * Defines the internal type of variable controlling once-only
 * execution with the #KosThreadOnce function.
 */
typedef RtlAtomicInt KosThreadOnceState;

/** Defines the reasons for the thread callback routine. */
typedef enum KosThreadCallbackReason {
    KosThreadCallbackReasonCreate,  /**< thread creation */
    KosThreadCallbackReasonDestroy, /**< thread destruction */
} KosThreadCallbackReason;

/** Defines the thread creation/deletion callback. */
typedef void KosThreadCallback(KosThreadCallbackReason reason);

/**
 * Registers the thread creation/deletion callback routine.
 *
 * @param[in]   callback    thread callback
 *
 * @return                  return code
 */
Retcode KosThreadCallbackRegister(KosThreadCallback *callback);

/**
 * Unregisters the thread creation/deletion callback routine.
 *
 * @param[in]   callback    thread callback
 *
 * @return                  return code
 */
Retcode KosThreadCallbackUnregister(KosThreadCallback *callback);

/**
 * Creates a thread.
 *
 * @param[out]  tid         thread ID of the created thread
 * @param[in]   priority    priority of a thread
 * @param[in]   stackSize   size of a stack for the created thread
 * @param[in]   routine     thread's entry point
 * @param[in]   context     the argument passed to the thread
 * @param[in]   suspended   0 - normal creation, 1 - suspended
 *
 * @return                  rcOk if success
 */
Retcode KosThreadCreate(Tid                *tid,
                        rtl_uint32_t        priority,
                        rtl_uint32_t        stackSize,
                        ThreadRoutine       routine,
                        void               *context,
                        int                 suspended);

/**
 * Returns the thread ID (TID) of the calling thread.
 *
 * @kos_thread_only
 *
 * @return              thread ID
 */
Tid KosThreadCurrentId(void);

/**
 * Suspends execution of the specified thread.
 *
 * @param[in]   tid     thread ID of the thread to be suspended
 *
 * @return              rcOk if success
 */
Retcode KosThreadSuspend(Tid tid);

/**
 * Resumes execution of the specified thread.
 *
 * @param[in]   tid     thread ID of the thread to be resumed
 *
 * @return              rcOk if success
 */
Retcode KosThreadResume(Tid tid);

/**
 * Terminates the current thread.
 *
 * @param[in]   exitCode    exit code
 */
void KosThreadExit(rtl_int32_t exitCode);

/**
 * Waits for the specified thread to terminate.
 *
 * @param[in]   tid         thread ID
 * @param[in]   timeout     wait timeout in milliseconds
 *
 * @return                  rcOk if success or rcTimeout if timeout
 */
int KosThreadWait(rtl_uint32_t tid, rtl_uint32_t timeout);

/**
 * Makes the calling thread sleep until the specified delay has elapsed.
 *
 * @param[in]   mdelay      time to sleep in milliseconds
 *
 * @return                  rcOk if success
 */
Retcode KosThreadSleep(rtl_uint32_t mdelay);

/**
 * Yields execution of the calling thread to another.
 */
void KosThreadYield(void);

/**
 * Terminates execution of the specified thread.
 *
 * @param[in]   tid         identifier of the thread to terminate
 * @param[in]   exitCode    exit code of the thread routine
 *
 * @return                  rcOk if success
 */
Retcode KosThreadTerminate(Tid tid, rtl_int32_t exitCode);

/**
 * Gets the base address of thread local storage (TLS).
 *
 * @kos_thread_only
 *
 * @return              pointer to a thread local storage
 *                      RTL_NULL if there is no TLS
 */
void *KosThreadTlsGet(void);

/**
 * Sets thread local storage.
 *
 * @kos_thread_only
 *
 * @param[in]   tls     address of TLS to set to the current thread
 *
 * @return              rcOk if success
 */
Retcode KosThreadTlsSet(void *tls);

/**
 * Gets the thread stack.
 *
 * @param[in]   tid     id of the thread to get the stack
 * @param[out]  size    pointer that will be initialized with the stack size
 *
 * @return              pointer to the requested thread stack
 */
void *KosThreadGetStack(Tid tid, rtl_uint32_t *size);

/**
 * Ensures once-only execution of a certain routine.
 *
 * Function checks if it was called the first time with the \a onceControl
 * variable and if so, invokes the initRoutine() function. Otherwise, the
 * initRoutine() function is not called and the function just returns success.
 *
 * @param[in]   onceControl     variable of type #KosThreadOnceState
 * @param[in]   initRoutine     function that should only be called once
 *
 * @return                      rcOk if success
 */
Retcode KosThreadOnce(KosThreadOnceState       *onceControl,
                      void                   (* initRoutine) (void));

__RTL_END_DECLS

#endif /* KOS_THREAD_H */

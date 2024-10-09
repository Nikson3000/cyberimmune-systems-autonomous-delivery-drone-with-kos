/**
 * @file
 * @brief Native Thread API.
 *
 * @copyright
 *   Copyright 2018 AO Kaspersky Lab. All Rights Reserved.
 *   Registered trademarks and service marks are the property of their
 *   respective owners.
 */
#ifndef CORESRV_THREAD_THREAD_API_H
#define CORESRV_THREAD_THREAD_API_H
#pragma once

#include <rtl/compiler.h>
#include <rtl/retcode.h>
#include <rtl/rtc.h>
#include <rtl/stddef.h>
#include <rtl/stdint.h>
#include <rtl/cpuset.h>

#include <hal/page.h>

#include <thread/tidtype.h>
#include <thread/tcbpage.h>
#include <handle/handletype.h>

/* Represents invalid tid value (this is for backward compatibility only, use
 * INVALID_TID directly). */
#define InvalidTid              (INVALID_TID)

/** Defines the default stack size. (1 Megabyte) */
#define ThreadStackSizeDefault  (0x100000  /* 1 Mb */)

/** Defines the minimal stack size. (one page) */
#define ThreadStackSizeMin      (PAGE_SIZE)

/**
 * Defines the thread priority values.
 */
typedef enum EThreadPriority {
    ThreadPriorityLowest  =  0, /**< lowest priority */
    ThreadPriorityNormal  = 10, /**< normal priority */
    ThreadPriorityHighest = 15, /**< highest priority  */
} ThreadPriority;

/**
 * Defines the thread creation flags.
 */
enum EThreadCreationFlags {
    /** thread is suspended on creation */
    ThreadFlagCreateSuspended  = THREAD_CREATE_SUSPENDED,
    /** thread is created as waitable   */
    ThreadFlagCreateWaitable   = THREAD_CREATE_WAITABLE,
    /** thread is scheduled by default scheduler */
    ThreadFlagCreateSchedOther = THREAD_CREATE_SCHED_OTHER,
    /** thread is scheduled by FIFO scheduler */
    ThreadFlagCreateSchedFifo = THREAD_CREATE_SCHED_FIFO,
    /** thread is scheduled by Round-Robin scheduler */
    ThreadFlagCreateSchedRR   = THREAD_CREATE_SCHED_RR,
};

/**
 * Defines type of parameter for the thread scheduler.
 */
typedef union {
    RtlTimeSpec rrInterval;
} ThreadSchedParam;

/** Defines the thread routine callback type. */
typedef int (* ThreadRoutine) (void *context);

__RTL_BEGIN_DECLS

/**
 * Creates a thread handle.
 *
 * @param[out]  thread          handle of the created thread
 * @param[out]  tid             ID of the created thread
 * @param[in]   priority        priority of the thread, see the definition
 *                              of #EThreadPriority enum
 * @param[in]   stackSize       size of thread's user-mode stack
 *                              or 0 to use default
 * @param[in]   startRoutine    function to call at thread start
 *                              or RTL_NULL to use the default start routine
 * @param[in]   routine         thread's routine
 * @param[in]   context         context to pass to thread's routine
 *                              or RTL_NULL
 * @param[in]   flags           see the definition of #EThreadCreationFlags
 *                              enum.
 *
 * @return                      rcOk if success
 */
Retcode KnThreadCreateByHandle(Handle              *thread,
                               Tid                 *tid,
                               ThreadPriority       priority,
                               rtl_uint32_t         stackSize,
                               void              (* startRoutine) (void),
                               ThreadRoutine        routine,
                               void                *context,
                               rtl_uint32_t         flags);

/**
 * Attach to the thread by TID.
 *
 * @param[in]   tid             id of the thread
 * @param[in]   rights          mask of handle rights
 * @param[out]  thread          handle of the attached thread
 *
 * @return                      rcOk if success
 */
Retcode KnThreadAttach(Tid tid, rtl_uint32_t rights, Handle *thread);

/**
 * Gets the priority of the specified thread by handle.
 *
 * @param[in]   thread     handle of the thread
 * @param[out]  priority   priority of the thread, see the definition
 *                         of #EThreadPriority enum.
 *
 * @return                 rcOk if success
 */
Retcode KnThreadGetPriorityByHandle(Handle thread, ThreadPriority *priority);

/**
 * Sets the priority of the specified thread by handle.
 *
 * @param[in]   thread     handle of the thread
 * @param[in]   priority   priority of the thread, see the definition
 *                         of #EThreadPriority enum.
 *
 * @return                 rcOk if success
 */
Retcode KnThreadSetPriorityByHandle(Handle thread, ThreadPriority priority);

/**
 * Suspends execution of the current thread.
 *
 * @return              rcOk if success
 */
Retcode KnThreadSuspendCurrent(void);

/**
 * Resumes a thread.
 *
 * The thread must be previously created in suspended state through
 * the #KnThreadCreate() call.
 *
 * @param[in]   thread  handle of the thread
 *
 * @return              rcOk if success
 */
Retcode KnThreadResumeByHandle(Handle thread);

/**
 * Terminates execution of the specified thread by handle.
 *
 * @param[in]   thread  handle of the thread
 * @param[in]   code    thread exit code
 *
 * @return              rcOk if success
 */
Retcode KnThreadTerminateByHandle(Handle thread, rtl_uint32_t code);

/**
 * Describes thread information.
 */
typedef struct SKnThreadInfo {
    rtl_size_t stackSize; /**< stack size */
    void *stackStart;     /**< stack base address  */
} KnThreadInfo;

/**
 * Gets thread information by handle.
 *
 * @param[in]   thread      handle of the thread
 * @param[out]  info        information about the thread
 *
 * @return                  rcOk if success
 */
Retcode KnThreadGetInfoByHandle(Handle thread, KnThreadInfo *info);

/**
 * Suspends execution of the calling thread until the specified thread
 * has finished execution by handle.
 *
 * @param[in]   thread      handle of the thread
 * @param[in]   msDelay     delay in milliseconds
 * @param[out]  exitCode    thread exit code
 *
 * @return                  rcOk if success
 */
Retcode KnThreadWaitByHandle(Handle thread,
                             rtl_uint32_t msDelay,
                             rtl_uint32_t *exitCode);

/**
 * Gets thread CPU affinity mask by handle.
 *
 * @param[in]   thread  handle of the thread
 * @param[out]  mask    thread affinity mask
 *
 * @return              rcOk if success
 */
Retcode KnThreadGetAffinityByHandle(Handle thread, RtlCpuSet *mask);

/**
 * Configures thread CPU affinity mask by handle.
 *
 * @param[in]   thread  handle of the thread
 * @param[in]   mask    thread affinity mask
 *
 * @return              rcOk if success
 */
Retcode KnThreadSetAffinityByHandle(Handle thread, const RtlCpuSet *mask);

/**
 * Gets thread scheduling configuration by handle.
 *
 * @param[in]   thread  handle of the thread
 * @param[out]  policy  thread schedling policy
 * @param[out]  param   scheduling policy configuration -
 *                      may be RTL_NULL to avoid requesting
 *                      must be RTL_NULL if class does not support one
 *
 * @return              rcOk if success
 */
Retcode KnThreadGetSchedPolicyByHandle(Handle             thread,
                                       ThreadSchedPolicy *policy,
                                       ThreadSchedParam  *param);

/**
 * Configures thread scheduling by handle.
 *
 * @param[in]   thread  handle of the thread
 * @param[in]   policy  thread schedling policy
 * @param[in]   prio    priority of the thread
 * @param[in]   param   scheduling policy configuration -
 *                      may be RTL_NULL to use the default if policy
 *                      supports it, must be RTL_NULL if policy does
 *                      not support one
 *
 * @return              rcOk if success
 */
Retcode KnThreadSetSchedPolicyByHandle(Handle                  thread,
                                       ThreadSchedPolicy       policy,
                                       ThreadPriority          prio,
                                       const ThreadSchedParam *param);

/**
 * Creates a thread.
 *
 * @param[out]  tid             ID of the created thread
 * @param[in]   priority        priority of the thread, see the definition
 *                              of #EThreadPriority enum.
 * @param[in]   stackSize       size of thread's user-mode stack
 *                              or 0 to use default
 * @param[in]   startRoutine    function to call at thread start
 *                              or RTL_NULL to use the default start routine
 * @param[in]   routine         thread's routine
 * @param[in]   context         context to pass to thread's routine
 *                              or RTL_NULL
 * @param[in]   flags           see the definition of #EThreadCreationFlags
 *                              enum.
 *
 * @return                      rcOk if success
 */
Retcode KnThreadCreate(Tid                 *tid,
                       ThreadPriority       priority,
                       rtl_uint32_t         stackSize,
                       void              (* startRoutine) (void),
                       ThreadRoutine        routine,
                       void                *context,
                       rtl_uint32_t         flags);

/**
 * Gets the priority of the specified thread.
 *
 * @param[in]   tid        id of the thread to get priority
 * @param[out]  priority   priority of the thread, see the definition
 *                         of #EThreadPriority enum.
 *
 * @return                 rcOk if success
 */
Retcode KnThreadGetPriority(Tid tid, ThreadPriority *priority);

/**
 * Sets the priority of the specified thread.
 *
 * @param[in]   tid        id of the thread to set priority
 * @param[in]   priority   priority of the thread, see the definition
 *                         of #EThreadPriority enum.
 *
 * @return                 rcOk if success
 */
Retcode KnThreadSetPriority(Tid tid, ThreadPriority priority);

/**
 * Suspends execution of the specified running thread.
 *
 * @param[in]   tid     id of the thread to suspend
 *
 * @return              rcOk if success
 */
Retcode KnThreadSuspend(Tid tid);

/**
 * Resumes a thread.
 *
 * The thread must be previously created in suspended state through
 * the #KnThreadCreate() call.
 *
 * @param[in]   tid     id of the thread to resume
 *
 * @return              rcOk if success
 */
Retcode KnThreadResume(Tid tid);

/**
 * Terminates execution of the specified thread.
 *
 * @param[in]   tid     id of the thread to terminate
 * @param[in]   code    thread exit code
 *
 * @return              rcOk if success
 */
Retcode KnThreadTerminate(Tid tid, rtl_uint32_t code);

/**
 * Terminates the calling thread.
 *
 * @param[in]   code    thread exit code
 *
 * @return              rcOk if success
 */
Retcode KnThreadExit(rtl_uint32_t code);

/**
 * Gets thread information.
 *
 * @param[in]   tid         id of the thread
 * @param[out]  info        information about the thread
 *
 * @return                  rcOk if success
 */
Retcode KnThreadGetInfo(Tid tid, KnThreadInfo *info);

/**
 * Suspends execution of the calling thread until the specified thread
 * has finished execution.
 *
 * @param[in]   tid         id of the thread to wait for
 * @param[in]   msDelay     delay in milliseconds
 * @param[out]  exitCode    thread exit code
 *
 * @return                  rcOk if success
 */
Retcode KnThreadWait(Tid tid, rtl_uint32_t msDelay, rtl_uint32_t *exitCode);

/**
 * Suspends execution of the calling thread until the specified delay
 * has elapsed.
 *
 * @param[in]   mdelay      time to sleep in milliseconds
 *
 * @return                  rcOk if success
 */
Retcode KnSleep(rtl_uint32_t mdelay);

/**
 * Returns the ID of the calling thread.
 *
 * @return              id of the current thread
 */
Tid KnThreadCurrent(void);

struct STcbPageHead;

/**
 * Returns the TCB (Thread Control Block) of the calling thread.
 *
 * TCB is a reserved memory area that describes the thread.
 *
 * @return              pointer to TCB memory of the current thread
 */
struct STcbPageHead *KnThreadGetTcb(void);

/**
 * Sets the TLS (Thread Local Storage) of the calling thread.
 *
 * @param[in]   tls     address of TLS to set
 *
 * @return              rcOk if success
 */

Retcode KnThreadSetTls(void *tls);

/**
 * Terminates irq servicing in the current thread.
 *
 * @return              rcOk if success
 */
Retcode KnThreadDetachIrq(void);

/**
 * Registers the user per-thread exception handler routine.
 *
 * @param[in]   handler     the function to be called in user
 *                          mode after exception
 *
 * @return                  previous handler address
 */
void *KnThreadSetExceptionHandler(void (* handler)(void));

/**
 * Gets last exception info.
 *
 * The function copies information about the last exception
 * of the calling thread to the specified address.
 *
 * @param[in]   exception   destination address of information
 *                          about exception
 */
void KnThreadGetLastException(ExceptionInfo *exception);

/**
 * Gets thread CPU affinity mask.
 *
 * @param[in]   tid     id of the thread
 * @param[out]  mask    thread affinity mask
 *
 * @return              rcOk if success
 */
Retcode KnThreadGetAffinity(Tid tid, RtlCpuSet *mask);

/**
 * Configures thread CPU affinity mask.
 *
 * @param[in]   tid     id of the thread
 * @param[in]   mask    thread affinity mask
 *
 * @return              rcOk if success
 */
Retcode KnThreadSetAffinity(Tid tid, const RtlCpuSet *mask);

/**
 * Gets thread scheduling configuration.
 *
 * @param[in]   tid     id of the thread
 * @param[out]  policy  thread schedling policy
 * @param[out]  param   scheduling policy configuration -
 *                      may be RTL_NULL to avoid requesting
 *                      must be RTL_NULL if class does not support one
 *
 * @return              rcOk if success
 */
Retcode KnThreadGetSchedPolicy(Tid                tid,
                               ThreadSchedPolicy *policy,
                               ThreadSchedParam  *param);

/**
 * Configures thread scheduling.
 *
 * @param[in]   tid     id of the thread
 * @param[in]   policy  thread schedling policy
 * @param[in]   prio    priority of the thread
 * @param[in]   param   scheduling policy configuration -
 *                      may be RTL_NULL to use the default if policy
 *                      supports it, must be RTL_NULL if policy does
 *                      not support one
 *
 * @return              rcOk if success
 */
Retcode KnThreadSetSchedPolicy(Tid                     tid,
                               ThreadSchedPolicy       policy,
                               ThreadPriority          prio,
                               const ThreadSchedParam *param);

__RTL_END_DECLS

#endif /* CORESRV_THREAD_THREAD_API_H */

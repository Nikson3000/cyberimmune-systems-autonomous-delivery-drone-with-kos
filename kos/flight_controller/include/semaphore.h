/**
 * @file
 * @brief Interface to Native KasperskyOS Semaphores.
 *
 * @copyright
 *   Copyright 2018 AO Kaspersky Lab. All Rights Reserved.
 *   Registered trademarks and service marks are the property of their
 *   respective owners.
 */
#ifndef KOS_SEMAPHORE_H
#define KOS_SEMAPHORE_H
#pragma once

#include "sync_types.h"

#include <rtl/compiler.h>
#include <rtl/retcode.h>

#include <coresrv/time/time_api.h>

__RTL_BEGIN_DECLS

/**
 * Initializes a semaphore.
 *
 * @param[in,out]   semaphore   pointer to the semaphore
 * @param[in]       count       semaphore initial count
 *
 * @return                      rcOk if successful;
 *                              rcInvalidArgument if the semaphore does not
 *                              refer to a valid semaphore;
 *                              rcFail if the count exceeds
 *                              #KOS_SEMAPHORE_VALUE_MAX
 */
Retcode KosSemaphoreInit(KosSemaphore *semaphore, unsigned count);

/**
 * Frees resources assigned with the semaphore.
 *
 * @note Only the semaphore that was initialized using #KosSemaphoreInit()
 * may be destroyed using #KosSemaphoreDeinit().
 *
 * @note According to POSIX specification:
 * It is safe to destroy an initialized semaphore upon which no threads are
 * currently blocked. The effect of destroying the semaphore upon which other
 * threads arecurrently blocked is undefined.
 *
 * @param[in,out]   semaphore   pointer to the semaphore
 *
 * @return                      rcOk if successful;
 *                              rcInvalidArgument if the semaphore does not
 *                              refer to a valid semaphore;
 *                              rcBusy if currently there are threads
 *                              blocked on the semaphore
 */
Retcode KosSemaphoreDeinit(KosSemaphore *semaphore);

/**
 * Signals a semaphore.
 *
 * @param[in,out]   semaphore   pointer to the semaphore
 *
 * @return                      rcOk if successful;
 *                              rcInvalidArgument if the semaphore does not
 *                              refer to a valid semaphore
 */
Retcode KosSemaphoreSignal(KosSemaphore *semaphore);

/**
 * Signals a semaphore N times.
 *
 * @param[in,out]   semaphore   pointer to the semaphore
 * @param[in]       n           number of times to signal the semaphore
 *
 * @return                      rcOk if successful;
 *                              rcInvalidArgument if the semaphore does not
 *                              refer to a valid semaphore or if n is
 *                              lower then one
 */
Retcode KosSemaphoreSignalN(KosSemaphore *semaphore, int n);

/**
 * Waits for a semaphore with timeout.
 *
 * @param[in,out]   semaphore   pointer to the semaphore
 * @param[in]       mdelay      timeout in milliseconds
 *
 * @return                      rcOk if successful;
 *                              rcInvalidArgument if the semaphore does not
 *                              refer to a valid semaphore;
 *                              rcTimeout if the semaphore could not be
 *                              locked before the specified timeout expires
 */
Retcode KosSemaphoreWaitTimeout(KosSemaphore *semaphore, rtl_uint32_t mdelay);

/**
 * Waits for a semaphore.
 *
 * @param[in,out]   semaphore   pointer to the semaphore
 *
 * @return                      rcOk if successful;
 *                              rcInvalidArgument if the semaphore does not
 *                              refer to a valid semaphore;
 */
Retcode KosSemaphoreWait(KosSemaphore *semaphore);

/**
 * Attempts to wait for a semaphore.
 *
 * @param[in,out]   semaphore   pointer to the semaphore
 *
 * @return                      rcOk if successful;
 *                              rcInvalidArgument if the semaphore does not
 *                              refer to a valid semaphore;
 *                              rcBusy if the semaphore was already locked,
 *                              so it cannot be immediately locked by the
 *                              #KosSemaphoreTryWait() call
 */
Retcode KosSemaphoreTryWait(KosSemaphore *semaphore);

__RTL_END_DECLS

#endif /* KOS_SEMAPHORE_H */


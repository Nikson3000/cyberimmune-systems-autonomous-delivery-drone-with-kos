/**
 * @file
 * @brief File describing data structures for synchronization primitives.
 *
 * @copyright
 *   Copyright 2018 AO Kaspersky Lab. All Rights Reserved.
 *   Registered trademarks and service marks are the property of their
 *   respective owners.
 */
#ifndef KOS_SYNC_TYPES_H
#define KOS_SYNC_TYPES_H
#pragma once

#include <rtl/stdint.h>
#include <rtl/atomic.h>

/**
 * Defines the list of Mutex states.
 */
typedef enum {
    KOS_MUTEX_FREE      = 0, /**< Mutex is free     */
    KOS_MUTEX_LOCKED    = 1, /**< Mutex is locked   */
    KOS_MUTEX_WAIT      = 2  /**< Mutex is waiting  */
} KosMutexState;

/**
 * Describes the internal representation of Mutex.
 */
typedef struct SMutex {
    /* FIXME(perevertkin): the union should have _Atomic qualifier */
    union {
        struct {
            rtl_uint32_t  state:7;     /**< Mutex state                     */
            rtl_uint32_t  recursive:1; /**< type of Mutex: recursive or
                                        *   non-recursive                   */
            rtl_uint32_t  count:8;     /**< lock count for recursive Mutex  */
            rtl_uint32_t  owner:16;    /**< ID of owner thread              */
        } s;
        int value;                     /**< internal Mutex value            */
    };
} KosMutex;

/**
 * Initializes the #KosMutex object statically selecting its scheme.
 */
#define  KosInitializedMutexEx(_recursive) { { {                             \
    /* .s.state = */            KOS_MUTEX_FREE,                              \
    /* .s.recursive = */        (_recursive),                                \
    /* .s.owner = */            0,                                           \
    /* .s.count = */            0,                                           \
} } }

/**
 * Initializes the #KosMutex object statically.
 */
#define  KosInitializedMutex KosInitializedMutexEx(0)

/**
 * Initializes the #KosMutex object statically with recursive scheme.
 */
#define  KosInitializedRecursiveMutex KosInitializedMutexEx(1)

/**
 * Describes the internal representation of Event.
 */
typedef struct SEvent {
    RtlAtomicInt signal; /**< Event state: signaled or non-signaled */
} KosEvent;

/**
 * Initializes the #KosEvent object statically.
 */
#define KosInitializedEvent { 0 }

/**
 * Describes the internal representation of Semaphore.
 */
typedef struct SSemaphore {
    RtlAtomicInt count;    /**< Semaphore counter                        */
    RtlAtomicInt sleeping; /**< number of threads waiting for Semaphore  */
} KosSemaphore;

/**
 * Initializes the #KosSemaphore object statically.
 */
#define KosInitializedSemaphore { 0, 0 }

/** Defines the maximum value of the semaphore counter. */
#define KOS_SEMAPHORE_VALUE_MAX     RTL_INT32_MAX

/**
 * Describes the internal representation of Conditional Variable.
 */
typedef struct SCondvar {
    RtlAtomicInt cvar; /**< current value of the conditional variable */
} KosCondvar;

/**
 * Initializes the #KosCondvar object statically.
 */
#define KosInitializedCondvar { 0 }

/**
 * Describes the internal representation of RWLock.
 */
typedef struct SRWlock {
    KosMutex      lock;          /**< Mutex for locking                 */
    rtl_uint32_t  writerWake;    /**< number of awakened writing
                                  *   threads                           */
    rtl_uint32_t  readersWake;   /**< number of awakened reading
                                  *   threads                           */
    int           writer;        /**< number of writing threads         */
    int           readers;       /**< number of reading threads         */
    int           queuedWriters; /**< number of writing threads
                                  *   queued at #KosRWLockWrite         */
    int           queuedReaders; /**< number of reading threads queued
                                  *   at #KosRWLockRead                 */
} KosRWLock;

/**
 * Initializes the #KosRWLock object statically.
 */
#define KosInitializedRWLock {                                               \
    .lock =          KosInitializedMutex,                                    \
    .writerWake =    0,                                                      \
    .readersWake =   0,                                                      \
    .writer =        0,                                                      \
    .readers =       0,                                                      \
    .queuedWriters = 0,                                                      \
    .queuedReaders = 0,                                                      \
}

#endif /* KOS_SYNC_TYPES_H */

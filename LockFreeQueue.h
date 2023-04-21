/******************************************************************************
**                                                                           **
** Copyright (C) Joyson Electronics (2022)                                   **
**                                                                           **
** All rights reserved.                                                      **
**                                                                           **
** This document contains proprietary information belonging to Joyson        **
** Electronics. Passing on and copying of this document, and communication   **
** of its contents is not permitted without prior written authorization.     **
**                                                                           **
******************************************************************************/

/*!
 * @file LockFreeQueue.h
 * @author haifeng.liu
 * @date 2022-08-19
 * @brief This file contains the declaration of the LockFreeQueue functions
 * @details none
 */

#pragma once

#ifndef __cplusplus
#    error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

#include <unistd.h>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <semaphore.h>
#include <dlt/dlt.h>

/*---------------------------------------------------------------------------*/
// Namesapce
namespace jqueue
{

/**
 * @class Semaphore
 *
 * @brief Interface to operate the Semaphore
 *
 */
class Semaphore
{
public:
    Semaphore(int32_t initialCount = 0)
    {
        sem_init(&m_sema, 0, initialCount);
    }

    ~Semaphore()
    {
        sem_destroy(&m_sema);
    }

    inline uint64_t getTimeConvSeconds(timespec* curTime, uint32_t factor)
    {
        clock_gettime(CLOCK_MONOTONIC, curTime);
        return static_cast<uint64_t>(curTime->tv_sec) * factor;
    }

    uint64_t getMonnotonicTime()
    {
        timespec curTime;
        uint64_t result = getTimeConvSeconds(&curTime, 1000000);
        result += static_cast<uint32_t>(curTime.tv_nsec) / 1000;
        return result;
    }

    bool wait(size_t timeout)
    {
        const size_t timeoutUs = timeout * 1000;
        const size_t maxTimewait = 10000;
        size_t timewait = 10;
        size_t delayUs = 0;
        const uint64_t startUs = getMonnotonicTime();
        uint64_t elapsedUs = 0;
        int ret = 0;

        do {
            if (sem_trywait(&m_sema) == 0) {
                return true;
            }

            if (errno != EAGAIN) {
                return false;
            }

            delayUs = timeoutUs - elapsedUs;
            timewait = std::min(delayUs, timewait);
            ret = usleep(timewait);
            if (ret != 0) {
                return false;
            }

            timewait *= 2;
            timewait = std::min(timewait, maxTimewait);
            elapsedUs = getMonnotonicTime() - startUs;
        } while (elapsedUs <= timeoutUs);

        return false;
    }

    bool timed_wait(std::uint64_t usecs)
    {
        return wait(usecs);
    }

    void signal()
    {
        sem_post(&m_sema);
    }

private:
    sem_t m_sema;

private:
    Semaphore(const Semaphore& other);
    Semaphore& operator=(const Semaphore& other);

};

/**
 * @class LockFreeQueue
 *
 * @brief Interface to operate the LockFreeQueue.
 *
 */
template<typename T>
class LockFreeQueue
{
public:
    /**
     * @class node
     *
     * @brief Interface to operate the node .
     *
     */
    struct node
    {
        T data;
        std::atomic<node*> next;
        node(T const& data_)
        : data(data_)
        , next(nullptr)
        {

        }
    };

public:
    LockFreeQueue()
    : m_head(nullptr)
    , m_tail(nullptr)
    , m_sema(new Semaphore())
    , m_quit(false)
    {
    }

    ~LockFreeQueue()
    {
        m_quit = true;
        if (NULL != m_sema) {
            m_sema->signal();
        }
    }

public:
    void enqueue(T const& data)
    {
        if (NULL == m_sema) {
            return;
        }
        node* const newNode = new node(data);
        if (NULL == newNode) {
            return;
        }

        while (!m_quit) {
            node* taild = m_tail.load();
            if (taild == NULL) {
                node *tmpvalue = NULL;
                if (m_head.compare_exchange_weak(tmpvalue, newNode)) {
                    m_tail.store(newNode);
                    m_sema->signal();
                    return;
                }
                else {
                }
            }
            else {

                if (m_tail.compare_exchange_weak(taild, newNode)) {
                    taild->next.store(newNode);
                    m_sema->signal();
                    return;
                }
                else {
                }
            }

        }
        delete newNode;
    }

    bool wait_dequeue_timed(T& item, uint32_t timeout_usecs)
    {
        if (NULL == m_sema) {
            return false;
        }
        if (!m_sema->timed_wait(timeout_usecs)) {
            return false;
        }
        if (m_quit) {
            return false;
        }
        while (!m_quit) {
            node*  tailed = m_tail.load();
            node*  headed = m_head.load();
            if (tailed == NULL) {
                item = NULL;
                return false;
            }
            else if (headed == tailed) {
                node *tmpvalue = NULL;
                if (m_tail.compare_exchange_weak(tailed, tmpvalue)) {
                    m_head.store(NULL);
                    item = headed->data;
                    delete headed;
                    return true;
                }
                else {
                }

            }
            else {

                node* headedNext = headed->next.load();

                if (headedNext != NULL && m_head.compare_exchange_weak(headed, headedNext)) {
                    item = headed->data;
                    delete headed;
                    return true;
                }
                else {
                }
            }

        }
        return true;
    }

private:
    std::atomic<node*> m_head;
    std::atomic<node*> m_tail;

    std::shared_ptr<Semaphore> m_sema;
    bool m_quit;
};

/*---------------------------------------------------------------------------*/
}  // Namespace 
/* EOF */

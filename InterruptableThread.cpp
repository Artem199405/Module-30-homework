#include "InterruptableThread.h"

thread_local bool thread_interrupt_flag = false;

InterruptableThread::InterruptableThread(ThreadPool* pool, int qindex) :
    m_pFlag(nullptr),
    m_thread(&InterruptableThread::startFunc, this, pool, qindex) {
}

InterruptableThread::~InterruptableThread() {
    m_thread.join();
}

void InterruptableThread::interrupt() {
    lock_guard<mutex> l(m_defender);
    if (m_pFlag) // защищаемся, чтоб не попасть, куда не нужно
        *m_pFlag = true;
}

void InterruptableThread::startFunc(ThreadPool* pool, int qindex) {
    {
        lock_guard<mutex> l(m_defender);
        m_pFlag = &thread_interrupt_flag;
    }
    pool->threadFunc(qindex);
    {
        lock_guard<mutex> l(m_defender);
        m_pFlag = nullptr;
    }
}

bool InterruptableThread::checkInterrupted() {
    return thread_interrupt_flag;
}
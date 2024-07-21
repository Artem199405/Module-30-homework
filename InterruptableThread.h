#pragma once

#include <thread>
#include <mutex>
#include "optimized_thread.h"

using namespace std;

class ThreadPool;

class InterruptableThread {
public:
    InterruptableThread(ThreadPool* pool, int qindex);

    ~InterruptableThread();

    void interrupt();

    void startFunc(ThreadPool* pool, int qindex);

    static bool checkInterrupted();

private:
    mutex m_defender;

    bool* m_pFlag;

public:
    thread m_thread;
};
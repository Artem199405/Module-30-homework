#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>
#include <iostream>
#include "InterruptableThread.h"

using namespace std;

// ������� ����������� ��� ���������� ����
typedef function<void()> task_type;

// ��� ��������� �� �������, ������� �������� �������� ��� ������� �����
typedef void (*FuncType) (int, int);

class InterruptableThread;

template<class T> class BlockedQueue {
public:
    void push(T& item) {
        lock_guard<mutex> l(m_locker);
        // ������� ���������������� push
        m_task_queue.push(item);
        // ������ ����������, ����� �����, ���������
        // pop ��������� � ������ ������� �� �������
        m_notifier.notify_one();
    }

    // ����������� ����� ��������� �������� �� �������
    void pop(T& item) {
        unique_lock<mutex> l(m_locker);
        if (m_task_queue.empty())
            // ����, ���� ������� push
            m_notifier.wait(l, [this] {return !m_task_queue.empty(); });
        item = m_task_queue.front();
        m_task_queue.pop();
    }

    // ������������� ����� ��������� �������� �� �������
    // ���������� false, ���� ������� �����
    bool fast_pop(T& item) {
        lock_guard<mutex> l(m_locker);
        if (m_task_queue.empty())
            // ������ �������
            return false;
        // �������� �������
        item = m_task_queue.front();
        m_task_queue.pop();
        return true;
    }

private:
    mutex m_locker;

    // ������� �����
    queue<task_type> m_task_queue;

    // �����������
    condition_variable m_notifier;
};

class ThreadPool {
public:
    ThreadPool();

    // ������
    void start();

    // ���������
    void stop();

    // ������� �����
    void push_task(FuncType f, int arg1, int arg2);

    // ������� ����� ��� ������
    void threadFunc(int qindex);

    void interrupt();

private:
    // ���������� �������
    int m_thread_count;

    // ������
    vector<InterruptableThread*> m_threads;

    // ������� ����� ��� �������
    vector<BlockedQueue<task_type>> m_thread_queues;

    // ��� ������������ ������������� �����
    int m_index;
};

class RequestHandler {
public:
    RequestHandler();

    ~RequestHandler();

    // �������� ������� �� ����������
    void pushRequest(FuncType f, int id, int arg);

    void interruptPool();

private:
    // ��� �������
    ThreadPool m_tpool;
};
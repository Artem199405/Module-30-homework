#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>
#include <future>

using namespace std;

void taskFunc(int arg1, int arg2);

// ������� ����������� ��� ���������� ����
typedef function<void()> task_type;

// ��� ��������� �� �������, ������� �������� �������� ��� ������� �����
typedef void (*FuncType) (int*, int, int);

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
    future<void> push_task(FuncType f, int* arr, int arg1, int arg2);

    // ������� ����� ��� ������
    void threadFunc(int qindex);

private:
    // ���������� �������
    int m_thread_count;

    // ������
    vector<thread> m_threads;

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
    future<void> push_task(FuncType f, int* arr, int arg1, int arg2);

private:
    // ��� �������
    ThreadPool m_tpool;
};
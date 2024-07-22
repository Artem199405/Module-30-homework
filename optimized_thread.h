#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>
#include <future>

using namespace std;

void taskFunc(int arg1, int arg2);

// удобное определение для сокращения кода
typedef function<void()> task_type;

// тип указатель на функцию, которая является эталоном для функций задач
typedef void (*FuncType) (int*, int, int);

template<class T> class BlockedQueue {
public:
    void push(T& item) {
        lock_guard<mutex> l(m_locker);
        // обычный потокобезопасный push
        m_task_queue.push(item);
        // делаем оповещение, чтобы поток, вызвавший
        // pop проснулся и забрал элемент из очереди
        m_notifier.notify_one();
    }

    // блокирующий метод получения элемента из очереди
    void pop(T& item) {
        unique_lock<mutex> l(m_locker);
        if (m_task_queue.empty())
            // ждем, пока вызовут push
            m_notifier.wait(l, [this] {return !m_task_queue.empty(); });
        item = m_task_queue.front();
        m_task_queue.pop();
    }

    // неблокирующий метод получения элемента из очереди
    // возвращает false, если очередь пуста
    bool fast_pop(T& item) {
        lock_guard<mutex> l(m_locker);
        if (m_task_queue.empty())
            // просто выходим
            return false;
        // забираем элемент
        item = m_task_queue.front();
        m_task_queue.pop();
        return true;
    }

private:
    mutex m_locker;

    // очередь задач
    queue<task_type> m_task_queue;

    // уведомитель
    condition_variable m_notifier;
};

class ThreadPool {
public:
    ThreadPool();

    // запуск
    void start();

    // остановка
    void stop();

    // проброс задач
    future<void> push_task(FuncType f, int* arr, int arg1, int arg2);

    // функция входа для потока
    void threadFunc(int qindex);

private:
    // количество потоков
    int m_thread_count;

    // потоки
    vector<thread> m_threads;

    // очереди задач для потоков
    vector<BlockedQueue<task_type>> m_thread_queues;

    // для равномерного распределения задач
    int m_index;
};

class RequestHandler {
public:
    RequestHandler();

    ~RequestHandler();

    // отправка запроса на выполнение
    future<void> push_task(FuncType f, int* arr, int arg1, int arg2);

private:
    // пул потоков
    ThreadPool m_tpool;
};
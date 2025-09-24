#pragma once

#include "base/stdafx.h"

#include <concurrentqueue.h>

int i = 0;
moodycamel::ConcurrentQueue<int> g_queue;

void enqueue1()
{
    while (i < 100)
    {
        if (i % 2 == 0)
        {
            g_queue.enqueue(i);
            std::cout << "enqueue1: " << i << std::endl;
            i++;
        }
    }
}

void enqueue2()
{
    while (i < 100)
    {
        if (i % 2 == 1)
        {
            g_queue.enqueue(i);
            std::cout << "enqueue2: " << i << std::endl;
            i++;
        }
    }
}

int main(int argc, char const *argv[])
{
    std::thread t1(enqueue1);
    std::thread t2(enqueue2);
    t1.join();
    t2.join();

    int j;
    while (g_queue.try_dequeue(j))
    {
        std::cout << "j: " << j << std::endl;
    }

    return 0;
}

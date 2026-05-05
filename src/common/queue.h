#pragma once
#include <iostream>
using namespace std;
template <class T>
class Queue
{
    T* arr;
    int fronti;
    int reari;

public:
    int size;
    int count;

    Queue(int s = 10)
    {
        size = s;
        arr = new T[size];
        fronti = 0;
        reari = -1;
        count = 0;
    }

    ~Queue()
    {
        delete[] arr;
    }

    bool empty()
    {
        return count == 0;
    }

    bool full()
    {
        return count == size;
    }

    void push(T val)
    {
        if(full())
        {
            return;
        }

        reari = (reari + 1) % size;
        arr[reari] = val;
        count++;
    }

    void pop()
    {
        if(empty())
        {
            return;
        }

        fronti = (fronti + 1) % size;
        count--;
    }

    T front()
    {
        return arr[fronti];
    }

    int getsize()
    {
        return count;
    }
};
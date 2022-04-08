#include "Timer.h"

TimerHeap::TimerHeap(int cap)
    : capacity(cap), num(0)
{
    arr = new Util_timer* [capacity];   //创建堆数组,这是一个装满指针的数组
    if (!arr) throw std::exception();
    for (int i = 0; i < capacity; ++i)
        arr[i] == nullptr;
}

TimerHeap::TimerHeap(Util_timer** init_arr, int cap, int n)
    : capacity(cap), num(n)
{
    if (capacity < num) throw std::exception();
    arr = new Util_timer* [capacity];
    if (!arr) throw std::exception();
    for (int i = 0; i < capacity; ++i)
        arr[i] == nullptr;
    if (num != 0 ) {
        for (int i = 0; i < num; ++i)
            arr[i] = init_arr[i];
        for (int i = (num-1)/2; i > 0; --i)
            percolate_down(i);
    }
}

TimerHeap::~TimerHeap()
{
    for (int i = 0; i < num; ++i) {
        delete arr[i];
    }
    delete[] arr;
}

void TimerHeap::add_timer(Util_timer* timer)
{
    if (!timer) return;
    if (num >= capacity) resize();
    int hole = ++num;   //新插入一个元素
    int parent = 0;    //父节点
    /*对从空穴到根结点的路径上的所有结点执行上滤操作*/
    while (hole > 0) {
        parent = (hole-1)/2;
        if (arr[parent]->expire > timer->expire) { 
            arr[hole] = arr[parent];
            hole = parent;
        } else {
            break;
        }
    }
    arr[hole] = timer;
}

void TimerHeap::del_timer(Util_timer* timer)
{
    if (!timer) return;
    /*仅仅将目标定时器的回调函数设置为空，即所谓的延迟销毁，大大节省开销
    但是会使得堆膨胀*/
    timer->cb_func = nullptr;
}

Util_timer* TimerHeap::top()
{
    if (num == 0) return nullptr;
    return arr[0];
}

void TimerHeap::pop()
{
    if (num == 0) return;
    if (arr[0]) {
        delete arr[0];
        arr[0] = arr[--num];
        percolate_down(0);      //对根结点执行下滤操作
    })
}

void TimerHeap::tick()
{
    Util_timer* tmp = arr[0];
    time_t cur = time(nullptr);
    while (num != 0) {
        if (!tmp) break;
        if (tmp->expire > cur) break;   //还没到期
        if (arr[0]->cb_func != nullptr) {   //到期了，且回调函数有效
            arr[0]->cb_func(arr[0]->user_data);
        }
        pop_timer();    //删除到期的根，生成新的根
        tmp = arr[0];
    }
}

void TimerHeap::percolate_down(int hole)
{
    int child = 0;
    Util_timer* tmp = arr[hole];
    while (hole * 2 + 1 < num) {
        child = hole * 2 + 1;   //左儿子
        /*如果有右儿子，还比左儿子小*/
        if (child + 1 < num && arr[child + 1]->expire < arr[child]->expire)
            ++child;
        //下滤操作
        if (arr[child]->expire < tmp->expire) {
            arr[hole] = arr[child];
            hole = child;
        } else {
            break;
        }
    }
    arr[hole] = tmp;
}

void TimerHeap::resize()
{
    Util_timer** tmp = new Util_timer* [capacity * 2];
    if (!tmp) throw std::exception();
    for (int i = 0; i < capacity * 2; ++i)
        tmp[i] == nullptr;
    for (int i = 0; i < num; ++i)
        tmp[i] = arr[i];
    delete[] arr;
    arr = tmp;  //更新一下
    capacity *= 2;
}
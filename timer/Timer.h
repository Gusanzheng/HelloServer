/*
*struct client_data
*定时器类
*定时器容器：
    1.升序链表
    2.时间轮(无序链表)
    3.时间堆(堆数组)
*/
#ifndef TIMER_H
#define TIMER_H

#include <netinet/in.h>
#include <time.h>
#include <exception>
#include <queue>
#include <memory>

using std::priority_queue;
using std::shared_ptr;
using std::make_shared;

cosnt int CLIENT_DATA_BUFFER_SIZE = 1024;
class Util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    char* buf[CLIENT_DATA_BUFFER_SIZE];//data缓冲区
    Util_timer *timer;
};

//定时器
class Util_timer
{
public:
    Util_timer() //用于升序链表
        : prev(nullptr), next(nullptr) {}
    Util_timer(int rot, int ts) //用于时间轮
        : prev(nullptr), next(nullptr), rotation(rot), timerSlot(ts) {}
    Util_timer(int delay) { //用于时间堆
        expire = time(nullptr) + delay;
    }
public:
    /*用于升序链表,时间堆*/
    time_t expire;

    /*用于时间轮*/
    int rotation;   //定时器在时间轮上转多少圈后生效
    int timerSlot;  //记录定时器处于哪个槽

    void (* cb_func)(client_data *);
    client_data *user_data;
    Util_timer *prev;
    Util_timer *next;
};

/*升序链表
*添加效率O(n),删除效率O(1),执行定时任务效率O(1)
*/
class Timer_lst
{
public:
    Timer_lst();
    ~Timer_lst();

    void add_timer(Util_timer *timer);
    void adjust_timer(Util_timer *timer);
    void del_timer(Util_timer *timer);
    void tick();

private:
    void add_timer(Util_timer *timer, Util_timer *lst_head);

    Util_timer *head;
    Util_timer *tail;
};

/*时间轮
*添加效率O(1),删除效率O(1),执行定时任务效率O(n)
执行效率随着轮子数和槽数的增加，无限接近O(1)
*/
class TimerWheel {
public:
    TimerWheel();
    ~TimerWheel();

    Util_timer* add_timer(int timeout); //根据timeout创建一个定时器，并插入合适的槽
    void del_timer(Util_timer *timer);
    void tick();    //时间到了，就调用，时间轮向前滚动一个槽

private:
    static const int n = 60;    //时间轮的槽数
    static const int si = 1;    //时间轮的槽间隔,1s
    Util_timer* m_wheel[n];     //时间轮的槽,每个槽是一个定时器链表,无序的!
    int curSlot;                //当前槽
};

/*时间堆-采用数组组织完全二叉树，实现最小堆
添加效率O(lgn),删除效率O(1),执行任务效率O(1),效率很高
*/
class TimerHeap {
public:
    TimerHeap(int cap) throw (std::exception);
    TimerHeap(Util_timer** init_arr, int cap, int n) throw (std::exception);
    ~TimerHeap();

public:
    void add_timer(Util_timer* timer);
    void del_timer(Util_timer* timer);
    Util_timer* top() const;
    void pop_timer();
    void tick();
    bool empty() const {return num == 0;}

private:
    void percolate_down(int hole);  //下滤操作
    void resize()  throw (std::exception);  //扩容为原来的2倍
private:
    Util_timer** arr;  //堆数组
    int capacity;   //容量
    int num;        //堆数组包含元素的个数
};

/*时间堆-基于priority_queue实现，实质还是最小堆
*/
class TimerPriorityQueue {
public:
    TimerPriorityQueue() = default;
    ~TimerPriorityQueue() = default;

    //禁止自动生成拷贝构造函数和赋值运算符，移动构造函数和移动赋值运算符
    TimerPriorityQueue(const TimerPriorityQueue&) = delete;
    TimerPriorityQueue& operator=(const TimerPriorityQueue&) = delete;
    TimerPriorityQueue(TimerPriorityQueue&&) = delete;
    TimerPriorityQueue& operator=(TimerPriorityQueue&&) = delete;
public:
    void add_timer(Util_timer* timer) {
        if (!timer) return;
        pq.push(make_shared(timer));
    }

    void del_timer(Util_timer* timer) {
        if (!timer) return;
        /*仅仅将目标定时器的回调函数设置为空，即所谓的延迟销毁，大大节省开销
        但是会使得堆膨胀*/
        timer->cb_func = nullptr;
    }
    
    void pop_timer() {
        if (pq.empty()) return;
        pq.pop();
    }

    //注意当top的智能指针释放掉后，get到的普通指针将失效
    //而且get到的普通指针不能delete
    Util_timer* top() const {
        if (pq.empty()) return nullptr;
        return pq.top().get();
    }

    int size() const {return pq.size();}

    bool empty() const {return pq.empty();}
    
    void tick() {
        Util_timer* tmp = pq.top();
        time_t cur = time(nullptr);
        while (!pq.empty()) {
            if (!tmp) break;
            if (tmp->expire > cur) break;   //还没到期
            if (arr[0]->cb_func != nullptr) {   //到期了，且回调函数有效
                arr[0]->cb_func(arr[0]->user_data);
        }
        pop_timer();    //删除到期的根，生成新的根
        tmp = pq.top();
    }
}

private:
    //重写仿函数
    struct TimerCmp {
        bool operator()(const shared_ptr<Util_timer>& lhs, const shared_ptr<Util_timer>& rhs) const {
            return lhs->expire < rhs->expire;
        }
    };
    priority_queue<shared_ptr<Util_timer>, vector<shared_ptr<Util_timer>>, TimerCmp > pq;
};

//void cb_func(client_data *user_data);

#endif

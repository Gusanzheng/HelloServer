#include "Timer.h"

TimerWheel::TimerWheel()
    :curSlot(0)
{
    for (int i = 0; i < n; ++i)
    {
        m_wheel[i] = nullptr;
    }
}

TimerWheel::~TimerWheel()
{
    for (int i = 0; i < n; ++i)
    {
        Util_timer* tmp = m_wheel[i];
        while (tmp) {
            m_wheel[i] = tmp->next;
            delete tmp;
            tmp = m_wheel[i];
        }
    }
}

Util_timer* TimerWheel::add_timer(int timeout)
{
    if (timeout < 0) return nullptr;
    int ticks = 0;
    if (timeout < si) ticks = 1;
    else ticks = timeout / si;
    int rot = ticks/n;  //rot圈后触发
    int ts = (curSlot + (ticks%n)) % n; //放到ts槽
    Util_timer *timer = new Util_timer(rot, ts);
    
    /*插入无序链表*/
    if (!m_wheel[ts]) { //头结点
        m_wheel[ts] = timer;
    }
    else {  //非头结点，则插入头部
        timer->next = m_wheel[ts];
        m_wheel[ts]->prev = timer;
        m_wheel[ts] = timer;
    }

    return timer;
}

void TimerWheel::del_timer(Util_timer *timer)
{
    if (!timer) return;
    int ts = timer->timerSlot;

    /*删除*/
    if (timer == m_wheel[ts]) { //就是该槽的头结点
        m_wheel[ts] = timer->next;  //后移一位
        if (m_wheel[ts]) m_wheel[ts]->prev = nullptr;
        delete timer;
    }
    else {  //不是头结点，则取出该结点，并删除
        timer->prev->next = timer->next;
        if (timer->next) timer->next->prev = timer->prev;
        delete timer;
    }
}

void TimerWheel::tick()
{
    Util_timer* tmp = m_wheel[curSlot];
    while (tmp) {
        if (tmp->rotation > 0) {
            tmp->rotation--;
            tmp = tmp->next;
        }
        else {  //到点了
            tmp->cb_func(tmp->user_data);

            /*删除并后移*/
            if (tmp == m_wheel[curSlot]) {
                m_wheel[curSlot] = tmp->next;
                if (m_wheel[curSlot]) m_wheel[curSlot]->prev = nullptr;
                delete tmp;
                tmp = m_wheel[curSlot];
            }
            else {
                tmp->prev->next = tmp->next;
                if (tmp->next) tmp->next->prev = tmp->prev;
                delete tmp;
                tmp = tmp->next;
            }
        }
    }
    curSlot = (curSlot + 1) % n;    //时间轮滚动
}
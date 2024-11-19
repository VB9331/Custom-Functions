/*
 * @Author: VB9331 vb9331@qq.com
 * @Date: 2024-11-20 00:00:00
 * @LastEditors: VB9331 vb9331@qq.com
 * @LastEditTime: 2024-11-20 00:55:27
 * @Description: 定时器封装
 * Copyright (c) 2024 by VB9331 , All Rights Reserved.
 */
#include <REG52.H>
#include "timer.h"

#define TIMER_MAX 2 // 定时器最大数量
#define EVENT_MAX 2 // 每个定时器能存储的最大事件数量

typedef struct _Timer_Event_Type
{
    unsigned char enabled;     // 是否绑定
    unsigned short delay;      // 延时时间
    void (*fun)(void);         // 回调函数
    unsigned short begin_time; // 记录开始时间，用于计算已绑定时间
} Timer_Event_Type;

Timer_Event_Type timer_events[TIMER_MAX][EVENT_MAX];

char cur_id_max[TIMER_MAX];

unsigned short action_time[2] = {0, 0}; // 定时器已启动时间

void timer_init(unsigned char timers)
{
    TMOD &= ((0xF00 >> (timers & TIMER0)) | (0xF00 >> (timers & TIMER1))) & 0xFF;

    EA = 1; // 打开总中断
    if (timers & TIMER0)
    {
        TMOD |= 0x01;
        TH0 = 0XFC;
        TL0 = 0X18;
        TF0 = 0; // interrupt sign
        ET0 = 1; // 打开定时器0中断允许
        TR0 = 1; // 打开定时器
        PT0 = 0; // priority level
    }
    if (timers & TIMER1)
    {
        TMOD |= 0x10;
        TH1 = 0XFC;
        TL1 = 0X18;
        TF1 = 0; // interrupt sign
        ET1 = 1; // 打开定时器1中断允许
        TR1 = 1; // 打开定时器
        PT1 = 1; // priority level
    }
}

// add timer event
char timer_add_event(unsigned char timer_id, void (*callback)(void), unsigned short ms)
{
    unsigned char i;
    char id;
    Timer_Event_Type new_event;

    // catch_begin
    // 非法值直接退出
    if (callback == NULL)
    {
        return -1;
    }
    // 负数和0延时调为1ms
    ms <= 0 ? ms = 1 : 0;
    // catch_end

    // find available id
    for (i = 0; i <= cur_id_max[timer_id] && timer_events[timer_id][i].enabled; i++)
        ;
    id = i;
    if (id >= EVENT_MAX)
    { // catch
        return -1;
    }
    if (id > cur_id_max[timer_id])
    {
        cur_id_max[timer_id] = id;
    }

    new_event.enabled = 1;
    new_event.delay = ms;
    new_event.fun = callback;
    new_event.begin_time = action_time[timer_id]; // +1 防止立即触发
    timer_events[timer_id][id] = new_event;
    return id;
}

char timer_remove_event(unsigned char timer_id, char id)
{
    if (id < 0 || id > cur_id_max[timer_id])
    { // catch
        return id;
    }
    timer_events[timer_id][id].enabled = 0;
    timer_events[timer_id][id].delay = 0;
    timer_events[timer_id][id].fun = NULL;
    timer_events[timer_id][id].begin_time = 0;
    if (id == cur_id_max[timer_id])
    {
        cur_id_max[timer_id]--;
    }
    return -1;
}

void time0_handler(void) interrupt 1 // 定时器0中断函数
{

    const unsigned char timer_id = 0; // 定时器id

    unsigned char i;
    TH0 = (65536 - 1000) / 256; // 1ms 中断一次
    TL0 = (65536 - 1000) % 256;
    action_time[0]++; // 定时器0已启动时间+1，放在函数前端防止立即触发

    for (i = 0; i <= cur_id_max[timer_id]; i++)
    {
        if (timer_events[timer_id][i].enabled == 0)
        {
            continue;
        }
        if (((action_time[0] - timer_events[timer_id][i].begin_time) % timer_events[timer_id][i].delay) == 0)
        {
            timer_events[timer_id][i].fun(); // TODO: Whether to add arguments about the timer object to this function.
        }
    }
}

void time1_handler() interrupt 3 // 定时器1中断函数
{
    const unsigned char timer_id = 1;

    unsigned char i;
    TH1 = (65536 - 1000) / 256;
    TL1 = (65536 - 1000) % 256;
    action_time[1]++;

    for (i = 0; i <= cur_id_max[timer_id]; i++)
    {
        if (timer_events[timer_id][i].enabled == 0)
        {
            continue;
        }

        if (((action_time[1] - timer_events[timer_id][i].begin_time) % timer_events[timer_id][i].delay) == 0)
        {
            timer_events[timer_id][i].fun(); // TODO: Whether to add arguments about the timer object to this function.
        }
    }
}
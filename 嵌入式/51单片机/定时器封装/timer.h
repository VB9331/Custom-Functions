/*
 * @Author: VB9331 vb9331@qq.com
 * @Date: 2024-11-20 00:00:00
 * @LastEditors: VB9331 vb9331@qq.com
 * @LastEditTime: 2024-11-20 00:55:27
 * @Description: 定时器封装
 * Copyright (c) 2024 by VB9331 , All Rights Reserved.
 */
#ifndef __TIMER_H__
#define __TIMER_H__

#define NULL ((void *)0)
enum
{
    TIMER0 = 1,
    TIMER1
};

void timer_init(unsigned char timers);
char timer_add_event(unsigned char timer_id, void (*callback)(void), unsigned short ms);
char timer_remove_event(unsigned char timer_id, char id);

#endif
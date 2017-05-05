/******************************************************************************
* Copyright (c) 2015-2017 jiangxiaogang<kerndev@foxmail.com>
*
* This file is part of KLite distribution.
*
* KLite is free software, you can redistribute it and/or modify it under
* the MIT Licence.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
******************************************************************************/
#include "kernel.h"
#include "internal.h"

struct sem_obj
{
	struct tcb_node* head;
	struct tcb_node* tail;
	uint32_t data;
};

ksem_t ksem_create(uint32_t value)
{
	struct sem_obj* obj;
	obj = kmem_alloc(sizeof(struct sem_obj));
	if(obj != NULL)
	{
		obj->data = value;
		obj->head = NULL;
		obj->tail = NULL;
	}
	return (ksem_t)obj;
}

void ksem_destroy(ksem_t sem)
{
	kmem_free(sem);
}

void ksem_wait(ksem_t sem)
{
	struct sem_obj* obj;
	obj = (struct sem_obj*)sem;
	
	ksched_lock();
	if(obj->data != 0)
	{
		obj->data--;
		ksched_unlock();
		return;
	}
	ksched_wait(sched_tcb_now, obj);
	ksched_unlock();
	ksched_switch();
}

int ksem_timedwait(ksem_t sem, uint32_t timeout)
{
	struct sem_obj* obj;
	obj = (struct sem_obj*)sem;
	
	ksched_lock();
	if(obj->data != 0)
	{
		obj->data--;
		ksched_unlock();
		return 1;
	}
	if(timeout == 0)
	{
		ksched_unlock();
		return 0;
	}
	ksched_timedwait(sched_tcb_now, obj, timeout);
	ksched_unlock();
	ksched_switch();
	return (sched_tcb_now->timeout != 0);
}

void ksem_post(ksem_t sem)
{
	struct sem_obj* obj;
	obj = (struct sem_obj*)sem;
	
	ksched_lock();
	if(obj->head == NULL)
	{
		obj->data++;
		ksched_unlock();
		return;
	}
	ksched_post(obj->head->tcb, obj);
	ksched_unlock();
}

uint32_t ksem_getvalue(ksem_t sem)
{
	struct sem_obj* obj;
	obj = (struct sem_obj*)sem;
	return obj->data;
}

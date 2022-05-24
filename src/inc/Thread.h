/*
 * Thread.h
 *
 *  Created on: Oct 15, 2015
 *      Author: user
 */

#include <cstdlib>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <pthread.h>

#ifndef THREAD_H_
#define THREAD_H_

class Thread
{
private:
    pthread_t thread;

    Thread(const Thread& copy);         // copy constructor denied
    static void *thread_func(void *d)
    {
    	((Thread *)d)->run();
    	return NULL;
    }

public:
    Thread()             {thread=0;}
    virtual ~Thread()    {}

    virtual void run() = 0;

    int thread_start()
    {
    	return pthread_create(&thread, NULL, Thread::thread_func, (void*)this);
    }

    int wait ()
    {
    	return pthread_join(thread, NULL);
    }
};

typedef std::unique_ptr<Thread> ThreadPtr;

#endif /* THREAD_H_ */

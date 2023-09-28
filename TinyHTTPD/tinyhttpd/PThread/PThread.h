#pragma once
#include <plibsys.h>
#include <puthread.h>
#include "plibcpp.h"
#include <iostream>
#include <psocket.h>

void* startUp(void*);

class PThread_Func {
public:
	virtual void run() = 0;
protected:
	PUThread* thisThread;
	friend void* startUp(void*);
};

class PThread : public plib_user {
public:
	PThread(PThread_Func* func, const char* name, PUThreadPriority prio = P_UTHREAD_PRIORITY_NORMAL)
		: thread_name{ name }, thread_priority{ prio }, container{ func }
	{
		globalCounter++;
	}

	PThread(PThread_Func* func, bool joinable, const char* name, PUThreadPriority prio = P_UTHREAD_PRIORITY_NORMAL)
		: thread_joinable{ joinable }, thread_name{ name }, thread_priority{ prio }, container{func}
	{
		globalCounter++;
	}

	~PThread() {
		globalCounter--;
	}

	void run();

	bool done() { return isDone; }

	void join() {
		if (!isStarted || isDone)
			return;
		p_uthread_join(thread);
	}
private:
	PUThread* thread_obj = NULL;
	pint thread_wakes = 0;
	P_HANDLE thread_id = (P_HANDLE)NULL;
	pboolean thread_joinable = TRUE;
	const char* thread_name;
	PUThreadPriority thread_priority;
	volatile pboolean thisThreadIsAllowedToWork = TRUE;
	volatile bool isDone = false;
	bool isStarted = false;
	PUThreadKey* tls_key;

	PUThread* thread = nullptr;
	void* container;

	void* start();
	friend void* startUp(void*);

	static volatile pboolean globalThreadsAreAllowedToWork;
	static pint globalCounter;
};

volatile pboolean PThread::globalThreadsAreAllowedToWork = TRUE;
pint PThread::globalCounter = 0;

void PThread::run()
{
	if (!isDone && isStarted)
		return;
	isStarted = true;
	thread_wakes = 1;
	thread_id = (P_HANDLE)NULL;
	thread_obj = NULL;

	tls_key = p_uthread_local_new(NULL);

	PUThread* thr = p_uthread_create_full((PUThreadFunc)startUp,
		(ppointer)this,
		TRUE,
		thread_priority,
		128 * 1024,
		thread_name);
	thread = thr;

	p_uthread_ref(thr);

	p_uthread_set_priority(thr, thread_priority);
}

void* PThread::start() {
	plib_user* safety = new plib_user();
	p_uthread_set_local(tls_key, (ppointer)p_uthread_current_id());

	PThread_Func* c = static_cast<PThread_Func*>(container);

	try {
		c->run();
	}
	catch (...) {
		delete safety;
		isDone = true;
		p_uthread_exit(1);
		return NULL;
	}
	delete safety;
	isDone = true;
	p_uthread_exit(1);
	return NULL;
}

void* startUp(void* pointer) {
	plib_user* safety = new plib_user();
	PThread* object = static_cast<PThread*>(pointer);
	p_uthread_set_local(object->tls_key, (ppointer)p_uthread_current_id());

	PThread_Func* c = static_cast<PThread_Func*>(object->container);
	c->thisThread = object->thread;
	try {
		c->run();
	}
	catch (...) {
		delete safety;
		object->isDone = true;
		p_uthread_exit(0);
		return NULL;
	}
	delete safety;
	object->isDone = true;
	p_uthread_exit(0);
	return NULL;
}
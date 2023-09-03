#pragma once
#include <plibsys.h>
#include <puthread.h>
#include "plibcpp.h"
#include <iostream>
#include <psocket.h>

static pint              thread_wakes_1 = 0;
static pint              thread_wakes_2 = 0;
static pint              thread_to_wakes = 0;
static volatile pboolean is_threads_working = FALSE;

static P_HANDLE   thread1_id = (P_HANDLE)NULL;
static P_HANDLE   thread2_id = (P_HANDLE)NULL;
static PUThread* thread1_obj = NULL;
static PUThread* thread2_obj = NULL;

static PUThreadKey* tls_key = NULL;
static PUThreadKey* tls_key_2 = NULL;
static volatile pint free_counter = 0;

extern "C" ppointer pmem_alloc(psize nbytes)
{
	P_UNUSED(nbytes);
	return (ppointer)NULL;
}

extern "C" ppointer pmem_realloc(ppointer block, psize nbytes)
{
	P_UNUSED(block);
	P_UNUSED(nbytes);
	return (ppointer)NULL;
}

extern "C" void pmem_free(ppointer block)
{
	P_UNUSED(block);
}

extern "C" void free_with_check(ppointer mem)
{
	p_free(mem);
	p_atomic_int_inc(&free_counter);
}

static void* test_thread_func(void* data)
{
	pint* counter = static_cast <pint*> (data);

	if ((*counter) == 1) {
		thread1_id = p_uthread_current_id();
		thread1_obj = p_uthread_current();
	}
	else {
		thread2_id = p_uthread_current_id();
		thread2_obj = p_uthread_current();
	}

	p_uthread_set_local(tls_key, (ppointer)p_uthread_current_id());

	*counter = 0;

	while (is_threads_working == TRUE) {
		p_uthread_sleep(10);
		++(*counter);
		p_uthread_yield();
		if (p_uthread_get_local(tls_key) != (ppointer)p_uthread_current_id())
			p_uthread_exit(-1);
	}

	p_uthread_exit(*counter);

	return NULL;
}

static void* test_thread_nonjoinable_func(void* data)
{
	pint* counter = static_cast <pint*> (data);

	is_threads_working = TRUE;

	for (int i = thread_to_wakes; i > 0; --i) {
		p_uthread_sleep(10);
		++(*counter);
		p_uthread_yield();
	}

	is_threads_working = FALSE;

	p_uthread_exit(0);

	return NULL;
}

static void* test_thread_tls_func(void* data)
{
	pint self_thread_free = *((pint*)data);

	pint* tls_value = (pint*)p_malloc0(sizeof(pint));
	*tls_value = 0;
	p_uthread_set_local(tls_key, (ppointer)tls_value);

	pint prev_tls = 0;
	pint counter = 0;

	while (is_threads_working == TRUE) {
		p_uthread_sleep(10);

		pint* last_tls = (pint*)p_uthread_get_local(tls_key);

		if ((*last_tls) != prev_tls)
			p_uthread_exit(-1);

		pint* tls_new_value = (pint*)p_malloc0(sizeof(pint));

		*tls_new_value = (*last_tls) + 1;
		prev_tls = (*last_tls) + 1;

		p_uthread_replace_local(tls_key, (ppointer)tls_new_value);

		if (self_thread_free)
			p_free(last_tls);

		++counter;

		p_uthread_yield();
	}

	if (self_thread_free) {
		pint* last_tls = (pint*)p_uthread_get_local(tls_key);

		if ((*last_tls) != prev_tls)
			p_uthread_exit(-1);

		p_free(last_tls);

		p_uthread_replace_local(tls_key, (ppointer)NULL);
	}

	p_uthread_exit(counter);

	return NULL;
}

static void* test_thread_tls_create_func(void* data)
{
	P_UNUSED(data);

	pint* tls_value = (pint*)p_malloc0(sizeof(pint));
	*tls_value = 0;
	p_uthread_set_local(tls_key, (ppointer)tls_value);

	pint* tls_value_2 = (pint*)p_malloc0(sizeof(pint));
	*tls_value_2 = 0;
	p_uthread_set_local(tls_key_2, (ppointer)tls_value_2);

	return NULL;
}

void puthread_nomem_test()
{
	p_libsys_init();

	PUThreadKey* thread_key = p_uthread_local_new(p_free);

	PMemVTable vtable;

	vtable.free = pmem_free;
	vtable.malloc = pmem_alloc;
	vtable.realloc = pmem_realloc;

	thread_wakes_1 = 0;
	thread_wakes_2 = 0;

	p_uthread_exit(0);

	p_uthread_set_local(thread_key, PINT_TO_POINTER(10));

	ppointer tls_value = p_uthread_get_local(thread_key);

	if (tls_value != NULL) {
		p_uthread_set_local(thread_key, NULL);
	}

	p_uthread_replace_local(thread_key, PINT_TO_POINTER(12));

	tls_value = p_uthread_get_local(thread_key);

	if (tls_value != NULL) {
		p_uthread_set_local(thread_key, NULL);
	}

	p_mem_restore_vtable();

	p_uthread_local_free(thread_key);

	p_libsys_shutdown();
}

void puthread_bad_input_test()
{
	p_libsys_init();

	p_uthread_set_local(NULL, NULL);
	p_uthread_replace_local(NULL, NULL);
	p_uthread_ref(NULL);
	p_uthread_unref(NULL);
	p_uthread_local_free(NULL);
	p_uthread_exit(0);

	p_libsys_shutdown();
}

void puthread_general_test()
{
	p_libsys_init();

	thread_wakes_1 = 1;
	thread_wakes_2 = 2;
	thread1_id = (P_HANDLE)NULL;
	thread2_id = (P_HANDLE)NULL;
	thread1_obj = NULL;
	thread2_obj = NULL;

	tls_key = p_uthread_local_new(NULL);

	/* Threre is no guarantee that we wouldn't get one of the IDs
	 * of the finished test threads */

	P_HANDLE main_id = p_uthread_current_id();

	is_threads_working = TRUE;

	PUThread* thr1 = p_uthread_create_full((PUThreadFunc)test_thread_func,
		(ppointer)&thread_wakes_1,
		TRUE,
		P_UTHREAD_PRIORITY_NORMAL,
		64 * 1024,
		"thread_name");

	PUThread* thr2 = p_uthread_create_full((PUThreadFunc)test_thread_func,
		(ppointer)&thread_wakes_2,
		TRUE,
		P_UTHREAD_PRIORITY_NORMAL,
		64 * 1024,
		"very_long_name_for_thread_testing");

	p_uthread_ref(thr1);

	p_uthread_set_priority(thr1, P_UTHREAD_PRIORITY_NORMAL);

	p_uthread_sleep(5000);

	is_threads_working = FALSE;

	p_uthread_local_free(tls_key);
	p_uthread_unref(thr1);
	p_uthread_unref(thr2);

	p_uthread_unref(thr1);

	PUThread* cur_thr = p_uthread_current();
}

void puthread_nonjoinable_test()
{
	p_libsys_init();

	thread_wakes_1 = 0;
	thread_to_wakes = 100;
	is_threads_working = TRUE;

	PUThread* thr1 = p_uthread_create((PUThreadFunc)test_thread_nonjoinable_func,
		(ppointer)&thread_wakes_1,
		FALSE,
		NULL);

	p_uthread_sleep(3000);

	while (is_threads_working == TRUE)
		p_uthread_sleep(10);

	p_uthread_unref(thr1);

	p_libsys_shutdown();
}

void puthread_tls_test()
{
	p_libsys_init();

	/* With destroy notification */
	tls_key = p_uthread_local_new(free_with_check);

	is_threads_working = TRUE;
	free_counter = 0;

	pint self_thread_free = 0;

	PUThread* thr1 = p_uthread_create((PUThreadFunc)test_thread_tls_func,
		(ppointer)&self_thread_free,
		TRUE,
		NULL);

	PUThread* thr2 = p_uthread_create((PUThreadFunc)test_thread_tls_func,
		(ppointer)&self_thread_free,
		TRUE,
		NULL);

	p_uthread_sleep(5000);

	is_threads_working = FALSE;

	pint total_counter = 0;

	total_counter += (p_uthread_join(thr1) + 1);
	total_counter += (p_uthread_join(thr2) + 1);


	p_uthread_local_free(tls_key);
	p_uthread_unref(thr1);
	p_uthread_unref(thr2);

	/* Without destroy notification */
	tls_key = p_uthread_local_new(NULL);

	free_counter = 0;
	is_threads_working = TRUE;
	self_thread_free = 1;

	thr1 = p_uthread_create((PUThreadFunc)test_thread_tls_func,
		(ppointer)&self_thread_free,
		TRUE,
		NULL);

	thr2 = p_uthread_create((PUThreadFunc)test_thread_tls_func,
		(ppointer)&self_thread_free,
		TRUE,
		NULL);

	p_uthread_sleep(5000);

	is_threads_working = FALSE;

	total_counter = 0;

	total_counter += (p_uthread_join(thr1) + 1);
	total_counter += (p_uthread_join(thr2) + 1);

	p_uthread_local_free(tls_key);
	p_uthread_unref(thr1);
	p_uthread_unref(thr2);

	/* With implicit thread exit */
	tls_key = p_uthread_local_new(free_with_check);
	tls_key_2 = p_uthread_local_new(free_with_check);

	free_counter = 0;

	thr1 = p_uthread_create((PUThreadFunc)test_thread_tls_create_func,
		NULL,
		TRUE,
		NULL);

	thr2 = p_uthread_create((PUThreadFunc)test_thread_tls_create_func,
		NULL,
		TRUE,
		NULL);

	p_uthread_join(thr1);
	p_uthread_join(thr2);

	p_uthread_local_free(tls_key);
	p_uthread_local_free(tls_key_2);
	p_uthread_unref(thr1);
	p_uthread_unref(thr2);

	p_libsys_shutdown();
}

// from here up is not written by me, it came from the plibsys test collection
// I used it as reference on how to make the p_thread stuff work

// This container class keeps a reference to the counter and socket for a PThread to manage
// I can only pass in one thing to the PThread reliably, so I wanted to do it with a small class
class Container : plib_user {
public:
	Container() 
		: counter{nullptr}, socket{nullptr}
	{	}
	Container(pint* count, PSocket* sock) 
		: counter{count}, socket{sock}
	{}
	pint* counter;
	PSocket* socket;

	bool operator==(const Container& other) const {
		return (counter == other.counter) && (socket == other.socket);
	}
};

const Container nullContainer(nullptr, nullptr);

// Make a thing to manage the threads and their PSocket objects
const size_t masterLength = 100;
Container master[masterLength];

int masterIndex = 0;

// This method is what actually runs in the new thread spawned on a PSocket connection to the minihttpd server.
// It takes an integer which will then reference the 'master' container array where it can then pull its required information from the container object stored there.
static void* socket_kill_worker(void* data)
{
	// grab our index in master and pull the information (like the socket reference and thread counter)
	pint* indexIntoMaster = static_cast <pint*> (data);
	PSocket* target = master[*indexIntoMaster].socket;
	pint* counter = master[*indexIntoMaster].counter;

	// get our thread local storage key and associated data
	p_uthread_set_local(tls_key, (ppointer)p_uthread_current_id());

	// assume the http transaction takes 100 ms or less and do nothing during that
		p_uthread_sleep(100);
		// if the socket wasn't doing anything...
		p_socket_set_blocking(target, false);
		// it sure isn't doing anything now and closed if it was on a hang-up waiting on a broken pipe
		while (p_socket_is_connected(target)) {
			// if the socket still is connected, wait until it is no longer...
			p_uthread_sleep(10);
		}
	// and kill it 
	p_socket_close(target, NULL);
	// clean up after ourselves
	master[*counter] = nullContainer;
	std::cout << "--------------KILLED---------------\n";
	p_uthread_exit(*counter);

	return NULL;
}

// Make a new thread to watch and kill the given socket if its connection gets closed and has an unfulfilled request sitting in minihttpd
void unused_socket_killer(PSocket* sock, pint* point)
{
	plib_user* safety = new plib_user();
	thread_wakes_1 = 1;
	thread_wakes_2 = 2;
	thread1_id = (P_HANDLE)NULL;
	thread2_id = (P_HANDLE)NULL;
	thread1_obj = NULL;
	thread2_obj = NULL;

	tls_key = p_uthread_local_new(NULL);

	/* Threre is no guarantee that we wouldn't get one of the IDs
	 * of the finished test threads */

	P_HANDLE main_id = p_uthread_current_id();

	is_threads_working = TRUE;

	PUThread* thr1 = p_uthread_create_full((PUThreadFunc)socket_kill_worker,
		(ppointer)point,
		TRUE,
		P_UTHREAD_PRIORITY_NORMAL,
		64 * 1024,
		"thread_name");

	p_uthread_ref(thr1);

	p_uthread_set_priority(thr1, P_UTHREAD_PRIORITY_NORMAL);

	delete safety;
}

// clean out the master container array
void threadInit() {
	for (int i = 0; i < masterLength; i++) {
		master[i] = nullContainer;
	}
}

// look for the first (effectively) null container and return that index as the spot in which to place a new thread's container
int getNewThreadIndex() {
	for (int i = 0; i < masterLength; i++) {
		if (master[i] == nullContainer) {
			return i;
		}
	}
	return 0;
}

// Make a new watcher thread for the given PSocket
void setNewThreadUp(PSocket* s) {
	int id = getNewThreadIndex();
	pint* threadId = new pint(id);
	Container c(threadId, s);
	std::cout << "new thread: " << *threadId << std::endl;
	master[id] = c;
	unused_socket_killer(s, threadId);
}
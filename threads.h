/*
 *
 * threads.h
 * ------------------------------------------
 *  Public API for Kernel Threads
 *
 */

#define INVALID_THREAD_ID 0
#define KTHREAD_MAX 64
#define KTHREAD_STACK_SMALL 5 * 1024
#define KTHREAD_STACK_NORMAL 10 * 1024


// Signature for a thread entry point
typedef void (*kThreadFunc)();

int kThread_Queue(char* name, kThreadFunc func, int stackSize);

// Yield to another thread
extern void kThread_Yield();

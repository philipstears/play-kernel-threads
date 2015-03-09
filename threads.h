/*
 *
 * threads.h
 * ------------------------------------------
 *  Public API for Kernel Threads
 *
 */

// Signature for a thread entry point
typedef void (*kThreadFunc)();

// Thread queueing
#define KTHREAD_STACK_SMALL 5 * 1024
#define KTHREAD_STACK_NORMAL 10 * 1024
int kThreadManager_QueueThread(char* name, kThreadFunc func, int stackSize);

// Yield to another thread
void kThreadManager_Yield();

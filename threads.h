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
int kThreadManager_QueueThread(char*, kThreadFunc);

// Yield to another thread
void kThreadManager_Yield();

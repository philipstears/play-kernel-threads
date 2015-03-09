#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "threadManager.h"
#include "threads.h"
#include "utility.h"

/* Structs */

typedef struct {
  // NOTE: tid zero is reserved for the idle process and never
  //       appears in the thread list, therefore we use tid zero
  //       to indicate that a slot in the thread table is
  //       free
  int         tid;
  char*       name;
  void*       stack;
  void*       stackHead;
} kThread;

typedef struct {
  kThread     threads[KTHREAD_MAX];
  int         lastTid;
  int         runningThreadSlot;
} kThreadManagerState;

/* Global Vars */

kThreadManagerState* gThreadManagerState;

/* Externs */

extern void kThread_Next_Core(void* stackHead);

/* Implementation */

static int kThreadManager_FindAvailableSlot() {
  for ( int i = 0; i < KTHREAD_MAX; i++ ) {
    if (gThreadManagerState->threads[i].tid == INVALID_THREAD_ID) {
      return i;
    }
  }

  return -1;
}

static int kThreadManager_FindSlotIndexOfNextThread() {
  int runningThreadSlot = gThreadManagerState->runningThreadSlot;
  kThread* threads = gThreadManagerState->threads;

  // Look for threads after the current thread
  for(int i = runningThreadSlot + 1; i < KTHREAD_MAX; i++) {
    if(threads[i].tid != INVALID_THREAD_ID) {
      return i;
    }
  }

  // Look for threads before the current thread
  for(int i = 0; i < runningThreadSlot; i++) {
    if(threads[i].tid != INVALID_THREAD_ID) {
      return i;
    }
  }

  // Fallback - run the current thread again
  if (runningThreadSlot >= 0) {
    if (threads[runningThreadSlot].tid != INVALID_THREAD_ID) {
      return runningThreadSlot;
    }

    DEBUG("The currently running thread is no longer valid, and no other threads are available to run");
    gThreadManagerState->runningThreadSlot = -1;
  }

  return -1;
}

static void kThreadManager_InvokeThread() {
  kThread* thread;
  kThreadFunc threadFunc;

  asm(""
      : "=a"(thread), "=b"(threadFunc)
      :
      :
     );

  DEBUG("InvokeThread: Running thread func %p for thread %p", threadFunc, thread);

  threadFunc();

  DEBUG("InvokeThread: Exit");

  // Ditch the thread data, note we don't
  // set gThreadManagerState->runningSlotIndex because
  // that would defeat scheduling fairness
  DEBUG("InvokeThread: Exit -> Disabling running thread");
  thread->tid = INVALID_THREAD_ID;

  // Run another thread :-)
  DEBUG("InvokeThread: Exit -> About to schedule next thread");
  kThreadManager_Next();
}

static void inline kThreadManager_Push(kThread* thread, intptr_t value) {
  thread->stackHead -= sizeof(intptr_t);
  *((intptr_t*)thread->stackHead) = value;
}

kThreadInitResult kThreadManager_Initialize() {
  gThreadManagerState = (kThreadManagerState*)calloc(1, sizeof(kThreadManagerState));
  gThreadManagerState->runningThreadSlot = -1;
  if (gThreadManagerState == 0) {
    DEBUG("Failed to initialize thread state");
    return kThreadInitResult_Fail;
  }

  return kThreadInitResult_Success;
}

void kThread_Yield_Core(void* stackHead) {

  kThread* thisThread = &gThreadManagerState->threads[gThreadManagerState->runningThreadSlot];
  thisThread->stackHead = stackHead;

  // Woohoo, time to run another thread
  kThreadManager_Next();
}

void kThreadManager_Next() {
  int nextThreadSlotIndex = kThreadManager_FindSlotIndexOfNextThread();

  if (nextThreadSlotIndex < 0) {
    DEBUG("No more threads to run, exiting...");
    exit(0);
  }

  kThread* nextThread = &gThreadManagerState->threads[nextThreadSlotIndex];

  DEBUG("Going to run thread %p", nextThread);

  DEBUG("Going to run thread with tid %d", nextThread->tid);

  gThreadManagerState->runningThreadSlot = nextThreadSlotIndex;

  kThread_Next_Core(nextThread->stackHead);

  // If we get here, something is seriously screwed!
  DEBUG("Gosh darn everything, we really oughn't to be here dear");
}

int kThread_Queue(char* name, kThreadFunc func, int stackSize) {

  int slotIndex = kThreadManager_FindAvailableSlot();

  if(slotIndex < 0) {
    DEBUG("Unable to find free slot");
    return -1;
  }

  int tid = ++gThreadManagerState->lastTid;

  DEBUG("Allocating thread with tid %d to slot %d", tid, slotIndex);

  kThread* thread = &gThreadManagerState->threads[slotIndex];

  // Thread metadata
  thread->name = (char*)malloc(strlen(name));
  strcpy(thread->name, name);
  thread->tid = tid;

  // Construct the initial thread stack
  thread->stack = calloc(stackSize, 1);
  thread->stackHead = thread->stack + stackSize;

  // Insert a null pointer at the top of the stack, so
  // that we can a seg fault if we return from the top
  // level method improperly
  kThreadManager_Push(thread, 0);

  // Construct register state on the stack - REMEMBER -
  // the stack grows DOWN so EIP is the last thing on the
  // stack!
  /* EIP */ kThreadManager_Push(thread, (intptr_t)&kThreadManager_InvokeThread);
  /* EAX */ kThreadManager_Push(thread, (intptr_t)thread);
  /* EBX */ kThreadManager_Push(thread, (intptr_t)func);
  /* ECX */ kThreadManager_Push(thread, 0);
  /* EDX */ kThreadManager_Push(thread, 0);
  /* EBP */ kThreadManager_Push(thread, 0);
  /* ESI */ kThreadManager_Push(thread, 0);
  /* EDI */ kThreadManager_Push(thread, 0);

  return thread->tid;
}

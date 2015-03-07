#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#define KTHREAD_MAX 64
#define DEBUG(STR, PARAMS...) (printf(STR "\n", ##PARAMS))

/*
 *
 * Types
 *
 */
typedef void (*kThreadFunc)();

typedef struct {
  // NOTE: tid zero is reserved for the idle process and never
  //       appears in the thread list, therefore we use tid zero
  //       to indicate that a slot in the thread table is
  //       free
  int         tid;
  char*       name;
  kThreadFunc func;
} kThread;

typedef struct {
  kThread     threads[KTHREAD_MAX];
  int         lastPid;
  int         runningThreadSlot;
} kThreadManagerState;

kThreadManagerState* gThreadManagerState;

typedef enum {
  kThreadInitResult_Success = 0,
  kThreadInitResult_Fail = 1
} kThreadInitResult;

/*
 *
 * Forward Declarations
 *
 */
kThreadInitResult kThreadManager_Initialize();
int kThreadManager_QueueThread(char*, kThreadFunc);
void kThreadManager_Run();
void kSystemThread();

/*
 *
 * Main
 *
 */
int main(int argc, char* argv[]) {
  DEBUG("Hello, kThread world!");

  kThreadManager_Initialize();
  kThreadManager_QueueThread("System Thread", kSystemThread);
  kThreadManager_Run();

  DEBUG("All threads have terminated, kernel (lol) exiting");
  return 0;
}

/*
 *
 * Thread Manager
 *
 */
kThreadInitResult kThreadManager_Initialize() {
  gThreadManagerState = (kThreadManagerState*)calloc(1, sizeof(kThreadManagerState));
  gThreadManagerState->runningThreadSlot = -1;
  if (gThreadManagerState == 0) {
    DEBUG("Failed to initialize thread state");
    return kThreadInitResult_Fail;
  }

  return kThreadInitResult_Success;
}

int kThreadManager_FindAvailableSlot() {
  for ( int i = 0; i < KTHREAD_MAX; i++ ) {
    if (gThreadManagerState->threads[i].tid == 0) {
      return i;
    }
  }

  return -1;
}

kThread* kThreadManager_FindNextThread() {
  for(int i = gThreadManagerState->runningThreadSlot + 1; i < KTHREAD_MAX; i++) {
    if(gThreadManagerState->threads[i].tid != 0) {
      return &gThreadManagerState->threads[i];
    }
  }

  for(int i = 0; i < gThreadManagerState->runningThreadSlot; i++) {
    if(gThreadManagerState->threads[i].tid != 0) {
      return &gThreadManagerState->threads[i];
    }
  }

  return NULL;
}

void kThreadManager_Run() {
  kThread* nextThread = kThreadManager_FindNextThread();

  DEBUG("Going to run thread %p", nextThread);

  DEBUG("Going to run thread with tid %d and entry point %p", nextThread->tid, nextThread->func);

  nextThread->func();

  DEBUG("%s finished, Queueing next one.. (actually don't!)", nextThread->name);

}

int kThreadManager_QueueThread(char* name, kThreadFunc func) {

  int slotIndex = kThreadManager_FindAvailableSlot();

  if(slotIndex < 0) {
    DEBUG("Unable to find free slot");
    return -1;
  }

  int tid = ++gThreadManagerState->lastPid;

  DEBUG("Allocating thread with tid %d to slot %d", tid, slotIndex);

  kThread* thread = &gThreadManagerState->threads[slotIndex];
  thread->name = (char*)malloc(strlen(name));
  strcpy(thread->name, name);

  thread->func = func;

  thread->tid = tid;

  return thread->tid;
}

void kSystemThread() {
  DEBUG("System thread running!");
}

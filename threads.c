#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define NO_INLINE __attribute__((noinline))
#define KTHREAD_MAX 64
#define KTHREAD_STACK_SIZE (512 * 1024)
#define DEBUG(STR, PARAMS...) printf(STR "\n", ##PARAMS);
#define BREAK() asm volatile("int $3":::);

#define REGISTER_COUNT 9
#define REGISTER_SIZE 4
#define EIP (0)
#define EAX (EIP + REGISTER_SIZE)
#define EBX (EAX + REGISTER_SIZE)
#define ECX (EBX + REGISTER_SIZE)
#define EDX (ECX + REGISTER_SIZE)
#define EBP (EDX + REGISTER_SIZE)
#define ESI (EBP + REGISTER_SIZE)
#define EDI (ESI + REGISTER_SIZE)

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
  void*       stack;
  void*       stackHead;
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
void kThreadManager_InvokeThread();
void kThreadManager_Yield();
void kThreadManager_Push(kThread* thread, intptr_t value);
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

int kThreadManager_FindSlotIndexOfNextThread() {
  int runningThreadSlot = gThreadManagerState->runningThreadSlot;

  // Look for threads after the current thread
  for(int i = runningThreadSlot + 1; i < KTHREAD_MAX; i++) {
    if(gThreadManagerState->threads[i].tid != 0) {
      return i;
    }
  }

  // Look for threads before the current thread
  for(int i = 0; i < runningThreadSlot; i++) {
    if(gThreadManagerState->threads[i].tid != 0) {
      return i;
    }
  }

  // Fallback - run the current thread again
  if (runningThreadSlot >= 0) {
    return runningThreadSlot;
  }

  return -1;
}

void kThreadManager_InvokeThread() {
  DEBUG("InvokeThread: Enter");

  kThread* thread;
  kThreadFunc threadFunc;

  asm(""
      : "=a"(thread), "=b"(threadFunc)
      :
      :
     );

  threadFunc();

  DEBUG("InvokeThread: Exit");

  // TODO: Remove the thread from the thread table
}

void inline kThreadManager_Push(kThread* thread, intptr_t value) {
  thread->stackHead -= sizeof(intptr_t);
  *((intptr_t*)thread->stackHead) = value;
}

void NO_INLINE kThreadManager_Yield() {

  // Save the state of the current thread
  void* stackHead;

  // NOTE: At this point, GCC will have:
  //       - pushed the caller ebp on to the stack
  //       - set ebp to esp
  //       - allocated space for locals by offsetting
  //         esp by some amount
  //
  //       We need to undo all of this
  asm("\
      mov  %%ebp, %%esp;                      \
      popl %%ebp;                             \
      "
      // NOTE: top of the stack will now be the
      // callers return EIP
      "                                       \
      pushl %%eax;                            \
      pushl %%ebx;                            \
      pushl %%ecx;                            \
      pushl %%edx;                            \
      "
      // NOTE: this is the callER ebp because
      // we restored it at the start
      "                                       \
      pushl %%ebp;                            \
      pushl %%esi;                            \
      pushl %%edi;                            \
      "
      // set ebp to a safe value so the
      // remaining C value doesn't clobber our
      // beautiful construction
      // return the stack head
      "                                       \
      mov %%esp, %%ebp;                       \
      "
      // HACK HACK HACK leave space for locals
      "                                       \
      sub $0x18, %%esp;                       \
      "
      // return the head of the stack
      "                                       \
      mov %%ebp, %%eax;                       \
      "
      : "=a"(stackHead)
      : // we have no inputs
      : // we clobber nothing
     );

  kThread* thisThread = &gThreadManagerState->threads[gThreadManagerState->runningThreadSlot];
  thisThread->stackHead = stackHead;

  // Woohoo, time to run another thread
  kThreadManager_Run();
}

void kThreadManager_Run() {
  int nextThreadSlotIndex = kThreadManager_FindSlotIndexOfNextThread();
  kThread* nextThread = &gThreadManagerState->threads[nextThreadSlotIndex];

  DEBUG("Going to run thread %p", nextThread);

  DEBUG("Going to run thread with tid %d", nextThread->tid);

  gThreadManagerState->runningThreadSlot = nextThreadSlotIndex;

  asm("\
      mov %%eax, %%esp; \
      popl %%edi; \
      popl %%esi; \
      popl %%ebp; \
      popl %%edx; \
      popl %%ecx; \
      popl %%ebx; \
      popl %%eax; \
      ret; \
      "
      : // we have no outputs
      : "a"(nextThread->stackHead)
      : // We clobber everything, but GCC doesn't need to know that
     );

  // If we get here, something is seriously screwed!
  DEBUG("Gosh darn everything, we really oughn't to be here dear");
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

  // Thread metadata
  thread->name = (char*)malloc(strlen(name));
  strcpy(thread->name, name);
  thread->tid = tid;

  // Construct the initial thread stack
  thread->stack = calloc(KTHREAD_STACK_SIZE, 1);
  thread->stackHead = thread->stack + KTHREAD_STACK_SIZE;

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

void kSystemThread() {
  DEBUG("kSystemThread: Entered");
  kThreadManager_Yield();
  DEBUG("kSystemThread: Passed First Yield");
  kThreadManager_Yield();
  DEBUG("kSystemThread: Passed Second Yield, Going to Exit");
}

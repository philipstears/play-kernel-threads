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

kThreadManagerState* gThreadManagerState;

typedef enum {
  kThreadInitResult_Success = 0,
  kThreadInitResult_Fail = 1
} kThreadInitResult;

// Signature for a thread entry point
typedef void (*kThreadFunc)();

int kThreadManager_QueueThread(char* name, kThreadFunc func, int stackSize);
kThreadInitResult kThreadManager_Initialize();
void kThreadManager_Run();

// Yield to another thread
extern void kThreadManager_Yield();

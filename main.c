#include <stdio.h>
#include "threads.h"
#include "threadManager.h"
#include "utility.h"

void kChildThread() {
  DEBUG("kChildThread: Entered, Yielding");
  kThread_Yield();
  DEBUG("kChildThread: Passed First Yield");
  kThread_Yield();
  DEBUG("kChildThread: Passed Second Yield, Going to Exit");
}

void kSystemThread() {
  DEBUG("kSystemThread: Entered, Queuing Child and Yielding");
  kThread_Queue("Child", kChildThread, KTHREAD_STACK_SMALL);
  kThread_Yield();
  DEBUG("kSystemThread: Passed First Yield");
  kThread_Yield();
  DEBUG("kSystemThread: Passed Second Yield, Going to Exit");
}

int main(int argc, char* argv[]) {
  DEBUG("Hello, kThread world!");

  kThreadManager_Initialize();
  kThread_Queue("System Thread", kSystemThread, KTHREAD_STACK_SMALL);

  // NOTE: Next should never actually exit because there
  // always *should* be more threads to run in a real
  // case
  kThreadManager_Next();

  DEBUG("All threads have terminated, kernel (lol) exiting");
  return 0;
}

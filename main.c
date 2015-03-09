#include "threads.h"
#include "utility.h"

void kChildThread() {
  DEBUG("kChildThread: Entered, Yielding");
  kThreadManager_Yield();
  DEBUG("kChildThread: Passed First Yield");
  kThreadManager_Yield();
  DEBUG("kChildThread: Passed Second Yield, Going to Exit");
}

void kSystemThread() {
  DEBUG("kSystemThread: Entered, Queuing Child and Yielding");
  kThreadManager_QueueThread("Child", kChildThread, KTHREAD_STACK_SMALL);
  kThreadManager_Yield();
  DEBUG("kSystemThread: Passed First Yield");
  kThreadManager_Yield();
  DEBUG("kSystemThread: Passed Second Yield, Going to Exit");
}

int main(int argc, char* argv[]) {
  DEBUG("Hello, kThread world!");

  kThreadManager_Initialize();
  kThreadManager_QueueThread("System Thread", kSystemThread, KTHREAD_STACK_SMALL);
  kThreadManager_Run();

  DEBUG("All threads have terminated, kernel (lol) exiting");
  return 0;
}

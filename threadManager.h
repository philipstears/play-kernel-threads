typedef enum {
  kThreadInitResult_Success = 0,
  kThreadInitResult_Fail = 1
} kThreadInitResult;


kThreadInitResult kThreadManager_Initialize();

void kThreadManager_Next();

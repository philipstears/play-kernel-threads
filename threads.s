; vim: ft=nasm
section .data

global kThread_Yield
global kThread_Next_Core
extern kThread_Yield_Core

kThread_Yield:
  push eax
  push ebx
  push ecx
  push edx
  push ebp
  push esi
  push edi

  push esp
  call kThread_Yield_Core

kThread_Next_Core:
  mov esp, [esp+4]
  pop edi
  pop esi
  pop ebp
  pop edx
  pop ecx
  pop ebx
  pop eax
  ret

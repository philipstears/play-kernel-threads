; vim: ft=nasm
section .data

global kThread_Yield
global kThread_Next_Core
extern kThread_Yield_Core

kThread_Yield:
  push ebx
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
  pop ebx
  ret

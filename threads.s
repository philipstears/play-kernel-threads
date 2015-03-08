; vim: ft=nasm
section .data

global kThreadManager_Yield
extern kThreadManager_Yield_Core

kThreadManager_Yield:
  push eax
  push ebx
  push ecx
  push edx
  push ebp
  push esi
  push edi

  push esp
  call kThreadManager_Yield_Core

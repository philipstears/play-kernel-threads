; vim: ft=nasm
section .data

global kThread_Yield
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

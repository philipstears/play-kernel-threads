# Todo

- Don't need a null pointer at the top of the stack now we've got a trampoline
- Make sure we're preserving the right registers and only the ones we need - EAX, ECX, EDX are preserved by the caller in cdecl for example
- Sleeping
- Break out into multiple files
- Fix naming
- Handle max threads reached and wrapping around to find empty stack slots


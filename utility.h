#define NO_INLINE __attribute__((noinline))
#define DEBUG(STR, PARAMS...) printf(STR "\n", ##PARAMS);
#define BREAK() asm volatile("int $3":::);

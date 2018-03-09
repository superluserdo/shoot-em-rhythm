#include <stdint.h> // For rdtsc for now

uint64_t rdtsc()
{
	uint32_t lo, hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ( ((uint64_t)hi) << (uint32_t)32 )
	     | ( ((uint64_t)lo) );
}

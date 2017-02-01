#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stdbool.h>

//============================================
// Macros, inline functions
//============================================

#define Swap(v0, v1) do {						\
	typeof(v0) zzzswap = v0; v0 = v1; v1 = zzzswap;			\
} while(0)

#define abort_printf(fmt, ...) do {					\
	printf("ERROR %s:%u : " fmt, __FILE__, __LINE__, ##__VA_ARGS__);\
	fflush(stdout);							\
	exit(EXIT_FAILURE);						\
} while(0)

#define malloc_check(size) malloc_with_loc(size, __FILE__, __LINE__)

static inline unsigned uint_roundup(unsigned addr, unsigned align)
{
	if(addr % align == 0) return addr;
	return addr + align - addr % align;
}

//============================================
// Functions
//============================================

double XTime_ToDouble(XTime *t);
double XTime_GetDiff_Double(XTime *oldtime, XTime *newtime);
double XTime_DiffCurrReal_Double(XTime *oldtime);
void* malloc_with_loc(unsigned size, char* file, unsigned line);

#endif

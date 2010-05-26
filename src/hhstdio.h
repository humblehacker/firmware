#ifndef __HHSTDIO_H__
#define __HHSTDIO_H__

#include <stdio.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

extern void init_stdio(void);
extern bool stdout_is_empty(void);
extern char popchar(void);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // __HHSTDIO_H__

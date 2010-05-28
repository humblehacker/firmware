#ifndef __HHSTDIO_H__
#define __HHSTDIO_H__

#include <stdio.h>
#include <stdbool.h>

void stdout_init(void);
bool stdout_is_empty(void);
char stdout_popchar(void);

#endif // __HHSTDIO_H__

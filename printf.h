#ifndef __TFP_PRINTF__
#define __TFP_PRINTF__

#include <stdarg.h>

void init_printf(void *putp, void (*putf), void *char);

void tfp_printf(char *fmt, ...);
void tfp_sprintf(char *s, char *fmt, ...);

void tfp_format(void *putp, void (*putf) // ...
// Copyright 2023 Lawrence Livermore National Security, LLC and other
// libjustify Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#ifndef __CPRINTF_H_HEADER
#define __CPRINTF_H_HEADER

#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void cprintf(const char *fmt, ...);

void cfprintf(FILE *stream, const char *fmt, ...);

void cvprintf(const char *fmt, va_list args);

void cvfprintf(FILE *stream, const char *fmt, va_list args);

void dump_graph();

void cflush(void);

#endif

#ifdef __cplusplus
}
#endif

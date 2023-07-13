% CPRINTF(3) Version 0.0 | cprintf, cfprintf, csnprintf, cvfprintf, cvsprintf, cvsnprintf, cflush
% Barry Rountree (rountree@llnl.gov)

NAME
====
cprintf and friends - automatically formatted table output

SYNOPSIS
========
#include <cprintf.h>

-lcprintf

int cprintf(const char *format, ...);

int cfprintf(FILE *stream, const char *format, ...);

void* cflush();

DESCRIPTION
===========

Libjustify is a simple library that wraps the printf() family of functions and offers a new family: `cprintf()` that emulates their behavior while automatically justifying and formatting output to create a table.   
   

Installing
===========
Installation is as simple as:

``` 
git clone https://github.com/LLNL/libjustify.git
mkdir build && cd build
cmake ..
cmake --build .
```
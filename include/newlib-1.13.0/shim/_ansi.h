/* shim: the newlib sources under include/newlib-1.13.0/ include their libc headers by name; this directory answers
   those includes with the few declarations the sources use (src/BOOT/newlib.h has the ANSI macros). */
#include "../../../src/BOOT/newlib.h"

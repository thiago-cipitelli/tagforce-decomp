/* Varargs for mwccpsp (MIPS EABI, 8 argument registers). Reproduces the MSL expansion seen in
   BOOT: va_start = top of frame minus the spill of the unused argument registers, computed from
   __builtin_args_info(2) (the number of fixed arguments) without constant folding. */
#ifndef STDARG_H
#define STDARG_H

typedef char *va_list;

#define va_start(ap, parm) \
    ((ap) = (va_list)__builtin_next_arg(parm) - ((__builtin_args_info(2) >= 8) ? 0 : (8 - __builtin_args_info(2)) * 4))
/* 8-byte types are aligned to 8 inside the argument area */
#define va_arg(ap, T) \
    (*(T *)(((ap) = (ap) + ((sizeof(T) == 8 && ((unsigned int)(ap) & 7)) ? 4 : 0) + sizeof(T)) - sizeof(T)))
#define va_end(ap) ((void)0)

#endif

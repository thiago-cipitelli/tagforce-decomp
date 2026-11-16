/* newlib 1.12.0's _REENT_INIT_PTR as the Sony SDK has it: _atexit0._next cleared, and a word the SDK added after
   __sf (inside the 8-byte padding of newlib's struct _reent). c_str is the "C" of the object that expands it (crt0
   and the libc's _REENT_INIT function each have their own). Needs newlib's <sys/reent.h> and memset. */
#ifndef SDK_REENT_H
#define SDK_REENT_H

#define SDK_REENT_INIT_PTR(var, c_str) \
  { \
    int i; \
    char *tmp_ptr; \
    var->_errno = 0; \
    var->_stdin = &var->__sf[0]; \
    var->_stdout = &var->__sf[1]; \
    var->_stderr = &var->__sf[2]; \
    var->_inc = 0; \
    for (i = 0; i < _REENT_EMERGENCY_SIZE; ++i) \
        var->_emergency[i] = 0; \
    var->_current_category = 0; \
    var->_current_locale = c_str; \
    var->__sdidinit = 0; \
    var->__cleanup = 0; \
    var->_result = 0; \
    var->_result_k = 0; \
    var->_p5s = 0; \
    var->_freelist = 0; \
    var->_cvtlen = 0; \
    var->_cvtbuf = 0; \
    var->_new._reent._unused_rand = 0; \
    var->_new._reent._strtok_last = 0; \
    var->_new._reent._asctime_buf[0] = 0; \
    tmp_ptr = (char *)&var->_new._reent._localtime_buf; \
    for (i = 0; i < sizeof(struct __tm); ++i) \
        tmp_ptr[i] = 0; \
    var->_new._reent._gamma_signgam = 0; \
    var->_new._reent._rand_next = 1; \
    var->_new._reent._r48._seed[0] = _RAND48_SEED_0; \
    var->_new._reent._r48._seed[1] = _RAND48_SEED_1; \
    var->_new._reent._r48._seed[2] = _RAND48_SEED_2; \
    var->_new._reent._r48._mult[0] = _RAND48_MULT_0; \
    var->_new._reent._r48._mult[1] = _RAND48_MULT_1; \
    var->_new._reent._r48._mult[2] = _RAND48_MULT_2; \
    var->_new._reent._r48._add = _RAND48_ADD; \
    var->_new._reent._mblen_state.__count = 0; \
    var->_new._reent._mblen_state.__value.__wch = 0; \
    var->_new._reent._mbtowc_state.__count = 0; \
    var->_new._reent._mbtowc_state.__value.__wch = 0; \
    var->_new._reent._wctomb_state.__count = 0; \
    var->_new._reent._wctomb_state.__value.__wch = 0; \
    var->_new._reent._mbrlen_state.__count = 0; \
    var->_new._reent._mbrlen_state.__value.__wch = 0; \
    var->_new._reent._mbrtowc_state.__count = 0; \
    var->_new._reent._mbrtowc_state.__value.__wch = 0; \
    var->_new._reent._mbsrtowcs_state.__count = 0; \
    var->_new._reent._mbsrtowcs_state.__value.__wch = 0; \
    var->_new._reent._wcrtomb_state.__count = 0; \
    var->_new._reent._wcrtomb_state.__value.__wch = 0; \
    var->_new._reent._wcsrtombs_state.__count = 0; \
    var->_new._reent._wcsrtombs_state.__value.__wch = 0; \
    var->_new._reent._l64a_buf[0] = '\0'; \
    var->_new._reent._signal_buf[0] = '\0'; \
    var->_new._reent._getdate_err = 0; \
    var->_atexit = 0; \
    var->_atexit0._next = 0; \
    var->_atexit0._ind = 0; \
    var->_atexit0._fns[0] = 0; \
    var->_atexit0._on_exit_args._fntypes = 0; \
    var->_atexit0._on_exit_args._fnargs[0] = 0; \
    var->_sig_func = 0; \
    var->__sglue._next = 0; \
    var->__sglue._niobs = 0; \
    var->__sglue._iobs = 0; \
    memset(var->__sf, 0, sizeof(var->__sf)); \
    *(int *)&var->__sf[3] = 0; \
  }

#endif

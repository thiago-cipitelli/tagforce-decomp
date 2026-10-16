extern int cfg_get_language(void);

/* 0x0880439c: maps the config language id to the download-site language index */
int lang_to_dl_index(void)
{
    switch (cfg_get_language()) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 6;
    case 3: return 2;
    case 4: return 5;
    case 5: return 3;
    default: return 1;
    }
}

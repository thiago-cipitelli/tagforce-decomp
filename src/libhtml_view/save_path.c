extern char *strcpy(char *, const char *); extern char *strcat(char *, const char *); extern void *memset(void *, int, unsigned int);

extern void region_game_id(char *out);

/* 0x088044b8: builds "/PSP/SAVEDATA/<GAMEID>" */
void save_path_build(char *out)
{
    char id[32];

    strcpy(out, "/PSP/SAVEDATA/");
    memset(id, 0, sizeof id);
    region_game_id(id);
    strcat(out, id);
}

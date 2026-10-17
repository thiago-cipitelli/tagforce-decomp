/* CIP / CPM card image archives (card/cardh_<l>.cip, card/cardm_<l>.cip): fixed-size slots of
   `chunks` x 0x800 bytes read on demand through ehfile_read_async into a caller-supplied buffer that
   holds the Cip header, the id table and the slot ring. See docs/formats.md. */
#ifndef CIP_H
#define CIP_H

typedef struct CipEntry {              /* one per card id from first_id to last_id */
    unsigned int chunk;                /* 0x800-byte chunk index; bit 31: offset of a variant list */
    unsigned int data;                 /* 0 none, 1 requested, 2 loading, else pointer to the image */
} CipEntry;

typedef struct CipHeader {             /* 0x30 bytes, "CIP\x1a" or "CPM\x1a" */
    unsigned int magic;
    unsigned int size;
    unsigned int pad8, padc;
    unsigned int table_size;           /* +0x10 bytes of header + table */
    int first_id;                      /* +0x14 */
    int last_id;                       /* +0x18 */
    unsigned int table_off;            /* +0x1C offset of the CipEntry table (0x30) */
    int chunks;                        /* +0x20 chunks per slot */
    unsigned int pad24, pad28, pad2c;
} CipHeader;

typedef struct Cip {
    char path[0x80];
    int dir;                           /* +0x80 ehfile dir (3) */
    int bufsize;                       /* +0x84 bytes available from this struct on */
    char ready : 1;                    /* +0x88 header and table loaded */
    char busy : 1;                     /* a slot read is in flight */
    char prefetch : 1;                 /* read the whole file sequentially (cip_prefetch) */
    char closed : 1;
    char pad88 : 4;
    unsigned char pad89[3];
    CipEntry *cur;                     /* +0x8C entry being loaded */
    unsigned char *cur_buf;            /* +0x90 */
    unsigned char *slots;              /* +0x94 16-aligned slot ring after the table */
    int nslots;                        /* +0x98 */
    int next_slot;                     /* +0x9C */
    int unka0;
    int prefetch_idx;                  /* +0xA4 slot being prefetched, -1 when idle */
    int unka8;
    CipHeader hdr;                     /* +0xAC file data starts here */
} Cip;                                 /* 0xDC */

/* The handle is the caller's buffer (a Cip followed by the file data); the exported API takes void *. */
void *cip_open_h(void *handle, int bufsize, int prefetch);
void *cip_open_m(void *handle, int bufsize, int prefetch);
int cip_is_loaded(Cip *c);
void cip_close(Cip *c);
int cip_is_idle(Cip *c);
unsigned int cip_read_card(void *handle, int id, int variant);
unsigned int *cip_get_image(unsigned int *list, void *handle, int id, int variant, int *loaded_out);
int cip_query(void *handle, int id, int variant);

#endif

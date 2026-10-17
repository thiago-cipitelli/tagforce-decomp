// @range 0x08862C5C-0x08862D28
/* Sound library random generator (state is volatile: the original reloads it at every use) (shift-register pair), used for random sound variations. The
   SAS wrappers right after this file come from a different toolchain (sw ra,0(sp) prologue) and
   stay in asm. */

extern volatile unsigned int snd_rng_b;     /* 0x08C1AFC0 */
extern volatile unsigned int snd_rng_a;     /* 0x08C1AFC4 */

unsigned int snd_rand(void);

void snd_rand_seed(unsigned int seed)
{
    snd_rng_a = seed;
    snd_rng_b = seed;
}

unsigned int snd_rand(void)
{
    unsigned int t = (snd_rng_a << 31) | (snd_rng_b >> 1);

    snd_rng_a += snd_rng_a + (snd_rng_b & 1);
    t ^= snd_rng_b << 12;
    return snd_rng_b = t ^ (t >> 20);
}

/* 0..n-1 */
int snd_rand_range(int n)
{
    if (n < 2)
        return 0;
    return (unsigned int)(n * (unsigned short)snd_rand()) >> 16;
}

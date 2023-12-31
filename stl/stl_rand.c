/*************************************************************************
    > File Name: stl_rand.c
  > Author:perrynzhou
  > Mail:perrynzhou@gmail.com
  > Created Time: Mon 05 Sep 2022 11:10:48 PM EDT
 ************************************************************************/

#include "stl_rand.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define UUID_MIN_LEN (37)
#define N 16
#define MASK ((1 << (N - 1)) + (1 << (N - 1)) - 1)
#define LOW(x) ((unsigned)(x)&MASK)
#define HIGH(x) LOW((x) >> N)
#define MUL(x, y, z)                       \
    {                                      \
        int32_t l = (long)(x) * (long)(y); \
        (z)[0] = LOW(l);                   \
        (z)[1] = HIGH(l);                  \
    }
#define CARRY(x, y) ((int32_t)(x) + (long)(y) > MASK)
#define ADDEQU(x, y, z) (z = CARRY(x, (y)), x = LOW(x + (y)))
#define X0 0x330E
#define X1 0xABCD
#define X2 0x1234
#define A0 0xE66D
#define A1 0xDEEC
#define A2 0x5
#define C 0xB
#define SET3(x, x0, x1, x2) ((x)[0] = (x0), (x)[1] = (x1), (x)[2] = (x2))
#define SETLOW(x, y, n) SET3(x, LOW((y)[n]), LOW((y)[(n) + 1]), LOW((y)[(n) + 2]))
#define REST(v)             \
    for (i = 0; i < 3; i++) \
    {                       \
        xsubi[i] = x[i];    \
        x[i] = temp[i];     \
    }                       \
    return (v);
#define HI_BIT (1L << (2 * N - 1))

static uint32_t x[3] = {X0, X1, X2}, a[3] = {A0, A1, A2}, c = C;

static const char *stl_rand_chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void stl_rand_next(void)
{
    uint32_t p[2], q[2], r[2], carry0, carry1;

    MUL(a[0], x[0], p);
    ADDEQU(p[0], c, carry0);
    ADDEQU(p[1], carry0, carry1);
    MUL(a[0], x[1], q);
    ADDEQU(p[1], q[0], carry0);
    MUL(a[1], x[0], r);
    x[2] = LOW(carry0 + carry1 + CARRY(p[1], r[0]) + q[1] + r[1] +
               a[0] * x[2] + a[1] * x[1] + a[2] * x[0]);
    x[1] = LOW(p[1] + r[0]);
    x[0] = LOW(p[0]);
}

int32_t stl_rand_int32()
{
    stl_rand_next();
    return (((int32_t)x[2] << (N - 1)) + (x[1] >> 1));
}
int stl_rand_str(char *buf, size_t n)
{
    if (buf && n > 0)
    {
        for (size_t i = 0; i < n; i++)
        {
            stl_rand_next();
            int32_t r = (((int32_t)x[2] << (N - 1)) + (x[1] >> 1));
            uint32_t hash = (r < 0) ? (~r + 1) : r;
            buf[i] = stl_rand_chars[hash % n];
        }
        buf[n] = '\0';
        return 0;
    }
    return -1;
}
int stl_gen_uuid(char *buf, size_t n)
{
    if (buf && n >= UUID_MIN_LEN)
    {
        size_t n = strlen(stl_rand_chars);
        for (size_t i = 0; i < UUID_MIN_LEN; i++)
        {

            if (i == 8 || i == 13 || i == 18 || i == 23)
            {
                buf[i] = '-';
            }
            else
            {
                stl_rand_next();
                int32_t r = (((int32_t)x[2] << (N - 1)) + (x[1] >> 1));
                uint32_t hash = (r < 0) ? (~r + 1) : r;
                char c = stl_rand_chars[hash % n];
                if (c >= 'A' && c <= 'Z')
                {
                    c += 32;
                }
                buf[i] = c;
            }
        }
        buf[UUID_MIN_LEN] = '\0';
    }
    return 0;
}
void stl_rand_init(stl_rand *r, const unsigned char *init)
{
    unsigned char t;

    *r = (stl_rand){0};

    memcpy(r->init, init, 256);

    for (int i = 0; i < 256; i++)
    {
        r->j += r->init[i] + init[i];
        t = r->init[r->j];
        r->init[r->j] = r->init[i];
        r->init[i] = t;
    }
}

void stl_rand_read(stl_rand *r, void *buf, int size)
{
    unsigned char t;
    unsigned char *p = buf;

    if (size <= 0 || buf == NULL)
    {
        return;
    }

    do
    {
        r->i++;
        t = r->init[r->i];
        r->j += t;
        r->init[r->i] = r->init[r->j];
        r->init[r->j] = t;
        t += r->init[r->i];
        *(p++) = r->init[t];
    } while (--size);
}

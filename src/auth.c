/* auth.c - SHA-256, HMAC, PBKDF2. Reines C, ohne Abhaengigkeiten. */
#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint32_t K[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};
static const uint32_t H0[8] = {
    0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
    0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
};

typedef struct { uint32_t h[8]; uint8_t buf[64]; size_t n; uint64_t total; } sha_t;

#define ROR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))

static void block(sha_t *s, const uint8_t *p)
{
    uint32_t w[64], a, b, c, d, e, f, g, h, t1, t2;
    int i;
    for (i = 0; i < 16; i++)
        w[i] = ((uint32_t)p[4*i] << 24) | ((uint32_t)p[4*i+1] << 16) |
               ((uint32_t)p[4*i+2] << 8) | (uint32_t)p[4*i+3];
    for (i = 16; i < 64; i++) {
        uint32_t s0 = ROR(w[i-15], 7) ^ ROR(w[i-15], 18) ^ (w[i-15] >> 3);
        uint32_t s1 = ROR(w[i-2], 17) ^ ROR(w[i-2], 19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    a = s->h[0]; b = s->h[1]; c = s->h[2]; d = s->h[3];
    e = s->h[4]; f = s->h[5]; g = s->h[6]; h = s->h[7];
    for (i = 0; i < 64; i++) {
        t1 = h + (ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25)) + ((e & f) ^ (~e & g)) + K[i] + w[i];
        t2 = (ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    s->h[0] += a; s->h[1] += b; s->h[2] += c; s->h[3] += d;
    s->h[4] += e; s->h[5] += f; s->h[6] += g; s->h[7] += h;
}

static void sha_init(sha_t *s)
{
    memcpy(s->h, H0, sizeof H0);
    s->n = 0; s->total = 0;
}

static void sha_update(sha_t *s, const uint8_t *d, size_t len)
{
    s->total += len;
    while (len > 0) {
        size_t k = 64 - s->n;
        if (k > len) k = len;
        memcpy(s->buf + s->n, d, k);
        s->n += k; d += k; len -= k;
        if (s->n == 64) { block(s, s->buf); s->n = 0; }
    }
}

static void sha_final(sha_t *s, uint8_t out[32])
{
    uint64_t bits = s->total * 8;
    uint8_t pad = 0x80, z = 0, l[8];
    int i;
    for (i = 0; i < 8; i++) l[i] = (uint8_t)(bits >> (56 - 8*i));
    sha_update(s, &pad, 1);
    while (s->n != 56) sha_update(s, &z, 1);
    sha_update(s, l, 8);
    for (i = 0; i < 8; i++) {
        out[4*i] = (uint8_t)(s->h[i] >> 24); out[4*i+1] = (uint8_t)(s->h[i] >> 16);
        out[4*i+2] = (uint8_t)(s->h[i] >> 8); out[4*i+3] = (uint8_t)s->h[i];
    }
}

void auth_sha256(const uint8_t *data, size_t len, uint8_t out[32])
{
    sha_t s; sha_init(&s); sha_update(&s, data, len); sha_final(&s, out);
}

void auth_hmac_sha256(const uint8_t *key, size_t klen, const uint8_t *msg,
                      size_t mlen, uint8_t out[32])
{
    uint8_t k[64], ip[64], op[64], in[32];
    sha_t s; size_t i;
    memset(k, 0, sizeof k);
    if (klen > 64) auth_sha256(key, klen, k); else if (klen) memcpy(k, key, klen);
    for (i = 0; i < 64; i++) { ip[i] = k[i] ^ 0x36; op[i] = k[i] ^ 0x5c; }
    sha_init(&s); sha_update(&s, ip, 64); sha_update(&s, msg, mlen); sha_final(&s, in);
    sha_init(&s); sha_update(&s, op, 64); sha_update(&s, in, 32); sha_final(&s, out);
}

void auth_pbkdf2(const char *pw, const uint8_t *salt, size_t slen,
                 unsigned iter, uint8_t out[32])
{
    uint8_t buf[256], u[32], t[32];
    size_t plen = strlen(pw), i;
    unsigned it;
    if (slen > 128) slen = 128;
    memcpy(buf, salt, slen);
    buf[slen] = 0; buf[slen+1] = 0; buf[slen+2] = 0; buf[slen+3] = 1;
    auth_hmac_sha256((const uint8_t *)pw, plen, buf, slen + 4, u);
    memcpy(t, u, 32);
    for (it = 1; it < iter; it++) {
        auth_hmac_sha256((const uint8_t *)pw, plen, u, 32, u);
        for (i = 0; i < 32; i++) t[i] ^= u[i];
    }
    memcpy(out, t, 32);
}

static void tohex(const uint8_t *d, size_t n, char *o)
{
    static const char x[] = "0123456789abcdef";
    size_t i;
    for (i = 0; i < n; i++) { o[2*i] = x[d[i] >> 4]; o[2*i+1] = x[d[i] & 15]; }
    o[2*n] = 0;
}

static int hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int fromhex(const char *s, size_t n, uint8_t *o)
{
    size_t i;
    for (i = 0; i < n; i++) {
        int a = hexval(s[2*i]), b = hexval(s[2*i+1]);
        if (a < 0 || b < 0) return 0;
        o[i] = (uint8_t)(a * 16 + b);
    }
    return 1;
}

int auth_make_hash(const char *pw, const uint8_t salt[16], unsigned iter,
                   char *out, size_t outsz)
{
    uint8_t h[32]; char sh[33], hh[65];
    int n;
    auth_pbkdf2(pw, salt, 16, iter, h);
    tohex(salt, 16, sh); tohex(h, 32, hh);
    n = snprintf(out, outsz, "pbkdf2-sha256:%u:%s:%s", iter, sh, hh);
    return n > 0 && (size_t)n < outsz;
}

/* Zerlegt; liefert 1 bei gueltigem Format. */
static int split(const char *st, unsigned *iter, uint8_t salt[16], uint8_t hash[32])
{
    const char *p; char *e; unsigned long it;
    if (strncmp(st, "pbkdf2-sha256:", 14) != 0) return 0;
    p = st + 14;
    it = strtoul(p, &e, 10);
    if (e == p || *e != ':' || it < 1 || it > 10000000UL) return 0;
    p = e + 1;
    if (strlen(p) != 32 + 1 + 64 || p[32] != ':') return 0;
    if (!fromhex(p, 16, salt) || !fromhex(p + 33, 32, hash)) return 0;
    *iter = (unsigned)it;
    return 1;
}

int auth_format_ok(const char *stored)
{
    unsigned it; uint8_t s[16], h[32];
    return split(stored, &it, s, h);
}

int auth_verify(const char *pw, const char *stored)
{
    unsigned it; uint8_t s[16], h[32], c[32], diff = 0;
    int i;
    if (!split(stored, &it, s, h)) return 0;
    auth_pbkdf2(pw, s, 16, it, c);
    for (i = 0; i < 32; i++) diff |= (uint8_t)(c[i] ^ h[i]);
    return diff == 0;
}

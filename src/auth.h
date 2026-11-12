/* auth.h - Passwort-Pruefung: PBKDF2-HMAC-SHA256, reines C, kein IO. */
#ifndef AUTH_H
#define AUTH_H
#include <stddef.h>
#include <stdint.h>

#define AUTH_ITER_DEFAULT 20000
#define AUTH_HASH_MAX 160

/* Rohfunktionen (auch fuer Tests) */
void auth_sha256(const uint8_t *data, size_t len, uint8_t out[32]);
void auth_hmac_sha256(const uint8_t *key, size_t klen, const uint8_t *msg,
                      size_t mlen, uint8_t out[32]);
void auth_pbkdf2(const char *pw, const uint8_t *salt, size_t slen,
                 unsigned iter, uint8_t out[32]);

/* Erzeugt "pbkdf2-sha256:ITER:SALTHEX:HASHHEX" (salt 16 Byte vorgegeben). */
int auth_make_hash(const char *pw, const uint8_t salt[16], unsigned iter,
                   char *out, size_t outsz);
/* 1 = Passwort passt, 0 = nein oder Format falsch. */
int auth_verify(const char *pw, const char *stored);
/* 1 = Format gueltig. */
int auth_format_ok(const char *stored);
#endif

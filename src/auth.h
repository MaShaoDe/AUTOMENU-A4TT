/*
 * Copyright (c) 2026, FerienBSD aka Marcel Sauder
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* auth.h - password check: PBKDF2-HMAC-SHA256, plain C, no IO. */
#ifndef AUTH_H
#define AUTH_H
#include <stddef.h>
#include <stdint.h>

#define AUTH_ITER_DEFAULT 20000
#define AUTH_HASH_MAX 160

/* Raw functions (also used by the tests) */
void auth_sha256(const uint8_t *data, size_t len, uint8_t out[32]);
void auth_hmac_sha256(const uint8_t *key, size_t klen, const uint8_t *msg,
                      size_t mlen, uint8_t out[32]);
void auth_pbkdf2(const char *pw, const uint8_t *salt, size_t slen,
                 unsigned iter, uint8_t out[32]);

/* Creates "pbkdf2-sha256:ITER:SALTHEX:HASHHEX" (16 byte salt is given). */
int auth_make_hash(const char *pw, const uint8_t salt[16], unsigned iter,
                   char *out, size_t outsz);
/* 1 = password matches, 0 = no or bad format. */
int auth_verify(const char *pw, const char *stored);
/* 1 = format valid. */
int auth_format_ok(const char *stored);
#endif

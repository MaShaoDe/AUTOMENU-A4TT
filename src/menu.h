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

/* menu.h - data model of a menu (plain data, no logic) */
#ifndef AUTOMENU_MENU_H
#define AUTOMENU_MENU_H

#include <stddef.h>

typedef enum {
    MENU_ITEM_RUN,   /* arg = command to run                   */
    MENU_ITEM_MENU,  /* arg = id of the submenu                */
    MENU_ITEM_BACK,  /* back to the previous menu              */
    MENU_ITEM_EXIT,  /* quit the program                       */
    MENU_ITEM_INFO   /* arg = text (optional), else the label  */
} menu_item_type_t;

typedef struct {
    const char *label;
    menu_item_type_t type;
    const char *arg;
    const char *desc;   /* help text for the grey bar, may be NULL */
    const char *lock;   /* password name, NULL = not locked */
} menu_item_t;

typedef struct {
    const char *id;
    const char *title;
    menu_item_t *items;
    size_t item_count;
} menu_t;

typedef struct {
    const char *name;
    const char *hash;   /* pbkdf2-sha256:ITER:SALTHEX:HASHHEX */
} password_t;

/* A whole menu definition file (.mdf) */
typedef struct {
    menu_t *menus;
    size_t menu_count;
    password_t *passwords;
    size_t password_count;
    int blackout_s;   /* seconds of inactivity until blackout, 0 = off */
    int clock_utc;    /* 1 = clock in UTC */
    int mouse;        /* 1 = mouse on (default) */
} mdf_t;

#endif

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

/* menu.h - Datenmodell eines Menüs (reine Daten, keine Logik) */
#ifndef AUTOMENU_MENU_H
#define AUTOMENU_MENU_H

#include <stddef.h>

typedef enum {
    MENU_ITEM_RUN,   /* arg = Befehl, der ausgeführt wird      */
    MENU_ITEM_MENU,  /* arg = ID des Untermenüs                */
    MENU_ITEM_BACK,  /* zurück zum vorigen Menü                */
    MENU_ITEM_EXIT,  /* Programm beenden                       */
    MENU_ITEM_INFO   /* arg = Text (optional), sonst das Label */
} menu_item_type_t;

typedef struct {
    const char *label;
    menu_item_type_t type;
    const char *arg;
    const char *desc;   /* Hilfetext für die graue Leiste, darf NULL sein */
    const char *lock;   /* Name des Passworts, NULL = nicht gesperrt */
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

/* Eine ganze Menüdefinitionsdatei (.mdf) */
typedef struct {
    menu_t *menus;
    size_t menu_count;
    password_t *passwords;
    size_t password_count;
    int blackout_s;   /* Sekunden Inaktivität bis Blackout, 0 = aus */
    int clock_utc;    /* 1 = Uhr in UTC */
    int mouse;        /* 1 = Maus an (Standard) */
} mdf_t;

#endif

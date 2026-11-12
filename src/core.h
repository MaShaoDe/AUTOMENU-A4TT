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

/* core.h - Menükern: reine State Machine, kein IO, kein Rendering, kein Parsen */
#ifndef AUTOMENU_CORE_H
#define AUTOMENU_CORE_H

#include <stddef.h>
#include "menu.h"

typedef struct core core_t;

typedef enum {
    CORE_EVENT_UP,
    CORE_EVENT_DOWN,
    CORE_EVENT_SELECT,
    CORE_EVENT_BACK
} core_event_t;

typedef struct {
    const char *id;
    const char *title;
    const menu_item_t *items;
    size_t item_count;
    size_t current_index;
    size_t menu_index;   /* Position des Menüs in der Datei (ab 0) */
    size_t menu_count;
} menu_view_t;

core_t *core_create(void);
void core_destroy(core_t *core);

/* Das erste Menü ist das Startmenü. Rückgabe: 1 = ok, 0 = Fehler. */
int core_load(core_t *core, const menu_t *menus, size_t menu_count);

void core_input(core_t *core, core_event_t event);
void core_select_index(core_t *core, size_t index);  /* springt und wählt aus */

menu_view_t core_view(const core_t *core);

/* Ergebnisse abholen (liefern NULL, wenn nichts ansteht) */
const char *core_poll_run(core_t *core);
const char *core_poll_info(core_t *core);
int core_should_exit(const core_t *core);

/* Passwortschutz: Eine Auswahl auf einem gesperrten Eintrag führt nichts aus,
   sondern setzt eine Abfrage. Der Adapter prüft das Passwort und ruft dann
   core_unlock_ok() (Eintrag wird ausgeführt) oder core_unlock_cancel(). */
const char *core_locked_name(const core_t *core);   /* NULL = keine Abfrage */
const char *core_locked_label(const core_t *core);
void core_unlock_ok(core_t *core);
void core_unlock_cancel(core_t *core);

#endif

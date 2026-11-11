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

#endif

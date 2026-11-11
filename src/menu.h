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
} menu_item_t;

typedef struct {
    const char *id;
    const char *title;
    menu_item_t *items;
    size_t item_count;
} menu_t;

#endif

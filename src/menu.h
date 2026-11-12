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

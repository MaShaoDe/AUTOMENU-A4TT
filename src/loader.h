/* loader.h - liest und prüft die Menüdatei (Format: siehe COOKBOOK.md) */
#ifndef AUTOMENU_LOADER_H
#define AUTOMENU_LOADER_H

#include <stddef.h>
#include "menu.h"

/* Rückgabe: 1 = ok, 0 = Fehler (Meldung "name:zeile: text" auf stderr). */
int menu_parse_text(const char *text, const char *name,
                    menu_t **out_menus, size_t *out_count);
int menu_load_file(const char *path, menu_t **out_menus, size_t *out_count);
void menu_free(menu_t *menus, size_t count);

#endif

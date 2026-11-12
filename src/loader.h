/* loader.h - liest und prüft die Menüdefinitionsdatei (.mdf, siehe COOKBOOK.md) */
#ifndef AUTOMENU_LOADER_H
#define AUTOMENU_LOADER_H

#include <stddef.h>
#include "menu.h"

/* Rückgabe: 1 = ok, 0 = Fehler (Meldung "name:zeile: text" auf stderr). */
int mdf_parse_text(const char *text, const char *name, mdf_t *out);
int mdf_load_file(const char *path, mdf_t *out);
void mdf_free(mdf_t *m);

#endif

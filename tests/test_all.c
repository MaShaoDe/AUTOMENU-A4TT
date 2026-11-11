/* Tests für Kern und Loader. Aufruf: ./build.sh test */
#include <stdio.h>
#include <string.h>
#include "core.h"
#include "loader.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FEHLER Zeile %d: %s\n", __LINE__, #c); failures++; } } while (0)

static const char *GOOD =
    "# Kommentar\r\n"
    "menu main \"Haupt\"\r\n"
    "  item \"Datum\" \"Zeigt die UTC-Zeit\"  run date -u\r\n"
    "  item \"Mehr\"    menu more\r\n"
    "  item \"Hinweis\" info Zeile1\\nZeile2\r\n"
    "  item \"Ende\"    exit\r\n"
    "\n"
    "menu more \"Mehr\"\n"
    "  item \"Zurück\"  back\n";

static void test_loader(void)
{
    menu_t *m = NULL; size_t n = 0;
    CHECK(menu_parse_text(GOOD, "t", &m, &n));
    CHECK(n == 2);
    CHECK(m[0].item_count == 4);
    CHECK(m[0].items[0].type == MENU_ITEM_RUN);
    CHECK(strcmp(m[0].items[0].arg, "date -u") == 0);   /* CRLF wurde entfernt */
    CHECK(m[0].items[0].desc && strcmp(m[0].items[0].desc, "Zeigt die UTC-Zeit") == 0);
    CHECK(m[0].items[1].desc == NULL);
    CHECK(strcmp(m[0].items[2].arg, "Zeile1\nZeile2") == 0);
    CHECK(m[1].items[0].type == MENU_ITEM_BACK);
    menu_free(m, n);

    const char *bad[] = {
        "",                                               /* leer */
        "item \"x\" exit\n",                              /* item vor menu */
        "menu a \"A\"\n",                                 /* Menü ohne Einträge */
        "menu a \"A\"\n item \"x\" run\n",                /* run ohne Befehl */
        "menu a \"A\"\n item \"x\" menu zz\n",            /* unbekanntes Ziel */
        "menu a \"A\"\n item \"x\" exit\nmenu a \"B\"\n item \"y\" exit\n", /* doppelte ID */
        "menu a \"A\"\n item \"x\" fliegen\n",            /* unbekannter Typ */
        "menu a \"A\"\n item \"x\" exit extra\n",         /* exit mit Rest */
        "menu a A\n",                                     /* Titel ohne Anführungszeichen */
        "menu a \"A\"\n item \"x\" \"offen exit\n",           /* Hilfetext nicht geschlossen */
        "hallo\n",                                        /* unbekanntes Wort */
    };
    for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
        m = NULL; n = 0;
        printf("  (erwartete Meldung) ");
        CHECK(!menu_parse_text(bad[i], "bad", &m, &n));
    }
}

static void test_core(void)
{
    menu_t *m = NULL; size_t n = 0;
    CHECK(menu_parse_text(GOOD, "t", &m, &n));
    core_t *c = core_create();
    CHECK(core_load(c, m, n));
    CHECK(strcmp(core_view(c).id, "main") == 0);
    CHECK(core_view(c).menu_index == 0 && core_view(c).menu_count == 2);

    core_input(c, CORE_EVENT_UP);                        /* bleibt bei 0 */
    CHECK(core_view(c).current_index == 0);
    core_input(c, CORE_EVENT_DOWN);
    core_input(c, CORE_EVENT_DOWN);
    core_input(c, CORE_EVENT_DOWN);
    core_input(c, CORE_EVENT_DOWN);                      /* Ende, bleibt bei 3 */
    CHECK(core_view(c).current_index == 3);

    core_select_index(c, 0);                             /* run */
    const char *cmd = core_poll_run(c);
    CHECK(cmd && strcmp(cmd, "date -u") == 0);
    CHECK(core_poll_run(c) == NULL);                     /* nur einmal */

    core_select_index(c, 2);                             /* info */
    const char *inf = core_poll_info(c);
    CHECK(inf && strcmp(inf, "Zeile1\nZeile2") == 0);

    core_select_index(c, 1);                             /* Untermenü */
    CHECK(strcmp(core_view(c).id, "more") == 0);
    core_input(c, CORE_EVENT_BACK);                      /* Esc */
    CHECK(strcmp(core_view(c).id, "main") == 0);
    core_input(c, CORE_EVENT_BACK);                      /* oben: bleibt */
    CHECK(strcmp(core_view(c).id, "main") == 0);

    core_select_index(c, 1);
    core_select_index(c, 0);                             /* back-Eintrag */
    CHECK(strcmp(core_view(c).id, "main") == 0);

    core_select_index(c, 99);                            /* ungültig: ignoriert */
    CHECK(!core_should_exit(c));
    core_select_index(c, 3);                             /* exit */
    CHECK(core_should_exit(c));

    core_destroy(c);
    menu_free(m, n);
}

int main(void)
{
    test_loader();
    test_core();
    if (failures) { printf("%d Fehler\n", failures); return 1; }
    printf("Alle Tests ok\n");
    return 0;
}

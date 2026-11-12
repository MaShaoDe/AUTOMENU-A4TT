/*
 * Copyright (c) 2026, FerienBSD
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

/* Tests für Kern und Loader. Aufruf: ./build.sh test */
#include <stdio.h>
#include <string.h>
#include "core.h"
#include "loader.h"
#include "auth.h"

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
    mdf_t d = {0};
    CHECK(mdf_parse_text(GOOD, "t", &d));
    menu_t *m = d.menus; size_t n = d.menu_count;
    CHECK(n == 2);
    CHECK(d.mouse == 1 && d.blackout_s == 0 && d.clock_utc == 0);
    CHECK(m[0].item_count == 4);
    CHECK(m[0].items[0].type == MENU_ITEM_RUN);
    CHECK(strcmp(m[0].items[0].arg, "date -u") == 0);   /* CRLF wurde entfernt */
    CHECK(m[0].items[0].desc && strcmp(m[0].items[0].desc, "Zeigt die UTC-Zeit") == 0);
    CHECK(m[0].items[1].desc == NULL);
    CHECK(strcmp(m[0].items[2].arg, "Zeile1\nZeile2") == 0);
    CHECK(m[1].items[0].type == MENU_ITEM_BACK);
    mdf_free(&d);

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
        "menu a \"A\"\n item \"x\" lock pw exit\n",       /* unbekanntes Passwort */
        "password p xyz\nmenu a \"A\"\n item \"x\" exit\n", /* Hash ungültig */
        "set blackout -5\nmenu a \"A\"\n item \"x\" exit\n",
        "set clock mars\nmenu a \"A\"\n item \"x\" exit\n",
        "set mouse vielleicht\nmenu a \"A\"\n item \"x\" exit\n",
        "set farbe rot\nmenu a \"A\"\n item \"x\" exit\n",
        "menu a \"A\"\n item \"x\" lock\n",              /* lock ohne Namen */
    };
    for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
        mdf_t e = {0};
        printf("  (erwartete Meldung) ");
        CHECK(!mdf_parse_text(bad[i], "bad", &e));
    }
}

static void test_core(void)
{
    mdf_t d = {0};
    CHECK(mdf_parse_text(GOOD, "t", &d));
    menu_t *m = d.menus; size_t n = d.menu_count;
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
    mdf_free(&d);
}

static void test_auth(void)
{
    uint8_t o[32], salt[16];
    char hex[65], h[AUTH_HASH_MAX];
    auth_sha256((const uint8_t *)"abc", 3, o);
    for (int i = 0; i < 32; i++) sprintf(hex + 2*i, "%02x", o[i]);
    CHECK(strcmp(hex, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0);
    auth_hmac_sha256((const uint8_t *)"Jefe", 4, (const uint8_t *)"what do ya want for nothing?", 28, o);
    for (int i = 0; i < 32; i++) sprintf(hex + 2*i, "%02x", o[i]);
    CHECK(strcmp(hex, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843") == 0);
    for (int i = 0; i < 16; i++) salt[i] = (uint8_t)(i * 7 + 1);
    CHECK(auth_make_hash("geheim", salt, 1000, h, sizeof h));
    CHECK(auth_format_ok(h));
    CHECK(auth_verify("geheim", h));
    CHECK(!auth_verify("Geheim", h));
    CHECK(!auth_verify("", h));
    CHECK(!auth_verify("geheim", "kaputt"));
}

static void test_lock(void)
{
    uint8_t salt[16] = {0};
    char h[AUTH_HASH_MAX], txt[600];
    auth_make_hash("pw1", salt, 100, h, sizeof h);
    snprintf(txt, sizeof txt,
             "password admin %s\nset blackout 30\nset clock utc\nset mouse off\n"
             "menu main \"M\"\n item \"Offen\" run echo a\n"
             " item \"Zu\" \"Hilfe\" lock admin run echo b\n item \"Ende\" exit\n", h);
    mdf_t d = {0};
    CHECK(mdf_parse_text(txt, "lock", &d));
    CHECK(d.password_count == 1 && d.blackout_s == 30 && d.clock_utc == 1 && d.mouse == 0);
    CHECK(d.menus[0].items[1].lock && strcmp(d.menus[0].items[1].lock, "admin") == 0);
    CHECK(strcmp(d.menus[0].items[1].desc, "Hilfe") == 0);
    CHECK(strcmp(d.menus[0].items[1].arg, "echo b") == 0);
    core_t *c = core_create();
    CHECK(core_load(c, d.menus, d.menu_count));
    core_select_index(c, 1);
    CHECK(core_poll_run(c) == NULL);                     /* nicht ausgeführt */
    CHECK(core_locked_name(c) && strcmp(core_locked_name(c), "admin") == 0);
    core_unlock_cancel(c);
    CHECK(core_locked_name(c) == NULL && core_poll_run(c) == NULL);
    core_select_index(c, 1);
    core_unlock_ok(c);
    const char *cmd = core_poll_run(c);
    CHECK(cmd && strcmp(cmd, "echo b") == 0);
    core_select_index(c, 1);                             /* wieder gesperrt */
    CHECK(core_locked_name(c) != NULL);
    core_destroy(c);
    mdf_free(&d);
}

int main(void)
{
    test_auth();
    test_lock();
    test_loader();
    test_core();
    if (failures) { printf("%d Fehler\n", failures); return 1; }
    printf("Alle Tests ok\n");
    return 0;
}

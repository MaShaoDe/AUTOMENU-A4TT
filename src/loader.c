/* loader.c - Menüdatei einlesen und streng prüfen */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loader.h"
#include "auth.h"

typedef struct {
    menu_t *menus;
    size_t count;
    password_t *pw;
    size_t pw_count;
    int blackout_s, clock_utc, mouse;
} cfg_t;

static char *dupstr(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

static char *skip_ws(char *p)
{
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

/* nächstes Wort; schreibt ein '\0' hinter das Wort */
static char *take_word(char **pp)
{
    char *p = skip_ws(*pp);
    if (!*p) { *pp = p; return NULL; }
    char *s = p;
    while (*p && *p != ' ' && *p != '\t') p++;
    if (*p) *p++ = '\0';
    *pp = p;
    return s;
}

/* "Text in Anführungszeichen" */
static char *take_quoted(char **pp)
{
    char *p = skip_ws(*pp);
    if (*p != '"') return NULL;
    char *s = ++p;
    while (*p && *p != '"') p++;
    if (*p != '"') return NULL;
    *p++ = '\0';
    *pp = p;
    return s;
}

/* Zeichenfolge \n wird zu einem Zeilenumbruch (nur für info) */
static void unescape_newlines(char *s)
{
    char *w = s;
    for (; *s; s++) {
        if (s[0] == '\\' && s[1] == 'n') { *w++ = '\n'; s++; }
        else *w++ = *s;
    }
    *w = '\0';
}

static void free_cfg(menu_t *menus, size_t count, password_t *pw, size_t pw_count)
{
    for (size_t i = 0; i < pw_count; i++) {
        free((char *)pw[i].name);
        free((char *)pw[i].hash);
    }
    free(pw);
    if (!menus) return;
    for (size_t i = 0; i < count; i++) {
        free((char *)menus[i].id);
        free((char *)menus[i].title);
        for (size_t j = 0; j < menus[i].item_count; j++) {
            free((char *)menus[i].items[j].label);
            free((char *)menus[i].items[j].arg);
            free((char *)menus[i].items[j].desc);
            free((char *)menus[i].items[j].lock);
        }
        free(menus[i].items);
    }
    free(menus);
}

void mdf_free(mdf_t *m)
{
    if (!m) return;
    free_cfg(m->menus, m->menu_count, m->passwords, m->password_count);
    memset(m, 0, sizeof(*m));
}

static int fail(const char *name, size_t line, const char *msg, const char *detail)
{
    if (line)
        fprintf(stderr, "%s:%zu: %s%s%s\n", name, line, msg,
                detail ? " " : "", detail ? detail : "");
    else
        fprintf(stderr, "%s: %s%s%s\n", name, msg,
                detail ? " " : "", detail ? detail : "");
    return 0;
}

static int parse_menu_line(cfg_t *cfg, char *p, const char *name, size_t ln)
{
    char *id = take_word(&p);
    char *title = take_quoted(&p);
    if (!id || !title || *skip_ws(p))
        return fail(name, ln, "Syntax: menu <id> \"Titel\"", NULL);

    menu_t *nm = realloc(cfg->menus, sizeof(menu_t) * (cfg->count + 1));
    if (!nm) return fail(name, ln, "zu wenig Speicher", NULL);
    cfg->menus = nm;
    menu_t *m = &cfg->menus[cfg->count];
    memset(m, 0, sizeof(*m));
    m->id = dupstr(id);
    m->title = dupstr(title);
    cfg->count++;
    if (!m->id || !m->title) return fail(name, ln, "zu wenig Speicher", NULL);
    return 1;
}

static int parse_item_line(cfg_t *cfg, char *p, const char *name, size_t ln)
{
    if (cfg->count == 0)
        return fail(name, ln, "item steht vor dem ersten menu", NULL);

    char *label = take_quoted(&p);
    char *desc = NULL;
    if (label && *skip_ws(p) == '"') {      /* optionaler Hilfetext */
        desc = take_quoted(&p);
        if (!desc) return fail(name, ln, "Hilfetext ohne schliessendes Anführungszeichen", NULL);
    }
    char *lock = NULL;
    if (label) {                            /* optional: lock <passwortname> */
        char *q = skip_ws(p);
        if (strncmp(q, "lock", 4) == 0 && (q[4] == ' ' || q[4] == '\t')) {
            p = q + 4;
            lock = take_word(&p);
            if (!lock) return fail(name, ln, "lock braucht einen Passwortnamen", NULL);
        }
    }
    char *type = take_word(&p);
    if (!label || !type)
        return fail(name, ln, "Syntax: item \"Text\" [\"Hilfetext\"] [lock <name>] <run|menu|info|back|exit> ...", NULL);

    char *rest = skip_ws(p);
    menu_item_t it = {0};
    char *arg = NULL;

    if (strcmp(type, "run") == 0) {
        if (!*rest) return fail(name, ln, "run braucht einen Befehl", NULL);
        it.type = MENU_ITEM_RUN; arg = rest;
    } else if (strcmp(type, "menu") == 0) {
        if (!*rest || strchr(rest, ' ') || strchr(rest, '\t'))
            return fail(name, ln, "menu braucht genau eine Menü-ID", NULL);
        it.type = MENU_ITEM_MENU; arg = rest;
    } else if (strcmp(type, "info") == 0) {
        it.type = MENU_ITEM_INFO;
        if (*rest) { unescape_newlines(rest); arg = rest; }
    } else if (strcmp(type, "back") == 0 || strcmp(type, "exit") == 0) {
        if (*rest) return fail(name, ln, type, "braucht keine Angaben");
        it.type = (type[0] == 'b') ? MENU_ITEM_BACK : MENU_ITEM_EXIT;
    } else {
        return fail(name, ln, "unbekannter Typ:", type);
    }

    menu_t *m = &cfg->menus[cfg->count - 1];
    menu_item_t *ni = realloc(m->items, sizeof(menu_item_t) * (m->item_count + 1));
    if (!ni) return fail(name, ln, "zu wenig Speicher", NULL);
    m->items = ni;
    it.label = dupstr(label);
    it.arg = arg ? dupstr(arg) : NULL;
    it.desc = desc ? dupstr(desc) : NULL;
    it.lock = lock ? dupstr(lock) : NULL;
    if (!it.label || (arg && !it.arg) || (desc && !it.desc) || (lock && !it.lock)) return fail(name, ln, "zu wenig Speicher", NULL);
    m->items[m->item_count++] = it;
    return 1;
}

static int parse_password_line(cfg_t *cfg, char *p, const char *name, size_t ln)
{
    char *pn = take_word(&p);
    char *hash = take_word(&p);
    if (!pn || !hash || *skip_ws(p))
        return fail(name, ln, "Syntax: password <name> <hash>  (Hash erzeugen: automenu --hash)", NULL);
    if (!auth_format_ok(hash))
        return fail(name, ln, "Passwort-Hash ungültig, bitte mit automenu --hash erzeugen", NULL);
    for (size_t i = 0; i < cfg->pw_count; i++)
        if (strcmp(cfg->pw[i].name, pn) == 0)
            return fail(name, ln, "Passwortname doppelt:", pn);
    password_t *np = realloc(cfg->pw, sizeof(password_t) * (cfg->pw_count + 1));
    if (!np) return fail(name, ln, "zu wenig Speicher", NULL);
    cfg->pw = np;
    cfg->pw[cfg->pw_count].name = dupstr(pn);
    cfg->pw[cfg->pw_count].hash = dupstr(hash);
    cfg->pw_count++;
    return 1;
}

static int parse_set_line(cfg_t *cfg, char *p, const char *name, size_t ln)
{
    char *key = take_word(&p);
    char *val = take_word(&p);
    if (!key || !val || *skip_ws(p))
        return fail(name, ln, "Syntax: set <blackout|clock|mouse> <wert>", NULL);
    if (strcmp(key, "blackout") == 0) {
        char *e; long v = strtol(val, &e, 10);
        if (*e || v < 0 || v > 86400)
            return fail(name, ln, "blackout: Sekunden 0 bis 86400 (0 = aus)", NULL);
        cfg->blackout_s = (int)v;
    } else if (strcmp(key, "clock") == 0) {
        if (strcmp(val, "utc") == 0) cfg->clock_utc = 1;
        else if (strcmp(val, "local") == 0) cfg->clock_utc = 0;
        else return fail(name, ln, "clock: utc oder local", NULL);
    } else if (strcmp(key, "mouse") == 0) {
        if (strcmp(val, "on") == 0) cfg->mouse = 1;
        else if (strcmp(val, "off") == 0) cfg->mouse = 0;
        else return fail(name, ln, "mouse: on oder off", NULL);
    } else {
        return fail(name, ln, "unbekannte Einstellung:", key);
    }
    return 1;
}

static int validate(const cfg_t *cfg, const char *name)
{
    if (cfg->count == 0)
        return fail(name, 0, "keine Menüs definiert", NULL);

    for (size_t i = 0; i < cfg->count; i++) {
        const menu_t *a = &cfg->menus[i];
        if (a->item_count == 0)
            return fail(name, 0, "Menü ohne Einträge:", a->id);
        for (size_t j = i + 1; j < cfg->count; j++)
            if (strcmp(a->id, cfg->menus[j].id) == 0)
                return fail(name, 0, "Menü-ID doppelt:", a->id);
    }
    for (size_t i = 0; i < cfg->count; i++) {
        const menu_t *a = &cfg->menus[i];
        for (size_t k = 0; k < a->item_count; k++) {
            const char *lk = a->items[k].lock;
            if (lk) {
                int ok = 0;
                for (size_t j = 0; j < cfg->pw_count; j++)
                    if (strcmp(lk, cfg->pw[j].name) == 0) ok = 1;
                if (!ok) {
                    fprintf(stderr, "%s: Eintrag \"%s\" nutzt unbekanntes Passwort '%s'\n",
                            name, a->items[k].label, lk);
                    return 0;
                }
            }
            if (a->items[k].type != MENU_ITEM_MENU) continue;
            int found = 0;
            for (size_t j = 0; j < cfg->count; j++)
                if (strcmp(a->items[k].arg, cfg->menus[j].id) == 0) found = 1;
            if (!found) {
                fprintf(stderr, "%s: Menü '%s' verweist auf unbekanntes Menü '%s'\n",
                        name, a->id, a->items[k].arg);
                return 0;
            }
        }
    }
    return 1;
}

int mdf_parse_text(const char *text, const char *name, mdf_t *out)
{
    cfg_t cfg = {0};
    cfg.mouse = 1;
    char *buf = dupstr(text);
    if (!buf) return fail(name, 0, "zu wenig Speicher", NULL);

    int ok = 1;
    size_t ln = 0;
    char *line = buf;
    while (ok && line) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        ln++;

        size_t n = strlen(line);   /* Zeilenende CR/LF und Leerzeichen entfernen */
        while (n > 0 && (line[n-1] == '\r' || line[n-1] == ' ' || line[n-1] == '\t'))
            line[--n] = '\0';

        char *p = skip_ws(line);
        if (*p && *p != '#') {
            char *kw = take_word(&p);
            if (strcmp(kw, "menu") == 0)      ok = parse_menu_line(&cfg, p, name, ln);
            else if (strcmp(kw, "item") == 0) ok = parse_item_line(&cfg, p, name, ln);
            else if (strcmp(kw, "password") == 0) ok = parse_password_line(&cfg, p, name, ln);
            else if (strcmp(kw, "set") == 0)  ok = parse_set_line(&cfg, p, name, ln);
            else ok = fail(name, ln, "unbekanntes Schlüsselwort:", kw);
        }
        line = nl ? nl + 1 : NULL;
    }
    free(buf);

    if (ok) ok = validate(&cfg, name);
    if (!ok) { free_cfg(cfg.menus, cfg.count, cfg.pw, cfg.pw_count); return 0; }
    out->menus = cfg.menus;
    out->menu_count = cfg.count;
    out->passwords = cfg.pw;
    out->password_count = cfg.pw_count;
    out->blackout_s = cfg.blackout_s;
    out->clock_utc = cfg.clock_utc;
    out->mouse = cfg.mouse;
    return 1;
}

int mdf_load_file(const char *path, mdf_t *out)
{
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return 0; }

    size_t cap = 4096, len = 0;
    char *text = malloc(cap);
    if (!text) { fclose(f); return 0; }
    size_t r;
    while ((r = fread(text + len, 1, cap - len - 1, f)) > 0) {
        len += r;
        if (len + 1 >= cap) {
            char *t = realloc(text, cap *= 2);
            if (!t) { free(text); fclose(f); return 0; }
            text = t;
        }
    }
    fclose(f);
    text[len] = '\0';

    int ok = mdf_parse_text(text, path, out);
    free(text);
    return ok;
}

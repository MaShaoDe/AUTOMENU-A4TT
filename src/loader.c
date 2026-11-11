/* loader.c - Menüdatei einlesen und streng prüfen */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loader.h"

typedef struct {
    menu_t *menus;
    size_t count;
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

void menu_free(menu_t *menus, size_t count)
{
    if (!menus) return;
    for (size_t i = 0; i < count; i++) {
        free((char *)menus[i].id);
        free((char *)menus[i].title);
        for (size_t j = 0; j < menus[i].item_count; j++) {
            free((char *)menus[i].items[j].label);
            free((char *)menus[i].items[j].arg);
            free((char *)menus[i].items[j].desc);
        }
        free(menus[i].items);
    }
    free(menus);
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
    char *type = take_word(&p);
    if (!label || !type)
        return fail(name, ln, "Syntax: item \"Text\" [\"Hilfetext\"] <run|menu|info|back|exit> ...", NULL);

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
    if (!it.label || (arg && !it.arg) || (desc && !it.desc)) return fail(name, ln, "zu wenig Speicher", NULL);
    m->items[m->item_count++] = it;
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

int menu_parse_text(const char *text, const char *name,
                    menu_t **out_menus, size_t *out_count)
{
    cfg_t cfg = {0};
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
            else ok = fail(name, ln, "unbekanntes Schlüsselwort:", kw);
        }
        line = nl ? nl + 1 : NULL;
    }
    free(buf);

    if (ok) ok = validate(&cfg, name);
    if (!ok) { menu_free(cfg.menus, cfg.count); return 0; }
    *out_menus = cfg.menus;
    *out_count = cfg.count;
    return 1;
}

int menu_load_file(const char *path, menu_t **out_menus, size_t *out_count)
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

    int ok = menu_parse_text(text, path, out_menus, out_count);
    free(text);
    return ok;
}

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

/* loader.c - read and strictly check the menu file */
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

/* next word; writes a '\0' after the word */
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

/* "text in quotation marks" */
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

/* the character sequence \n becomes a line break (info only) */
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
        return fail(name, ln, "syntax: menu <id> \"Title\"", NULL);

    menu_t *nm = realloc(cfg->menus, sizeof(menu_t) * (cfg->count + 1));
    if (!nm) return fail(name, ln, "out of memory", NULL);
    cfg->menus = nm;
    menu_t *m = &cfg->menus[cfg->count];
    memset(m, 0, sizeof(*m));
    m->id = dupstr(id);
    m->title = dupstr(title);
    cfg->count++;
    if (!m->id || !m->title) return fail(name, ln, "out of memory", NULL);
    return 1;
}

static int parse_item_line(cfg_t *cfg, char *p, const char *name, size_t ln)
{
    if (cfg->count == 0)
        return fail(name, ln, "item before the first menu", NULL);

    char *label = take_quoted(&p);
    char *desc = NULL;
    if (label && *skip_ws(p) == '"') {      /* optional help text */
        desc = take_quoted(&p);
        if (!desc) return fail(name, ln, "help text without closing quotation mark", NULL);
    }
    char *lock = NULL;
    if (label) {                            /* optional: lock <password name> */
        char *q = skip_ws(p);
        if (strncmp(q, "lock", 4) == 0 && (q[4] == ' ' || q[4] == '\t')) {
            p = q + 4;
            lock = take_word(&p);
            if (!lock) return fail(name, ln, "lock needs a password name", NULL);
        }
    }
    char *type = take_word(&p);
    if (!label || !type)
        return fail(name, ln, "syntax: item \"Text\" [\"Help text\"] [lock <name>] <run|menu|info|back|exit> ...", NULL);

    char *rest = skip_ws(p);
    menu_item_t it = {0};
    char *arg = NULL;

    if (strcmp(type, "run") == 0) {
        if (!*rest) return fail(name, ln, "run needs a command", NULL);
        it.type = MENU_ITEM_RUN; arg = rest;
    } else if (strcmp(type, "menu") == 0) {
        if (!*rest || strchr(rest, ' ') || strchr(rest, '\t'))
            return fail(name, ln, "menu needs exactly one menu id", NULL);
        it.type = MENU_ITEM_MENU; arg = rest;
    } else if (strcmp(type, "info") == 0) {
        it.type = MENU_ITEM_INFO;
        if (*rest) { unescape_newlines(rest); arg = rest; }
    } else if (strcmp(type, "back") == 0 || strcmp(type, "exit") == 0) {
        if (*rest) return fail(name, ln, type, "takes no arguments");
        it.type = (type[0] == 'b') ? MENU_ITEM_BACK : MENU_ITEM_EXIT;
    } else {
        return fail(name, ln, "unknown type:", type);
    }

    menu_t *m = &cfg->menus[cfg->count - 1];
    menu_item_t *ni = realloc(m->items, sizeof(menu_item_t) * (m->item_count + 1));
    if (!ni) return fail(name, ln, "out of memory", NULL);
    m->items = ni;
    it.label = dupstr(label);
    it.arg = arg ? dupstr(arg) : NULL;
    it.desc = desc ? dupstr(desc) : NULL;
    it.lock = lock ? dupstr(lock) : NULL;
    if (!it.label || (arg && !it.arg) || (desc && !it.desc) || (lock && !it.lock)) return fail(name, ln, "out of memory", NULL);
    m->items[m->item_count++] = it;
    return 1;
}

static int parse_password_line(cfg_t *cfg, char *p, const char *name, size_t ln)
{
    char *pn = take_word(&p);
    char *hash = take_word(&p);
    if (!pn || !hash || *skip_ws(p))
        return fail(name, ln, "syntax: password <name> <hash>  (create the hash with: automenu --hash)", NULL);
    if (!auth_format_ok(hash))
        return fail(name, ln, "invalid password hash, please create it with automenu --hash", NULL);
    for (size_t i = 0; i < cfg->pw_count; i++)
        if (strcmp(cfg->pw[i].name, pn) == 0)
            return fail(name, ln, "duplicate password name:", pn);
    password_t *np = realloc(cfg->pw, sizeof(password_t) * (cfg->pw_count + 1));
    if (!np) return fail(name, ln, "out of memory", NULL);
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
        return fail(name, ln, "syntax: set <blackout|clock|mouse> <value>", NULL);
    if (strcmp(key, "blackout") == 0) {
        char *e; long v = strtol(val, &e, 10);
        if (*e || v < 0 || v > 86400)
            return fail(name, ln, "blackout: seconds 0 to 86400 (0 = off)", NULL);
        cfg->blackout_s = (int)v;
    } else if (strcmp(key, "clock") == 0) {
        if (strcmp(val, "utc") == 0) cfg->clock_utc = 1;
        else if (strcmp(val, "local") == 0) cfg->clock_utc = 0;
        else return fail(name, ln, "clock: utc or local", NULL);
    } else if (strcmp(key, "mouse") == 0) {
        if (strcmp(val, "on") == 0) cfg->mouse = 1;
        else if (strcmp(val, "off") == 0) cfg->mouse = 0;
        else return fail(name, ln, "mouse: on or off", NULL);
    } else {
        return fail(name, ln, "unknown setting:", key);
    }
    return 1;
}

static int validate(const cfg_t *cfg, const char *name)
{
    if (cfg->count == 0)
        return fail(name, 0, "no menus defined", NULL);

    for (size_t i = 0; i < cfg->count; i++) {
        const menu_t *a = &cfg->menus[i];
        if (a->item_count == 0)
            return fail(name, 0, "menu without items:", a->id);
        for (size_t j = i + 1; j < cfg->count; j++)
            if (strcmp(a->id, cfg->menus[j].id) == 0)
                return fail(name, 0, "duplicate menu id:", a->id);
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
                    fprintf(stderr, "%s: item \"%s\" uses unknown password '%s'\n",
                            name, a->items[k].label, lk);
                    return 0;
                }
            }
            if (a->items[k].type != MENU_ITEM_MENU) continue;
            int found = 0;
            for (size_t j = 0; j < cfg->count; j++)
                if (strcmp(a->items[k].arg, cfg->menus[j].id) == 0) found = 1;
            if (!found) {
                fprintf(stderr, "%s: menu '%s' refers to unknown menu '%s'\n",
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
    if (!buf) return fail(name, 0, "out of memory", NULL);

    int ok = 1;
    size_t ln = 0;
    char *line = buf;
    while (ok && line) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        ln++;

        size_t n = strlen(line);   /* strip CR/LF and trailing blanks */
        while (n > 0 && (line[n-1] == '\r' || line[n-1] == ' ' || line[n-1] == '\t'))
            line[--n] = '\0';

        char *p = skip_ws(line);
        if (*p && *p != '#') {
            char *kw = take_word(&p);
            if (strcmp(kw, "menu") == 0)      ok = parse_menu_line(&cfg, p, name, ln);
            else if (strcmp(kw, "item") == 0) ok = parse_item_line(&cfg, p, name, ln);
            else if (strcmp(kw, "password") == 0) ok = parse_password_line(&cfg, p, name, ln);
            else if (strcmp(kw, "set") == 0)  ok = parse_set_line(&cfg, p, name, ln);
            else ok = fail(name, ln, "unknown keyword:", kw);
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

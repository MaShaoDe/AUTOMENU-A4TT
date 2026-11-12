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

/* core.c - Menükern (siehe core.h) */
#include <stdlib.h>
#include <string.h>
#include "core.h"

#define STACK_MAX 16

struct core {
    const menu_t *menus;
    size_t menu_count;
    size_t current_menu;
    size_t current_index;
    size_t stack[STACK_MAX];
    size_t depth;
    const char *pending_run;
    const char *pending_info;
    const menu_item_t *pending_lock;
    int exit_requested;
};

static size_t find_menu(const core_t *c, const char *id)
{
    for (size_t i = 0; i < c->menu_count; i++)
        if (strcmp(c->menus[i].id, id) == 0)
            return i;
    return (size_t)-1;
}

core_t *core_create(void) { return calloc(1, sizeof(core_t)); }
void core_destroy(core_t *core) { free(core); }

int core_load(core_t *core, const menu_t *menus, size_t menu_count)
{
    if (!core || !menus || menu_count == 0 || menus[0].item_count == 0)
        return 0;
    memset(core, 0, sizeof(*core));
    core->menus = menus;
    core->menu_count = menu_count;
    return 1;
}

static void go_back(core_t *c)
{
    if (c->depth > 0) {
        c->current_menu = c->stack[--c->depth];
        c->current_index = 0;
    }
}

static void activate(core_t *c, const menu_item_t *it)
{
    switch (it->type) {
    case MENU_ITEM_RUN:
        c->pending_run = it->arg;
        break;
    case MENU_ITEM_MENU: {
        size_t next = find_menu(c, it->arg);
        if (next != (size_t)-1 && c->depth < STACK_MAX) {
            c->stack[c->depth++] = c->current_menu;
            c->current_menu = next;
            c->current_index = 0;
        }
        break;
    }
    case MENU_ITEM_BACK:
        go_back(c);
        break;
    case MENU_ITEM_EXIT:
        c->exit_requested = 1;
        break;
    case MENU_ITEM_INFO:
        c->pending_info = (it->arg && *it->arg) ? it->arg : it->label;
        break;
    }
}

void core_input(core_t *c, core_event_t ev)
{
    if (!c || !c->menus) return;
    const menu_t *m = &c->menus[c->current_menu];

    switch (ev) {
    case CORE_EVENT_UP:
        if (c->current_index > 0) c->current_index--;
        break;
    case CORE_EVENT_DOWN:
        if (c->current_index + 1 < m->item_count) c->current_index++;
        break;
    case CORE_EVENT_BACK:
        go_back(c);
        break;
    case CORE_EVENT_SELECT: {
        const menu_item_t *it = &m->items[c->current_index];
        if (it->lock && *it->lock)
            c->pending_lock = it;      /* erst Passwort abfragen */
        else
            activate(c, it);
        break;
    }
    }
}

void core_select_index(core_t *c, size_t index)
{
    if (!c || !c->menus) return;
    if (index >= c->menus[c->current_menu].item_count) return;
    c->current_index = index;
    core_input(c, CORE_EVENT_SELECT);
}

menu_view_t core_view(const core_t *c)
{
    menu_view_t v = {0};
    if (!c || !c->menus) return v;
    const menu_t *m = &c->menus[c->current_menu];
    v.id = m->id;
    v.title = m->title;
    v.items = m->items;
    v.item_count = m->item_count;
    v.current_index = c->current_index;
    v.menu_index = c->current_menu;
    v.menu_count = c->menu_count;
    return v;
}

const char *core_poll_run(core_t *c)
{
    if (!c) return NULL;
    const char *a = c->pending_run;
    c->pending_run = NULL;
    return a;
}

const char *core_poll_info(core_t *c)
{
    if (!c) return NULL;
    const char *i = c->pending_info;
    c->pending_info = NULL;
    return i;
}

int core_should_exit(const core_t *c) { return c ? c->exit_requested : 0; }

/* Sperre: liefert den Passwortnamen, solange eine Abfrage aussteht */
const char *core_locked_name(const core_t *c)
{
    return (c && c->pending_lock) ? c->pending_lock->lock : NULL;
}

const char *core_locked_label(const core_t *c)
{
    return (c && c->pending_lock) ? c->pending_lock->label : NULL;
}

void core_unlock_ok(core_t *c)
{
    if (!c || !c->pending_lock) return;
    const menu_item_t *it = c->pending_lock;
    c->pending_lock = NULL;
    activate(c, it);
}

void core_unlock_cancel(core_t *c)
{
    if (c) c->pending_lock = NULL;
}

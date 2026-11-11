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

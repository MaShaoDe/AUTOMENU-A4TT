//
//  menu.h
//  
//
//  Created by Marcel Sauder on 21.01.2026.
//
#ifndef AUTOMENU_MENU_H
#define AUTOMENU_MENU_H

#include <stddef.h>

/*
 * Menu item types.
 * Version 0.1.0: minimal set.
 */
typedef enum {
    MENU_ITEM_ACTION = 0,
    MENU_ITEM_SUBMENU
} menu_item_type_t;

/*
 * Single menu item.
 */
typedef struct menu_item {
    const char *label;              /* text shown to the user */
    menu_item_type_t type;           /* action or submenu */
    void (*action)(void);            /* function for actions */
    struct menu *submenu;            /* target submenu */
} menu_item_t;

/*
 * Menu definition.
 */
typedef struct menu {
    const char *title;               /* menu title */
    menu_item_t *items;              /* array of items */
    size_t item_count;               /* number of items */
} menu_t;

#endif /* AUTOMENU_MENU_H */

/*
 * Render menu to stdout.
 */
void menu_render(const menu_t *menu);

//
//  core.c
//  
//
//  Created by Marcel Sauder on 21.01.2026.
//

#include <stdio.h>
#include "core.h"
#include "io.h"
#include "menu.h"

/*
 * Minimal core loop.
 * Version 0.1.0: read keys until 'q' is pressed.
 */
static int menu_index_from_key(int ch, size_t item_count)
{
    if (ch < '1' || ch > '9') {
        return -1;
    }

    int index = (ch - '1');

    if ((size_t)index >= item_count) {
        return -1;
    }

    return index;
}

int core_run(automenu_context_t *ctx)
{
    (void)ctx;
    
    extern menu_t main_menu;

    menu_render(&main_menu);

    printf("Press keys, 'q' to quit:\n");
    fflush(stdout);

    while (1) {
        int ch = io_read_key();

        if (ch < 0) {
            break;
        }

        if (ch == 'q') {
            printf("Quit requested\n");
            fflush(stdout);
            break;
        }

        int index = menu_index_from_key(ch, main_menu.item_count);

        if (index >= 0) {
            menu_item_t *item = &main_menu.items[index];

            printf("Selected item %d: %s\n",
                   index + 1,
                   item->label);
            fflush(stdout);

            if (item->type == MENU_ITEM_ACTION && item->action) {
                item->action();
            }
        } else {
            printf("Invalid selection\n");
            fflush(stdout);
        }
        }

    return 0;
}

//
//  terminal.c
//
//  Created by Marcel Sauder on 21.01.2026.
//
//  Phase B – Terminal adapter with raw mode and arrow keys (macOS safe)
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#include "core.h"
#include "config_loader.h"

/* ---------- raw mode handling ---------- */

static struct termios orig_termios;

static void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void enable_raw_mode(void)
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/* ---------- rendering ---------- */

static void render(core_t *core)
{
    const menu_view_t *view = core_get_current_menu(core);
    if (!view) return;

    write(STDOUT_FILENO, "\033[2J\033[H", 7); /* clear screen */

    printf("%s\n", view->title);
    printf("------------\n");

    for (size_t i = 0; i < view->item_count; i++) {
        printf("%c %zu: %s\n",
               (i == view->current_index) ? '>' : ' ',
               i + 1,
               view->items[i].label);
    }

    printf("\n[↑↓] navigate  [enter] select  [1-9] direct  [q] quit\n");
    fflush(stdout);
}

/* ---------- main ---------- */

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <menu.txt>\n", argv[0]);
        return 1;
    }

    enable_raw_mode();

    core_t *core = core_create();
    if (!core) {
        fprintf(stderr, "Core allocation failed\n");
        return 1;
    }

    menu_t *menus = NULL;
    size_t menu_count = 0;

    if (!load_menu_config(argv[1], &menus, &menu_count)) {
        fprintf(stderr, "Failed to load config: %s\n", argv[1]);
        core_destroy(core);
        return 1;
    }

    if (!core_load(core, menus, menu_count, "main")) {
        fprintf(stderr, "Core load failed\n");
        free_menu_config(menus, menu_count);
        core_destroy(core);
        return 1;
    }

    render(core);

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1)
            continue;

        if (c == 'q') {
            break;
        } else if (c == '\033') {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) continue;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) continue;

            if (seq[0] == '[') {
                if (seq[1] == 'A') {
                    core_input(core, CORE_EVENT_UP);
                } else if (seq[1] == 'B') {
                    core_input(core, CORE_EVENT_DOWN);
                }
            }
        } else if (c == '\r' || c == '\n') {
            core_input(core, CORE_EVENT_SELECT);
        } else if (isdigit((unsigned char)c)) {
            size_t idx = (size_t)(c - '1');
            core_select_index(core, idx);
        }

        const char *action = core_poll_action(core);
        if (action) {
            write(STDOUT_FILENO, "\033[2J\033[H", 7);
            printf("Action triggered: %s\n", action);
            printf("Press any key to continue...\n");
            fflush(stdout);
            char tmp;
            read(STDIN_FILENO, &tmp, 1);
        }

        render(core);
    }

    free_menu_config(menus, menu_count);
    core_destroy(core);
    return 0;
}

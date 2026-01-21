//
//  main.c
//  
//
//  Created by Marcel Sauder on 21.01.2026.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "automenu.h"
#include "core.h"
#include "config.h"
#include "io.h"

/* startup modes */
typedef enum {
    STARTUP_NORMAL = 0,
    STARTUP_SAFE,
    STARTUP_FACTORY_DEFAULT,
    STARTUP_DIRECT_CONFIG
} startup_mode_t;

static void print_usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [--safe] [--default] [--config]\n", prog);
}

int main(int argc, char *argv[])
{
    int exit_code = EXIT_SUCCESS;
    startup_mode_t mode = STARTUP_NORMAL;

    int flag_safe = 0;
    int flag_default = 0;
    int flag_config = 0;

    /* 1. parse startup parameters */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--safe") == 0) {
            flag_safe = 1;
        } else if (strcmp(argv[i], "--default") == 0) {
            flag_default = 1;
        } else if (strcmp(argv[i], "--config") == 0) {
            flag_config = 1;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    /* 2. determine startup mode */
    if ((flag_safe + flag_default + flag_config) > 1) {
        fprintf(stderr, "Conflicting startup parameters\n");
        return EXIT_FAILURE;
    }

    if (flag_safe) {
        mode = STARTUP_SAFE;
    } else if (flag_default) {
        mode = STARTUP_FACTORY_DEFAULT;
    } else if (flag_config) {
        mode = STARTUP_DIRECT_CONFIG;
    }

    /* 3. basic initialization */
    automenu_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));

    ctx.startup_mode = mode;

    /* 4. load configuration */
    int cfg_rc;
    if (mode == STARTUP_FACTORY_DEFAULT) {
        cfg_rc = config_load_default(&ctx);
    } else {
        cfg_rc = config_load(&ctx);
        if (cfg_rc != 0) {
            config_backup(&ctx);
            cfg_rc = config_load_default(&ctx);
            ctx.recovery_active = 1;
        }
    }

    if (cfg_rc != 0) {
        fprintf(stderr, "Failed to load configuration\n");
        return EXIT_FAILURE;
    }

    /* 5. apply safe mode */
    if (mode == STARTUP_SAFE) {
        ctx.safe_mode = 1;
    }

    /* 6. force configuration menu */
    if (mode == STARTUP_DIRECT_CONFIG) {
        ctx.force_config_menu = 1;
    }

    /* 7. initialize IO */
    if (io_init(&ctx) != 0) {
        fprintf(stderr, "IO initialization failed\n");
        return EXIT_FAILURE;
    }

    /* 8. start core engine */
    int core_rc = core_run(&ctx);

    /* 9. shutdown */
    io_shutdown(&ctx);

    if (core_rc != 0) {
        exit_code = EXIT_FAILURE;
    }

    return exit_code;
}


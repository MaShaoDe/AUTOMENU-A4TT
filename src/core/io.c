//
//  io.c
//  
//
//  Created by Marcel Sauder on 21.01.2026.
//

#include <stdio.h>
#include "io.h"

/*
 * Minimal IO initialization stub.
 * Version 0.1.0: first visible output.
 */
int io_init(automenu_context_t *ctx)
{
    (void)ctx;

    printf("Automenu starting\n");
    fflush(stdout);

    return 0;
}

/*
 * Minimal IO shutdown stub.
 */
void io_shutdown(automenu_context_t *ctx)
{
    (void)ctx;

    printf("Automenu shutting down\n");
    fflush(stdout);
}
#include <stdio.h>
#include "io.h"

int io_read_key(void)
{
    int ch = getchar();

    if (ch == EOF) {
        return -1;
    }

    return ch;
}

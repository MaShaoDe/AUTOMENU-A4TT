//
//  core.h
//  
//
//  Created by Marcel Sauder on 21.01.2026.
//
#ifndef AUTOMENU_CORE_H
#define AUTOMENU_CORE_H

#include "automenu.h"

/*
 * Runs the main core loop.
 * Returns 0 on normal exit, non-zero on error.
 */
int core_run(automenu_context_t *ctx);

#endif /* AUTOMENU_CORE_H */


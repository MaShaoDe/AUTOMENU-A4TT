//
//  automenu.h
//  
//
//  Created by Marcel Sauder on 21.01.2026.
//
#ifndef AUTOMENU_H
#define AUTOMENU_H

#include <stddef.h>

/*
 * Global runtime context for Automenu.
 * This structure is shared across all modules.
 * Version 0.1.0: minimal fields only.
 */
typedef struct automenu_context {
    int startup_mode;
    int safe_mode;
    int force_config_menu;
    int recovery_active;
} automenu_context_t;

#endif /* AUTOMENU_H */


    # CONFIGURATION

## Automenu Configuration Format v0.1

### by Marcel Sauder
### Purpose

This document defines the configuration format used by Automenu to describe menu structures, navigation and action bindings.

The configuration is purely declarative. It contains no logic and no platform-specific behaviour.

## Design Principles

The configuration format follows these rules:

* Human-readable
* Machine-parseable
* Platform-neutral
* Explicit over implicit
* No executable content
* No control flow

The configuration describes what exists, not how it is executed.

## Top-Level Structure

A configuration file consists of a single root object.

```json
{
  "menus": {
    "...": {}
  }
}
```

Only one root element is allowed.

## Menus

Menus are defined as named objects inside the `menus` map.

Each menu is identified by a unique string key.

```json
{
  "menus": {
    "main": {},
    "settings": {}
  }
}
```

Menu identifiers are opaque strings. They have no semantic meaning to the Core.

## Menu Definition

Each menu object has the following structure:

```json
{
  "title": "Menu Title",
  "items": [ ... ]
}
```

### Fields

* `title`
  Human-readable title displayed by the UI.

* `items`
  Ordered list of menu items.

## Menu Items

Each menu item is an object with the following structure:

```json
{
  "label": "Item Label",
  "action": "action_id"
}
```

### Fields

* `label`
  Human-readable label shown in the menu.

* `action`
  Identifier of the action to be triggered when selected.

Menu items are evaluated strictly in order.

## Navigation Items (Submenus)

Navigation to another menu is expressed explicitly.

```json
{
  "label": "Settings",
  "submenu": "settings"
}
```

### Rules

* An item has either `action` or `submenu`, never both.
* `submenu` must reference an existing menu identifier.

Navigation logic is handled by the Core, not the configuration.

## Action Identifiers

Action identifiers are plain strings.

Examples:

* `exit`
* `toggle_led`
* `save_config`

The Core does not interpret action identifiers. Their meaning is defined entirely by the platform-specific action handler.

## Example Configuration

```json
{
  "menus": {
    "main": {
      "title": "Main Menu",
      "items": [
        {
          "label": "Start",
          "action": "start_system"
        },
        {
          "label": "Settings",
          "submenu": "settings"
        },
        {
          "label": "Exit",
          "action": "exit"
        }
      ]
    },
    "settings": {
      "title": "Settings",
      "items": [
        {
          "label": "Brightness",
          "action": "set_brightness"
        },
        {
          "label": "Back",
          "action": "back"
        }
      ]
    }
  }
}
```

## Explicit Non-Features

The configuration format does not support:

* Conditional logic
* Loops
* Expressions
* Variables
* Platform flags
* Inline scripts

Any such behaviour must be implemented outside the configuration.

## Versioning

This specification defines configuration format version v0.1.

Breaking changes require a new major format version. Minor extensions must remain backward compatible.

## Summary

The Automenu configuration format is intentionally simple.

It describes:

* Menu hierarchy
* Labels
* Navigation
* Action bindings

It does not describe:

* Rendering
* Input handling
* Execution logic


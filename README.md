# AUTOMENU-for-the-Things---A4TT

Automenu for the Things is a platform-independent, text-based menu system for technical devices and systems.  
It provides a consistent user interface over serial consoles, Telnet connections and classic Unix terminals, without web servers, browsers or graphical user interfaces.

The project is inspired by classic Auto-Menu systems and traditional Unix tools, but implemented with a clean and modern architecture.

---

## Motivation

Many devices and tools require simple and reliable interfaces for:

- configuration
- status display
- debugging
- maintenance
- control

Web-based interfaces are often unnecessarily complex, resource-heavy and short-lived.  
Automenu focuses instead on clarity, keyboard control and minimal dependencies.

---

## Core Principles

- Identical look and behavior across all platforms
- Strict separation of rendering, logic and configuration
- Menu structure fully driven by configuration files
- No security logic at menu level
- Configuration is visible, editable and recoverable
- Everything can be repaired, nothing is permanently broken

If you have terminal access, you are considered authorized.

---

## Supported Platforms

- ESP32 via serial console
- ESP32 via Telnet
- macOS terminal
- Linux terminal
- FreeBSD terminal

All platforms use the same core engine. Differences exist only in the IO layer.

---

## Architecture Overview

Automenu is built from four clearly separated layers:

1. IO Abstraction  
   Serial, Telnet, stdin and stdout

2. Core Engine  
   Navigation, state handling, menu stack, paging

3. Renderer  
   Layout, frames, colors, status lines

4. Configuration  
   Menu structure and display configuration are separate

No platform knows UI details.  
No renderer knows platform specifics.

---

## Menu System

- Menus are generated automatically from configuration files
- Support for submenus and actions
- Automatic paging for long menus
- Consistent keyboard navigation
- Actions always return using the SPACE key
- State handling based on a stack model

Actions are logically separated from rendering.

---

## Configuration

At least two configuration files are used:

- Menu configuration  
  Defines which menus and functions exist

- Display configuration  
  Defines colors, profiles and rendering behavior

A factory default configuration always exists as a recovery anchor.

When saving changes, the existing user configuration is automatically backed up as `.bak`.

---

## Display Profiles

Automenu supports predefined display profiles, for example:

- VT100 Green
- Amber Terminal
- Classic Mono
- Modern Auto

Profiles define background, text colors and highlights.  
Colors can be forced or aligned with terminal defaults.

---

## Configuration via Menu

All relevant settings can be modified directly inside Automenu:

- Change values
- Enable or disable options
- Switch display profiles
- Save or discard configuration

No artificial restrictions are imposed.

---

## The Way Out

Anything that can be broken from inside the menu must be repairable from outside.

Automenu therefore supports startup parameters that always take precedence:

- `--default` or `--factory`
- `--reset`
- `--config`
- `--safe`
- `--profile <name>`

These mechanisms cannot be disabled by configuration.

On embedded systems, equivalent boot or input triggers are provided.

---

## Unix Integration

Automenu is a classic Unix-style tool:

- Source-based
- `make`, `make install`, `make clean`
- FreeBSD port
- macOS installation via Homebrew or MacPorts
- Man pages as the primary documentation

Man pages form the complete technical manual.

---

## Project Status

The project is in early development.  
The goal is a stable, minimal core with clear documentation.

---

## License

To be defined.  
A permissive open-source license is intended.

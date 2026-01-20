# Automenu for the Things – Display Profiles

## 1. Purpose

This document describes the display and color handling of *Automenu for the Things*.

It defines display profiles, color roles, background enforcement and fallback behavior
for different terminal capabilities.

---

## 2. Design Intent

Display profiles are not themes in a graphical sense.

They exist to:

- ensure readability
- provide consistent appearance across platforms
- support historical terminal styles
- allow controlled enforcement of colors and backgrounds

A limited, well-defined set of profiles is preferred over unlimited customization.

---

## 3. Role-Based Rendering

The renderer does not use raw ANSI escape codes directly.

Instead, it operates on abstract display roles, such as:

- normal text
- selected item
- title
- status line
- error message

Each display profile maps these roles to concrete colors and attributes.

This abstraction ensures portability and consistent behavior.

---

## 4. Display Profiles

A display profile defines a complete visual configuration.

Each profile specifies:

- background handling
- default text color
- highlight color
- status color
- error color
- optional attributes (bold, dim)

Only one profile can be active at a time.

---

## 5. Built-In Profiles

Automenu provides a small set of built-in profiles.

### 5.1 VT100 Green

Characteristics:

- black background
- green text
- minimal highlighting
- high contrast

Intended for:

- serial consoles
- embedded systems
- long-running maintenance sessions

---

### 5.2 Amber Terminal

Characteristics:

- black background
- amber or yellow text
- warm, low-glare appearance

Intended for:

- historically inspired environments
- extended reading sessions
- users preferring reduced eye strain

---

### 5.3 Classic Mono

Characteristics:

- black background
- white or light gray text
- restrained use of accent colors

Intended for:

- neutral technical environments
- mixed lighting conditions
- maximum clarity

---

### 5.4 Modern Auto

Characteristics:

- respects terminal default colors
- uses accent colors only when available
- minimal background enforcement

Intended for:

- local terminals
- personal systems
- environments with customized terminal themes

---

## 6. Background and Color Enforcement

Display profiles may enforce colors explicitly.

Enforcement behavior includes:

- setting background color
- setting foreground color
- repainting the entire screen

Enforcement is commonly enabled on embedded systems
and optional on general-purpose Unix systems.

---

## 7. ASCII Fallback Mode

Automenu must always support ASCII-only operation.

Fallback behavior:

- no color output
- no background control
- plain text rendering
- simplified framing

ASCII mode may be triggered by:

- terminal capability detection
- explicit configuration
- startup parameters
- safe or recovery modes

---

## 8. Terminal Capability Handling

Terminal capabilities vary widely.

Automenu follows these rules:

- never assume color support
- never assume background control
- always provide a readable fallback
- avoid advanced cursor manipulation

Capability detection is conservative by design.

---

## 9. Profile Selection

The active display profile may be selected:

- via display configuration file
- via Automenu configuration menu
- via startup parameters

Startup parameter selection always takes precedence.

Profile changes may trigger immediate screen repainting.

---

## 10. User Customization

User customization is intentionally limited.

Users may:

- select among predefined profiles
- adjust colors within a profile
- enable or disable enforcement

Users may not:

- redefine rendering logic
- inject raw escape sequences by default
- bypass renderer abstractions

This ensures stability and predictability.

---

## 11. Safety and Recovery

Display configuration errors must not make the system unusable.

Rules:

- invalid display settings trigger fallback behavior
- factory default display profile always exists
- startup parameters can force safe display modes

The system must remain readable under all conditions.

---

## 12. Summary

Display profiles in Automenu are designed to provide:

- consistent appearance
- controlled color usage
- historical terminal styles
- guaranteed fallback behavior

Visual clarity always takes precedence over customization.

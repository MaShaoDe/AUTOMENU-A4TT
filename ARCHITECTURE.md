# ARCHITECTURE

## Automenu – Architecture Status v0.1.0
### by Marcel Sauder 2026
### Purpose

Automenu is a platform-neutral menu and control framework designed for
embedded systems and terminal environments.

The system follows a strict separation of concerns to ensure clarity,
predictability and long-term maintainability.

The core is intentionally small, deterministic and free of platform
assumptions.

---

## Architectural Overview

Automenu consists of three strictly separated layers:

1. Core
2. Adapters
3. Configuration

No layer depends on the internal implementation details of another.

---

## 1. Core

The Core is the heart of Automenu and is fully platform-independent.

The Core is responsible for:
- Managing menu structures
- Managing the current selection state
- Handling navigation within a menu
- Triggering actions by identifier

The Core explicitly does **not** know about:
- Displays
- Terminals
- Keyboards
- Buttons
- GPIO
- Operating systems
- Filesystems

The Core processes only abstract input events such as:
- up
- down
- select
- back

The Core never performs rendering and never reads input directly.

---

## 2. Adapters

Adapters connect the Core to the outside world.

There are two primary adapter types.

### Display Adapters

Examples:
- Terminal output
- Framebuffer
- OLED
- TFT
- Serial output

Responsibilities:
- Render the current menu state
- Highlight the active selection
- Display titles and menu entries

Display adapters do not manage navigation state.
All state remains in the Core.

### Input Adapters

Examples:
- Keyboard
- Rotary encoder
- Buttons
- Serial input
- SSH session

Responsibilities:
- Translate physical or logical input into Core events

Input adapters may be stateless or minimally stateful.
Navigation logic always remains in the Core.

---

## 3. Actions

Actions are defined abstractly.

An action consists of:
- A unique identifier
- Optional parameters

The Core only knows the identifier.
Execution happens entirely within the platform context.

Examples:
- Embedded: toggle GPIO, read a sensor
- Terminal: execute a script or command

Actions are replaceable and testable.

---

## 4. Configuration

Menu structures are fully data-driven.

Configuration data defines:
- Menu titles
- Menu entries
- Assigned action identifiers
- Optional submenus

Configuration explicitly contains:
- No logic
- No platform knowledge
- No execution semantics

One configuration format applies to all platforms.

The concrete format is intentionally defined **after**
the architectural decision.

---

## Explicit Non-Goals for v0.1.0

Automenu v0.1.0 is intentionally **not**:
- An emulator of historical systems
- A GUI framework
- A web frontend
- A cloud-dependent system
- A scripting language

These may be added later via adapters,
but they are not part of the Core.

---

## Status

Version v0.1.0 is considered architecturally frozen when:
- Core, adapters and actions are strictly separated
- At least one adapter is implemented
- No platform-specific logic exists in the Core


# STATUS

## Automenu – Project Status
### by Marcel Sauder

### Current Phase

Architecture frozen.

The conceptual foundation of Automenu is complete and documented.
Core responsibilities, adapter boundaries and configuration rules are
clearly defined and agreed upon.

No further architectural changes are planned for version v0.1.0.

---

## Implemented

- Clear separation between core logic and platform-specific code
- Menu rendering concept without platform assumptions
- Abstract input event model (up, down, select, back)
- Action triggering via identifiers
- Modular design suitable for embedded and terminal environments

---

## Not Yet Implemented

The following items are intentionally **not** part of the current state
and will be addressed incrementally:

- Submenu navigation
- Cursor wrapping or advanced navigation logic
- Persistent configuration loading
- Concrete configuration file format
- Platform-specific adapters
- Desktop or graphical user interface
- Web or cloud-related components

---

## Scope of v0.1.0

Version v0.1.0 focuses exclusively on:

- A stable and minimal core
- Deterministic behaviour
- Predictable execution
- Architectural clarity

Anything beyond this scope is considered future work.

---

## Next Possible Steps

Any of the following may be addressed next, independently:

- Implement first reference adapter (terminal or embedded)
- Define configuration file format
- Introduce submenu support
- Add basic test coverage for core logic

None of these are required to validate the current state.

---

## Overall Assessment

The project is in a healthy and stable state.

The current code and structure form a solid foundation for further
development without technical debt or architectural pressure.


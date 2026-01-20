# Automenu for the Things – Design Rationale

## 1. Purpose of This Document

This document explains the design decisions behind *Automenu for the Things*.

It describes *why* certain architectural, behavioral and stylistic choices were made.
It is not a replacement for the technical specification, but a complement to it.

Where conflicts exist, the technical specification (`spec.md`) takes precedence.

---

## 2. Design Philosophy

Automenu is designed as a technical tool, not as a consumer product.

Its design follows principles that originate from:

- classic Unix utilities
- historical Auto-Menu systems
- embedded device interfaces
- long-lived technical infrastructure

The primary goals are clarity, predictability and repairability.

---

## 3. Text-Based Interface by Design

Automenu intentionally avoids graphical user interfaces.

Reasons:

- text interfaces are stable and long-lived
- they work over slow or unreliable connections
- they are easy to debug and inspect
- they scale from embedded systems to servers

A terminal is treated as a deterministic character display, not as a visual canvas.

---

## 4. No Web Interfaces

Automenu deliberately avoids web technologies.

Reasons:

- unnecessary complexity for simple configuration tasks
- increased attack surface
- dependency on browsers and client-side frameworks
- short life cycles compared to plain text interfaces

Web interfaces are not rejected in general, but considered out of scope for this project.

---

## 5. Identical Behavior Across Platforms

Automenu is designed to behave identically across all supported platforms.

This includes:

- menu layout
- key bindings
- navigation behavior
- recovery mechanisms

Platform-specific behavior is restricted to the IO layer.

This avoids fragmentation and platform-specific documentation.

---

## 6. Strict Separation of Concerns

Automenu enforces strict separation between:

- input/output handling
- core logic
- rendering
- configuration

This separation ensures:

- testability
- portability
- maintainability
- predictable behavior

No layer may bypass another layer.

---

## 7. Declarative Menu Configuration

Menus are defined declaratively, not procedurally.

Reasons:

- menu structure can be changed without code changes
- configuration can be inspected and modified manually
- the same menu model can be used across platforms

Automenu generates menus dynamically from configuration data.

---

## 8. Human-Readable Configuration

Configuration files are intended to be edited by humans.

Design decisions:

- simple key-value structures
- comment support
- minimal syntax
- no hidden magic

Complexity is handled in code, not pushed into configuration syntax.

---

## 9. Configuration as a First-Class Feature

Configuration is not hidden behind a special mode.

Design implications:

- configuration can be viewed and modified from within Automenu
- configuration changes are explicit and reversible
- users are trusted to understand what they are doing

This reflects a classic Unix trust model.

---

## 10. No Menu-Level Security

Automenu does not implement authentication, authorization or access control.

Assumptions:

- terminal access already implies trust
- security is handled at the system or network level
- adding security at the menu level would complicate the design without real benefit

This is a deliberate and documented choice.

---

## 11. Recovery and Repairability

A core design goal is that the system must always be recoverable.

Key principles:

- no configuration can permanently break the system
- factory defaults always exist
- recovery is possible without menu access

Startup parameters and boot triggers provide guaranteed recovery paths.

---

## 12. Display Profiles and Color Enforcement

Display appearance is controlled via profiles.

Design decisions:

- a small set of predefined profiles
- role-based color definitions
- optional enforcement of background and colors
- ASCII-only fallback always available

The goal is a consistent and readable interface, not aesthetic customization.

---

## 13. Historical Inspiration

Automenu is inspired by historical systems such as:

- classic Auto-Menu implementations
- VT100-style terminals
- early Unix administration tools

These inspirations are used as guidance, not as strict templates.

Modern requirements such as portability and configurability are addressed explicitly.

---

## 14. Explicit Non-Goals

Automenu explicitly does not aim to:

- replace shell environments
- provide scripting languages
- act as a full application framework
- offer GUI-like interactivity
- optimize for casual end users

These limitations are intentional.

---

## 15. Stability Over Features

Design favors stability and predictability over feature growth.

Consequences:

- features are added conservatively
- backward compatibility is preferred
- configuration formats change rarely and deliberately

Automenu values long-term usability over short-term convenience.

---

## 16. Summary

Automenu for the Things is designed as a small, reliable and understandable tool.

It prioritizes:

- technical clarity
- consistent behavior
- human-readable configuration
- guaranteed recovery paths

The design deliberately resists unnecessary complexity.


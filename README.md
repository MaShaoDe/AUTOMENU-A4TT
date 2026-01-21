# Automenu for the Things
### by Marcel Sauder 2026

Automenu for the Things is a small, portable, text-based menu system designed for embedded devices and Unix-like systems.

It provides a consistent, character-based user interface over serial consoles, Telnet connections and terminal sessions, without relying on graphical interfaces, web technologies or heavy frameworks.

Automenu is inspired by classic Automenu systems and Unix tools, focusing on clarity, predictability and long-term maintainability.

## Key Characteristics

* text-based menu system
* identical behavior across platforms
* serial, Telnet and terminal operation
* human-readable configuration
* strong recovery mechanisms
* no web UI, no browser dependencies

## Supported Environments

Automenu is designed to run on:

* embedded systems (ESP32)
* FreeBSD
* Linux
* macOS

The same core logic is used on all platforms. Platform-specific code is limited to input and output handling.

## Design Goals

Automenu aims to:

* provide simple and reliable menu navigation
* avoid graphical user interfaces
* avoid web servers and browser-based configuration
* remain debuggable with standard Unix tools
* ensure that misconfiguration is always recoverable

Automenu deliberately avoids feature bloat and unnecessary abstraction.

## Documentation

The complete technical documentation is located in the `docs/` directory.

* `docs/spec.md` – technical specification and architecture
* `docs/design.md` – design rationale and guiding principles
* `docs/configuration.md` – configuration system and file formats
* `docs/display_profiles.md` – display profiles and color handling
* `docs/startup_and_recovery.md` – startup parameters and recovery paths
* `docs/build_and_install.md` – build and installation instructions
* `docs/portability.md` – platform portability considerations
* `docs/manpages.md` – manual page overview
* `docs/roadmap.md` – development roadmap

## Build and Install

On Unix-like systems, Automenu is built using a traditional make-based workflow:

```
make
sudo make install
```

Installation paths follow standard Unix conventions and integrate cleanly with FreeBSD Ports, Homebrew and MacPorts.

Detailed instructions are available in `docs/build_and_install.md`.

## Configuration

Automenu uses human-readable configuration files to define menu structure and display behavior.

Configuration can be:

* edited manually
* modified through the Automenu interface
* reset to factory defaults at any time

Broken configurations are preserved and never silently discarded.

See `docs/configuration.md` for details.

## Startup and Recovery

Automenu provides explicit startup parameters and recovery modes to ensure the system is always usable.

Recovery features include:

* factory reset
* safe mode (ASCII-only rendering)
* forced display profiles
* direct configuration access

These mechanisms cannot be disabled by configuration.

See `docs/startup_and_recovery.md` for details.

## Status

Automenu for the Things is in early development.

The documentation represents a stabilized conceptual foundation. Implementation work proceeds incrementally, guided by the specification.

## License

The license for this project is documented in the `LICENSE` file.


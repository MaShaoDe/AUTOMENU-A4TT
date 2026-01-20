# AUTOMENU-for-the-Things---A4TT

Automenu for the Things ist ein plattformunabhängiges, textbasiertes Menüsystem für technische Geräte und Systeme.  
Es bietet eine einheitliche Benutzerführung über serielle Konsolen, Telnet und klassische Unix Terminals, ohne Webserver, ohne Browser und ohne grafische Oberflächen.

Das Projekt ist bewusst in der Tradition klassischer Auto-Menü-Systeme und Unix Werkzeuge gedacht, technisch jedoch modern umgesetzt.

---

## Motivation

Viele Geräte und Tools benötigen einfache, robuste Bedienoberflächen für:

- Konfiguration
- Statusanzeigen
- Debugging
- Wartung
- Steuerung

Web-UIs sind dafür oft unnötig komplex, ressourcenintensiv und kurzlebig.  
Automenu setzt stattdessen auf Klartext, Tastaturbedienung und minimale Abhängigkeiten.

---

## Grundprinzipien

- Identisches Outfit und identische Bedienung auf allen Plattformen
- Strikte Trennung von Darstellung, Logik und Konfiguration
- Menüstruktur vollständig konfigurationsgetrieben
- Keine Sicherheitslogik auf Menüebene
- Konfiguration ist sichtbar, editierbar und rücksetzbar
- Alles ist reparierbar, nichts endgültig kaputt

Wer Terminalzugang hat, ist berechtigt.

---

## Unterstützte Plattformen

- ESP32 über serielle Konsole
- ESP32 über Telnet
- macOS Terminal
- Linux Terminal
- FreeBSD Terminal

Alle Plattformen nutzen denselben Core. Unterschiede bestehen nur in der IO-Anbindung.

---

## Architekturüberblick

Automenu besteht aus vier klar getrennten Schichten:

1. IO-Abstraktion  
   Serial, Telnet, stdin und stdout

2. Core Engine  
   Navigation, Zustände, Menü-Stack, Paging

3. Renderer  
   Darstellung, Rahmen, Farben, Statuszeilen

4. Konfiguration  
   Menüstruktur und Display getrennt

Keine Plattform kennt UI-Details. Kein Renderer kennt Plattformen.

---

## Menüsystem

- Automatisch generierte Menüs aus Konfigurationsdateien
- Unterstützung für Untermenüs und Aktionen
- Paging bei langen Menüs
- Konsistente Tastaturbedienung
- Rückkehr aus Aktionen immer mit der Space-Taste
- Zustandsverwaltung über Stack

Aktionen sind logisch getrennt von der Darstellung.

---

## Konfiguration

Es existieren mindestens zwei Konfigurationsdateien:

- Menü-Konfiguration  
  Beschreibt, welche Menüs und Funktionen existieren

- Display-Konfiguration  
  Beschreibt Farben, Profile, Verhalten und Darstellung

Zusätzlich existiert immer eine Factory-Default-Konfiguration als Rettungsanker.

Beim Speichern wird die bestehende Benutzerkonfiguration automatisch als `.bak` gesichert.

---

## Display Profile

Automenu unterstützt vordefinierte Anzeigeprofile, zum Beispiel:

- VT100 Green
- Amber Terminal
- Classic Mono
- Modern Auto

Profile definieren Hintergrund, Textfarben und Hervorhebungen.  
Farben können erzwungen oder an Terminal-Defaults angepasst werden.

---

## Konfiguration im Menü

Alle relevanten Einstellungen können direkt im Automenu geändert werden:

- Werte anpassen
- Optionen ein oder ausschalten
- Display-Profile wechseln
- Konfiguration speichern oder verwerfen

Es gibt keine künstlichen Einschränkungen.

---

## The Way Out

Was im Menü kaputtgemacht werden kann, muss ausserhalb des Menüs reparierbar sein.

Automenu unterstützt deshalb Startparameter, die immer Vorrang haben:

- `--default` oder `--factory`
- `--reset`
- `--config`
- `--safe`
- `--profile <name>`

Diese Mechanismen können nicht durch Konfiguration deaktiviert werden.

Auf Embedded-Systemen existieren äquivalente Boot- oder Eingabemechanismen.

---

## Unix Integration

Automenu ist ein klassisches Unix Werkzeug:

- Source-basiert
- `make`, `make install`, `make clean`
- FreeBSD Port
- macOS Installation über Homebrew oder MacPorts
- Man-Pages als primäres Handbuch

Man-Pages bilden das vollständige technische Manual.

---

## Projektstatus

Das Projekt befindet sich im Aufbau.  
Ziel ist ein stabiler, minimaler Core mit klarer Dokumentation.

---

## Lizenz

Wird festgelegt. Eine permissive Lizenz ist vorgesehen.

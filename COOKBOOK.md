# Automenu Cookbook

Rezepte für eigene Menüs. Du brauchst nur einen Texteditor.

## 1. Bauen und starten

Läuft auf macOS, Linux und FreeBSD. Gebraucht wird nur ein C-Compiler (`cc`): auf dem Mac `xcode-select --install`, unter FreeBSD ist er dabei, unter Linux z.B. `sudo apt install build-essential`.

    make                             # normale Version
    make kiosk                       # Kiosk-Version: Ctrl-C tut nichts
    make test                        # Tests
    ./automenu mein_menu.txt         # Menü starten
    ./automenu --check mein_menu.txt # Datei nur prüfen, ohne Menü zu starten
    ./automenu                       # ohne Datei: Standardpfade (siehe unten)

Tasten: Pfeil hoch/runter und Enter, Ziffern 1 bis 9 springen direkt zum Eintrag und führen ihn aus, Esc oder Backspace gehen ein Menü zurück, H zeigt die Hilfe, Ctrl-C beendet (nicht in der Kiosk-Version).

Der Bildschirm hat einen Rahmen, einen roten Titelbalken, die Einträge in Grün, eine graue Leiste mit dem Hilfetext des gewählten Eintrags und unten Datum, Uhrzeit und `benutzer@rechner`. Unter dem Rahmen steht rechts die Programmversion. Oben rechts steht "MENÜ 1 VON 2" (Position des Menüs in der Datei). Der Rahmen füllt das ganze Terminalfenster und passt sich sofort an, wenn du das Fenster vergrösserst oder die Schrift mit Cmd und + grösser stellst. Mindestgrösse: 60 x 16. Die Darstellung lässt sich mit Umgebungsvariablen umstellen:

| Variable | Wirkung |
|---|---|
| `NO_COLOR=1` | ohne Farben, nur invertiert |
| `AUTOMENU_COLORS=256` | 256 Farben erzwingen (Normalfall in Terminal und iTerm2) |
| `AUTOMENU_COLORS=16` | nur die 16 Grundfarben (sehr alte Terminals) |
| `AUTOMENU_FRAME=solid` | Rahmen aus türkisen Farbflächen, geht mit jeder Schrift (Standard) |
| `AUTOMENU_FRAME=lines` | feine Doppellinien wie auf den Screenshots im README, braucht eine Schrift mit Linienzeichen (zum Beispiel Menlo) |
| `AUTOMENU_FRAME=ascii` | Rahmen aus `+ - \|` |

Dauerhaft setzt du sie in `~/.bashrc`, zum Beispiel `export AUTOMENU_FRAME=lines`.

## 2. Installieren und Wo liegt was

    sudo make install                    # nach /usr/local
    make install PREFIX=$HOME/.local     # ohne Admin-Rechte (dann ~/.local/bin im PATH)
    sudo make uninstall

Auf FreeBSD später als Port oder Package (`pkg install automenu`).

| Was | Pfad (bei PREFIX=/usr/local) |
|---|---|
| Programm | `/usr/local/bin/automenu` |
| Man-Page | `/usr/local/share/man/man1/automenu.1` |
| Beispielmenü für die Systemkonfiguration | `/usr/local/etc/automenu/menu.txt.sample` |
| Weitere Beispiele | `/usr/local/share/examples/automenu/` |
| Dein Menü (pro Benutzer) | `~/.config/automenu/menu.txt` |
| Systemweites Menü | `/usr/local/etc/automenu/menu.txt` |

Ohne Dateiangabe sucht `automenu` in dieser Reihenfolge: `$XDG_CONFIG_HOME/automenu/menu.txt`, `~/.config/automenu/menu.txt`, `/usr/local/etc/automenu/menu.txt`. Die erste vorhandene Datei gilt.

`make install` überschreibt nie dein Menü. Es legt nur `menu.txt.sample` und die Beispiele ab. Der erste Start geht so:

    mkdir -p ~/.config/automenu
    cp /usr/local/share/examples/automenu/menu.txt ~/.config/automenu/menu.txt
    automenu

Auf dem Mac ist `/usr/local` ein üblicher Ort. Wer Homebrew nutzt, kann später eine eigene Formel bekommen. Bis dahin ist `PREFIX=$HOME/.local` die sauberste Lösung, weil nichts im System angefasst wird.

### Installation auf macOS, Schritt für Schritt

Getestet auf macOS Golden Gate 27.2 mit dem Terminal von macOS (Apple Silicon, bash).

1. **Compiler prüfen:** `cc --version`. Fehlt er, installiert `xcode-select --install` die Kommandozeilen-Tools.
2. **ZIP entpacken** und in den Ordner wechseln, in dem die `Makefile` liegt:
   `cd ~/Downloads/automenu` (mit `ls` prüfen).
3. **Bauen und testen:** `make`, dann `make test`. Am Ende muss "Alle Tests ok" stehen.
4. **Installieren ohne Admin-Rechte:** `make install PREFIX=$HOME/.local`
5. **PATH eintragen,** damit `automenu` überall gefunden wird. Bei bash:
   `echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc`
   Bei zsh (Standard von macOS) dieselbe Zeile in `~/.zshrc`.
   Bei bash liest das Terminal von macOS beim Start `~/.bash_profile`. Damit die `~/.bashrc` gilt, muss dort stehen: `[ -f ~/.bashrc ] && . ~/.bashrc`
6. **Eigenes Menü anlegen:**
   `mkdir -p ~/.config/automenu` und
   `cp examples/menu.txt ~/.config/automenu/menu.txt`
7. **Prüfen und starten:** neues Terminalfenster öffnen (oder `hash -r`), dann `automenu --version` und `automenu`.

**Terminal für die Darstellung wie auf den Screenshots einstellen**

- Terminal, Einstellungen, Profile, Reiter **Erweitert**, "Terminal angeben": **xterm-256color**. Gilt nur für neue Fenster. Mit `echo $TERM` prüfen.
- Reiter **Text**, Schrift **Ändern…**: **Menlo Regular**. Diese Schrift hat die Linienzeichen für den Doppelrahmen.
- In `~/.bashrc` (oder `~/.zshrc`): `export AUTOMENU_FRAME=lines`
- Die Fenstergrösse ist frei wählbar. Der Rahmen passt sich sofort an. Mit Cmd und + wird die Schrift grösser.

**Aktualisieren auf eine neue Version**

Neue ZIP entpacken, in den neuen Ordner wechseln und: `make clean`, `make`, `make install PREFIX=$HOME/.local`, `hash -r`, `automenu --version`. Dein Menü unter `~/.config/automenu/` bleibt unverändert.

**Deinstallieren**

Im Ordner der installierten Version: `make uninstall PREFIX=$HOME/.local`. Dein Menü bleibt erhalten. Es liegt unter `~/.config/automenu/` und kann von Hand gelöscht werden.

**Stolpersteine aus der Praxis**

| Problem | Ursache und Lösung |
|---|---|
| `automenu: Kommando nicht gefunden` | `make install` fehlt noch oder `~/.local/bin` ist nicht im PATH (Schritt 4 und 5) |
| Es läuft eine alte Version | `hash -r`, dann `which automenu` und `automenu --version` prüfen |
| Rahmen fehlt ganz | Die Schrift hat keine Linienzeichen: Menlo wählen oder `AUTOMENU_FRAME=solid` nutzen |
| Rahmen ist ein dicker Farbbalken | Standard `solid` ist aktiv: `export AUTOMENU_FRAME=lines` setzen, Schrift Menlo |
| Rahmen aus `+ - \|` obwohl nicht gewollt | Eine alte Zeile `AUTOMENU_FRAME=ascii` steht noch in `~/.bashrc`: `env \| grep AUTOMENU` prüfen, dann `sed -i '' '/AUTOMENU_FRAME/d' ~/.bashrc` |
| Farben olivgrün oder blass | `TERM` steht auf `vt100`. Ab Version 0.3.2 wird das erkannt. Zusätzlich im Profil `xterm-256color` einstellen |
| Alias wie `ll` funktioniert im Menü nicht | `run` startet `/bin/sh` ohne die eigenen Aliase. Den echten Befehl schreiben, zum Beispiel `ls -alFh` |

## 3. Die Menüdatei

Eine Zeile pro Ding. Leerzeilen und Zeilen mit `#` am Anfang werden ignoriert.
Das erste Menü der Datei ist das Startmenü.

    menu <id> "Titel"                 beginnt ein neues Menü
    item "Text" run <Befehl>          führt einen Befehl aus
    item "Text" menu <id>             öffnet ein anderes Menü
    item "Text" info <Text>           zeigt einen Text an
    item "Text" back                  geht zurück
    item "Text" exit                  beendet das Programm

Nach dem Text darf ein zweiter Text in Anführungszeichen stehen. Er erscheint in der grauen Leiste, sobald der Eintrag gewählt ist:

    item "Speicherplatz" "Zeigt den freien Platz auf allen Laufwerken" run df -h

Ohne Hilfetext zeigt die Leiste selbst etwas Passendes an, zum Beispiel "Befehl: df -h".

`<id>` ist ein Wort ohne Leerzeichen, zum Beispiel `werkzeuge`.
Texte in Anführungszeichen dürfen Umlaute enthalten. Die Datei muss UTF-8 sein.
Windows-Zeilenenden (CRLF) sind kein Problem.

## 4. Rezepte

### Ein Befehl

    menu start "Mein Menü"
      item "Datum anzeigen"   run date
      item "Beenden"          exit

Alles nach `run` wird von der Shell ausgeführt. Nach dem Befehl wartet das Menü auf eine Taste, damit du die Ausgabe lesen kannst.

### Befehl mit Pipe, Parametern oder mehreren Schritten

    item "Grosse Dateien"   run du -sh * | sort -h | tail -5
    item "Zwei Dinge"       run date; uptime
    item "Nur wenn online"  run ping -c 1 1.1.1.1 && echo online

### Untermenü

    menu start "Haupt"
      item "Werkzeuge"        menu werkzeuge
      item "Beenden"          exit

    menu werkzeuge "Werkzeuge"
      item "Speicherplatz"    run df -h
      item "Zurück"           back

Die Reihenfolge der Menüs in der Datei ist egal, nur das erste ist das Startmenü.

### Programm mit eigener Oberfläche (Editor, htop, ssh)

    item "Notizen"          run ${EDITOR:-vi} ~/notizen.txt
    item "Prozesse"         run htop
    item "Server"           run ssh benutzer@server

Das Menü gibt dem Programm das Terminal und übernimmt es danach wieder zurück.

### Infotext, auch mehrzeilig

    item "Hilfe"   info Zeile 1\nZeile 2\nZeile 3

`\n` ergibt einen Zeilenumbruch. Ohne Text wird der Eintragstext selbst angezeigt.

### Eigenes Skript starten

    item "Backup"   run ~/bin/backup.sh

Mach das Skript vorher ausführbar: `chmod +x ~/bin/backup.sh`.

### Menü beim Anmelden automatisch starten (Kiosk)

1. Kiosk-Version bauen und installieren: `make kiosk && sudo make install`
2. Am Ende von `~/.profile` eintragen:

       exec /pfad/zu/automenu /pfad/zu/menu.txt

3. Im Menü muss ein `exit`-Eintrag stehen, wenn man wieder rauskommen soll. Ohne `exit` und ohne Ctrl-C gibt es keinen Weg hinaus ausser Abmelden per Konsole von aussen.

## 5. Wenn etwas nicht geht

`./automenu --check menu.txt` nennt die Zeile mit dem Fehler.

| Meldung | Ursache |
|---|---|
| `item steht vor dem ersten menu` | Vor dem ersten `item` fehlt eine `menu`-Zeile |
| `run braucht einen Befehl` | Nach `run` steht nichts |
| `verweist auf unbekanntes Menü` | Bei `menu <id>` stimmt die ID nicht |
| `Menü-ID doppelt` | Zwei Menüs mit derselben ID |
| `Menü ohne Einträge` | Ein `menu` ohne `item` darunter |
| `unbekannter Typ` | Tippfehler bei `run`, `menu`, `info`, `back`, `exit` |
| `Syntax: menu <id> "Titel"` | Titel ohne Anführungszeichen |
| `Hilfetext ohne schliessendes Anführungszeichen` | Bei einem Hilfetext fehlt das zweite `"` |

## 6. Gute Gewohnheiten

- Höchstens 7 Einträge pro Menü: dann hat jeder Eintrag eine Leerzeile dazwischen (bei mehr Einträgen rücken sie ohne Leerzeilen zusammen). Bei bis zu 9 funktionieren die Zifferntasten für alle, bei mehr scrollt die Liste ohne Leerzeilen.
- Lieber mehrere kleine Untermenüs als ein langes Menü.
- Immer einen `back`-Eintrag am Ende jedes Untermenüs.
- Erst `--check`, dann starten.

## 7. Die Technik dahinter (nur falls du basteln willst)

- `core.c` kennt nur Ereignisse (hoch, runter, wählen, zurück) und meldet, was ausgeführt werden soll. Es zeichnet nichts und startet nichts.
- `terminal.c` übersetzt Tasten in Ereignisse, zeichnet den Bildschirm und führt Befehle aus.
- Für den ESP32 wird nur `terminal.c` durch einen Adapter für die serielle Konsole ersetzt. `loader.c` kann Menüs auch aus einem Textpuffer lesen (`menu_parse_text`), das Menü kann also fest im Flash liegen.

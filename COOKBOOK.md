# Automenu Cookbook

Rezepte für eigene Menüs. Du brauchst nur einen Texteditor.

## 1. Bauen und starten

Läuft auf macOS, Linux und FreeBSD. Gebraucht wird nur ein C-Compiler (`cc`): auf dem Mac `xcode-select --install`, unter FreeBSD ist er dabei, unter Linux z.B. `sudo apt install build-essential`.

    make                             # normale Version
    make test                        # Tests
    ./automenu mein_menu.mdf         # Menü starten
    ./automenu --check mein_menu.mdf # Datei nur prüfen, ohne Menü zu starten
    ./automenu --kiosk mein_menu.mdf # Kiosk-Modus: Ctrl-C beendet nicht
    ./automenu                       # ohne Datei: Standardpfade (siehe unten)

### Startparameter

Aufruf: `automenu [Option] [datei]`. Ohne Datei sucht `automenu` an den Standardpfaden (Abschnitt 2).

| Aufruf | Wirkung |
|---|---|
| `automenu` | Menü aus der Standarddatei starten |
| `automenu datei.mdf` | Menü aus dieser Datei starten |
| `automenu --kiosk [datei]` | Kiosk-Modus: Ctrl-C beendet das Programm nicht. Raus nur über einen `exit`-Eintrag (der mit Passwort gesperrt sein kann) |
| `automenu --check [datei]` | Datei nur prüfen, Menü wird nicht gestartet. Meldet Fehler mit Zeilennummer, sonst "ok" mit Anzahl Menüs und Passwörter |
| `automenu --hash [name]` | Zweimal Passwort eingeben (ohne Echo), gibt die fertige `password`-Zeile aus. Ohne Name gilt `admin` |
| `automenu --version` | Versionsnummer anzeigen |

Es steht immer nur eine Option vor der Datei, `--kiosk` und `--check` lassen sich nicht kombinieren. Bei falschem Aufruf zeigt `automenu` eine kurze Übersicht. Rückgabewert: 0 = ok, 1 = Fehler in der Datei oder beim Start, 2 = falscher Aufruf.

Tasten: Pfeil hoch/runter und Enter, Ziffern 1 bis 9 springen direkt zum Eintrag und führen ihn aus, Esc oder Backspace gehen ein Menü zurück, H zeigt die Hilfe, Ctrl-C beendet (nicht im Kiosk-Modus), Mausklick führt aus, Mausrad wählt.

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
| Beispielmenü für die Systemkonfiguration | `/usr/local/etc/automenu/menu.mdf.sample` |
| Weitere Beispiele | `/usr/local/share/examples/automenu/` |
| Dein Menü (pro Benutzer) | `~/.config/automenu/menu.mdf` |
| Systemweites Menü | `/usr/local/etc/automenu/menu.mdf` |

Ohne Dateiangabe sucht `automenu` in dieser Reihenfolge: `$XDG_CONFIG_HOME/automenu/`, `~/.config/automenu/`, `/usr/local/etc/automenu/`. In jedem Ordner zählt zuerst `menu.mdf`, danach `menu.txt` (alter Name, geht weiterhin). Die erste vorhandene Datei gilt.

`make install` überschreibt nie dein Menü. Es legt nur `menu.mdf.sample` und die Beispiele ab. Der erste Start geht so:

    mkdir -p ~/.config/automenu
    cp /usr/local/share/examples/automenu/menu.mdf ~/.config/automenu/menu.mdf
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
   `cp examples/menu.mdf ~/.config/automenu/menu.mdf`
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

## 3. Die Menüdatei (.mdf)

Die Menüdefinitionsdatei (MDF, Menu Definition File) heisst `menu.mdf`. Eine Zeile pro Ding. Leerzeilen und Zeilen mit `#` am Anfang werden ignoriert.
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

### Einstellungen

Alle optional, am besten oben in der Datei:

    set blackout 300      Schirm nach 300 Sekunden ohne Eingabe schwarz (0 = aus, Standard)
    set clock utc         Uhr in UTC statt Ortszeit (local = Standard)
    set mouse off         Maus abschalten (on = Standard)

### Maus

Linksklick auf einen Eintrag führt ihn aus, das Mausrad wählt auf und ab. Ein Klick auf den Schwarzschirm weckt nur auf. Beim Starten eines Befehls ist die Maus aus, damit andere Programme sie normal nutzen können.

### Blackout

Nach der eingestellten Zeit ohne Taste oder Klick wird der Schirm schwarz. Die erste Eingabe weckt nur auf und wird sonst nicht ausgewertet, damit kein Eintrag versehentlich startet.

### Passwortschutz für einzelne Einträge

**Schritt für Schritt**

1. Passwort-Zeile erzeugen (fragt 2 Mal ohne Echo):

       automenu --hash admin

   `admin` ist nur ein frei gewählter Name für dieses Passwort, kein Benutzerkonto. Ohne Name gilt `admin`. Das Programm ändert dabei nichts am System und schreibt keine Datei, es druckt nur eine Zeile:

       password admin pbkdf2-sha256:20000:3f9a...:b71c...

2. Diese Zeile in die `.mdf`-Datei kopieren, ausserhalb der Menüs (am besten oben).
3. Beim gewünschten Eintrag `lock admin` vor den Typ schreiben:

       item "Neustart" "Fragt nach dem Passwort" lock admin run sudo reboot
       item "Beenden"  "Nur mit Passwort"        lock admin exit

4. Mit `automenu --check datei.mdf` prüfen, dann starten.

**Ablauf beim Benutzer**

Eintrag wählen (Enter, Ziffer oder Mausklick), Fenster "Passwort" erscheint, die Eingabe zeigt Sternchen. Enter bestätigt, Esc bricht ab. Bei falschem Passwort kommt eine Meldung und eine Wartezeit von 1 Sekunde. Nach 3 Fehlversuchen geht es zurück ins Menü, mit 3 Sekunden Wartezeit. Nach der Ausführung ist der Eintrag wieder gesperrt. Es wird bei jeder Auswahl neu gefragt, es gibt keine Freischaltung auf Zeit.

**Regeln**

- Geschützt wird immer nur der einzelne Eintrag, nie ein ganzes Menü. Wer ein Untermenü schützen will, sperrt den Eintrag, der dorthin führt.
- Jeder Eintrag darf `lock` haben, auch `info`, `back` und `exit`.
- Mehrere Passwörter mit verschiedenen Namen sind möglich (`password chef ...`, `password gast ...`, dann `lock chef` oder `lock gast`).
- Ein Name darf nur einmal definiert sein. `lock` mit unbekanntem Namen ist ein Fehler, den `--check` meldet.

**Passwort ändern oder vergessen**

Es gibt kein Zurücksetzen. Mit `automenu --hash admin` eine neue Zeile erzeugen und die alte `password admin`-Zeile in der Datei ersetzen. Wer das Passwort vergessen hat, ersetzt die Zeile genauso, denn die Datei kann man ja bearbeiten.

**Wie das Passwort gespeichert wird**

Nie im Klartext. In der Datei steht nur ein Hash: PBKDF2 mit HMAC-SHA-256, 20000 Runden und einem zufälligen Salz (16 Bytes aus `/dev/urandom`). Gleiche Passwörter ergeben deshalb verschiedene Zeilen. Das Format ist `pbkdf2-sha256:Runden:Salz:Hash`. Die Zeile nicht von Hand ändern, sonst meldet `--check` einen ungültigen Hash.

**Was der Schutz kann und was nicht (bitte lesen)**

Das ist eine Sperre gegen Neugierige und Versehen, keine Sicherheitsfunktion. Sie schützt nicht gegen:

- Ctrl-C im normalen Modus. Das beendet das Programm ohne Passwort. Mit `automenu --kiosk` ist Ctrl-C aus.
- Fenster schliessen, Terminal-Sitzung, SSH, eine zweite Shell. Wer so ins System kommt, ist wieder in der Shell.
- Das Lesen der Datei. Wer die `.mdf` lesen kann, sieht die Befehle und kann sie direkt in der Shell ausführen. Der Hash selbst lässt sich nicht zurückrechnen, aber ein schwaches Passwort kann man raten.
- Programme, die du aus dem Menü startest. Hat `top` oder `vi` eine Shell-Funktion, ist man draussen.

Der Kiosk-Modus sperrt nur Ctrl-C, mehr nicht. Sinnvoll ist die Kombination für einfache Dinge: Menü startet beim Anmelden (`exec automenu --kiosk datei.mdf` in `~/.profile`), gefährliche Einträge sind mit `lock` gesperrt, die Datei ist nur für dich lesbar (`chmod 600 datei.mdf`) und gehört dir, nicht dem Benutzer des Menüs. Wo echte Sicherheit gebraucht wird, brauchst du Benutzerrechte des Systems.

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

Kiosk heisst bei Automenu: Ctrl-C beendet das Programm nicht. Raus kommt man nur über einen `exit`-Eintrag, der auch mit Passwort gesperrt sein kann. Ein normales `automenu` genügt, der Modus wird beim Start eingeschaltet.

1. Am Ende von `~/.profile` eintragen:

       exec automenu --kiosk /pfad/zu/menu.mdf

2. Im Menü muss ein `exit`-Eintrag stehen, wenn man wieder rauskommen soll. Ohne `exit` und ohne Ctrl-C gibt es keinen Weg hinaus ausser Abmelden per Konsole von aussen.

## 5. Wenn etwas nicht geht

`./automenu --check menu.mdf` nennt die Zeile mit dem Fehler.

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
| `nutzt unbekanntes Passwort` | Bei `lock` fehlt die passende `password`-Zeile |
| `Passwort-Hash ungültig` | Hash nicht mit `automenu --hash` erzeugt oder beim Kopieren abgeschnitten |
| `unbekannte Einstellung` | Tippfehler bei `set blackout`, `set clock`, `set mouse` |
| Maus reagiert nicht | `set mouse off` in der Datei, oder das Terminal unterstützt keine Mausmeldungen |

## 6. Gute Gewohnheiten

- Höchstens 7 Einträge pro Menü: dann hat jeder Eintrag eine Leerzeile dazwischen (bei mehr Einträgen rücken sie ohne Leerzeilen zusammen). Bei bis zu 9 funktionieren die Zifferntasten für alle, bei mehr scrollt die Liste ohne Leerzeilen.
- Lieber mehrere kleine Untermenüs als ein langes Menü.
- Immer einen `back`-Eintrag am Ende jedes Untermenüs.
- Erst `--check`, dann starten.

## 7. Die Technik dahinter (nur falls du basteln willst)

- `core.c` kennt nur Ereignisse (hoch, runter, wählen, zurück) und meldet, was ausgeführt werden soll. Es zeichnet nichts und startet nichts.
- `auth.c` prüft Passwörter (PBKDF2-HMAC-SHA256, reines C). `terminal.c` übersetzt Tasten und Mausklicks in Ereignisse, zeichnet den Bildschirm und führt Befehle aus.
- Für den ESP32 wird nur `terminal.c` durch einen Adapter für die serielle Konsole ersetzt. `loader.c` kann Menüs auch aus einem Textpuffer lesen (`mdf_parse_text`), das Menü kann also fest im Flash liegen.

# Automenu Cookbook

Recipes for your own menus. All you need is a text editor.

## 1. Building and starting

Runs on macOS, Linux and FreeBSD. All you need is a C compiler (`cc`): on the Mac `xcode-select --install`, FreeBSD has one, on Linux e.g. `sudo apt install build-essential`.

    make                             # build
    make test                        # tests
    ./automenu my_menu.mdf           # start a menu
    ./automenu --check my_menu.mdf   # only check the file, do not start the menu
    ./automenu --kiosk my_menu.mdf   # kiosk mode: Ctrl-C does not quit
    ./automenu                       # without a file: default paths (see below)

### Start options

Call: `automenu [option] [file]`. Without a file `automenu` looks in the default paths (section 2).

| Call | Effect |
|---|---|
| `automenu` | start the menu from the default file |
| `automenu file.mdf` | start the menu from this file |
| `automenu --kiosk [file]` | kiosk mode: Ctrl-C does not quit the program. The only way out is an `exit` item (which can be locked with a password) |
| `automenu --check [file]` | only check the file, do not start the menu. Reports errors with the line number, otherwise "ok" with the number of menus and passwords |
| `automenu --hash [name]` | asks twice for a password (no echo) and prints the finished `password` line. Without a name `admin` is used |
| `automenu --version` | show the version number |

Only one option may stand before the file, `--kiosk` and `--check` cannot be combined. With a wrong call `automenu` prints a short overview. Exit status: 0 = ok, 1 = error in the file or at start, 2 = wrong call.

### Keys and mouse

Arrow up/down and Enter, digits 1 to 9 jump to the item and run it, Esc or Backspace go back one menu, H shows the help, Ctrl-C quits (not in kiosk mode). A mouse click runs an item, the mouse wheel selects.

### The screen

The screen has a frame, a red title bar, the items in green, a grey bar with the help text of the selected item, and at the bottom date, time and `user@host`. Below the frame on the right is the program version. At the top right it says "MENU 1 OF 2" (position of the menu in the file). The frame fills the whole terminal window and adapts at once when you enlarge the window or make the font bigger with Cmd and +. Minimum size: 60 x 16. The date and month names follow the language of your system. The look can be changed with environment variables:

| Variable | Effect |
|---|---|
| `NO_COLOR=1` | no colours, inverse video only |
| `AUTOMENU_COLORS=256` | force 256 colours (normal case in Terminal and iTerm2) |
| `AUTOMENU_COLORS=16` | only the 16 basic colours (very old terminals) |
| `AUTOMENU_FRAME=solid` | frame made of teal coloured areas, works with any font (default) |
| `AUTOMENU_FRAME=lines` | fine double lines as in the screenshots in the README, needs a font with line characters (for example Menlo) |
| `AUTOMENU_FRAME=ascii` | frame made of `+ - \|` |

To make them permanent put them in `~/.bashrc`, for example `export AUTOMENU_FRAME=lines`.

## 2. Installing and where things live

    sudo make install                    # to /usr/local
    make install PREFIX=$HOME/.local     # without admin rights (then ~/.local/bin must be in PATH)
    sudo make uninstall

On FreeBSD later as a port or package (`pkg install automenu`).

| What | Path (with PREFIX=/usr/local) |
|---|---|
| Program | `/usr/local/bin/automenu` |
| Man page | `/usr/local/share/man/man1/automenu.1` |
| Example menu for the system configuration | `/usr/local/etc/automenu/menu.mdf.sample` |
| More examples | `/usr/local/share/examples/automenu/` |
| Your menu (per user) | `~/.config/automenu/menu.mdf` |
| System-wide menu | `/usr/local/etc/automenu/menu.mdf` |

Without a file name `automenu` searches `$XDG_CONFIG_HOME/automenu/`, `~/.config/automenu/` and `/usr/local/etc/automenu/`, in this order. In each folder `menu.mdf` counts first, then `menu.txt` (the old name, still works). The first file found wins.

`make install` never overwrites your menu. It only installs `menu.mdf.sample` and the examples. The first start goes like this:

    mkdir -p ~/.config/automenu
    cp /usr/local/share/examples/automenu/menu.mdf ~/.config/automenu/menu.mdf
    automenu

On the Mac `/usr/local` is a common place. If you use Homebrew you may get your own formula later. Until then `PREFIX=$HOME/.local` is the cleanest solution because nothing in the system is touched.

### Installation on macOS, step by step

Tested on macOS Golden Gate 27.2 with the macOS Terminal (Apple Silicon, bash).

1. **Check the compiler:** `cc --version`. If it is missing, `xcode-select --install` installs the command line tools.
2. **Unpack the ZIP** and change into the folder that holds the `Makefile`:
   `cd ~/Downloads/automenu` (check with `ls`).
3. **Build and test:** `make`, then `make test`. At the end it must say "All tests ok".
4. **Install without admin rights:** `make install PREFIX=$HOME/.local`
5. **Set the PATH** so `automenu` is found everywhere. With bash:
   `echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc`
   With zsh (the macOS default) the same line goes into `~/.zshrc`.
   With bash the macOS Terminal reads `~/.bash_profile` at start. For `~/.bashrc` to apply, that file must contain: `[ -f ~/.bashrc ] && . ~/.bashrc`
6. **Create your own menu:**
   `mkdir -p ~/.config/automenu` and
   `cp examples/menu.mdf ~/.config/automenu/menu.mdf`
7. **Check and start:** open a new Terminal window (or `hash -r`), then `automenu --version` and `automenu`.
8. **Man page (optional):** `echo 'export MANPATH="$HOME/.local/share/man:$MANPATH"' >> ~/.bashrc`, then in a new window `man automenu`.

**Set up the Terminal for the look of the screenshots**

- Terminal, Settings, Profiles, tab **Advanced**, "Declare terminal as": **xterm-256color**. Applies to new windows only. Check with `echo $TERM`.
- Tab **Text**, font **Change…**: **Menlo Regular**. This font has the line characters for the double frame.
- In `~/.bashrc` (or `~/.zshrc`): `export AUTOMENU_FRAME=lines`
- The window size is free. The frame adapts at once. Cmd and + make the font bigger.

**Updating to a new version**

Unpack the new ZIP, change into the new folder and run: `make clean`, `make`, `make install PREFIX=$HOME/.local`, `hash -r`, `automenu --version`. Your menu under `~/.config/automenu/` stays unchanged.

**Uninstalling**

In the folder of the installed version: `make uninstall PREFIX=$HOME/.local`. Your menu stays. It lives under `~/.config/automenu/` and can be deleted by hand.

**Pitfalls from practice**

| Problem | Cause and solution |
|---|---|
| `automenu: command not found` | `make install` is still missing, or `~/.local/bin` is not in PATH (steps 4 and 5) |
| An old version runs | `hash -r`, then check `which automenu` and `automenu --version` |
| Frame is missing entirely | The font has no line characters: choose Menlo or use `AUTOMENU_FRAME=solid` |
| Frame is a thick colour bar | The default `solid` is active: set `export AUTOMENU_FRAME=lines` and use the font Menlo |
| Frame made of `+ - \|` although not wanted | An old line `AUTOMENU_FRAME=ascii` is still in `~/.bashrc`: check with `env \| grep AUTOMENU`, then `sed -i '' '/AUTOMENU_FRAME/d' ~/.bashrc` |
| Colours olive green or pale | `TERM` is set to `vt100`. Since version 0.3.2 this is detected. Additionally set `xterm-256color` in the profile |
| An alias like `ll` does not work in the menu | `run` starts `/bin/sh` without your aliases. Write the real command, for example `ls -alFh` |
| Wrong build after `make clean` was skipped | Always run `make clean` before `make` after unpacking a new version over an old folder |

## 3. The menu file (.mdf)

The menu definition file (MDF, Menu Definition File) is called `menu.mdf`. One thing per line. Empty lines and lines starting with `#` are ignored.
The first menu in the file is the start menu.

    menu <id> "Title"                 starts a new menu
    item "Text" run <command>         runs a command
    item "Text" menu <id>             opens another menu
    item "Text" info <text>           shows a text
    item "Text" back                  goes back
    item "Text" exit                  quits the program

After the text a second quoted text may follow. It appears in the grey bar as soon as the item is selected:

    item "Disk space" "Shows the free space on all drives" run df -h

Without a help text the bar shows something suitable itself, for example "Command: df -h".

`<id>` is a single word without blanks, for example `tools`.
Quoted texts may contain umlauts. The file must be UTF-8.
Windows line endings (CRLF) are no problem.

### Settings

All optional, best placed at the top of the file:

    set blackout 300      screen goes black after 300 seconds without input (0 = off, default)
    set clock utc         clock in UTC instead of local time (local = default)
    set mouse off         switch the mouse off (on = default)

### Mouse

A left click on an item runs it, the mouse wheel selects up and down. A click on the black screen only wakes it up. While a command runs the mouse is off, so other programs can use it normally.

### Blackout

After the set time without a key press or click the screen goes black. The first input only wakes it up and is otherwise ignored, so no item is started by accident.

### Password lock for single items

**Step by step**

1. Create the password line (asks twice, no echo):

       automenu --hash admin

   `admin` is just a freely chosen name for this password, not a user account. Without a name `admin` is used. The program changes nothing in the system and writes no file, it only prints one line:

       password admin pbkdf2-sha256:20000:3f9a...:b71c...

2. Copy this line into the `.mdf` file, outside the menus (best at the top).
3. On the item you want to protect write `lock admin` before the type:

       item "Reboot" "Asks for the password" lock admin run sudo reboot
       item "Quit"   "Only with password"    lock admin exit

4. Check with `automenu --check file.mdf`, then start.

**What the user sees**

Select the item (Enter, digit or mouse click), the window "Password" appears, the input shows asterisks. Enter confirms, Esc cancels. A wrong password shows a message and waits 1 second. After 3 failed attempts you are back in the menu, after a wait of 3 seconds. After the item has run it is locked again. The password is asked on every selection, there is no timed unlock.

**Rules**

- Only the single item is protected, never a whole menu. To protect a submenu, lock the item that leads there.
- Any item may have `lock`, also `info`, `back` and `exit`.
- Several passwords with different names are possible (`password boss ...`, `password guest ...`, then `lock boss` or `lock guest`).
- A name may be defined only once. `lock` with an unknown name is an error that `--check` reports.

**Changing or forgetting a password**

There is no reset. Create a new line with `automenu --hash admin` and replace the old `password admin` line in the file. If you forgot the password, replace the line in the same way, because the file can be edited anyway.

**How the password is stored**

Never in clear text. The file holds only a hash: PBKDF2 with HMAC-SHA-256, 20000 rounds and a random salt (16 bytes from `/dev/urandom`). Equal passwords therefore give different lines. The format is `pbkdf2-sha256:rounds:salt:hash`. Do not edit the line by hand, otherwise `--check` reports an invalid hash.

**What the lock can and cannot do (please read)**

This is a lock against the curious and against mistakes, not a security feature. It does not protect against:

- Ctrl-C in normal mode. It quits the program without a password. With `automenu --kiosk` Ctrl-C is off.
- Closing the window, a terminal session, SSH, a second shell. Whoever gets into the system this way is in the shell again.
- Reading the file. Whoever can read the `.mdf` sees the commands and can run them directly in the shell. The hash itself cannot be reversed, but a weak password can be guessed.
- Programs you start from the menu. If `top` or `vi` has a shell function, the user is outside.

Kiosk mode locks only Ctrl-C, nothing more. The combination makes sense for simple things: the menu starts at login (`exec automenu --kiosk file.mdf` in `~/.profile`), dangerous items are locked with `lock`, the file is readable only by you (`chmod 600 file.mdf`) and owned by you, not by the user of the menu. Where real security is needed, use the user rights of the system.

## 4. Recipes

### One command

    menu start "My menu"
      item "Show the date"   run date
      item "Quit"            exit

Everything after `run` is executed by the shell. After the command the menu waits for a key so you can read the output.

### Command with a pipe, parameters or several steps

    item "Big files"        run du -sh * | sort -h | tail -5
    item "Two things"       run date; uptime
    item "Only when online" run ping -c 1 1.1.1.1 && echo online

### Submenu

    menu start "Main"
      item "Tools"            menu tools
      item "Quit"             exit

    menu tools "Tools"
      item "Disk space"       run df -h
      item "Back"             back

The order of the menus in the file does not matter, only the first one is the start menu.

### Program with its own interface (editor, htop, ssh)

    item "Notes"            run ${EDITOR:-vi} ~/notes.txt
    item "Processes"        run htop
    item "Server"           run ssh user@server

The menu hands the terminal to the program and takes it back afterwards.

### Info text, also multi-line

    item "Help"   info Line 1\nLine 2\nLine 3

`\n` gives a line break. Without a text the item text itself is shown.

### Starting your own script

    item "Backup"   run ~/bin/backup.sh

Make the script executable first: `chmod +x ~/bin/backup.sh`.

### Starting the menu at login (kiosk)

Kiosk in Automenu means: Ctrl-C does not quit the program. The only way out is an `exit` item, which can also be locked with a password. A normal `automenu` is enough, the mode is switched on at start.

1. At the end of `~/.profile` add:

       exec automenu --kiosk /path/to/menu.mdf

2. The menu must contain an `exit` item if you want to get out again. Without `exit` and without Ctrl-C there is no way out except logging out from outside on the console.

## 5. When something does not work

`./automenu --check menu.mdf` names the line with the error.

| Message | Cause |
|---|---|
| `item before the first menu` | A `menu` line is missing before the first `item` |
| `run needs a command` | Nothing follows `run` |
| `refers to unknown menu` | The id in `menu <id>` does not match |
| `duplicate menu id` | Two menus with the same id |
| `menu without items` | A `menu` without any `item` below it |
| `unknown type` | Typo in `run`, `menu`, `info`, `back`, `exit` |
| `syntax: menu <id> "Title"` | Title without quotation marks |
| `help text without closing quotation mark` | The second `"` of a help text is missing |
| `uses unknown password` | A `lock` has no matching `password` line |
| `invalid password hash` | Hash not created with `automenu --hash`, or cut off while copying |
| `unknown setting` | Typo in `set blackout`, `set clock`, `set mouse` |
| Mouse does not react | `set mouse off` in the file, or the terminal does not report mouse events |

## 6. Good habits

- At most 7 items per menu: then every item has a blank row between them (with more items they move together without blank rows). Up to 9 items the digit keys work for all of them, with more the list scrolls without blank rows.
- Prefer several small submenus over one long menu.
- Always put a `back` item at the end of every submenu.
- First `--check`, then start.

## 7. The technology behind it (only if you want to tinker)

- `core.c` only knows events (up, down, select, back) and reports what should be run. It draws nothing and starts nothing.
- `auth.c` checks passwords (PBKDF2-HMAC-SHA256, plain C). `terminal.c` turns keys and mouse clicks into events, draws the screen and runs commands.
- For the ESP32 only `terminal.c` is replaced by an adapter for the serial console. `loader.c` can also read menus from a text buffer (`mdf_parse_text`), so the menu can live in flash.

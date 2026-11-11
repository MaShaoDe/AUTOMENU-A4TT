/* terminal.c - Terminal-Adapter für Mac, Linux, FreeBSD
 *
 * Liest Tasten, zeichnet das Menü (Rahmen, Farben, Uhr), führt Befehle aus.
 * Bauen mit -DKIOSK: Ctrl-C beendet das Programm nicht mehr.
 * Umgebungsvariablen: NO_COLOR (ohne Farben), AUTOMENU_COLORS=256|16|mono,
 * AUTOMENU_FRAME=solid|lines|ascii (Rahmenart, Standard solid). */
#include <errno.h>
#include <locale.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "core.h"
#include "loader.h"
#include "version.h"

/* Wird vom Makefile gesetzt (PREFIX/etc). */
#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

/* ---------- Terminalmodus ---------- */

static struct termios orig_tio;
static int raw_active = 0;

static void restore_terminal(void)
{
    if (raw_active) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_tio);
        raw_active = 0;
    }
    /* Farben zurücksetzen, Cursor wieder zeigen */
    if (write(STDOUT_FILENO, "\033[0m\033[?25h", 10) < 0) { /* egal */ }
}

static void raw_on(void)
{
    struct termios raw = orig_tio;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_active = 1;
}

static void on_winch(int sig) { (void)sig; }   /* weckt nur poll() auf */

static void on_signal(int sig)
{
    (void)sig;
    if (raw_active) tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_tio);
    _exit(1);
}

/* ---------- Farben ---------- */

typedef struct {
    const char *reset, *frame, *title, *num, *item, *sel,
               *menu, *desc, *date, *host, *help;
} theme_t;

static theme_t th;

/* Rahmenarten (AUTOMENU_FRAME): solid = Farbflächen, geht mit jeder Schrift (Standard),
 * lines = Doppellinien, ascii = + - | */
typedef struct { const char *tl, *tr, *bl, *br, *v, *h, *sl, *sr, *sh; } frame_t;
static const frame_t FRAME_LINES = { "╔", "╗", "╚", "╝", "║", "═", "╟", "╢", "─" };
static const frame_t FRAME_ASCII = { "+", "+", "+", "+", "|", "=", "+", "+", "-" };
static const frame_t FRAME_SOLID = { " ", " ", " ", " ", " ", " ", " ", " ", " " };
static const frame_t *fr = &FRAME_SOLID;
static int frame_solid = 1;
static int color_mode = 2;                 /* 0 = ohne Farben, 1 = 16, 2 = 256 */

static void theme_init(void)
{
    const char *term = getenv("TERM");
    const char *nocolor = getenv("NO_COLOR");

    const char *force = getenv("AUTOMENU_COLORS");   /* 256, 16 oder mono */
    int mono = (nocolor && *nocolor) || (term && strcmp(term, "dumb") == 0) ||
               (force && strcmp(force, "mono") == 0);
    if (mono) {
        color_mode = 0;
        /* ohne Farben: nur invertiert */
        th = (theme_t){ "\033[0m", "\033[0m", "\033[7m", "\033[0m", "\033[0m",
                        "\033[7m", "\033[0m", "\033[7m", "\033[0m", "\033[0m",
                        "\033[0m" };
        return;
    }

    /* Fast alle Terminals können 256 Farben. Nur sehr alte nicht. */
    /* Moderne Terminalprogramme (macOS Terminal, iTerm2 ...) melden sich über
     * TERM_PROGRAM oder COLORTERM, auch wenn TERM noch "vt100" sagt. */
    const char *tp = getenv("TERM_PROGRAM"), *ct = getenv("COLORTERM");
    int modern = (tp && *tp) || (ct && *ct);
    int old = !modern && (!term || strstr(term, "vt") || strcmp(term, "ansi") == 0);
    if (force && strcmp(force, "16") == 0) old = 1;
    if (force && strcmp(force, "256") == 0) old = 0;
    if (old) {
        color_mode = 1;
        th = (theme_t){
            "\033[0m\033[40m", "\033[36;40m", "\033[97;41m", "\033[92;40m",
            "\033[92;40m", "\033[30;47m", "\033[33;40m", "\033[30;47m",
            "\033[31;40m", "\033[33;40m", "\033[92;40m"
        };
        return;
    }
    /* feste Palettenwerte: sieht in jedem Farbschema gleich aus */
    th = (theme_t){
        "\033[0m\033[48;5;16m",               /* reset: schwarzer Hintergrund */
        "\033[38;5;37;48;5;16m",               /* Rahmen: türkis */
        "\033[38;5;231;48;5;124m",             /* Titelbalken: weiss auf rot */
        "\033[38;5;120;48;5;16m",              /* Ziffern: hellgrün */
        "\033[38;5;120;48;5;16m",              /* Einträge: hellgrün */
        "\033[38;5;16;48;5;250m",              /* Auswahl: schwarz auf grau */
        "\033[38;5;166;48;5;16m",              /* "MENÜ 1 VON 2": orange */
        "\033[38;5;16;48;5;250m",              /* Beschreibungsleiste */
        "\033[38;5;160;48;5;16m",              /* Datum: rot */
        "\033[38;5;166;48;5;16m",              /* Benutzer@Rechner: orange */
        "\033[38;5;120;48;5;16m"               /* Hinweis H für Hilfe */
    };
}

/* ---------- Tasten ---------- */

typedef enum {
    K_NONE, K_TICK, K_UP, K_DOWN, K_ENTER, K_BACK, K_QUIT, K_DIGIT, K_HELP, K_EOF
} key_t_;

static key_t_ read_key(int *digit)
{
    /* höchstens 1 Sekunde warten, damit die Uhr weiterläuft */
    struct pollfd w = { STDIN_FILENO, POLLIN, 0 };
    int pr = poll(&w, 1, 1000);
    if (pr == 0) return K_TICK;
    if (pr < 0) return (errno == EINTR) ? K_NONE : K_EOF;

    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == 0) return K_EOF;
    if (n < 0) return (errno == EINTR) ? K_NONE : K_EOF;

    if (c == 27) {                           /* Esc oder Pfeiltaste */
        struct pollfd p = { STDIN_FILENO, POLLIN, 0 };
        if (poll(&p, 1, 50) <= 0) return K_BACK;
        unsigned char intro;
        if (read(STDIN_FILENO, &intro, 1) != 1) return K_NONE;
        if (intro != '[' && intro != 'O') return K_NONE;
        unsigned char fin;
        do {
            if (read(STDIN_FILENO, &fin, 1) != 1) return K_NONE;
        } while (fin < 0x40);                /* Parameterbytes überspringen */
        if (fin == 'A') return K_UP;
        if (fin == 'B') return K_DOWN;
        return K_NONE;
    }
    if (c == '\r' || c == '\n') return K_ENTER;
    if (c == 127 || c == 8) return K_BACK;
    if (c >= '1' && c <= '9') { *digit = c - '1'; return K_DIGIT; }
    if (c == 'h' || c == 'H' || c == '?') return K_HELP;
#ifndef KIOSK
    if (c == 3) return K_QUIT;               /* Ctrl-C */
#endif
    return K_NONE;
}

static void wait_any_key(void)
{
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) < 0) { /* egal */ }
}

/* ---------- Zeichenhilfen ---------- */

static int cols = 80, rows = 24;

static void get_size(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        cols = ws.ws_col;
        rows = ws.ws_row;
    }
}

static int u8len(const char *s)            /* Zeichen, nicht Bytes */
{
    int n = 0;
    for (; *s; s++) if (((unsigned char)*s & 0xC0) != 0x80) n++;
    return n;
}

/* gibt höchstens maxc Zeichen aus, Rückgabe: Anzahl */
static int put_cut(const char *s, int maxc)
{
    int c = 0;
    for (; *s; s++) {
        if (((unsigned char)*s & 0xC0) != 0x80) {
            if (c == maxc) break;
            c++;
        }
        putchar(*s);
    }
    return c;
}

static void spaces(int n) { while (n-- > 0) putchar(' '); }
static void rep(const char *s, int n) { while (n-- > 0) fputs(s, stdout); }
static void at(int row, int col) { printf("\033[%d;%dH", row, col); }

/* Balken mit zentriertem Text */
static void bar(int row, int col, int w, const char *style, const char *text)
{
    int len = u8len(text);
    if (len > w) len = w;
    int left = (w - len) / 2;
    at(row, col);
    fputs(style, stdout);
    spaces(left);
    put_cut(text, w);
    spaces(w - left - len);
    fputs(th.reset, stdout);
}

/* ---------- Bildschirmaufbau ----------
 *
 *  Zeile 0        Rahmen oben
 *  Zeile 1        Titelbalken
 *  Zeile 2        Trennlinie
 *  Zeile 3        "MENÜ n VON m"
 *  Zeile 4 ...    Einträge
 *  Zeile h-4      Beschreibungsleiste
 *  Zeile h-3      Trennlinie
 *  Zeile h-2      Datum und Uhrzeit, Benutzer@Rechner
 *  Zeile h-1      Rahmen unten
 *  Zeile h        "Drücken Sie H für Hilfe"
 */

static struct { int x, y, w, h; } g;       /* Lage des Rahmens (ab 1) */
static char hostinfo[128];

#define MIN_COLS 60
#define MIN_ROWS 16

static void geometry(void)
{
    g.w = cols;                  /* der Rahmen füllt das ganze Fenster */
    g.x = 1;
    g.h = rows - 1;              /* 1 Zeile bleibt für den Hilfehinweis */
    g.y = 1;
}

static void draw_footer(void)
{
    char ts[64];
    time_t t = time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);
    strftime(ts, sizeof ts, "%d. %B %Y  %H:%M:%S", &tmv);

    int inner = g.w - 4;
    int tl = u8len(ts);
    int hl = u8len(hostinfo);
    if (tl + hl + 2 > inner) hl = inner - tl - 2;
    if (hl < 0) hl = 0;

    at(g.y + g.h - 2, g.x + 2);
    fputs(th.date, stdout);
    put_cut(ts, inner);
    fputs(th.reset, stdout);
    spaces(inner - tl - hl);
    fputs(th.host, stdout);
    put_cut(hostinfo, hl);
    fputs(th.reset, stdout);
    at(rows, 1);
    fflush(stdout);
}

static const char *item_desc(const menu_item_t *it, char *buf, size_t n)
{
    if (it->desc) return it->desc;
    switch (it->type) {
    case MENU_ITEM_RUN:  snprintf(buf, n, "Befehl: %s", it->arg); return buf;
    case MENU_ITEM_MENU: return "Untermenü öffnen";
    case MENU_ITEM_INFO: return "Text anzeigen";
    case MENU_ITEM_BACK: return "Zurück zum vorigen Menü";
    case MENU_ITEM_EXIT: return "Programm beenden";
    }
    return "";
}

static void render(core_t *core)
{
    menu_view_t v = core_view(core);
    get_size();
    printf("%s\033[2J\033[H\033[?25l", th.reset);

    if (cols < MIN_COLS || rows < MIN_ROWS) {
        printf("Terminal zu klein (mindestens %d x %d)", MIN_COLS, MIN_ROWS);
        fflush(stdout);
        return;
    }
    geometry();
    int x = g.x, y = g.y, w = g.w, h = g.h;

    /* Rahmen */
    const char *fs = th.frame, *ss = th.frame;         /* Stil: Rahmen, Trennlinien */
    if (frame_solid) {                                  /* Farbflächen statt Zeichen */
        fs = (color_mode == 2) ? "\033[48;5;37m" : (color_mode == 1) ? "\033[46m" : "\033[7m";
    }
    fputs(fs, stdout);
    at(y, x);         fputs(fr->tl, stdout); rep(fr->h, w - 2); fputs(fr->tr, stdout);
    for (int r = 1; r < h - 1; r++) {
        at(y + r, x);         fputs(fr->v, stdout);
        at(y + r, x + w - 1); fputs(fr->v, stdout);
    }
    if (!frame_solid) {                                 /* Farbrahmen braucht keine Trennlinien */
        fputs(ss, stdout);
        at(y + 2, x);     fputs(fr->sl, stdout); rep(fr->sh, w - 2); fputs(fr->sr, stdout);
        at(y + h - 3, x); fputs(fr->sl, stdout); rep(fr->sh, w - 2); fputs(fr->sr, stdout);
    }
    fputs(fs, stdout);
    at(y + h - 1, x); fputs(fr->bl, stdout); rep(fr->h, w - 2); fputs(fr->br, stdout);
    fputs(th.reset, stdout);

    /* Titelbalken und "MENÜ n VON m" */
    bar(y + 1, x + 2, w - 4, th.title, v.title);
    char mb[48];
    snprintf(mb, sizeof mb, "MENÜ %zu VON %zu", v.menu_index + 1, v.menu_count);
    at(y + 3, x + w - 2 - u8len(mb));
    printf("%s%s%s", th.menu, mb, th.reset);

    /* Einträge: mit 1 Leerzeile dazwischen, wenn es Platz hat; Block ungefähr mittig */
    int items_h = h - 9;
    int n = (int)v.item_count;
    int spacing = (n > 1) ? (items_h - 1) / (n - 1) : 1;
    if (spacing < 1) spacing = 1;
    if (spacing > 2) spacing = 2;
    int visible = (items_h - 1) / spacing + 1;
    int shown = n < visible ? n : visible;
    int top = (items_h - ((shown - 1) * spacing + 1)) / 2;
    if (top < 0) top = 0;
    size_t start = (v.current_index >= (size_t)visible)
                   ? v.current_index - (size_t)visible + 1 : 0;
    int maxlen = 0;                      /* breitester Eintrag bestimmt die Blockbreite */
    for (int k = 0; k < shown && start + (size_t)k < v.item_count; k++) {
        int l = u8len(v.items[start + (size_t)k].label);
        if (l > maxlen) maxlen = l;
    }
    if (maxlen > w - 14) maxlen = w - 14;
    int blockw = 4 + 1 + maxlen + 1;     /* "N - " + Eintrag mit je 1 Leerzeichen Rand */
    int col = x + 1 + (w - 2 - blockw) / 2;
    if (col < x + 2) col = x + 2;
    int avail = (x + w - 2) - (col + 4) - 2;

    for (int k = 0; k < shown && start + (size_t)k < v.item_count; k++) {
        size_t i = start + (size_t)k;
        at(y + 4 + top + k * spacing, col);
        if (i < 9) printf("%s%zu - ", th.num, i + 1);
        else       printf("%s    ", th.num);
        if (i == v.current_index) {
            printf("%s ", th.sel);
            put_cut(v.items[i].label, avail);
            printf(" %s", th.reset);
        } else {
            printf("%s", th.item);
            put_cut(v.items[i].label, avail);
            printf("%s", th.reset);
        }
    }

    /* Beschreibungsleiste */
    char buf[300];
    bar(y + h - 4, x + 3, w - 6, th.desc,
        item_desc(&v.items[v.current_index], buf, sizeof buf));

    draw_footer();

    /* Hinweis unter dem Rahmen */
    const char *help = "Drücken Sie H für Hilfe";
    at(y + h, x + (w - u8len(help)) / 2);
    printf("%s%s%s", th.help, help, th.reset);
    char vb[32];
    snprintf(vb, sizeof vb, "automenu %s", AUTOMENU_VERSION);
    at(y + h, x + w - 1 - u8len(vb));
    printf("%s%s%s", th.help, vb, th.reset);
    at(rows, 1);
    fflush(stdout);
}

/* ---------- Aktionen ---------- */

static void run_command(const char *cmd)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_tio);   /* normaler Modus für das Programm */
    raw_active = 0;
    printf("\033[0m\033[2J\033[H\033[?25h");
    fflush(stdout);

    int rc = system(cmd);

    if (rc != 0) {
        if (rc != -1 && WIFEXITED(rc))
            printf("\n[Befehl endete mit Fehlercode %d]\n", WEXITSTATUS(rc));
        else
            printf("\n[Befehl konnte nicht ausgeführt werden]\n");
    }
    printf("\nTaste drücken ...");
    fflush(stdout);
    raw_on();
    wait_any_key();
}

static void show_info(const char *text)
{
    printf("\033[0m\033[2J\033[H%s\n\nTaste drücken ...", text);
    fflush(stdout);
    wait_any_key();
}

static const char *HELP_TEXT =
    "Tasten\n\n"
    "  Pfeil hoch / runter   Eintrag wählen\n"
    "  Enter                 Eintrag ausführen\n"
    "  1 bis 9               Eintrag direkt ausführen\n"
    "  Esc oder Backspace    ein Menü zurück\n"
#ifndef KIOSK
    "  Ctrl-C                Programm beenden\n"
#endif
    "  H                     diese Hilfe";

/* ---------- main ---------- */

static void usage(const char *prog)
{
    fprintf(stderr,
        "automenu %s\n"
        "Aufruf:  %s [datei]            Menü starten\n"
        "         %s --check [datei]    Menüdatei nur prüfen\n"
        "         %s --version\n"
        "Ohne Datei wird gesucht in:\n"
        "  $XDG_CONFIG_HOME/automenu/menu.txt\n"
        "  ~/.config/automenu/menu.txt\n"
        "  %s/automenu/menu.txt\n",
        AUTOMENU_VERSION, prog, prog, prog, SYSCONFDIR);
}

/* Standard-Menüdatei suchen. Rückgabe: 1 = gefunden. */
static int find_default_menu(char *buf, size_t n)
{
    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");

    if (xdg && *xdg) {
        snprintf(buf, n, "%s/automenu/menu.txt", xdg);
        if (access(buf, R_OK) == 0) return 1;
    }
    if (home && *home) {
        snprintf(buf, n, "%s/.config/automenu/menu.txt", home);
        if (access(buf, R_OK) == 0) return 1;
    }
    snprintf(buf, n, "%s/automenu/menu.txt", SYSCONFDIR);
    return access(buf, R_OK) == 0;
}

int main(int argc, char **argv)
{
    int check_only = 0;
    const char *path = NULL;
    char found[1024];

    setlocale(LC_ALL, "");                   /* Monatsnamen in der eigenen Sprache */

    int i = 1;
    if (i < argc && strcmp(argv[i], "--version") == 0) {
        printf("automenu %s\n", AUTOMENU_VERSION);
        return 0;
    }
    if (i < argc && strcmp(argv[i], "--check") == 0) { check_only = 1; i++; }
    if (i < argc) path = argv[i++];
    if (i < argc || (path && path[0] == '-')) { usage(argv[0]); return 2; }

    if (!path) {
        if (!find_default_menu(found, sizeof found)) {
            fprintf(stderr, "Keine Menüdatei gefunden.\n\n");
            usage(argv[0]);
            return 1;
        }
        path = found;
    }

    menu_t *menus = NULL;
    size_t count = 0;
    if (!menu_load_file(path, &menus, &count)) return 1;

    if (check_only) {
        printf("%s: ok (%zu Menüs)\n", path, count);
        menu_free(menus, count);
        return 0;
    }

    core_t *core = core_create();
    if (!core || !core_load(core, menus, count)) {
        fprintf(stderr, "Startmenü ist leer oder ungültig\n");
        menu_free(menus, count);
        return 1;
    }

    if (tcgetattr(STDIN_FILENO, &orig_tio) != 0) {
        fprintf(stderr, "automenu braucht ein Terminal (stdin ist keines)\n");
        menu_free(menus, count);
        core_destroy(core);
        return 1;
    }

    {   /* "benutzer@rechner" für die Fusszeile */
        const char *user = getenv("USER");
        char host[64] = "";
        gethostname(host, sizeof host - 1);
        snprintf(hostinfo, sizeof hostinfo, "%s@%s", user ? user : "user", host);
    }

    theme_init();
    {
        const char *f = getenv("AUTOMENU_FRAME");
        if (f && strcmp(f, "ascii") == 0) { fr = &FRAME_ASCII; frame_solid = 0; }
        if (f && strcmp(f, "lines") == 0) { fr = &FRAME_LINES; frame_solid = 0; }
    }
    atexit(restore_terminal);
    signal(SIGTERM, on_signal);
    signal(SIGHUP, on_signal);
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof sa);
        sa.sa_handler = on_winch;            /* ohne SA_RESTART: poll() kehrt zurück */
        sigemptyset(&sa.sa_mask);
        sigaction(SIGWINCH, &sa, NULL);
    }
    raw_on();

    int need_render = 1;
    for (;;) {
        if (need_render) { render(core); need_render = 0; }

        int digit = 0;
        key_t_ k = read_key(&digit);
        if (k == K_EOF || k == K_QUIT) break;

        if (k == K_TICK) {                   /* Uhr nachführen, Fenstergrösse prüfen */
            int oc = cols, orows = rows;
            get_size();
            if (oc != cols || orows != rows) need_render = 1;
            else if (cols >= MIN_COLS && rows >= MIN_ROWS) draw_footer();
            continue;
        }
        need_render = 1;

        switch (k) {
        case K_UP:    core_input(core, CORE_EVENT_UP); break;
        case K_DOWN:  core_input(core, CORE_EVENT_DOWN); break;
        case K_ENTER: core_input(core, CORE_EVENT_SELECT); break;
        case K_BACK:  core_input(core, CORE_EVENT_BACK); break;
        case K_DIGIT: core_select_index(core, (size_t)digit); break;
        case K_HELP:  show_info(HELP_TEXT); break;
        default: break;
        }

        const char *cmd = core_poll_run(core);
        if (cmd) run_command(cmd);
        const char *info = core_poll_info(core);
        if (info) show_info(info);
        if (core_should_exit(core)) break;
    }

    printf("\033[0m\033[2J\033[H");
    restore_terminal();
    core_destroy(core);
    menu_free(menus, count);
    return 0;
}

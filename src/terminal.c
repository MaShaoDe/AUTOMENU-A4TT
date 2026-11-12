/*
 * Copyright (c) 2026, FerienBSD aka Marcel Sauder
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* terminal.c - terminal adapter for macOS, Linux, FreeBSD
 *
 * Reads keys and mouse, draws the menu (frame, colours, clock), runs commands.
 * Kiosk: start with --kiosk: Ctrl-C does not quit.
 * Environment: NO_COLOR (no colours), AUTOMENU_COLORS=256|16|mono,
 * AUTOMENU_FRAME=solid|lines|ascii (frame style, default solid). */
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

#include <stdint.h>

#include "auth.h"
#include "core.h"
#include "loader.h"
#include "version.h"

/* Set by the Makefile (PREFIX/etc). */
#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

static mdf_t cfg;                          /* loaded menu definition (.mdf) */
static int kiosk = 0;                       /* 1 = Ctrl-C does not quit (--kiosk) */

/* ---------- Terminal mode ---------- */

static struct termios orig_tio;
static int raw_active = 0;

static void restore_terminal(void)
{
    if (raw_active) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_tio);
        raw_active = 0;
    }
    /* mouse off, reset colours, show cursor again */
    static const char reset[] = "\033[?1000l\033[?1006l\033[0m\033[?25h";
    if (write(STDOUT_FILENO, reset, sizeof reset - 1) < 0) { /* ignore */ }
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

/* Mouse: xterm mode 1000 (clicks, wheel) with SGR coordinates (1006) */
static void mouse_set(int on)
{
    const char *s = (on && cfg.mouse) ? "\033[?1000h\033[?1006h" : "\033[?1000l\033[?1006l";
    if (write(STDOUT_FILENO, s, strlen(s)) < 0) { /* ignore */ }
}

static void on_winch(int sig) { (void)sig; }   /* only wakes up poll() */

static void on_signal(int sig)
{
    (void)sig;
    if (raw_active) tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_tio);
    _exit(1);
}

/* ---------- Colours ---------- */

typedef struct {
    const char *reset, *frame, *title, *num, *item, *sel,
               *menu, *desc, *date, *host, *help;
} theme_t;

static theme_t th;

/* Frame styles (AUTOMENU_FRAME): solid = coloured areas, works with any font (default),
 * lines = double lines, ascii = + - | */
typedef struct { const char *tl, *tr, *bl, *br, *v, *h, *sl, *sr, *sh; } frame_t;
static const frame_t FRAME_LINES = { "╔", "╗", "╚", "╝", "║", "═", "╟", "╢", "─" };
static const frame_t FRAME_ASCII = { "+", "+", "+", "+", "|", "=", "+", "+", "-" };
static const frame_t FRAME_SOLID = { " ", " ", " ", " ", " ", " ", " ", " ", " " };
static const frame_t *fr = &FRAME_SOLID;
static int frame_solid = 1;
static int color_mode = 2;                 /* 0 = no colours, 1 = 16, 2 = 256 */

static void theme_init(void)
{
    const char *term = getenv("TERM");
    const char *nocolor = getenv("NO_COLOR");

    const char *force = getenv("AUTOMENU_COLORS");   /* 256, 16 or mono */
    int mono = (nocolor && *nocolor) || (term && strcmp(term, "dumb") == 0) ||
               (force && strcmp(force, "mono") == 0);
    if (mono) {
        color_mode = 0;
        /* no colours: inverse video only */
        th = (theme_t){ "\033[0m", "\033[0m", "\033[7m", "\033[0m", "\033[0m",
                        "\033[7m", "\033[0m", "\033[7m", "\033[0m", "\033[0m",
                        "\033[0m" };
        return;
    }

    /* Almost all terminals can do 256 colours. Only very old ones cannot. */
    /* Modern terminal programs (macOS Terminal, iTerm2 ...) announce themselves
     * via TERM_PROGRAM or COLORTERM, even when TERM still says "vt100". */
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
    /* fixed palette values: looks the same in every colour scheme */
    th = (theme_t){
        "\033[0m\033[48;5;16m",               /* reset: black background */
        "\033[38;5;37;48;5;16m",               /* frame: teal */
        "\033[38;5;231;48;5;124m",             /* title bar: white on red */
        "\033[38;5;120;48;5;16m",              /* numbers: light green */
        "\033[38;5;120;48;5;16m",              /* items: light green */
        "\033[38;5;16;48;5;250m",              /* selection: black on grey */
        "\033[38;5;166;48;5;16m",              /* "MENU 1 OF 2": orange */
        "\033[38;5;16;48;5;250m",              /* description bar */
        "\033[38;5;160;48;5;16m",              /* date: red */
        "\033[38;5;166;48;5;16m",              /* user@host: orange */
        "\033[38;5;120;48;5;16m"               /* hint: H for help */
    };
}

/* ---------- Keys ---------- */

typedef enum {
    K_NONE, K_TICK, K_UP, K_DOWN, K_ENTER, K_BACK, K_QUIT, K_DIGIT, K_HELP, K_CLICK, K_EOF
} key_t_;

static int mx, my;                         /* position of the last mouse click (from 1) */

static key_t_ mouse_event(int b, int x, int y, int release)
{
    if (b & 64) return (b & 1) ? K_DOWN : K_UP;      /* mouse wheel */
    if (release || (b & 32) || (b & 3) != 0) return K_NONE;   /* left button only */
    mx = x; my = y;
    return K_CLICK;
}

static key_t_ read_key(int *digit)
{
    /* wait at most 1 second so the clock keeps running */
    struct pollfd w = { STDIN_FILENO, POLLIN, 0 };
    int pr = poll(&w, 1, 1000);
    if (pr == 0) return K_TICK;
    if (pr < 0) return (errno == EINTR) ? K_NONE : K_EOF;

    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == 0) return K_EOF;
    if (n < 0) return (errno == EINTR) ? K_NONE : K_EOF;

    if (c == 27) {                           /* Esc or arrow key */
        struct pollfd p = { STDIN_FILENO, POLLIN, 0 };
        if (poll(&p, 1, 50) <= 0) return K_BACK;
        unsigned char intro;
        if (read(STDIN_FILENO, &intro, 1) != 1) return K_NONE;
        if (intro != '[' && intro != 'O') return K_NONE;
        unsigned char fin;
        if (read(STDIN_FILENO, &fin, 1) != 1) return K_NONE;
        if (intro == '[' && fin == 'M') {    /* mouse, legacy form: 3 bytes */
            unsigned char m[3];
            for (int i = 0; i < 3; i++)
                if (read(STDIN_FILENO, &m[i], 1) != 1) return K_NONE;
            int b = m[0] - 32;
            return mouse_event(b, m[1] - 32, m[2] - 32, (b & 3) == 3);
        }
        char pb[32];
        int pn = 0;
        while (fin < 0x40) {                 /* collect parameter bytes */
            if (pn < (int)sizeof pb - 1) pb[pn++] = (char)fin;
            if (read(STDIN_FILENO, &fin, 1) != 1) return K_NONE;
        }
        pb[pn] = '\0';
        if (intro == '[' && pb[0] == '<' && (fin == 'M' || fin == 'm')) {
            int b = 0, x = 0, y = 0;         /* mouse, SGR form: b;x;y */
            if (sscanf(pb + 1, "%d;%d;%d", &b, &x, &y) != 3) return K_NONE;
            return mouse_event(b, x, y, fin == 'm');
        }
        if (fin == 'A') return K_UP;
        if (fin == 'B') return K_DOWN;
        return K_NONE;
    }
    if (c == '\r' || c == '\n') return K_ENTER;
    if (c == 127 || c == 8) return K_BACK;
    if (c >= '1' && c <= '9') { *digit = c - '1'; return K_DIGIT; }
    if (c == 'h' || c == 'H' || c == '?') return K_HELP;
    if (c == 3 && !kiosk) return K_QUIT;     /* Ctrl-C */
    return K_NONE;
}

static void wait_any_key(void)
{
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) < 0) { /* ignore */ }
}

/* ---------- Drawing helpers ---------- */

static int cols = 80, rows = 24;

static void get_size(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        cols = ws.ws_col;
        rows = ws.ws_row;
    }
}

static int u8len(const char *s)            /* characters, not bytes */
{
    int n = 0;
    for (; *s; s++) if (((unsigned char)*s & 0xC0) != 0x80) n++;
    return n;
}

/* prints at most maxc characters, returns the count */
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

/* bar with centred text */
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

/* ---------- Screen layout ----------
 *
 *  row 0          top frame
 *  row 1          title bar
 *  row 2          separator
 *  row 3          "MENU n OF m"
 *  row 4 ...      items
 *  row h-4        description bar
 *  row h-3        separator
 *  row h-2        date and time, user@host
 *  row h-1        bottom frame
 *  row h          "Press H for help"
 */

static struct { int x, y, w, h; } g;       /* position of the frame (from 1) */
static char hostinfo[128];
static struct { int top, spacing, shown, col, w; size_t start; } lay;   /* for mouse clicks */

#define MIN_COLS 60
#define MIN_ROWS 16

static void geometry(void)
{
    g.w = cols;                  /* the frame fills the whole window */
    g.x = 1;
    g.h = rows - 1;              /* 1 row is left for the help hint */
    g.y = 1;
}

static void draw_footer(void)
{
    char ts[64];
    time_t t = time(NULL);
    struct tm tmv;
    if (cfg.clock_utc) gmtime_r(&t, &tmv); else localtime_r(&t, &tmv);
    strftime(ts, sizeof ts, cfg.clock_utc ? "%d. %B %Y  %H:%M:%S UTC" : "%d. %B %Y  %H:%M:%S", &tmv);

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
    case MENU_ITEM_RUN:  snprintf(buf, n, "Command: %s", it->arg); return buf;
    case MENU_ITEM_MENU: return "Open submenu";
    case MENU_ITEM_INFO: return "Show text";
    case MENU_ITEM_BACK: return "Back to the previous menu";
    case MENU_ITEM_EXIT: return "Quit the program";
    }
    return "";
}

static void render(core_t *core)
{
    menu_view_t v = core_view(core);
    lay.shown = 0;
    get_size();
    printf("%s\033[2J\033[H\033[?25l", th.reset);

    if (cols < MIN_COLS || rows < MIN_ROWS) {
        printf("Terminal too small (minimum %d x %d)", MIN_COLS, MIN_ROWS);
        fflush(stdout);
        return;
    }
    geometry();
    int x = g.x, y = g.y, w = g.w, h = g.h;

    /* frame */
    const char *fs = th.frame, *ss = th.frame;         /* style: frame, separators */
    if (frame_solid) {                                  /* coloured areas instead of characters */
        fs = (color_mode == 2) ? "\033[48;5;37m" : (color_mode == 1) ? "\033[46m" : "\033[7m";
    }
    fputs(fs, stdout);
    at(y, x);         fputs(fr->tl, stdout); rep(fr->h, w - 2); fputs(fr->tr, stdout);
    for (int r = 1; r < h - 1; r++) {
        at(y + r, x);         fputs(fr->v, stdout);
        at(y + r, x + w - 1); fputs(fr->v, stdout);
    }
    if (!frame_solid) {                                 /* coloured frame needs no separators */
        fputs(ss, stdout);
        at(y + 2, x);     fputs(fr->sl, stdout); rep(fr->sh, w - 2); fputs(fr->sr, stdout);
        at(y + h - 3, x); fputs(fr->sl, stdout); rep(fr->sh, w - 2); fputs(fr->sr, stdout);
    }
    fputs(fs, stdout);
    at(y + h - 1, x); fputs(fr->bl, stdout); rep(fr->h, w - 2); fputs(fr->br, stdout);
    fputs(th.reset, stdout);

    /* title bar and "MENU n OF m" */
    bar(y + 1, x + 2, w - 4, th.title, v.title);
    char mb[48];
    snprintf(mb, sizeof mb, "MENU %zu OF %zu", v.menu_index + 1, v.menu_count);
    at(y + 3, x + w - 2 - u8len(mb));
    printf("%s%s%s", th.menu, mb, th.reset);

    /* items: one blank row between them if there is room; block roughly centred */
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
    int maxlen = 0;                      /* widest item sets the block width */
    for (int k = 0; k < shown && start + (size_t)k < v.item_count; k++) {
        int l = u8len(v.items[start + (size_t)k].label);
        if (l > maxlen) maxlen = l;
    }
    if (maxlen > w - 14) maxlen = w - 14;
    int blockw = 4 + 1 + maxlen + 1;     /* "N - " + item with 1 space margin each side */
    int col = x + 1 + (w - 2 - blockw) / 2;
    if (col < x + 2) col = x + 2;
    int avail = (x + w - 2) - (col + 4) - 2;
    lay.top = y + 4 + top; lay.spacing = spacing; lay.shown = shown;
    lay.col = col; lay.w = blockw; lay.start = start;

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

    /* description bar */
    char buf[300];
    bar(y + h - 4, x + 3, w - 6, th.desc,
        item_desc(&v.items[v.current_index], buf, sizeof buf));

    draw_footer();

    /* hint below the frame */
    const char *help = "Press H for help";
    at(y + h, x + (w - u8len(help)) / 2);
    printf("%s%s%s", th.help, help, th.reset);
    char vb[32];
    snprintf(vb, sizeof vb, "automenu %s", AUTOMENU_VERSION);
    at(y + h, x + w - 1 - u8len(vb));
    printf("%s%s%s", th.help, vb, th.reset);
    at(rows, 1);
    fflush(stdout);
}

/* ---------- Actions ---------- */

static void run_command(const char *cmd)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_tio);   /* normal mode for the program */
    raw_active = 0;
    printf("\033[0m\033[2J\033[H\033[?25h");
    fflush(stdout);

    mouse_set(0);
    int rc = system(cmd);

    if (rc != 0) {
        if (rc != -1 && WIFEXITED(rc))
            printf("\n[Command exited with code %d]\n", WEXITSTATUS(rc));
        else
            printf("\n[Command could not be run]\n");
    }
    printf("\nPress any key ...");
    fflush(stdout);
    raw_on();
    wait_any_key();
    tcflush(STDIN_FILENO, TCIFLUSH);
    mouse_set(1);
}

static void show_info(const char *text)
{
    mouse_set(0);
    printf("\033[0m\033[2J\033[H%s\n\nPress any key ...", text);
    fflush(stdout);
    wait_any_key();
    tcflush(STDIN_FILENO, TCIFLUSH);
    mouse_set(1);
}

/* ---------- Password window ---------- */

static void dline(int row, int left, int w, const char *style, const char *text)
{
    int len = u8len(text);
    int room = w - 4;
    if (len > room) len = room;
    at(row, left);
    fputs(style, stdout);
    spaces(2);
    put_cut(text, room);
    spaces(w - 2 - len);
    fputs(th.reset, stdout);
}

static void draw_dialog(const char *label, int stars, const char *msg)
{
    get_size();
    int w = cols < 48 ? cols - 2 : 44, h = 7;
    int left = (cols - w) / 2 + 1, top = (rows - h) / 2 + 1;
    if (w < 20 || top < 1) return;
    char buf[128];
    int i;

    bar(top, left, w, th.title, "Password");
    dline(top + 1, left, w, th.sel, "");
    snprintf(buf, sizeof buf, "Item: %s", label);
    dline(top + 2, left, w, th.sel, buf);
    snprintf(buf, sizeof buf, "Password: ");
    for (i = 0; i < stars && i < 40; i++) strcat(buf, "*");
    dline(top + 3, left, w, th.sel, buf);
    dline(top + 4, left, w, th.sel, "");
    if (msg) bar(top + 5, left, w, th.title, msg);
    else     dline(top + 5, left, w, th.sel, "");
    dline(top + 6, left, w, th.sel, "Enter = OK     Esc = Cancel");
    at(rows, 1);
    fflush(stdout);
}

/* 1 = password correct, 0 = cancelled or 3 failed attempts */
static int ask_password(const char *label, const char *hash)
{
    char pw[128];
    size_t len = 0;
    int tries = 0, ok = 0, done = 0;
    const char *msg = NULL;

    mouse_set(0);
    tcflush(STDIN_FILENO, TCIFLUSH);
    pw[0] = '\0';
    while (!done) {
        int stars = 0;
        for (size_t i = 0; i < len; i++) if (((unsigned char)pw[i] & 0xC0) != 0x80) stars++;
        draw_dialog(label, stars, msg);
        msg = NULL;

        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n < 0 && errno == EINTR) continue;        /* window changed: redraw */
        if (n <= 0) break;
        if (c == 27) {
            struct pollfd p = { STDIN_FILENO, POLLIN, 0 };
            if (poll(&p, 1, 50) <= 0) break;          /* lone Esc = cancel */
            while (poll(&p, 1, 20) > 0 && read(STDIN_FILENO, &c, 1) == 1) { }
            continue;                                 /* ignore arrow keys etc. */
        }
        if (c == 3) break;
        if (c == '\r' || c == '\n') {
            if (auth_verify(pw, hash)) { ok = 1; break; }
            tries++;
            len = 0; pw[0] = '\0';
            if (tries >= 3) {
                draw_dialog(label, 0, "3 failed attempts, please wait ...");
                sleep(3);
                break;
            }
            draw_dialog(label, 0, "Wrong password");
            sleep(1);
            tcflush(STDIN_FILENO, TCIFLUSH);
            msg = "Wrong password, try again";
        } else if (c == 127 || c == 8) {
            while (len > 0 && ((unsigned char)pw[len - 1] & 0xC0) == 0x80) len--;
            if (len > 0) len--;
            pw[len] = '\0';
        } else if (c >= 32 && len < sizeof pw - 1) {
            pw[len++] = (char)c;
            pw[len] = '\0';
        }
    }
    memset(pw, 0, sizeof pw);
    tcflush(STDIN_FILENO, TCIFLUSH);
    mouse_set(1);
    return ok;
}

static const char *find_hash(const char *name)
{
    for (size_t i = 0; i < cfg.password_count; i++)
        if (strcmp(cfg.passwords[i].name, name) == 0) return cfg.passwords[i].hash;
    return NULL;
}

static const char *HELP_TEXT =
    "Keys\n\n"
    "  Up / Down arrow       select an item\n"
    "  Enter                 run the item\n"
    "  Mouse                 click = run, wheel = select\n"
    "  1 to 9                run an item directly\n"
    "  Esc or Backspace      back one menu\n"
    "  H                     this help";

static const char *HELP_CTRLC = "\n  Ctrl-C                quit the program";

/* ---------- main ---------- */

static void usage(const char *prog)
{
    fprintf(stderr,
        "automenu %s\n"
        "Usage:   %s [file.mdf]         start the menu\n"
        "         %s --kiosk [file]     start the menu, Ctrl-C does not quit\n"
        "         %s --check [file]     check the menu file only\n"
        "         %s --hash [name]      create a password line for the menu file\n"
        "         %s --version\n"
        "Without a file, menu.mdf (else menu.txt) is searched in:\n"
        "  $XDG_CONFIG_HOME/automenu/\n"
        "  ~/.config/automenu/\n"
        "  %s/automenu/\n",
        AUTOMENU_VERSION, prog, prog, prog, prog, prog, SYSCONFDIR);
}

/* Find the default menu file (menu.mdf, else menu.txt). Returns 1 if found. */
static int find_default_menu(char *buf, size_t n)
{
    static const char *names[] = { "menu.mdf", "menu.txt" };
    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");
    char dir[900];

    for (int d = 0; d < 3; d++) {
        dir[0] = '\0';
        if (d == 0 && xdg && *xdg) snprintf(dir, sizeof dir, "%s/automenu", xdg);
        if (d == 1 && home && *home) snprintf(dir, sizeof dir, "%s/.config/automenu", home);
        if (d == 2) snprintf(dir, sizeof dir, "%s/automenu", SYSCONFDIR);
        if (!dir[0]) continue;
        for (int i = 0; i < 2; i++) {
            snprintf(buf, n, "%s/%s", dir, names[i]);
            if (access(buf, R_OK) == 0) return 1;
        }
    }
    return 0;
}

/* Read input without echo (if stdin is a terminal) */
static int read_secret(const char *prompt, char *out, size_t n)
{
    struct termios t, o;
    int tty = tcgetattr(STDIN_FILENO, &o) == 0;
    fputs(prompt, stderr);
    fflush(stderr);
    if (tty) { t = o; t.c_lflag &= ~(tcflag_t)ECHO; tcsetattr(STDIN_FILENO, TCSAFLUSH, &t); }
    char *r = fgets(out, (int)n, stdin);
    if (tty) { tcsetattr(STDIN_FILENO, TCSAFLUSH, &o); fputs("\n", stderr); }
    if (!r) return 0;
    out[strcspn(out, "\r\n")] = '\0';
    return 1;
}

/* automenu --hash [name]: asks twice and prints the password line */
static int make_password_line(const char *name)
{
    char a[128], b[128], hash[AUTH_HASH_MAX];
    uint8_t salt[16];

    if (!read_secret("Password: ", a, sizeof a) ||
        !read_secret("Repeat: ", b, sizeof b)) return 1;
    if (strcmp(a, b) != 0) { fprintf(stderr, "The two entries differ.\n"); return 1; }
    if (!a[0]) { fprintf(stderr, "An empty password is not allowed.\n"); return 1; }
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f || fread(salt, 1, sizeof salt, f) != sizeof salt) {
        fprintf(stderr, "No random source available (/dev/urandom).\n");
        if (f) fclose(f);
        return 1;
    }
    fclose(f);
    if (!auth_make_hash(a, salt, AUTH_ITER_DEFAULT, hash, sizeof hash)) return 1;
    printf("password %s %s\n", name, hash);
    memset(a, 0, sizeof a); memset(b, 0, sizeof b);
    return 0;
}

int main(int argc, char **argv)
{
    int check_only = 0;
    const char *path = NULL;
    char found[1024];

    setlocale(LC_ALL, "");                   /* month names in the system language */

    int i = 1;
    if (i < argc && strcmp(argv[i], "--version") == 0) {
        printf("automenu %s\n", AUTOMENU_VERSION);
        return 0;
    }
    if (i < argc && strcmp(argv[i], "--hash") == 0) {
        const char *nm = (i + 1 < argc) ? argv[i + 1] : "admin";
        if (i + 2 < argc || strchr(nm, ' ') || nm[0] == '-') { usage(argv[0]); return 2; }
        return make_password_line(nm);
    }
    if (i < argc && strcmp(argv[i], "--kiosk") == 0) { kiosk = 1; i++; }
    if (i < argc && strcmp(argv[i], "--check") == 0) { check_only = 1; i++; }
    if (i < argc) path = argv[i++];
    if (i < argc || (path && path[0] == '-')) { usage(argv[0]); return 2; }

    if (!path) {
        if (!find_default_menu(found, sizeof found)) {
            fprintf(stderr, "No menu file found.\n\n");
            usage(argv[0]);
            return 1;
        }
        path = found;
    }

    if (!mdf_load_file(path, &cfg)) return 1;

    if (check_only) {
        printf("%s: ok (%zu menus, %zu passwords)\n", path, cfg.menu_count, cfg.password_count);
        mdf_free(&cfg);
        return 0;
    }

    core_t *core = core_create();
    if (!core || !core_load(core, cfg.menus, cfg.menu_count)) {
        fprintf(stderr, "The start menu is empty or invalid\n");
        mdf_free(&cfg);
        return 1;
    }

    if (tcgetattr(STDIN_FILENO, &orig_tio) != 0) {
        fprintf(stderr, "automenu needs a terminal (stdin is not one)\n");
        mdf_free(&cfg);
        core_destroy(core);
        return 1;
    }

    {   /* "user@host" for the footer */
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
        sa.sa_handler = on_winch;            /* without SA_RESTART: poll() returns */
        sigemptyset(&sa.sa_mask);
        sigaction(SIGWINCH, &sa, NULL);
    }
    raw_on();
    mouse_set(1);

    int need_render = 1, blacked = 0;
    time_t last_act = time(NULL);
    for (;;) {
        if (need_render) { render(core); need_render = 0; }

        int digit = 0;
        key_t_ k = read_key(&digit);
        if (k == K_EOF || k == K_QUIT) break;

        if (k == K_TICK) {                   /* update the clock, check the window size */
            if (blacked) continue;
            if (cfg.blackout_s > 0 && time(NULL) - last_act >= cfg.blackout_s) {
                blacked = 1;                 /* blackout: black screen until the next input */
                printf("\033[0m\033[40m\033[2J\033[H\033[?25l");
                fflush(stdout);
                continue;
            }
            int oc = cols, orows = rows;
            get_size();
            if (oc != cols || orows != rows) need_render = 1;
            else if (cols >= MIN_COLS && rows >= MIN_ROWS) draw_footer();
            continue;
        }
        last_act = time(NULL);
        if (blacked) {                       /* first input only wakes up */
            blacked = 0;
            need_render = 1;
            continue;
        }
        need_render = 1;

        switch (k) {
        case K_UP:    core_input(core, CORE_EVENT_UP); break;
        case K_DOWN:  core_input(core, CORE_EVENT_DOWN); break;
        case K_ENTER: core_input(core, CORE_EVENT_SELECT); break;
        case K_BACK:  core_input(core, CORE_EVENT_BACK); break;
        case K_DIGIT: core_select_index(core, (size_t)digit); break;
        case K_HELP: {
            char hb[600];
            snprintf(hb, sizeof hb, "%s%s", HELP_TEXT, kiosk ? "" : HELP_CTRLC);
            show_info(hb);
            break;
        }
        case K_CLICK:
            for (int j = 0; j < lay.shown; j++) {
                if (my == lay.top + j * lay.spacing && mx >= lay.col && mx < lay.col + lay.w) {
                    core_select_index(core, lay.start + (size_t)j);
                    break;
                }
            }
            break;
        default: break;
        }

        const char *lname = core_locked_name(core);
        if (lname) {                         /* locked item: ask for the password first */
            const char *hash = find_hash(lname);
            render(core);
            if (hash && ask_password(core_locked_label(core), hash)) core_unlock_ok(core);
            else core_unlock_cancel(core);
            last_act = time(NULL);
        }

        const char *cmd = core_poll_run(core);
        if (cmd) run_command(cmd);
        const char *info = core_poll_info(core);
        if (info) show_info(info);
        last_act = time(NULL);
        if (core_should_exit(core)) break;
    }

    printf("\033[0m\033[2J\033[H");
    restore_terminal();
    core_destroy(core);
    mdf_free(&cfg);
    return 0;
}

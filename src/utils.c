#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

#include "utils.h"

/* ------------------------------------------------------------------ */
/* Date / time                                                         */
/* ------------------------------------------------------------------ */

void utils_get_date(char *buf, size_t size)
{
    time_t     now     = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, size, "%Y-%m-%d %H:%M", tm_info);
}

/* ------------------------------------------------------------------ */
/* Terminal helpers                                                    */
/* ------------------------------------------------------------------ */

void utils_clear_screen(void)
{
#ifdef _WIN32
    (void)system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

void utils_print_divider(void)
{
    printf("============================================================\n");
}

void utils_print_header(const char *title)
{
    int total = 60;
    int len   = (int)strlen(title);
    int left  = (total - len - 2) / 2;
    int right = total - len - 2 - left;

    utils_print_divider();
    printf("%*s %s %*s\n", left, "", title, right, "");
    utils_print_divider();
}

void utils_press_enter(void)
{
    printf("\n  Press Enter to continue...");
    fflush(stdout);
    utils_flush_stdin();
}

/* ------------------------------------------------------------------ */
/* Input helpers                                                       */
/* ------------------------------------------------------------------ */

void utils_flush_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int utils_get_string(const char *prompt, char *buf, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);

    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return -1;
    }

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else if (len == size - 1) {
        /* Input was truncated; discard the rest of the line */
        utils_flush_stdin();
    }
    return 0;
}

int utils_get_int(const char *prompt, int *value, int min, int max)
{
    char buf[32];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);

        if (fgets(buf, sizeof(buf), stdin) == NULL)
            return -1;

        char *ep;
        long v = strtol(buf, &ep, 10);

        if (ep == buf || (*ep != '\n' && *ep != '\0')) {
            printf("  Invalid input. Please enter a whole number.\n");
            continue;
        }
        if ((int)v < min || (int)v > max) {
            printf("  Please enter a value between %d and %d.\n", min, max);
            continue;
        }

        *value = (int)v;
        return 0;
    }
}

int utils_get_double(const char *prompt, double *value,
                     double min, double max)
{
    char buf[64];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);

        if (fgets(buf, sizeof(buf), stdin) == NULL)
            return -1;

        char  *ep;
        double v = strtod(buf, &ep);

        if (ep == buf || (*ep != '\n' && *ep != '\0')) {
            printf("  Invalid input. Please enter a number.\n");
            continue;
        }
        if (v < min || v > max) {
            printf("  Please enter a value between %.2f and %.2f.\n",
                   min, max);
            continue;
        }

        *value = v;
        return 0;
    }
}

void utils_get_password(const char *prompt, char *buf, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);

#ifndef _WIN32
    /* Disable terminal echo while reading password */
    struct termios old_attr, new_attr;
    int echo_disabled = 0;

    if (tcgetattr(STDIN_FILENO, &old_attr) == 0) {
        new_attr = old_attr;
        new_attr.c_lflag &= ~(tcflag_t)ECHO;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_attr) == 0)
            echo_disabled = 1;
    }
#endif

    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
    } else {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        } else {
            utils_flush_stdin();
        }
    }

#ifndef _WIN32
    if (echo_disabled) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);
        putchar('\n');  /* newline since echo was off */
    }
#endif
}

/* ------------------------------------------------------------------ */
/* Password hashing                                                    */
/* ------------------------------------------------------------------ */

/*
 * utils_hash_password — demonstration-only password hashing.
 *
 * WARNING: This is NOT a cryptographically secure hash.  It must be
 * replaced with a proper password hashing function (bcrypt, Argon2,
 * or PBKDF2-SHA256 via libsodium or OpenSSL) before any production
 * deployment.  Passwords stored with this function are vulnerable to
 * offline brute-force and rainbow-table attacks.
 */
void utils_hash_password(const char *password,
                         char *out_hash, size_t out_size)
{
    unsigned long h1 = 5381UL;
    unsigned long h2 = 0xDEADBEEFUL;
    const unsigned char *s = (const unsigned char *)password;

    while (*s) {
        h1 = ((h1 << 5) + h1) ^ *s;
        h2 = ((h2 >> 3) | (h2 << 29))
             + (unsigned long)(*s) * 0x9e3779b9UL;
        s++;
    }

    /* Second mixing pass */
    h1 ^= h2 >> 16;
    h2 ^= h1 << 13;

    snprintf(out_hash, out_size, "%08lx%08lx",
             h1 & 0xFFFFFFFFUL, h2 & 0xFFFFFFFFUL);
}

/* ------------------------------------------------------------------ */
/* File-system helpers                                                 */
/* ------------------------------------------------------------------ */

int utils_ensure_dir(const char *dir)
{
    struct stat st;

    if (stat(dir, &st) == -1) {
#ifdef _WIN32
        if (_mkdir(dir) != 0) return -1;
#else
        if (mkdir(dir, 0755) != 0) return -1;
#endif
    }
    return 0;
}

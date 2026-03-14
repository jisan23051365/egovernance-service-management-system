#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* Date / time -------------------------------------------------------- */
void utils_get_date(char *buf, size_t size);

/* Terminal ----------------------------------------------------------- */
void utils_clear_screen(void);
void utils_print_divider(void);
void utils_print_header(const char *title);
void utils_press_enter(void);

/* Safe input helpers ------------------------------------------------- */
void utils_flush_stdin(void);
int  utils_get_string(const char *prompt, char *buf, size_t size);
int  utils_get_int(const char *prompt, int *value, int min, int max);
int  utils_get_double(const char *prompt, double *value,
                      double min, double max);
void utils_get_password(const char *prompt, char *buf, size_t size);

/* Password hashing --------------------------------------------------- */
/*
 * DEMONSTRATION-ONLY hash — NOT cryptographically secure.
 * Replace with bcrypt, Argon2, or PBKDF2-SHA256 before any
 * production deployment.
 */
void utils_hash_password(const char *password,
                         char *out_hash, size_t out_size);

/* File-system -------------------------------------------------------- */
int utils_ensure_dir(const char *dir);

#endif /* UTILS_H */

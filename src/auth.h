#ifndef AUTH_H
#define AUTH_H

#include "constants.h"

/* ---------------------------------------------------------------
 * User role
 * --------------------------------------------------------------- */
typedef enum {
    ROLE_CITIZEN = 0,
    ROLE_ADMIN   = 1
} UserRole;

/* ---------------------------------------------------------------
 * User record stored in users.dat
 * --------------------------------------------------------------- */
typedef struct {
    int      id;
    char     username[LEN_USERNAME];
    char     password_hash[LEN_HASH];
    UserRole role;
    char     name[LEN_NAME];
    char     email[LEN_EMAIL];
    char     phone[LEN_PHONE];
    char     address[LEN_ADDRESS];
    char     created_date[LEN_DATE];
    int      active; /* 1 = active, 0 = deactivated */
} User;

/* ---------------------------------------------------------------
 * Module lifecycle
 * --------------------------------------------------------------- */

/* Load users from disk; create default admin if file is empty. */
void auth_init(void);

/* ---------------------------------------------------------------
 * Citizen self-service
 * --------------------------------------------------------------- */

/*
 * Register a new citizen account.
 * Returns the new user ID (>= 1) on success, -1 on failure.
 */
int auth_register(const char *username, const char *password,
                  const char *name,     const char *email,
                  const char *phone,    const char *address);

/*
 * Authenticate a user by username / password.
 * Fills *out_user (if non-NULL) and returns 1 on success, 0 on failure.
 */
int auth_login(const char *username, const char *password,
               User *out_user);

/*
 * Change a user's password.
 * Returns 0 on success, -1 on failure.
 */
int auth_change_password(int user_id,
                         const char *old_password,
                         const char *new_password);

/* ---------------------------------------------------------------
 * Lookups
 * --------------------------------------------------------------- */

/* Fill *out_user from user ID. Returns 1 if found, 0 if not. */
int auth_get_user(int user_id, User *out_user);

/* Total number of stored users (all roles). */
int auth_get_count(void);

/* Return a const pointer to the n-th internal User record (0-based). */
const User *auth_get_at(int index);

/* ---------------------------------------------------------------
 * Admin operations
 * --------------------------------------------------------------- */

/* Print a formatted table of all citizen accounts. */
void auth_display_citizens(void);

/*
 * Deactivate a citizen account.
 * Returns 0 on success, -1 on failure.
 */
int auth_deactivate(int user_id);

#endif /* AUTH_H */

#include <stdio.h>
#include <string.h>

#include "auth.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* Module-level storage                                               */
/* ------------------------------------------------------------------ */

static User users[MAX_USERS];
static int  user_count = 0;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                   */
/* ------------------------------------------------------------------ */

static void save_users(void)
{
    FILE *fp = fopen(USERS_FILE, "wb");
    if (!fp) { perror("Cannot save users"); return; }

    fwrite(&user_count, sizeof(int), 1, fp);
    fwrite(users, sizeof(User), (size_t)user_count, fp);
    fclose(fp);
}

static void load_users(void)
{
    FILE *fp = fopen(USERS_FILE, "rb");
    if (!fp) { user_count = 0; return; }

    if (fread(&user_count, sizeof(int), 1, fp) != 1 ||
        user_count < 0 || user_count > MAX_USERS) {
        user_count = 0;
        fclose(fp);
        return;
    }

    if ((int)fread(users, sizeof(User), (size_t)user_count, fp)
        != user_count) {
        user_count = 0;
    }

    fclose(fp);
}

static int find_by_username(const char *username)
{
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0)
            return i;
    }
    return -1;
}

static int find_by_id(int id)
{
    for (int i = 0; i < user_count; i++) {
        if (users[i].id == id)
            return i;
    }
    return -1;
}

static int next_id(void)
{
    int max = 0;
    for (int i = 0; i < user_count; i++) {
        if (users[i].id > max) max = users[i].id;
    }
    return max + 1;
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

void auth_init(void)
{
    load_users();

    if (user_count == 0) {
        /* Bootstrap the default admin account */
        User admin;
        memset(&admin, 0, sizeof(admin));

        admin.id = 1;
        strncpy(admin.username, DEFAULT_ADMIN_USERNAME, LEN_USERNAME - 1);
        utils_hash_password(DEFAULT_ADMIN_PASSWORD,
                            admin.password_hash, LEN_HASH);
        admin.role = ROLE_ADMIN;
        strncpy(admin.name,    DEFAULT_ADMIN_NAME,  LEN_NAME    - 1);
        strncpy(admin.email,   DEFAULT_ADMIN_EMAIL, LEN_EMAIL   - 1);
        strncpy(admin.phone,   "N/A",               LEN_PHONE   - 1);
        strncpy(admin.address, "Government Office", LEN_ADDRESS - 1);
        utils_get_date(admin.created_date, LEN_DATE);
        admin.active = 1;

        users[0] = admin;
        user_count = 1;
        save_users();
    }
}

int auth_register(const char *username, const char *password,
                  const char *name,     const char *email,
                  const char *phone,    const char *address)
{
    if (user_count >= MAX_USERS) {
        puts("  Error: User limit reached.");
        return -1;
    }
    if (!username || *username == '\0' || !password || *password == '\0') {
        puts("  Error: Username and password are required.");
        return -1;
    }
    if (strlen(password) < 6) {
        puts("  Error: Password must be at least 6 characters.");
        return -1;
    }
    if (find_by_username(username) >= 0) {
        puts("  Error: Username already exists.");
        return -1;
    }

    User u;
    memset(&u, 0, sizeof(u));

    u.id = next_id();
    strncpy(u.username, username, LEN_USERNAME - 1);
    utils_hash_password(password, u.password_hash, LEN_HASH);
    u.role = ROLE_CITIZEN;
    strncpy(u.name,    name    ? name    : "", LEN_NAME    - 1);
    strncpy(u.email,   email   ? email   : "", LEN_EMAIL   - 1);
    strncpy(u.phone,   phone   ? phone   : "", LEN_PHONE   - 1);
    strncpy(u.address, address ? address : "", LEN_ADDRESS - 1);
    utils_get_date(u.created_date, LEN_DATE);
    u.active = 1;

    users[user_count++] = u;
    save_users();
    return u.id;
}

int auth_login(const char *username, const char *password, User *out_user)
{
    int idx = find_by_username(username);
    if (idx < 0 || !users[idx].active)
        return 0;

    char hash[LEN_HASH];
    utils_hash_password(password, hash, LEN_HASH);

    if (strcmp(users[idx].password_hash, hash) != 0)
        return 0;

    if (out_user) *out_user = users[idx];
    return 1;
}

int auth_change_password(int user_id,
                         const char *old_password,
                         const char *new_password)
{
    int idx = find_by_id(user_id);
    if (idx < 0) {
        puts("  Error: User not found.");
        return -1;
    }

    char hash[LEN_HASH];
    utils_hash_password(old_password, hash, LEN_HASH);
    if (strcmp(users[idx].password_hash, hash) != 0) {
        puts("  Error: Current password is incorrect.");
        return -1;
    }
    if (!new_password || strlen(new_password) < 6) {
        puts("  Error: New password must be at least 6 characters.");
        return -1;
    }

    utils_hash_password(new_password, users[idx].password_hash, LEN_HASH);
    save_users();
    return 0;
}

int auth_get_user(int user_id, User *out_user)
{
    int idx = find_by_id(user_id);
    if (idx < 0) return 0;
    if (out_user) *out_user = users[idx];
    return 1;
}

int auth_get_count(void)
{
    return user_count;
}

const User *auth_get_at(int index)
{
    if (index < 0 || index >= user_count) return NULL;
    return &users[index];
}

void auth_display_citizens(void)
{
    printf("\n  %-5s  %-20s  %-24s  %-14s  %-8s\n",
           "ID", "Name", "Username", "Phone", "Status");
    utils_print_divider();

    int found = 0;
    for (int i = 0; i < user_count; i++) {
        if (users[i].role != ROLE_CITIZEN) continue;
        printf("  %-5d  %-20.20s  %-24.24s  %-14.14s  %-8s\n",
               users[i].id,
               users[i].name,
               users[i].username,
               users[i].phone,
               users[i].active ? "Active" : "Disabled");
        found++;
    }

    if (!found) printf("  No citizens registered yet.\n");
    printf("\n  Total citizens: %d\n", found);
}

int auth_deactivate(int user_id)
{
    int idx = find_by_id(user_id);
    if (idx < 0) {
        puts("  Error: User not found.");
        return -1;
    }
    if (users[idx].role == ROLE_ADMIN) {
        puts("  Error: Cannot deactivate an admin account.");
        return -1;
    }
    users[idx].active = 0;
    save_users();
    return 0;
}

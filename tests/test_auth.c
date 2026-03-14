/*
 * test_auth.c — Unit tests for the auth module.
 *
 * Each test function returns 0 on pass, 1 on failure.
 * Tests run against a temporary data directory so they do not
 * touch real application data.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

/* Include constants.h first, then override file paths for test isolation */
#include "../src/constants.h"

#undef  DATA_DIR
#undef  USERS_FILE
#undef  SERVICE_REQUESTS_FILE
#undef  TAX_RECORDS_FILE
#define DATA_DIR               "test_data"
#define USERS_FILE             "test_data/users.dat"
#define SERVICE_REQUESTS_FILE  "test_data/service_requests.dat"
#define TAX_RECORDS_FILE       "test_data/tax_records.dat"
#include "../src/utils.h"
#include "../src/auth.h"

/* Re-include the implementation so it picks up the redefined paths */
#include "../src/utils.c"
#include "../src/auth.c"

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static void setup(void)
{
    /* Remove any leftover test data then ensure the directory exists */
    remove(USERS_FILE);
    utils_ensure_dir("test_data");
}

static int pass_count = 0;
static int fail_count = 0;

#define TEST(name) static int name(void)
#define RUN(name) \
    do { \
        if (name() == 0) { printf("  [PASS] " #name "\n"); pass_count++; } \
        else             { printf("  [FAIL] " #name "\n"); fail_count++; } \
    } while (0)
#define ASSERT(expr) \
    do { if (!(expr)) { printf("    Assertion failed: %s (line %d)\n", \
                                #expr, __LINE__); return 1; } } while (0)

/* ------------------------------------------------------------------ */
/* Tests                                                              */
/* ------------------------------------------------------------------ */

TEST(test_hash_password_deterministic)
{
    char h1[LEN_HASH], h2[LEN_HASH];
    utils_hash_password("secret123", h1, sizeof(h1));
    utils_hash_password("secret123", h2, sizeof(h2));
    ASSERT(strcmp(h1, h2) == 0);
    return 0;
}

TEST(test_hash_password_different_inputs)
{
    char h1[LEN_HASH], h2[LEN_HASH];
    utils_hash_password("password1", h1, sizeof(h1));
    utils_hash_password("password2", h2, sizeof(h2));
    ASSERT(strcmp(h1, h2) != 0);
    return 0;
}

TEST(test_register_and_login)
{
    setup();
    auth_init();

    int id = auth_register("alice", "Pass@123",
                           "Alice Smith", "alice@example.com",
                           "555-1234", "1 Main St");
    ASSERT(id > 0);

    User u;
    int ok = auth_login("alice", "Pass@123", &u);
    ASSERT(ok == 1);
    ASSERT(strcmp(u.username, "alice") == 0);
    ASSERT(u.role == ROLE_CITIZEN);
    ASSERT(u.active == 1);
    return 0;
}

TEST(test_login_wrong_password)
{
    setup();
    auth_init();
    auth_register("bob", "correct_pw", "Bob", "", "", "");

    User u;
    int ok = auth_login("bob", "wrong_pw", &u);
    ASSERT(ok == 0);
    return 0;
}

TEST(test_login_nonexistent_user)
{
    setup();
    auth_init();

    User u;
    int ok = auth_login("ghost", "anything", &u);
    ASSERT(ok == 0);
    return 0;
}

TEST(test_duplicate_username_rejected)
{
    setup();
    auth_init();

    int id1 = auth_register("carol", "pw1234", "Carol", "", "", "");
    int id2 = auth_register("carol", "pw5678", "Carol2", "", "", "");
    ASSERT(id1 > 0);
    ASSERT(id2 < 0);
    return 0;
}

TEST(test_short_password_rejected)
{
    setup();
    auth_init();

    int id = auth_register("dave", "short", "Dave", "", "", "");
    ASSERT(id < 0);
    return 0;
}

TEST(test_change_password)
{
    setup();
    auth_init();

    int id = auth_register("eve", "OldPass1", "Eve", "", "", "");
    ASSERT(id > 0);

    int rc = auth_change_password(id, "OldPass1", "NewPass2");
    ASSERT(rc == 0);

    User u;
    ASSERT(auth_login("eve", "OldPass1", &u) == 0);  /* old fails */
    ASSERT(auth_login("eve", "NewPass2", &u) == 1);  /* new works */
    return 0;
}

TEST(test_change_password_wrong_old)
{
    setup();
    auth_init();

    int id = auth_register("frank", "GoodPass1", "Frank", "", "", "");
    ASSERT(id > 0);

    int rc = auth_change_password(id, "BadOld123", "NewPass99");
    ASSERT(rc < 0);
    return 0;
}

TEST(test_deactivate_citizen)
{
    setup();
    auth_init();

    int id = auth_register("grace", "Pw123456", "Grace", "", "", "");
    ASSERT(id > 0);

    ASSERT(auth_deactivate(id) == 0);

    User u;
    ASSERT(auth_login("grace", "Pw123456", &u) == 0); /* deactivated */
    return 0;
}

TEST(test_default_admin_created)
{
    setup();
    auth_init();   /* first call — creates default admin */

    User u;
    int ok = auth_login(DEFAULT_ADMIN_USERNAME, DEFAULT_ADMIN_PASSWORD, &u);
    ASSERT(ok == 1);
    ASSERT(u.role == ROLE_ADMIN);
    return 0;
}

TEST(test_get_user_by_id)
{
    setup();
    auth_init();

    int id = auth_register("hank", "HankPass1", "Hank", "", "", "");
    ASSERT(id > 0);

    User u;
    ASSERT(auth_get_user(id, &u) == 1);
    ASSERT(strcmp(u.username, "hank") == 0);
    return 0;
}

TEST(test_get_user_by_id_not_found)
{
    setup();
    auth_init();

    User u;
    ASSERT(auth_get_user(9999, &u) == 0);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Runner                                                             */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("\n=== Auth Module Tests ===\n\n");

    RUN(test_hash_password_deterministic);
    RUN(test_hash_password_different_inputs);
    RUN(test_register_and_login);
    RUN(test_login_wrong_password);
    RUN(test_login_nonexistent_user);
    RUN(test_duplicate_username_rejected);
    RUN(test_short_password_rejected);
    RUN(test_change_password);
    RUN(test_change_password_wrong_old);
    RUN(test_deactivate_citizen);
    RUN(test_default_admin_created);
    RUN(test_get_user_by_id);
    RUN(test_get_user_by_id_not_found);

    printf("\n  Passed: %d  Failed: %d\n\n", pass_count, fail_count);

    /* Clean up test data */
    remove(USERS_FILE);
    rmdir("test_data");

    return fail_count > 0 ? 1 : 0;
}

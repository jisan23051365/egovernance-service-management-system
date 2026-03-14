/*
 * test_service_request.c — Unit tests for the service_request module.
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
#include "../src/service_request.h"

#include "../src/utils.c"
#include "../src/service_request.c"

/* ------------------------------------------------------------------ */
/* Test framework macros                                              */
/* ------------------------------------------------------------------ */

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

static void setup(void)
{
    remove(SERVICE_REQUESTS_FILE);
    utils_ensure_dir("test_data");
}

/* ------------------------------------------------------------------ */
/* Tests                                                              */
/* ------------------------------------------------------------------ */

TEST(test_submit_returns_valid_id)
{
    setup();
    sr_init();

    int id = sr_submit(42, SVC_PASSPORT, "Need a new passport");
    ASSERT(id > 0);
    return 0;
}

TEST(test_submit_increments_count)
{
    setup();
    sr_init();

    ASSERT(sr_get_count() == 0);
    sr_submit(1, SVC_DRIVING_LICENSE, "First request");
    ASSERT(sr_get_count() == 1);
    sr_submit(2, SVC_PASSPORT, "Second request");
    ASSERT(sr_get_count() == 2);
    return 0;
}

TEST(test_submitted_request_is_pending)
{
    setup();
    sr_init();

    int id = sr_submit(10, SVC_BIRTH_CERTIFICATE, "Birth cert");
    ASSERT(id > 0);

    /* Find the request we just created */
    int found = 0;
    for (int i = 0; i < sr_get_count(); i++) {
        const ServiceRequest *r = sr_get_at(i);
        if (r->id == id) {
            ASSERT(r->status == REQ_PENDING);
            ASSERT(r->citizen_id == 10);
            ASSERT(r->type == SVC_BIRTH_CERTIFICATE);
            found = 1;
        }
    }
    ASSERT(found == 1);
    return 0;
}

TEST(test_update_status_approved)
{
    setup();
    sr_init();

    int id = sr_submit(5, SVC_BUSINESS_LICENSE, "Business license application");
    ASSERT(id > 0);

    int rc = sr_update_status(id, REQ_APPROVED, "All documents verified");
    ASSERT(rc == 0);

    for (int i = 0; i < sr_get_count(); i++) {
        const ServiceRequest *r = sr_get_at(i);
        if (r->id == id) {
            ASSERT(r->status == REQ_APPROVED);
            ASSERT(strcmp(r->admin_notes, "All documents verified") == 0);
        }
    }
    return 0;
}

TEST(test_update_status_invalid_id)
{
    setup();
    sr_init();

    int rc = sr_update_status(9999, REQ_REJECTED, "");
    ASSERT(rc < 0);
    return 0;
}

TEST(test_update_status_in_progress_then_rejected)
{
    setup();
    sr_init();

    int id = sr_submit(7, SVC_PROPERTY_REGISTRATION, "Property reg");
    ASSERT(id > 0);

    ASSERT(sr_update_status(id, REQ_IN_PROGRESS, "Under review") == 0);
    ASSERT(sr_update_status(id, REQ_REJECTED, "Docs missing") == 0);

    for (int i = 0; i < sr_get_count(); i++) {
        const ServiceRequest *r = sr_get_at(i);
        if (r->id == id)
            ASSERT(r->status == REQ_REJECTED);
    }
    return 0;
}

TEST(test_type_string_helpers)
{
    ASSERT(strcmp(sr_type_str(SVC_BIRTH_CERTIFICATE),    "Birth Certificate")    == 0);
    ASSERT(strcmp(sr_type_str(SVC_PASSPORT),             "Passport")             == 0);
    ASSERT(strcmp(sr_type_str(SVC_OTHER),                "Other")                == 0);
    return 0;
}

TEST(test_status_string_helpers)
{
    ASSERT(strcmp(sr_status_str(REQ_PENDING),     "Pending")     == 0);
    ASSERT(strcmp(sr_status_str(REQ_IN_PROGRESS), "In Progress") == 0);
    ASSERT(strcmp(sr_status_str(REQ_APPROVED),    "Approved")    == 0);
    ASSERT(strcmp(sr_status_str(REQ_REJECTED),    "Rejected")    == 0);
    return 0;
}

TEST(test_multiple_requests_same_citizen)
{
    setup();
    sr_init();

    int id1 = sr_submit(20, SVC_DRIVING_LICENSE,  "Request A");
    int id2 = sr_submit(20, SVC_MARRIAGE_CERTIFICATE, "Request B");
    ASSERT(id1 > 0 && id2 > 0);
    ASSERT(id1 != id2);
    ASSERT(sr_get_count() == 2);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Runner                                                             */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("\n=== Service Request Module Tests ===\n\n");

    RUN(test_submit_returns_valid_id);
    RUN(test_submit_increments_count);
    RUN(test_submitted_request_is_pending);
    RUN(test_update_status_approved);
    RUN(test_update_status_invalid_id);
    RUN(test_update_status_in_progress_then_rejected);
    RUN(test_type_string_helpers);
    RUN(test_status_string_helpers);
    RUN(test_multiple_requests_same_citizen);

    printf("\n  Passed: %d  Failed: %d\n\n", pass_count, fail_count);

    remove(SERVICE_REQUESTS_FILE);
    rmdir("test_data");

    return fail_count > 0 ? 1 : 0;
}

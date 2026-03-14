/*
 * test_tax_record.c — Unit tests for the tax_record module.
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
#include "../src/tax_record.h"

#include "../src/utils.c"
#include "../src/tax_record.c"

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
    remove(TAX_RECORDS_FILE);
    utils_ensure_dir("test_data");
}

/* ------------------------------------------------------------------ */
/* Tests                                                              */
/* ------------------------------------------------------------------ */

TEST(test_add_record_returns_valid_id)
{
    setup();
    tr_init();

    int id = tr_add(1, 2024, TAX_INCOME, 50000.0, 5000.0,
                    "2024-12-31", "");
    ASSERT(id > 0);
    return 0;
}

TEST(test_add_record_increments_count)
{
    setup();
    tr_init();

    ASSERT(tr_get_count() == 0);
    tr_add(1, 2024, TAX_INCOME,    50000.0, 5000.0, "2024-12-31", "");
    ASSERT(tr_get_count() == 1);
    tr_add(1, 2024, TAX_PROPERTY, 200000.0, 2000.0, "2024-12-31", "");
    ASSERT(tr_get_count() == 2);
    return 0;
}

TEST(test_new_record_is_unpaid)
{
    setup();
    tr_init();

    int id = tr_add(2, 2023, TAX_BUSINESS, 100000.0, 10000.0,
                    "2023-12-31", "");
    ASSERT(id > 0);

    for (int i = 0; i < tr_get_count(); i++) {
        const TaxRecord *r = tr_get_at(i);
        if (r->id == id) {
            ASSERT(r->status == TAX_UNPAID);
            ASSERT(r->amount_paid == 0.0);
        }
    }
    return 0;
}

TEST(test_pay_full_amount_marks_paid)
{
    setup();
    tr_init();

    int id = tr_add(3, 2024, TAX_INCOME, 80000.0, 8000.0,
                    "2024-12-31", "");
    ASSERT(id > 0);

    int rc = tr_pay(id, 8000.0);
    ASSERT(rc == 0);

    for (int i = 0; i < tr_get_count(); i++) {
        const TaxRecord *r = tr_get_at(i);
        if (r->id == id) {
            ASSERT(r->status == TAX_PAID);
            ASSERT(r->amount_paid == 8000.0);
        }
    }
    return 0;
}

TEST(test_pay_partial_amount_marks_partial)
{
    setup();
    tr_init();

    int id = tr_add(4, 2024, TAX_VEHICLE, 30000.0, 3000.0,
                    "2024-12-31", "");
    ASSERT(id > 0);

    int rc = tr_pay(id, 1000.0);
    ASSERT(rc == 0);

    for (int i = 0; i < tr_get_count(); i++) {
        const TaxRecord *r = tr_get_at(i);
        if (r->id == id) {
            ASSERT(r->status == TAX_PARTIAL);
            ASSERT(r->amount_paid == 1000.0);
        }
    }
    return 0;
}

TEST(test_pay_twice_accumulates)
{
    setup();
    tr_init();

    int id = tr_add(5, 2024, TAX_INCOME, 60000.0, 6000.0,
                    "2024-12-31", "");
    ASSERT(id > 0);

    tr_pay(id, 2000.0);
    tr_pay(id, 4000.0);

    for (int i = 0; i < tr_get_count(); i++) {
        const TaxRecord *r = tr_get_at(i);
        if (r->id == id) {
            ASSERT(r->status == TAX_PAID);
            ASSERT(r->amount_paid == 6000.0);
        }
    }
    return 0;
}

TEST(test_pay_already_paid_rejected)
{
    setup();
    tr_init();

    int id = tr_add(6, 2024, TAX_INCOME, 40000.0, 4000.0,
                    "2024-12-31", "");
    tr_pay(id, 4000.0); /* fully paid */

    int rc = tr_pay(id, 100.0);
    ASSERT(rc < 0);
    return 0;
}

TEST(test_pay_zero_amount_rejected)
{
    setup();
    tr_init();

    int id = tr_add(7, 2024, TAX_PROPERTY, 150000.0, 1500.0,
                    "2024-12-31", "");
    int rc = tr_pay(id, 0.0);
    ASSERT(rc < 0);
    return 0;
}

TEST(test_pay_invalid_id)
{
    setup();
    tr_init();

    int rc = tr_pay(9999, 100.0);
    ASSERT(rc < 0);
    return 0;
}

TEST(test_negative_amount_rejected)
{
    setup();
    tr_init();

    int id = tr_add(8, 2024, TAX_INCOME, 50000.0, -500.0,
                    "2024-12-31", "");
    ASSERT(id < 0);
    return 0;
}

TEST(test_type_string_helpers)
{
    ASSERT(strcmp(tr_type_str(TAX_INCOME),   "Income Tax")   == 0);
    ASSERT(strcmp(tr_type_str(TAX_PROPERTY), "Property Tax") == 0);
    ASSERT(strcmp(tr_type_str(TAX_BUSINESS), "Business Tax") == 0);
    ASSERT(strcmp(tr_type_str(TAX_VEHICLE),  "Vehicle Tax")  == 0);
    return 0;
}

TEST(test_status_string_helpers)
{
    ASSERT(strcmp(tr_status_str(TAX_UNPAID),  "Unpaid")  == 0);
    ASSERT(strcmp(tr_status_str(TAX_PAID),    "Paid")    == 0);
    ASSERT(strcmp(tr_status_str(TAX_PARTIAL), "Partial") == 0);
    ASSERT(strcmp(tr_status_str(TAX_WAIVED),  "Waived")  == 0);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Runner                                                             */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("\n=== Tax Record Module Tests ===\n\n");

    RUN(test_add_record_returns_valid_id);
    RUN(test_add_record_increments_count);
    RUN(test_new_record_is_unpaid);
    RUN(test_pay_full_amount_marks_paid);
    RUN(test_pay_partial_amount_marks_partial);
    RUN(test_pay_twice_accumulates);
    RUN(test_pay_already_paid_rejected);
    RUN(test_pay_zero_amount_rejected);
    RUN(test_pay_invalid_id);
    RUN(test_negative_amount_rejected);
    RUN(test_type_string_helpers);
    RUN(test_status_string_helpers);

    printf("\n  Passed: %d  Failed: %d\n\n", pass_count, fail_count);

    remove(TAX_RECORDS_FILE);
    rmdir("test_data");

    return fail_count > 0 ? 1 : 0;
}

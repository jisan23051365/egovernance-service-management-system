#include <stdio.h>
#include <string.h>

#include "tax_record.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* Module-level storage                                               */
/* ------------------------------------------------------------------ */

static TaxRecord records[MAX_TAX_RECORDS];
static int       rec_count = 0;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                   */
/* ------------------------------------------------------------------ */

static void save_records(void)
{
    FILE *fp = fopen(TAX_RECORDS_FILE, "wb");
    if (!fp) { perror("Cannot save tax records"); return; }

    fwrite(&rec_count, sizeof(int), 1, fp);
    fwrite(records, sizeof(TaxRecord), (size_t)rec_count, fp);
    fclose(fp);
}

static void load_records(void)
{
    FILE *fp = fopen(TAX_RECORDS_FILE, "rb");
    if (!fp) { rec_count = 0; return; }

    if (fread(&rec_count, sizeof(int), 1, fp) != 1 ||
        rec_count < 0 || rec_count > MAX_TAX_RECORDS) {
        rec_count = 0;
        fclose(fp);
        return;
    }

    if ((int)fread(records, sizeof(TaxRecord),
                   (size_t)rec_count, fp) != rec_count) {
        rec_count = 0;
    }

    fclose(fp);
}

static int next_id(void)
{
    int max = 0;
    for (int i = 0; i < rec_count; i++) {
        if (records[i].id > max) max = records[i].id;
    }
    return max + 1;
}

/* ------------------------------------------------------------------ */
/* String helpers                                                     */
/* ------------------------------------------------------------------ */

const char *tr_type_str(TaxType t)
{
    static const char *names[TAX_TYPE_COUNT] = {
        "Income Tax", "Property Tax", "Business Tax", "Vehicle Tax"
    };
    if (t < 0 || t >= TAX_TYPE_COUNT) return "Unknown";
    return names[t];
}

const char *tr_status_str(TaxStatus s)
{
    static const char *names[TAX_STATUS_COUNT] = {
        "Unpaid", "Paid", "Partial", "Waived"
    };
    if (s < 0 || s >= TAX_STATUS_COUNT) return "Unknown";
    return names[s];
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

void tr_init(void)
{
    load_records();
}

int tr_add(int citizen_id, int year, TaxType type,
           double taxable_amount, double tax_due,
           const char *due_date, const char *notes)
{
    if (rec_count >= MAX_TAX_RECORDS) {
        puts("  Error: Tax record limit reached.");
        return -1;
    }
    if (tax_due < 0.0 || taxable_amount < 0.0) {
        puts("  Error: Amounts cannot be negative.");
        return -1;
    }

    TaxRecord r;
    memset(&r, 0, sizeof(r));

    r.id             = next_id();
    r.citizen_id     = citizen_id;
    r.year           = year;
    r.type           = type;
    r.taxable_amount = taxable_amount;
    r.tax_due        = tax_due;
    r.amount_paid    = 0.0;
    r.status         = TAX_UNPAID;
    strncpy(r.due_date, due_date ? due_date : "", LEN_DATE  - 1);
    strncpy(r.notes,    notes    ? notes    : "", LEN_NOTES - 1);

    records[rec_count++] = r;
    save_records();
    return r.id;
}

void tr_display_citizen(int citizen_id)
{
    printf("\n  %-5s  %-14s  %-4s  %-14s  %10s  %10s  %-9s\n",
           "ID", "Type", "Year", "Due Date",
           "Tax Due", "Paid", "Status");
    utils_print_divider();

    int found = 0;
    for (int i = 0; i < rec_count; i++) {
        if (records[i].citizen_id != citizen_id) continue;
        printf("  %-5d  %-14.14s  %-4d  %-14.14s  %10.2f  %10.2f  %-9s\n",
               records[i].id,
               tr_type_str(records[i].type),
               records[i].year,
               records[i].due_date,
               records[i].tax_due,
               records[i].amount_paid,
               tr_status_str(records[i].status));
        found++;
    }

    if (!found) printf("  No tax records found for your account.\n");
    printf("\n  Total: %d record(s)\n", found);
}

void tr_display_all(void)
{
    printf("\n  %-5s  %-8s  %-14s  %-4s  %10s  %10s  %-9s\n",
           "ID", "Cit.ID", "Type", "Year",
           "Tax Due", "Paid", "Status");
    utils_print_divider();

    for (int i = 0; i < rec_count; i++) {
        printf("  %-5d  %-8d  %-14.14s  %-4d  %10.2f  %10.2f  %-9s\n",
               records[i].id,
               records[i].citizen_id,
               tr_type_str(records[i].type),
               records[i].year,
               records[i].tax_due,
               records[i].amount_paid,
               tr_status_str(records[i].status));
    }

    if (!rec_count) printf("  No tax records on file.\n");
    printf("\n  Total: %d record(s)\n", rec_count);
}

int tr_pay(int record_id, double amount)
{
    if (amount <= 0.0) {
        puts("  Error: Payment amount must be greater than zero.");
        return -1;
    }

    for (int i = 0; i < rec_count; i++) {
        if (records[i].id != record_id) continue;

        if (records[i].status == TAX_PAID) {
            puts("  This record is already fully paid.");
            return -1;
        }
        if (records[i].status == TAX_WAIVED) {
            puts("  This record has been waived; no payment needed.");
            return -1;
        }

        double outstanding = records[i].tax_due - records[i].amount_paid;
        if (amount > outstanding) {
            printf("  Note: Payment (%.2f) exceeds outstanding amount "
                   "(%.2f). Capping to outstanding balance.\n",
                   amount, outstanding);
            amount = outstanding;
        }

        records[i].amount_paid += amount;
        utils_get_date(records[i].paid_date, LEN_DATE);

        if (records[i].amount_paid >= records[i].tax_due)
            records[i].status = TAX_PAID;
        else
            records[i].status = TAX_PARTIAL;

        save_records();
        printf("  Payment of %.2f applied. Outstanding: %.2f  Status: %s\n",
               amount,
               records[i].tax_due - records[i].amount_paid,
               tr_status_str(records[i].status));
        return 0;
    }

    puts("  Error: Tax record ID not found.");
    return -1;
}

int tr_get_count(void)
{
    return rec_count;
}

const TaxRecord *tr_get_at(int index)
{
    if (index < 0 || index >= rec_count) return NULL;
    return &records[index];
}

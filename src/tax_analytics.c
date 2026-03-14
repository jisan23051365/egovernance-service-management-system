#include <stdio.h>
#include <string.h>

#include "tax_analytics.h"
#include "tax_record.h"
#include "auth.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* Internal helpers                                                   */
/* ------------------------------------------------------------------ */

/* Return the citizen's name for display; falls back to "Unknown". */
static void citizen_name(int citizen_id, char *buf, size_t size)
{
    User u;
    if (auth_get_user(citizen_id, &u))
        strncpy(buf, u.name, size - 1);
    else
        strncpy(buf, "Unknown", size - 1);
    buf[size - 1] = '\0';
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

void analytics_summary_by_year(void)
{
    utils_print_header("TAX SUMMARY BY YEAR");

    int   count = tr_get_count();
    int   years[50];
    int   year_cnt = 0;

    /* Collect distinct years */
    for (int i = 0; i < count; i++) {
        const TaxRecord *r = tr_get_at(i);
        int found = 0;
        for (int j = 0; j < year_cnt; j++) {
            if (years[j] == r->year) { found = 1; break; }
        }
        if (!found && year_cnt < 50)
            years[year_cnt++] = r->year;
    }

    if (year_cnt == 0) {
        printf("\n  No tax records available.\n");
        return;
    }

    /* Sort years ascending (simple bubble sort — small dataset) */
    for (int i = 0; i < year_cnt - 1; i++) {
        for (int j = i + 1; j < year_cnt; j++) {
            if (years[j] < years[i]) {
                int tmp = years[i];
                years[i] = years[j];
                years[j] = tmp;
            }
        }
    }

    printf("\n  %-6s  %12s  %12s  %12s  %10s\n",
           "Year", "Total Due", "Total Paid", "Outstanding", "Records");
    utils_print_divider();

    for (int y = 0; y < year_cnt; y++) {
        double due = 0.0, paid = 0.0;
        int    recs = 0;

        for (int i = 0; i < count; i++) {
            const TaxRecord *r = tr_get_at(i);
            if (r->year != years[y]) continue;
            due  += r->tax_due;
            paid += r->amount_paid;
            recs++;
        }

        printf("  %-6d  %12.2f  %12.2f  %12.2f  %10d\n",
               years[y], due, paid, due - paid, recs);
    }

    /* Grand totals */
    double grand_due = 0.0, grand_paid = 0.0;
    for (int i = 0; i < count; i++) {
        const TaxRecord *r = tr_get_at(i);
        grand_due  += r->tax_due;
        grand_paid += r->amount_paid;
    }
    utils_print_divider();
    printf("  %-6s  %12.2f  %12.2f  %12.2f  %10d\n",
           "TOTAL", grand_due, grand_paid, grand_due - grand_paid, count);
}

void analytics_by_type(void)
{
    utils_print_header("TAX REVENUE BY TYPE");

    double due[TAX_TYPE_COUNT]  = {0};
    double paid[TAX_TYPE_COUNT] = {0};
    int    cnt[TAX_TYPE_COUNT]  = {0};
    int    total_count = tr_get_count();

    for (int i = 0; i < total_count; i++) {
        const TaxRecord *r = tr_get_at(i);
        int t = (int)r->type;
        if (t >= 0 && t < TAX_TYPE_COUNT) {
            due[t]  += r->tax_due;
            paid[t] += r->amount_paid;
            cnt[t]++;
        }
    }

    printf("\n  %-16s  %12s  %12s  %12s  %8s\n",
           "Tax Type", "Total Due", "Total Paid", "Outstanding", "Records");
    utils_print_divider();

    double grand_due = 0.0, grand_paid = 0.0;
    int    grand_cnt = 0;

    for (int t = 0; t < TAX_TYPE_COUNT; t++) {
        printf("  %-16.16s  %12.2f  %12.2f  %12.2f  %8d\n",
               tr_type_str((TaxType)t),
               due[t], paid[t], due[t] - paid[t], cnt[t]);
        grand_due  += due[t];
        grand_paid += paid[t];
        grand_cnt  += cnt[t];
    }

    utils_print_divider();
    printf("  %-16s  %12.2f  %12.2f  %12.2f  %8d\n",
           "TOTAL", grand_due, grand_paid,
           grand_due - grand_paid, grand_cnt);
}

void analytics_delinquent(void)
{
    utils_print_header("DELINQUENT TAXPAYERS REPORT");

    printf("\n  %-5s  %-22s  %-14s  %-4s  %10s  %10s\n",
           "RecID", "Citizen Name", "Tax Type", "Year",
           "Outstanding", "Status");
    utils_print_divider();

    int    count  = tr_get_count();
    int    found  = 0;
    double total_outstanding = 0.0;

    for (int i = 0; i < count; i++) {
        const TaxRecord *r = tr_get_at(i);
        if (r->status != TAX_UNPAID && r->status != TAX_PARTIAL)
            continue;

        char name[LEN_NAME];
        citizen_name(r->citizen_id, name, sizeof(name));

        double outstanding = r->tax_due - r->amount_paid;
        printf("  %-5d  %-22.22s  %-14.14s  %-4d  %10.2f  %-9s\n",
               r->id, name,
               tr_type_str(r->type), r->year,
               outstanding, tr_status_str(r->status));

        total_outstanding += outstanding;
        found++;
    }

    if (!found) {
        printf("  No delinquent taxpayers found.\n");
        return;
    }

    utils_print_divider();
    printf("  Total delinquent records: %d   "
           "Total outstanding: %.2f\n", found, total_outstanding);
}

void analytics_year_comparison(void)
{
    utils_print_header("YEAR-OVER-YEAR COMPARISON");

    int year1, year2;

    if (utils_get_int("  Enter first year : ", &year1, 1900, 9999) < 0)
        return;
    if (utils_get_int("  Enter second year: ", &year2, 1900, 9999) < 0)
        return;

    int    count = tr_get_count();
    double due1 = 0.0, paid1 = 0.0;
    double due2 = 0.0, paid2 = 0.0;
    int    cnt1 = 0, cnt2 = 0;

    for (int i = 0; i < count; i++) {
        const TaxRecord *r = tr_get_at(i);
        if (r->year == year1) {
            due1  += r->tax_due;
            paid1 += r->amount_paid;
            cnt1++;
        } else if (r->year == year2) {
            due2  += r->tax_due;
            paid2 += r->amount_paid;
            cnt2++;
        }
    }

    printf("\n  %-20s  %12s  %12s\n", "Metric",
           "Year 1", "Year 2");
    utils_print_divider();
    printf("  %-20s  %12d  %12d\n",   "Year",       year1,      year2);
    printf("  %-20s  %12d  %12d\n",   "Records",     cnt1,       cnt2);
    printf("  %-20s  %12.2f  %12.2f\n","Total Due",   due1,       due2);
    printf("  %-20s  %12.2f  %12.2f\n","Total Paid",  paid1,      paid2);
    printf("  %-20s  %12.2f  %12.2f\n","Outstanding", due1-paid1, due2-paid2);

    utils_print_divider();
    if (cnt1 > 0 || cnt2 > 0) {
        if (paid1 <= 0.0) {
            if (paid2 > 0.0)
                printf("  Revenue change from %d to %d: N/A "
                       "(no revenue in %d)\n", year1, year2, year1);
            else
                printf("  Revenue change from %d to %d: N/A "
                       "(no revenue in either year)\n", year1, year2);
        } else {
            double change = ((paid2 - paid1) / paid1) * 100.0;
            printf("  Revenue change from %d to %d: %+.1f%%\n",
                   year1, year2, change);
        }
    }
}

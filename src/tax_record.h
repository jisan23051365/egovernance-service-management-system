#ifndef TAX_RECORD_H
#define TAX_RECORD_H

#include "constants.h"

/* ---------------------------------------------------------------
 * Tax category
 * --------------------------------------------------------------- */
typedef enum {
    TAX_INCOME   = 0,
    TAX_PROPERTY = 1,
    TAX_BUSINESS = 2,
    TAX_VEHICLE  = 3
} TaxType;

#define TAX_TYPE_COUNT 4

/* ---------------------------------------------------------------
 * Payment status
 * --------------------------------------------------------------- */
typedef enum {
    TAX_UNPAID  = 0,
    TAX_PAID    = 1,
    TAX_PARTIAL = 2,
    TAX_WAIVED  = 3
} TaxStatus;

#define TAX_STATUS_COUNT 4

/* ---------------------------------------------------------------
 * Tax record stored in tax_records.dat
 * --------------------------------------------------------------- */
typedef struct {
    int       id;
    int       citizen_id;
    int       year;
    TaxType   type;
    double    taxable_amount; /* income, property value, etc. */
    double    tax_due;        /* total amount owed            */
    double    amount_paid;    /* cumulative payments          */
    TaxStatus status;
    char      due_date[LEN_DATE];
    char      paid_date[LEN_DATE]; /* date last paid / fully paid */
    char      notes[LEN_NOTES];
} TaxRecord;

/* ---------------------------------------------------------------
 * Module lifecycle
 * --------------------------------------------------------------- */
void tr_init(void);

/* ---------------------------------------------------------------
 * Admin operations
 * --------------------------------------------------------------- */

/*
 * Create a new tax record for a citizen.
 * Returns the new record ID (>= 1) on success, -1 on failure.
 */
int  tr_add(int citizen_id, int year, TaxType type,
            double taxable_amount, double tax_due,
            const char *due_date, const char *notes);

/* Print all tax records. */
void tr_display_all(void);

/* ---------------------------------------------------------------
 * Citizen operations
 * --------------------------------------------------------------- */

/* Print tax records belonging to citizen_id. */
void tr_display_citizen(int citizen_id);

/*
 * Record a payment against a tax record.
 * Automatically transitions status to PAID or PARTIAL.
 * Returns 0 on success, -1 on failure.
 */
int  tr_pay(int record_id, double amount);

/* ---------------------------------------------------------------
 * Helpers / string conversion
 * --------------------------------------------------------------- */
const char *tr_type_str(TaxType t);
const char *tr_status_str(TaxStatus s);

/* Total number of stored records. */
int tr_get_count(void);

/* Return a const pointer to the n-th record (0-based). */
const TaxRecord *tr_get_at(int index);

#endif /* TAX_RECORD_H */

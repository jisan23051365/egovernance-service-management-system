#ifndef TAX_ANALYTICS_H
#define TAX_ANALYTICS_H

/*
 * Tax Analytics Module
 *
 * All functions read data from the in-memory tax_record and auth
 * modules (which must be initialised before calling these).
 */

/* Total tax collected vs. outstanding, broken down by year. */
void analytics_summary_by_year(void);

/* Revenue breakdown by tax type (Income / Property / Business / Vehicle). */
void analytics_by_type(void);

/* List citizens who have at least one unpaid or partially-paid record. */
void analytics_delinquent(void);

/* Side-by-side comparison of tax revenue for two fiscal years. */
void analytics_year_comparison(void);

#endif /* TAX_ANALYTICS_H */

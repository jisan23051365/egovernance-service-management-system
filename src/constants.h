#ifndef CONSTANTS_H
#define CONSTANTS_H

/* ---------------------------------------------------------------
 * Data directory and file paths
 * --------------------------------------------------------------- */
#define DATA_DIR               "data"
#define USERS_FILE             "data/users.dat"
#define SERVICE_REQUESTS_FILE  "data/service_requests.dat"
#define TAX_RECORDS_FILE       "data/tax_records.dat"

/* ---------------------------------------------------------------
 * String buffer sizes (including null terminator)
 * --------------------------------------------------------------- */
#define LEN_USERNAME     51
#define LEN_HASH         17
#define LEN_NAME        101
#define LEN_EMAIL       101
#define LEN_PHONE        21
#define LEN_ADDRESS     201
#define LEN_DESC        501
#define LEN_NOTES       201
#define LEN_DATE         25

/* ---------------------------------------------------------------
 * Maximum record counts for static storage
 * --------------------------------------------------------------- */
#define MAX_USERS        500
#define MAX_REQUESTS    2000
#define MAX_TAX_RECORDS 2000

/* ---------------------------------------------------------------
 * Default administrator credentials (change after first login)
 * --------------------------------------------------------------- */
#define DEFAULT_ADMIN_USERNAME  "admin"
#define DEFAULT_ADMIN_PASSWORD  "Admin@123"
#define DEFAULT_ADMIN_NAME      "System Administrator"
#define DEFAULT_ADMIN_EMAIL     "admin@egovernance.gov"

#endif /* CONSTANTS_H */

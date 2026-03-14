#ifndef SERVICE_REQUEST_H
#define SERVICE_REQUEST_H

#include "constants.h"

/* ---------------------------------------------------------------
 * Service type catalogue
 * --------------------------------------------------------------- */
typedef enum {
    SVC_BIRTH_CERTIFICATE    = 0,
    SVC_DEATH_CERTIFICATE    = 1,
    SVC_MARRIAGE_CERTIFICATE = 2,
    SVC_DRIVING_LICENSE      = 3,
    SVC_PASSPORT             = 4,
    SVC_PROPERTY_REGISTRATION= 5,
    SVC_BUSINESS_LICENSE     = 6,
    SVC_UTILITY_CONNECTION   = 7,
    SVC_OTHER                = 8
} ServiceType;

#define SERVICE_TYPE_COUNT 9

/* ---------------------------------------------------------------
 * Request lifecycle status
 * --------------------------------------------------------------- */
typedef enum {
    REQ_PENDING     = 0,
    REQ_IN_PROGRESS = 1,
    REQ_APPROVED    = 2,
    REQ_REJECTED    = 3
} RequestStatus;

#define REQUEST_STATUS_COUNT 4

/* ---------------------------------------------------------------
 * Service request record stored in service_requests.dat
 * --------------------------------------------------------------- */
typedef struct {
    int           id;
    int           citizen_id;
    ServiceType   type;
    char          description[LEN_DESC];
    RequestStatus status;
    char          created_date[LEN_DATE];
    char          updated_date[LEN_DATE];
    char          admin_notes[LEN_NOTES];
} ServiceRequest;

/* ---------------------------------------------------------------
 * Module lifecycle
 * --------------------------------------------------------------- */
void sr_init(void);

/* ---------------------------------------------------------------
 * Citizen operations
 * --------------------------------------------------------------- */

/*
 * Submit a new service request.
 * Returns the new request ID (>= 1) on success, -1 on failure.
 */
int  sr_submit(int citizen_id, ServiceType type,
               const char *description);

/* Print a table of all requests belonging to citizen_id. */
void sr_display_citizen(int citizen_id);

/* ---------------------------------------------------------------
 * Admin operations
 * --------------------------------------------------------------- */

/* Print a table of all service requests. */
void sr_display_all(void);

/*
 * Update a request's status and optionally add admin notes.
 * Returns 0 on success, -1 if the request ID was not found.
 */
int  sr_update_status(int request_id, RequestStatus new_status,
                      const char *notes);

/* ---------------------------------------------------------------
 * Helpers / string conversion
 * --------------------------------------------------------------- */
const char *sr_type_str(ServiceType t);
const char *sr_status_str(RequestStatus s);

/* Total number of stored requests. */
int sr_get_count(void);

/* Return a const pointer to the n-th request (0-based). */
const ServiceRequest *sr_get_at(int index);

#endif /* SERVICE_REQUEST_H */

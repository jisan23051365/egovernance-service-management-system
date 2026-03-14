#include <stdio.h>
#include <string.h>

#include "service_request.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* Module-level storage                                               */
/* ------------------------------------------------------------------ */

static ServiceRequest requests[MAX_REQUESTS];
static int            req_count = 0;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                   */
/* ------------------------------------------------------------------ */

static void save_requests(void)
{
    FILE *fp = fopen(SERVICE_REQUESTS_FILE, "wb");
    if (!fp) { perror("Cannot save service requests"); return; }

    fwrite(&req_count, sizeof(int), 1, fp);
    fwrite(requests, sizeof(ServiceRequest), (size_t)req_count, fp);
    fclose(fp);
}

static void load_requests(void)
{
    FILE *fp = fopen(SERVICE_REQUESTS_FILE, "rb");
    if (!fp) { req_count = 0; return; }

    if (fread(&req_count, sizeof(int), 1, fp) != 1 ||
        req_count < 0 || req_count > MAX_REQUESTS) {
        req_count = 0;
        fclose(fp);
        return;
    }

    if ((int)fread(requests, sizeof(ServiceRequest),
                   (size_t)req_count, fp) != req_count) {
        req_count = 0;
    }

    fclose(fp);
}

static int next_id(void)
{
    int max = 0;
    for (int i = 0; i < req_count; i++) {
        if (requests[i].id > max) max = requests[i].id;
    }
    return max + 1;
}

/* ------------------------------------------------------------------ */
/* String helpers                                                     */
/* ------------------------------------------------------------------ */

const char *sr_type_str(ServiceType t)
{
    static const char *names[SERVICE_TYPE_COUNT] = {
        "Birth Certificate",
        "Death Certificate",
        "Marriage Certificate",
        "Driving License",
        "Passport",
        "Property Registration",
        "Business License",
        "Utility Connection",
        "Other"
    };
    if (t < 0 || t >= SERVICE_TYPE_COUNT) return "Unknown";
    return names[t];
}

const char *sr_status_str(RequestStatus s)
{
    static const char *names[REQUEST_STATUS_COUNT] = {
        "Pending", "In Progress", "Approved", "Rejected"
    };
    if (s < 0 || s >= REQUEST_STATUS_COUNT) return "Unknown";
    return names[s];
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

void sr_init(void)
{
    load_requests();
}

int sr_submit(int citizen_id, ServiceType type,
              const char *description)
{
    if (req_count >= MAX_REQUESTS) {
        puts("  Error: Request limit reached.");
        return -1;
    }

    ServiceRequest r;
    memset(&r, 0, sizeof(r));

    r.id         = next_id();
    r.citizen_id = citizen_id;
    r.type       = type;
    strncpy(r.description, description ? description : "", LEN_DESC - 1);
    r.status = REQ_PENDING;
    utils_get_date(r.created_date, LEN_DATE);
    strncpy(r.updated_date, r.created_date, LEN_DATE - 1);

    requests[req_count++] = r;
    save_requests();
    return r.id;
}

void sr_display_citizen(int citizen_id)
{
    printf("\n  %-5s  %-24s  %-14s  %-16s\n",
           "ID", "Service Type", "Status", "Date");
    utils_print_divider();

    int found = 0;
    for (int i = 0; i < req_count; i++) {
        if (requests[i].citizen_id != citizen_id) continue;
        printf("  %-5d  %-24.24s  %-14.14s  %-16.16s\n",
               requests[i].id,
               sr_type_str(requests[i].type),
               sr_status_str(requests[i].status),
               requests[i].created_date);
        found++;
    }

    if (!found) printf("  You have no service requests yet.\n");
    printf("\n  Total: %d request(s)\n", found);
}

void sr_display_all(void)
{
    printf("\n  %-5s  %-8s  %-24s  %-14s  %-16s\n",
           "ID", "Cit.ID", "Service Type", "Status", "Date");
    utils_print_divider();

    for (int i = 0; i < req_count; i++) {
        printf("  %-5d  %-8d  %-24.24s  %-14.14s  %-16.16s\n",
               requests[i].id,
               requests[i].citizen_id,
               sr_type_str(requests[i].type),
               sr_status_str(requests[i].status),
               requests[i].created_date);
    }

    if (!req_count) printf("  No service requests on record.\n");
    printf("\n  Total: %d request(s)\n", req_count);
}

int sr_update_status(int request_id, RequestStatus new_status,
                     const char *notes)
{
    for (int i = 0; i < req_count; i++) {
        if (requests[i].id != request_id) continue;

        requests[i].status = new_status;
        if (notes && *notes)
            strncpy(requests[i].admin_notes, notes, LEN_NOTES - 1);
        utils_get_date(requests[i].updated_date, LEN_DATE);

        save_requests();
        return 0;
    }

    puts("  Error: Request ID not found.");
    return -1;
}

int sr_get_count(void)
{
    return req_count;
}

const ServiceRequest *sr_get_at(int index)
{
    if (index < 0 || index >= req_count) return NULL;
    return &requests[index];
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* =========================================================
   GOVERNMENT E-GOVERNANCE SERVICE MANAGEMENT SYSTEM
   ========================================================= */

/* ===================== STRUCTURES ===================== */

typedef struct {
    int id;
    char username[30];
    unsigned long password_hash;
} User;

typedef struct {
    int citizen_id;
    char name[50];
    char address[100];
    char service_type[50];
    char status[20];   // Pending / Approved / Rejected
} ServiceRequest;

typedef struct {
    int tax_id;
    int citizen_id;
    float income;
    float tax_amount;
} TaxRecord;

/* ===================== UTILITIES ===================== */

unsigned long hashPassword(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void logActivity(const char *msg) {
    FILE *fp = fopen("egov_logs.txt", "a");
    if (!fp) return;
    time_t now = time(NULL);
    fprintf(fp, "%s - %s\n", ctime(&now), msg);
    fprintf(fp, "--------------------------------\n");
    fclose(fp);
}

/* ===================== AUTH ===================== */

void initializeAdmin() {
    FILE *fp = fopen("egov_users.dat", "rb");
    if (fp) { fclose(fp); return; }

    fp = fopen("egov_users.dat", "wb");
    User admin = {1, "admin", hashPassword("admin123")};
    fwrite(&admin, sizeof(User), 1, fp);
    fclose(fp);
}

int login() {
    char username[30], password[30];
    unsigned long hash;
    User u;

    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    hash = hashPassword(password);

    FILE *fp = fopen("egov_users.dat", "rb");
    if (!fp) return 0;

    while (fread(&u, sizeof(User), 1, fp)) {
        if (strcmp(u.username, username) == 0 && u.password_hash == hash) {
            fclose(fp);
            logActivity("Admin login successful.");
            return 1;
        }
    }

    fclose(fp);
    printf("Invalid credentials.\n");
    return 0;
}

/* ===================== SERVICE REQUEST ===================== */

void submitRequest() {
    FILE *fp = fopen("service_requests.dat", "ab");
    if (!fp) return;

    ServiceRequest sr;
    printf("Citizen ID: "); scanf("%d", &sr.citizen_id);
    printf("Name: "); scanf(" %[^\n]", sr.name);
    printf("Address: "); scanf(" %[^\n]", sr.address);
    printf("Service Type: "); scanf(" %[^\n]", sr.service_type);
    strcpy(sr.status, "Pending");

    fwrite(&sr, sizeof(ServiceRequest), 1, fp);
    fclose(fp);
    logActivity("Service request submitted.");
}

void viewRequests() {
    FILE *fp = fopen("service_requests.dat", "rb");
    if (!fp) return;

    ServiceRequest sr;
    printf("\n--- Service Requests ---\n");
    while (fread(&sr, sizeof(ServiceRequest), 1, fp)) {
        printf("Citizen:%d | %s | Service:%s | Status:%s\n",
               sr.citizen_id, sr.name, sr.service_type, sr.status);
    }
    fclose(fp);
}

void updateRequestStatus() {
    FILE *fp = fopen("service_requests.dat", "rb+");
    if (!fp) return;

    int id;
    char new_status[20];
    ServiceRequest sr;

    printf("Citizen ID: "); scanf("%d", &id);
    printf("New Status (Approved/Rejected): "); scanf("%s", new_status);

    while (fread(&sr, sizeof(ServiceRequest), 1, fp)) {
        if (sr.citizen_id == id) {
            strcpy(sr.status, new_status);
            fseek(fp, -sizeof(ServiceRequest), SEEK_CUR);
            fwrite(&sr, sizeof(ServiceRequest), 1, fp);
            printf("Status updated.\n");
            logActivity("Service request status updated.");
            break;
        }
    }

    fclose(fp);
}

/* ===================== TAX MODULE ===================== */

void calculateTax() {
    FILE *fp = fopen("tax_records.dat", "ab");
    if (!fp) return;

    TaxRecord tr;
    printf("Tax Record ID: "); scanf("%d", &tr.tax_id);
    printf("Citizen ID: "); scanf("%d", &tr.citizen_id);
    printf("Annual Income: "); scanf("%f", &tr.income);

    tr.tax_amount = tr.income * 0.15;

    fwrite(&tr, sizeof(TaxRecord), 1, fp);
    fclose(fp);

    printf("Calculated Tax: %.2f\n", tr.tax_amount);
    logActivity("Tax calculated.");
}

/* ===================== ANALYTICS ===================== */

void taxAnalytics() {
    FILE *fp = fopen("tax_records.dat", "rb");
    if (!fp) return;

    TaxRecord tr;
    float arr[500];
    int n = 0;

    while (fread(&tr, sizeof(TaxRecord), 1, fp) && n < 500) {
        arr[n++] = tr.tax_amount;
    }
    fclose(fp);

    if (n == 0) {
        printf("No tax data.\n");
        return;
    }

    float sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    float mean = sum / n;

    float variance = 0;
    for (int i = 0; i < n; i++)
        variance += pow(arr[i] - mean, 2);
    variance /= n;

    float std = sqrt(variance);

    printf("\n--- Tax Analytics ---\n");
    printf("Total Records: %d\n", n);
    printf("Average Tax: %.2f\n", mean);
    printf("Variance: %.2f\n", variance);
    printf("Standard Deviation: %.2f\n", std);

    logActivity("Tax analytics generated.");
}

/* ===================== MAIN ===================== */

int main() {
    initializeAdmin();

    printf("=== E-GOVERNANCE SERVICE SYSTEM ===\n");

    if (!login()) return 0;

    int choice;

    while (1) {
        printf("\n1.Submit Service Request\n");
        printf("2.View Requests\n");
        printf("3.Update Request Status\n");
        printf("4.Calculate Tax\n");
        printf("5.Tax Analytics\n");
        printf("6.Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: submitRequest(); break;
            case 2: viewRequests(); break;
            case 3: updateRequestStatus(); break;
            case 4: calculateTax(); break;
            case 5: taxAnalytics(); break;
            case 6:
                logActivity("System exited.");
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}
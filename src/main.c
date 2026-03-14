/*
 * main.c — E-Governance Service Management System
 *
 * Entry point and interactive CLI menus for both citizen and
 * administrator portals.
 */

#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "utils.h"
#include "auth.h"
#include "service_request.h"
#include "tax_record.h"
#include "tax_analytics.h"

/* ================================================================== */
/* Forward declarations                                               */
/* ================================================================== */
static void menu_main(void);
static void menu_register(void);
static void menu_citizen(User *u);
static void menu_admin(User *u);

/* Citizen sub-menus */
static void citizen_submit_request(User *u);
static void citizen_view_requests(User *u);
static void citizen_view_tax(User *u);
static void citizen_pay_tax(User *u);
static void citizen_change_password(User *u);

/* Admin sub-menus */
static void admin_view_all_requests(void);
static void admin_update_request(void);
static void admin_add_tax_record(void);
static void admin_view_all_tax(void);
static void admin_view_citizens(void);
static void admin_deactivate_citizen(void);
static void admin_change_password(User *u);

/* ================================================================== */
/* Main menu (unauthenticated)                                        */
/* ================================================================== */

static void menu_main(void)
{
    for (;;) {
        utils_clear_screen();
        utils_print_header("E-GOVERNANCE SERVICE MANAGEMENT SYSTEM");
        printf("\n  1. Login\n");
        printf("  2. Register as Citizen\n");
        printf("  3. Exit\n\n");

        int choice;
        utils_get_int("  Select option: ", &choice, 1, 3);

        switch (choice) {
        case 1: {
            utils_clear_screen();
            utils_print_header("LOGIN");

            char username[LEN_USERNAME];
            char password[LEN_USERNAME]; /* same max length */

            utils_get_string("  Username: ", username, sizeof(username));
            utils_get_password("  Password: ", password, sizeof(password));

            User u;
            if (auth_login(username, password, &u)) {
                printf("\n  Welcome, %s!\n", u.name);
                utils_press_enter();
                if (u.role == ROLE_ADMIN)
                    menu_admin(&u);
                else
                    menu_citizen(&u);
            } else {
                puts("\n  Login failed. Invalid username or password.");
                utils_press_enter();
            }
            break;
        }
        case 2:
            menu_register();
            break;
        case 3:
            utils_clear_screen();
            puts("  Thank you for using the E-Governance System. Goodbye!");
            return;
        }
    }
}

/* ================================================================== */
/* Registration                                                       */
/* ================================================================== */

static void menu_register(void)
{
    utils_clear_screen();
    utils_print_header("CITIZEN REGISTRATION");

    char username[LEN_USERNAME];
    char password[LEN_USERNAME];
    char confirm[LEN_USERNAME];
    char name[LEN_NAME];
    char email[LEN_EMAIL];
    char phone[LEN_PHONE];
    char address[LEN_ADDRESS];

    utils_get_string("  Username      : ", username, sizeof(username));
    utils_get_password("  Password      : ", password, sizeof(password));
    utils_get_password("  Confirm Pass  : ", confirm,  sizeof(confirm));

    if (strcmp(password, confirm) != 0) {
        puts("\n  Error: Passwords do not match.");
        utils_press_enter();
        return;
    }

    utils_get_string("  Full Name     : ", name,    sizeof(name));
    utils_get_string("  Email         : ", email,   sizeof(email));
    utils_get_string("  Phone         : ", phone,   sizeof(phone));
    utils_get_string("  Address       : ", address, sizeof(address));

    int id = auth_register(username, password, name, email, phone, address);
    if (id > 0)
        printf("\n  Registration successful! Your citizen ID is %d.\n", id);
    utils_press_enter();
}

/* ================================================================== */
/* Citizen portal                                                     */
/* ================================================================== */

static void menu_citizen(User *u)
{
    for (;;) {
        utils_clear_screen();
        utils_print_header("CITIZEN PORTAL");
        printf("\n  Welcome, %s  (ID: %d)\n\n", u->name, u->id);
        printf("  1. Submit Service Request\n");
        printf("  2. View My Service Requests\n");
        printf("  3. View My Tax Records\n");
        printf("  4. Pay Tax\n");
        printf("  5. Change Password\n");
        printf("  6. Logout\n\n");

        int choice;
        utils_get_int("  Select option: ", &choice, 1, 6);

        switch (choice) {
        case 1: citizen_submit_request(u);  break;
        case 2: citizen_view_requests(u);   break;
        case 3: citizen_view_tax(u);        break;
        case 4: citizen_pay_tax(u);         break;
        case 5: citizen_change_password(u); break;
        case 6: return;
        }
    }
}

static void citizen_submit_request(User *u)
{
    utils_clear_screen();
    utils_print_header("SUBMIT SERVICE REQUEST");

    printf("\n  Select service type:\n");
    for (int i = 0; i < SERVICE_TYPE_COUNT; i++)
        printf("  %2d. %s\n", i + 1, sr_type_str((ServiceType)i));
    printf("\n");

    int type_choice;
    utils_get_int("  Service type [1-%d]: ", &type_choice,
                  1, SERVICE_TYPE_COUNT);

    char description[LEN_DESC];
    utils_get_string("  Description   : ", description, sizeof(description));

    int id = sr_submit(u->id, (ServiceType)(type_choice - 1), description);
    if (id > 0)
        printf("\n  Request #%d submitted successfully. Status: Pending\n", id);
    utils_press_enter();
}

static void citizen_view_requests(User *u)
{
    utils_clear_screen();
    utils_print_header("MY SERVICE REQUESTS");
    sr_display_citizen(u->id);
    utils_press_enter();
}

static void citizen_view_tax(User *u)
{
    utils_clear_screen();
    utils_print_header("MY TAX RECORDS");
    tr_display_citizen(u->id);
    utils_press_enter();
}

static void citizen_pay_tax(User *u)
{
    utils_clear_screen();
    utils_print_header("PAY TAX");

    tr_display_citizen(u->id);

    int record_id;
    if (utils_get_int("\n  Enter Record ID to pay (0 to cancel): ",
                      &record_id, 0, 99999) < 0 || record_id == 0) {
        return;
    }

    double amount;
    if (utils_get_double("  Amount to pay : ", &amount,
                         0.01, 9999999.99) < 0) {
        return;
    }

    tr_pay(record_id, amount);
    utils_press_enter();
}

static void citizen_change_password(User *u)
{
    utils_clear_screen();
    utils_print_header("CHANGE PASSWORD");

    char old_pw[LEN_USERNAME];
    char new_pw[LEN_USERNAME];
    char confirm[LEN_USERNAME];

    utils_get_password("  Current password : ", old_pw,  sizeof(old_pw));
    utils_get_password("  New password      : ", new_pw,  sizeof(new_pw));
    utils_get_password("  Confirm new pass  : ", confirm, sizeof(confirm));

    if (strcmp(new_pw, confirm) != 0) {
        puts("\n  Error: New passwords do not match.");
        utils_press_enter();
        return;
    }

    if (auth_change_password(u->id, old_pw, new_pw) == 0)
        puts("\n  Password changed successfully.");
    utils_press_enter();
}

/* ================================================================== */
/* Admin portal                                                       */
/* ================================================================== */

static void menu_admin(User *u)
{
    for (;;) {
        utils_clear_screen();
        utils_print_header("ADMIN PORTAL");
        printf("\n  Logged in as: %s\n\n", u->name);

        printf("  --- Service Requests ---\n");
        printf("   1. View All Service Requests\n");
        printf("   2. Update Request Status\n\n");

        printf("  --- Tax Records ---\n");
        printf("   3. Add Tax Record\n");
        printf("   4. View All Tax Records\n\n");

        printf("  --- Analytics ---\n");
        printf("   5. Tax Summary by Year\n");
        printf("   6. Tax Revenue by Type\n");
        printf("   7. Delinquent Taxpayers Report\n");
        printf("   8. Year-over-Year Comparison\n\n");

        printf("  --- Citizens ---\n");
        printf("   9. View All Citizens\n");
        printf("  10. Deactivate Citizen Account\n\n");

        printf("  --- Account ---\n");
        printf("  11. Change Password\n");
        printf("  12. Logout\n\n");

        int choice;
        utils_get_int("  Select option: ", &choice, 1, 12);

        switch (choice) {
        case  1: admin_view_all_requests();   break;
        case  2: admin_update_request();      break;
        case  3: admin_add_tax_record();      break;
        case  4: admin_view_all_tax();        break;
        case  5:
            utils_clear_screen();
            analytics_summary_by_year();
            utils_press_enter();
            break;
        case  6:
            utils_clear_screen();
            analytics_by_type();
            utils_press_enter();
            break;
        case  7:
            utils_clear_screen();
            analytics_delinquent();
            utils_press_enter();
            break;
        case  8:
            utils_clear_screen();
            analytics_year_comparison();
            utils_press_enter();
            break;
        case  9: admin_view_citizens();       break;
        case 10: admin_deactivate_citizen();  break;
        case 11: admin_change_password(u);    break;
        case 12: return;
        }
    }
}

static void admin_view_all_requests(void)
{
    utils_clear_screen();
    utils_print_header("ALL SERVICE REQUESTS");
    sr_display_all();
    utils_press_enter();
}

static void admin_update_request(void)
{
    utils_clear_screen();
    utils_print_header("UPDATE REQUEST STATUS");

    sr_display_all();

    int req_id;
    if (utils_get_int("\n  Enter Request ID to update (0 to cancel): ",
                      &req_id, 0, 99999) < 0 || req_id == 0) {
        return;
    }

    printf("\n  New status:\n");
    for (int i = 0; i < REQUEST_STATUS_COUNT; i++)
        printf("  %d. %s\n", i + 1, sr_status_str((RequestStatus)i));

    int status_choice;
    utils_get_int("\n  Select status [1-%d]: ", &status_choice,
                  1, REQUEST_STATUS_COUNT);

    char notes[LEN_NOTES];
    utils_get_string("  Admin notes (optional): ", notes, sizeof(notes));

    if (sr_update_status(req_id,
                         (RequestStatus)(status_choice - 1),
                         notes) == 0) {
        puts("\n  Request status updated successfully.");
    }
    utils_press_enter();
}

static void admin_add_tax_record(void)
{
    utils_clear_screen();
    utils_print_header("ADD TAX RECORD");

    int citizen_id;
    utils_get_int("  Citizen ID      : ", &citizen_id, 1, 99999);

    User u;
    if (!auth_get_user(citizen_id, &u)) {
        puts("  Error: Citizen not found.");
        utils_press_enter();
        return;
    }
    printf("  Citizen name    : %s\n", u.name);

    int year;
    utils_get_int("  Tax year        : ", &year, 1900, 9999);

    printf("\n  Tax type:\n");
    for (int i = 0; i < TAX_TYPE_COUNT; i++)
        printf("  %d. %s\n", i + 1, tr_type_str((TaxType)i));

    int type_choice;
    utils_get_int("\n  Select type [1-%d]: ", &type_choice, 1, TAX_TYPE_COUNT);

    double taxable_amount;
    utils_get_double("  Taxable amount  : ", &taxable_amount,
                     0.0, 999999999.99);

    double tax_due;
    utils_get_double("  Tax due         : ", &tax_due, 0.0, 999999999.99);

    char due_date[LEN_DATE];
    utils_get_string("  Due date        : ", due_date, sizeof(due_date));

    char notes[LEN_NOTES];
    utils_get_string("  Notes (optional): ", notes, sizeof(notes));

    int id = tr_add(citizen_id, year,
                    (TaxType)(type_choice - 1),
                    taxable_amount, tax_due, due_date, notes);
    if (id > 0)
        printf("\n  Tax record #%d created successfully.\n", id);
    utils_press_enter();
}

static void admin_view_all_tax(void)
{
    utils_clear_screen();
    utils_print_header("ALL TAX RECORDS");
    tr_display_all();
    utils_press_enter();
}

static void admin_view_citizens(void)
{
    utils_clear_screen();
    utils_print_header("REGISTERED CITIZENS");
    auth_display_citizens();
    utils_press_enter();
}

static void admin_deactivate_citizen(void)
{
    utils_clear_screen();
    utils_print_header("DEACTIVATE CITIZEN ACCOUNT");

    auth_display_citizens();

    int citizen_id;
    if (utils_get_int("\n  Enter Citizen ID to deactivate (0 to cancel): ",
                      &citizen_id, 0, 99999) < 0 || citizen_id == 0) {
        return;
    }

    if (auth_deactivate(citizen_id) == 0)
        puts("\n  Citizen account deactivated.");
    utils_press_enter();
}

static void admin_change_password(User *u)
{
    utils_clear_screen();
    utils_print_header("CHANGE ADMIN PASSWORD");

    char old_pw[LEN_USERNAME];
    char new_pw[LEN_USERNAME];
    char confirm[LEN_USERNAME];

    utils_get_password("  Current password : ", old_pw,  sizeof(old_pw));
    utils_get_password("  New password      : ", new_pw,  sizeof(new_pw));
    utils_get_password("  Confirm new pass  : ", confirm, sizeof(confirm));

    if (strcmp(new_pw, confirm) != 0) {
        puts("\n  Error: New passwords do not match.");
        utils_press_enter();
        return;
    }

    if (auth_change_password(u->id, old_pw, new_pw) == 0)
        puts("\n  Password changed successfully.");
    utils_press_enter();
}

/* ================================================================== */
/* Entry point                                                        */
/* ================================================================== */

int main(void)
{
    /* Ensure data directory exists */
    if (utils_ensure_dir(DATA_DIR) != 0) {
        fprintf(stderr, "Fatal: Cannot create data directory '%s'.\n",
                DATA_DIR);
        return 1;
    }

    /* Initialise all modules (load data from disk) */
    auth_init();
    sr_init();
    tr_init();

    menu_main();
    return 0;
}

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h> 
#include <time.h>
#include <ctype.h>

#define RESET ""
#define RED ""
#define YELLOW ""
#define CYAN ""
#define GREEN ""
#define MAGENTA ""
#define BLUE ""
#define BOLD ""
#define DIM ""

#define BACK_SPACE 8
#define ENTER_KEY 13
#define MAX_ATTEMPTS 5
#define TOTAL_SEAT 24
#define MAX_BUSES 50
#define MAX_CUSTOMERS 200
// Function dec for loading

void cat_loading_animation();


// Function declarations for menues
int handle_login();
void main_menu();
void book_ticket();
void view_bookings();
void cancel_booking();
void show_lockout_page();
void print_header(const char *title);

int EDIT_();
void ADD_NEW_BUS();
void DISPLAY_BUSES();
void DISPLAY_CUSTOMERS();
void DISPLAY_HISTORY(); // shows bookings that were archived when their bus expired

//admin functions for editing/deleting buses and a sales summary
void EDIT_BUS();
void DELETE_BUS();
void SALES_REPORT();


//typedef allows me to call this struct without having to type struct again and again
typedef struct{
    char ID[20]; //its short for number plate
    char des[20]; //its short for destination the bus gonna reach
    int date; //The date this bus travels on, stored as YYYYMMDD (e.g. 20260708)
    int T; //its for time i didnt write time cause im gonna use it somewhere else
    float fare; //ticket price for this bus
    int seats[TOTAL_SEAT]; //uses the TOTAL_SEAT constant for changeable seat size
}BUS;

typedef struct{//need this struct to ensure that that same person doesnt get booked in two places at once
    long long phon; //for phone number long long is a int type variable for long intigers string
    char name[30]; //passenger name, needed for a real ticket/receipt instead of just a phone number
    int B_ID; //for the specific id a user gets
    int bookDate; // date the user booked the ticket on, stored as YYYYMMDD (e.g. 20260708) so we can break down sales by day
}Cus; //Cus means customers 

int load_all_buses(BUS allBuses[], int maxBuses);
int read_int_line(int *out);
int read_llong_line(long long *out);
int read_phone_line(long long *out); // like read_llong_line but forces EXACTLY 10 digits
int read_float_line(float *out);
void get_console_size(int *cols, int *rows); // detects actual terminal window size
void print_centered(int cols, const char *text, const char *colorCode); // centers a line within the given width
void print_menu_box(const char *title, const char *options[], int count, const char *backLabel); // backLabel may be NULL to omit the [0] row
void print_success(const char *msg);
void print_error(const char *msg);
int contains_pipe(const char *s); 
int dest_matches(const char *busDest, const char *searchTerm); 
int get_next_ticket_id(); //hands out a unique ticket/PNR number for every booking

int valid_time_format(int t); 
int is_duplicate_bus(BUS allBuses[], int busCount, const char *plate, int date, int time); 
void save_ticket_receipt(int ticketID, const char *name, long long phone, const char *plate, const char *dest, int deptTime, int seatNum, float fare, struct tm *bookLt); // writes a standalone receipt file the clerk can reprint/refile later
void cleanup_expired_buses(); // removes buses (and their bookings) whose travel date is before today, since they've already departed

int main() 
{
    // Sets the whole console window to a soft light-grey background with
    // black text (easier on the eyes than pure bright white).
    // "color 70" -> 7 = background (light grey), 0 = foreground (black).
    system("color 70");

    // System Loading Animation not quiter necessary
    // we can remove if we want cause it does not affect the code but just give a loading effect
    
    cat_loading_animation();
    
    
    //the while loop below will continue uless the username and password are correct
    //Even if the program goes to lockdown due to failed attempts it still runs from here
    //How I inted it to work is when the program goes on lockdown and time runs out it sends value 0 then !0 means 1 which says its true
    //So the loop runs When the password and username is correct i put return value 1 then !1 means 0 which says its false
    //Thus closing the loop
    while (!handle_login()) {
    }
    //I had to do this cause of bleeding memory it may seems unrelated but it could give problems to the user in future
    // Proceed to menu safely once authorized
    cleanup_expired_buses(); // wipe out any bus (and its bookings) that already departed on a previous day
    main_menu(); 
    return 0;
}
//I used int as I want it to return a value
int handle_login() 
{
    char username[20];
    // int can only safely hold up to ~9 digits before overflowing
    // long long gives us space for much bigger numbers safely
    long long password;
    int attempt_count = 0;

    const char *busLogo[] = {
        "   ______________   ",
        "  |  ____________|__ ",
        "  | |  __  __  __|  \\",
        "  |_|_[]_[]_[]_[]___|",
        "     (o)        (o)  "
    };

    while (attempt_count < MAX_ATTEMPTS) {
        system("cls");

        int cols, rows;
        get_console_size(&cols, &rows);

        int boxWidth = 50;
        if (boxWidth > cols - 6) boxWidth = cols - 6;
        int margin = (cols - boxWidth - 2) / 2;
        if (margin < 0) margin = 0;
        if (margin > 190) margin = 190;
        char pad[200];
        memset(pad, ' ', margin);
        pad[margin] = '\0';

        int topPad = (rows - 18) / 2;
        if (topPad < 0) topPad = 0;
        for (int i = 0; i < topPad; i++) printf("\n");

        print_header("BUS TICKET BOOKING RECORDER");
        printf("\n");
        for (int i = 0; i < 5; i++) print_centered(cols, busLogo[i], CYAN);
        printf("\n");

        printf("%s" MAGENTA "+", pad);
        for (int i = 0; i < boxWidth; i++) printf("-");
        printf("+\n" RESET);
        printf("%s" MAGENTA "|" RESET BOLD "  %-*s" RESET MAGENTA "|\n" RESET, pad, boxWidth - 2, "LOGIN");
        printf("%s" MAGENTA "+", pad);
        for (int i = 0; i < boxWidth; i++) printf("-");
        printf("+\n" RESET);

        printf("%s" MAGENTA "|  " RESET CYAN "Username: " RESET, pad);
        char userInput[64];
        if (fgets(userInput, sizeof(userInput), stdin) == NULL) {
            userInput[0] = '\0';
        }
        userInput[strcspn(userInput, "\n")] = 0; // strip trailing newline
        strncpy(username, userInput, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';

        //prevent burning one of the 5 attempts on an accidental blank Enter press -
        //just re-ask for the username instead of treating it as a real failed login
        if (strlen(username) == 0) {
            printf("%s" MAGENTA "+", pad);
            for (int i = 0; i < boxWidth; i++) printf("-");
            printf("+\n" RESET "\n");
            print_error("Username cannot be blank. Press any key to try again...");
            getch();
            continue;
        }

        printf("%s" MAGENTA "|  " RESET CYAN "Password: " RESET, pad);
        password = 0;
        int digits_entered = 0;
        char ch;

        while (1) {
            ch = getch();
            if (ch == ENTER_KEY) { 
                break;
            } else if (ch == BACK_SPACE) { 
                if (digits_entered > 0) {
                    digits_entered--;
                    password = password / 10;
                    printf("\b \b"); 
                }
            } else if (ch >= '0' && ch <= '9') { 
                password = (password * 10) + (ch - '0'); 
                digits_entered++;
                printf("*"); 
            }
        }
        printf("\n");
        printf("%s" MAGENTA "+", pad);
        for (int i = 0; i < boxWidth; i++) printf("-");
        printf("+\n" RESET);

        if (strcmp(username, "ADMINp") == 0 && password == 1234) {
            system("cls");
            get_console_size(&cols, &rows);
            int vPad = (rows - 4) / 2;
            if (vPad < 0) vPad = 0;
            for (int i = 0; i < vPad; i++) printf("\n");
            print_centered(cols, "ACCESS GRANTED", GREEN BOLD);
            Sleep(1000);
            return 1; // Success
        } 
        
        attempt_count++;
        system("cls");
        get_console_size(&cols, &rows);
        int vPad = (rows - 6) / 2;
        if (vPad < 0) vPad = 0;
        for (int i = 0; i < vPad; i++) printf("\n");
        
        if (strcmp(username, "ADMINp") != 0 && password != 1234) {
            print_centered(cols, "WRONG USERNAME AND PASSWORD", RED);
        } else if (strcmp(username, "ADMINp") != 0) {
            print_centered(cols, "WRONG USERNAME", RED);
        } else {
            print_centered(cols, "WRONG PASSWORD", RED);
        }

        char attemptMsg[40];
        sprintf(attemptMsg, "Remaining attempts: %d", MAX_ATTEMPTS - attempt_count);
        print_centered(cols, attemptMsg, YELLOW);
        getch();
    }

    // If loop ends,if user failed 5 times
    show_lockout_page();
    return 0; //after the waiting time returns user back to loginpage
}

void main_menu() 
{
    int running = 1;
    int choice;
    system("cls");
    while (running) {
        system("cls");

        time_t now = time(NULL);
        struct tm *lt = localtime(&now);

        int cols, rows;
        get_console_size(&cols, &rows);
        int topPad = (rows - 16) / 2; // ~16 lines make up the menu block below
        if (topPad < 0) topPad = 0;
        for (int i = 0; i < topPad; i++) printf("\n");

        print_header("WELCOME TO THE BUS TICKET BOOKING SYSTEM");
        print_centered(cols, "", NULL);
        char dateLine[40];
        sprintf(dateLine, "Date: %04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
        print_centered(cols, dateLine, CYAN);
        printf("\n");

        const char *mainOptions[] = {
            "Book Ticket",
            "View Bookings",
            "Cancel Booking",
            "Admin / Edit"
        };
        print_menu_box("MAIN MENU", mainOptions, 4, "Exit");
        printf("\n");
        {
            int promptLen = (int)strlen("Your Choice: ");
            int promptPad = (cols - promptLen) / 2;
            if (promptPad < 0) promptPad = 0;
            for (int i = 0; i < promptPad; i++) putchar(' ');
            printf(CYAN "Your Choice: " RESET);
        }

        //Using read_int_line instead of scanf("%d", ...) here: scanf skips blank Enter presses
        //and waits forever for an actual digit, which is what was causing the cursor to
        //just keep moving down with every Enter press instead of registering as invalid input.
        if (!read_int_line(&choice)) {
            choice = -1;
        }
        
        switch (choice) {
            case 1: 
             book_ticket();
             break;

            case 2: 
             view_bookings();
             break;
            
            case 3: 
             cancel_booking(); 
             break;
            
            case 4:
             while(!EDIT_()){

             }
             break;

            case 0: 
             running = 0; 
             break;
            
            default: {
                system("cls");
                int c2, r2;
                get_console_size(&c2, &r2);
                print_header("INVALID CHOICE");
                printf("\n");
                print_centered(c2, "Please try again.", RED);
                print_centered(c2, "Press any key to continue...", CYAN);
                getch();
                break;
            }
        }
    }
}

int EDIT_(){
    system("cls");

    int choice;
    int cols, rows;
    get_console_size(&cols, &rows);
    int topPad = (rows - 18) / 2; // ~18 lines make up the menu block below
    if (topPad < 0) topPad = 0;
    for (int i = 0; i < topPad; i++) printf("\n");

    print_header("ADMIN / DEVELOPER OPTIONS");
    printf("\n");

    const char *adminOptions[] = {
        "Add new bus",
        "Display buses",
        "Display customers",
        "Edit a bus",
        "Delete a bus",
        "Sales report",
        "View booking history"
    };
    print_menu_box("ADMIN MENU", adminOptions, 7, "Back");
    printf("\n");
    {
        int promptLen = (int)strlen("Enter your choice: ");
        int promptPad = (cols - promptLen) / 2;
        if (promptPad < 0) promptPad = 0;
        for (int i = 0; i < promptPad; i++) putchar(' ');
        printf(CYAN "Enter your choice: " RESET);
    }

    if (!read_int_line(&choice)) {
        choice = -1;
    }

    switch (choice) {
        case 1:
         ADD_NEW_BUS();
         break;

        case 2:
         DISPLAY_BUSES();
         break;
        
        case 3:
         DISPLAY_CUSTOMERS();
         break;

        case 4:                    
         EDIT_BUS();
         break;

        case 5:                   
         DELETE_BUS();
         break;

        case 6:                   
         SALES_REPORT();
         break;

        case 7:
         DISPLAY_HISTORY();
         break;

        case 0:                   
         printf("Returning..");
         Sleep(250);
         return 1;

        default: {
         system("cls");
         int c2, r2;
         get_console_size(&c2, &r2);
         print_header("INVALID CHOICE");
         printf("\n");
         print_centered(c2, "Please try again.", RED);
         print_centered(c2, "Press any key to continue...", CYAN);
         getch();
         break;
        }
    }
    return 0;
}

void book_ticket() 
{
    system("cls");
    print_header("BOOK A TICKET");

    //Load every bus first (unfiltered) - we'll narrow it down after the user tells us
    //what date and destination they're looking for.
    BUS allBuses[MAX_BUSES]; // load all buses into memory this give the struct we meade seperate spaces to hold their data
    int busCount = load_all_buses(allBuses, MAX_BUSES);

    if (busCount == 0) {
        printf("No buses available right now!\n");
        getch();
        return;
    }

    int searchDate;
    printf("Enter travel date to search (YYYYMMDD, e.g. 20260708, or 0 for any date): ");
    if (!read_int_line(&searchDate)) {
        searchDate = 0;
    }

    printf("Enter destination to search (leave blank for any): ");
    char destSearch[20];
    if (fgets(destSearch, sizeof(destSearch), stdin) == NULL) {
        destSearch[0] = '\0';
    }
    destSearch[strcspn(destSearch, "\n")] = 0;

    //Build a list of just the buses that match both criteria (0 date / blank destination = "any")
    int matchIdx[MAX_BUSES]; // matchIdx[displayed row] = real index into allBuses[]
    int matchCount = 0;
    for (int i = 0; i < busCount; i++) {
        int dateOK = (searchDate == 0 || allBuses[i].date == searchDate);
        int destOK = dest_matches(allBuses[i].des, destSearch);
        if (dateOK && destOK) {
            matchIdx[matchCount] = i;
            matchCount++;
        }
    }

    if (matchCount == 0) {
        printf("\nNo buses found matching that date/destination.\n");
        getch();
        return;
    }

    printf("\n%-5s %-15s %-20s %-10s %-10s %-10s\n", "No.", "Number Plate", "Destination", "Date", "Time", "Fare");
    printf("------------------------------------------------------------------------------------\n");
    for (int k = 0; k < matchCount; k++) {
        int i = matchIdx[k];
        printf("%-5d %-15s %-20s %-10d %-10d %-10.2f\n", k + 1, allBuses[i].ID, allBuses[i].des, allBuses[i].date, allBuses[i].T, allBuses[i].fare);
    }

    //User picks a bus (number is relative to the filtered list shown above)
    int busChoice;
    printf("\nEnter bus number: ");
    if (!read_int_line(&busChoice)) {
        printf("Invalid input!\n");
        getch();
        return;
    }
    busChoice--; // convert to 0-based index just so we dont get erroe in the program cause if user want to select bus 4 which is stored in the array 3 so just sub 1 to make it equal to the address of array so it fits vissually and practically||

    if (busChoice < 0 || busChoice >= matchCount) {
        printf("Invalid choice!\n");
        getch();
        return;
    }
    busChoice = matchIdx[busChoice]; // map the filtered row back to its real index in allBuses[]

    //Block booking if this bus has already departed
    //Departure time is stored as HHMM (e.g. 1430 = 2:30pm), so we build the current time the same way to compare
    {
        time_t now = time(NULL);
        struct tm *lt = localtime(&now);
        int currentTime = (lt->tm_hour * 100) + lt->tm_min; // e.g. 14:35 becomes 1435
        int today = ((lt->tm_year + 1900) * 10000) + ((lt->tm_mon + 1) * 100) + lt->tm_mday; // build today as YYYYMMDD so it can be compared to the bus date

        //only worry about "already departed" if the bus is actually travelling today -
        //comparing HH:MM against a bus that leaves next week doesn't make sense
        if (allBuses[busChoice].date == today && currentTime >= allBuses[busChoice].T) {
            printf("\nSorry, this bus has already departed (departure time: %04d). Booking is not allowed.\n", allBuses[busChoice].T);
            getch();
            return;
        }
    }

    //Show seat grid for chosen bus
    printf("\nSeats for %s to %s:\n\n", allBuses[busChoice].ID, allBuses[busChoice].des);
    printf(GREEN "[##]" RESET " = Available   " RED "[XX]" RESET " = Booked\n\n");
    for (int i = 0; i < TOTAL_SEAT; i++) {
        if (allBuses[busChoice].seats[i] == 0) {
            printf(GREEN "[%2d]" RESET " ", i + 1); // empty seat
        } else {
            printf(RED "[XX]" RESET " "); // booked seat
        }
        if ((i + 1) % 4 == 0) printf("\n"); // 4 seats per row
    }

    //Pick a seat
    int seatChoice;
    printf("\nEnter seat number (1-%d): ", TOTAL_SEAT);
    if (!read_int_line(&seatChoice)) {
        printf("Invalid input!\n");
        getch();
        return;
    }
    seatChoice--; // convert to 0-based index(meaning we take have 24 seats but in array maximum int it take is 23)||
    //so we sub 1 value to make it real like if the user type 24 we get error so we just - then get the 24 seat which address is stored by array 23||

    if (seatChoice < 0 || seatChoice >= TOTAL_SEAT || allBuses[busChoice].seats[seatChoice] == 1) {
        printf("Invalid or already booked seat!\n");
        getch();
        return;
    }

    Cus newCustomer;
    printf(CYAN "Enter phone number (10 digits): " RESET);
    if (!read_phone_line(&newCustomer.phon)) {
        print_error("Booking cancelled - no phone number entered.");
        getch();
        return;
    }

    printf("Enter your name: ");
    char nameBuf[64];
    if (fgets(nameBuf, sizeof(nameBuf), stdin) == NULL) {
        nameBuf[0] = '\0';
    }
    nameBuf[strcspn(nameBuf, "\n")] = 0;
    if (strlen(nameBuf) == 0 || contains_pipe(nameBuf)) {
        printf("Invalid name (cannot be blank or contain '|').\n");
        getch();
        return;
    }
    strncpy(newCustomer.name, nameBuf, sizeof(newCustomer.name) - 1);
    newCustomer.name[sizeof(newCustomer.name) - 1] = '\0';

    newCustomer.B_ID = busChoice;

    //Make sure this phone number doesn't already have a booking on this exact bus
    FILE *custCheck = fopen("customers.txt", "r");
    if (custCheck != NULL) {
        int existingTicketID;
        long long existingPhone;
        char existingName[64];
        int existingBusID, existingSeat, existingDate;
        while (fscanf(custCheck, "%d|%lld|%[^|]|%d|%d|%d\n", &existingTicketID, &existingPhone, existingName, &existingBusID, &existingSeat, &existingDate) == 6) {
            if (existingPhone == newCustomer.phon && existingBusID == busChoice) {
                fclose(custCheck);
                printf("\nYou already have a booking on this bus (seat %d). Cancel it first if you want a different seat.\n", existingSeat);
                getch();
                return;
            }
        }
        fclose(custCheck);
    }

    //Mark seat as booked and rewrite buses.txt3

    allBuses[busChoice].seats[seatChoice] = 1;

    FILE *file = fopen("buses.txt", "w"); // "w" overwrites the whole file fresh
    if (file == NULL) {
        printf("\nError saving booking, please try again.\n");
        getch();
        return;
    }
    for (int i = 0; i < busCount; i++) {
        fprintf(file, "%s|%s|%d|%d|%.2f", allBuses[i].ID, allBuses[i].des, allBuses[i].date, allBuses[i].T, allBuses[i].fare); // === CHANGED: date now written back too ===
        for (int j = 0; j < TOTAL_SEAT; j++) {
            fprintf(file, "|%d", allBuses[i].seats[j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);

    //every booking gets a unique ticket number (PNR) that never repeats
    int ticketID = get_next_ticket_id();

    time_t bookTime = time(NULL);
    struct tm *bookLt = localtime(&bookTime);
    int bookDate = ((bookLt->tm_year + 1900) * 10000) + ((bookLt->tm_mon + 1) * 100) + bookLt->tm_mday;

    //Save customer to customers.txt
    file = fopen("customers.txt", "a");
    if (file == NULL) {
        printf("\nError saving booking, please try again.\n");
        getch();
        return;
    }
    fprintf(file, "%d|%lld|%s|%d|%d|%d\n", ticketID, newCustomer.phon, newCustomer.name, newCustomer.B_ID, seatChoice + 1, bookDate);
    fclose(file);

    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    system("cls");
    print_header("TICKET CONFIRMED");
    print_success("Your seat is booked! Here are your ticket details:");
    printf("\n  " CYAN "Ticket No   :" RESET " " BOLD "T%06d" RESET "\n", ticketID);
    printf("  " CYAN "Passenger   :" RESET " %s\n", newCustomer.name);
    printf("  " CYAN "Phone       :" RESET " %lld\n", newCustomer.phon);
    printf("  " CYAN "Bus         :" RESET " %s\n", allBuses[busChoice].ID);
    printf("  " CYAN "Destination :" RESET " %s\n", allBuses[busChoice].des);
    printf("  " CYAN "Departure   :" RESET " %04d\n", allBuses[busChoice].T);
    printf("  " CYAN "Seat No     :" RESET " %d\n", seatChoice + 1);
    printf("  " CYAN "Fare        :" RESET " " GREEN "%.2f" RESET "\n", allBuses[busChoice].fare);
    printf("  " CYAN "Booked On   :" RESET " %04d-%02d-%02d\n", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);

    save_ticket_receipt(ticketID, newCustomer.name, newCustomer.phon, allBuses[busChoice].ID, allBuses[busChoice].des, allBuses[busChoice].T, seatChoice + 1, allBuses[busChoice].fare, lt);

    printf("\n" CYAN "Press any key to return..." RESET);
    getch();
}

void save_ticket_receipt(int ticketID, const char *name, long long phone, const char *plate, const char *dest, int deptTime, int seatNum, float fare, struct tm *bookLt)
{
    char filename[32];
    sprintf(filename, "ticket_T%06d.txt", ticketID);

    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        return; 
    }

    fprintf(f, "================ BUS TICKET ================\n");
    fprintf(f, "Ticket No   : T%06d\n", ticketID);
    fprintf(f, "Passenger   : %s\n", name);
    fprintf(f, "Phone       : %lld\n", phone);
    fprintf(f, "Bus         : %s\n", plate);
    fprintf(f, "Destination : %s\n", dest);
    fprintf(f, "Departure   : %04d\n", deptTime);
    fprintf(f, "Seat No     : %d\n", seatNum);
    fprintf(f, "Fare        : %.2f\n", fare);
    fprintf(f, "Booked On   : %04d-%02d-%02d\n", bookLt->tm_year + 1900, bookLt->tm_mon + 1, bookLt->tm_mday);
    fprintf(f, "=============================================\n");
    fclose(f);
}

//Helper used by both view_bookings and cancel_booking so we don't load buses.txt twice with two copies of the same logic
int load_all_buses(BUS allBuses[], int maxBuses)
{
    FILE *file = fopen("buses.txt", "r");
    int busCount = 0;
    if (file == NULL) {
        return 0;
    }

    while (busCount < maxBuses && fscanf(file, "%[^|]|%[^|]|%d|%d|%f", allBuses[busCount].ID, allBuses[busCount].des, &allBuses[busCount].date, &allBuses[busCount].T, &allBuses[busCount].fare) == 5) {
        for (int i = 0; i < TOTAL_SEAT; i++) {
            fscanf(file, "|%d", &allBuses[busCount].seats[i]);
        }
        fscanf(file, "\n");
        busCount++;
    }
    fclose(file);
    return busCount;
}

int read_float_line(float *out)
{
    char buffer[64];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }
    return sscanf(buffer, "%f", out) == 1;
}

//our file format is pipe-delimited, so any '|' typed into a bus plate,
// destination, or passenger name would silently corrupt every fscanf that reads the
// file afterwards. This rejects that character wherever a user types free text.
int contains_pipe(const char *s)
{
    return strchr(s, '|') != NULL;
}

int dest_matches(const char *busDest, const char *searchTerm)
{
    if (strlen(searchTerm) == 0) {
        return 1; // no destination filter entered, so every bus counts as a match
    }

    char busLower[20];
    char searchLower[20];

    int i;
    for (i = 0; busDest[i] != '\0' && i < (int)sizeof(busLower) - 1; i++) {
        busLower[i] = (char)tolower((unsigned char)busDest[i]);
    }
    busLower[i] = '\0';

    for (i = 0; searchTerm[i] != '\0' && i < (int)sizeof(searchLower) - 1; i++) {
        searchLower[i] = (char)tolower((unsigned char)searchTerm[i]);
    }
    searchLower[i] = '\0';

    return strstr(busLower, searchLower) != NULL;
}

// hands out a unique, ever-increasing ticket/PNR number for every booking.
// The last number given out is kept in ticket_counter.txt so numbering survives restarts.
int get_next_ticket_id()
{
    int last = 0;
    FILE *f = fopen("ticket_counter.txt", "r");
    if (f != NULL) {
        fscanf(f, "%d", &last);
        fclose(f);
    }
    int next = last + 1;
    f = fopen("ticket_counter.txt", "w");
    if (f != NULL) {
        fprintf(f, "%d", next);
        fclose(f);
    }
    return next;
}

int read_int_line(int *out)
{
    char buffer[64];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }
    return sscanf(buffer, "%d", out) == 1;
}

int valid_time_format(int t)
{
    if (t < 0 || t > 2359) return 0;
    int hour = t / 100;
    int minute = t % 100;
    return (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59);
}

int is_duplicate_bus(BUS allBuses[], int busCount, const char *plate, int date, int time)
{
    for (int i = 0; i < busCount; i++) {
        if (strcmp(allBuses[i].ID, plate) == 0 && allBuses[i].date == date && allBuses[i].T == time) {
            return 1;
        }
    }
    return 0;
}

int read_llong_line(long long *out)
{
    char buffer[64];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }
    return sscanf(buffer, "%lld", out) == 1;
}

int read_phone_line(long long *out)
{
    char buffer[32];

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return 0;
        }
        buffer[strcspn(buffer, "\n")] = 0; // strip trailing newline

        if (strlen(buffer) == 0) {
            return 0; // blank Enter = cancel, matches the rest of the program
        }

        int len = (int)strlen(buffer);
        int digitsOnly = 1;
        for (int i = 0; i < len; i++) {
            if (!isdigit((unsigned char)buffer[i])) {
                digitsOnly = 0;
                break;
            }
        }

        if (!digitsOnly) {
            print_error("Phone number can only contain digits.");
            printf(CYAN "Enter phone number (10 digits): " RESET);
            continue;
        }
        if (len < 10) {
            print_error("Too short! Phone number must be exactly 10 digits.");
            printf(CYAN "Enter phone number (10 digits): " RESET);
            continue;
        }
        if (len > 10) {
            print_error("Too long! Phone number must be exactly 10 digits.");
            printf(CYAN "Enter phone number (10 digits): " RESET);
            continue;
        }

        sscanf(buffer, "%lld", out);
        return 1;
    }
}

void print_success(const char *msg)
{
    printf(GREEN "  [OK] %s" RESET "\n", msg);
}

void print_error(const char *msg)
{
    printf(RED "  [!] %s" RESET "\n", msg);
}


void print_menu_box(const char *title, const char *options[], int count, const char *backLabel)
{
    int cols, rows;
    get_console_size(&cols, &rows);

    int width = (int)strlen(title) + 4;
    for (int i = 0; i < count; i++) {
        int optLen = (int)strlen(options[i]) + 8; // room for "[N]  " + right padding
        if (optLen > width) width = optLen;
    }
    if (backLabel) {
        int backLen = (int)strlen(backLabel) + 8;
        if (backLen > width) width = backLen;
    }
    if (width < 40) width = 40;      // don't let the box look cramped
    if (width > cols - 6) width = cols - 6; // never wider than the terminal

    int margin = (cols - width - 2) / 2;
    if (margin < 0) margin = 0;
    char pad[200];
    if (margin > 190) margin = 190;
    memset(pad, ' ', margin);
    pad[margin] = '\0';

    printf("%s" MAGENTA "+", pad);
    for (int i = 0; i < width; i++) printf("-");
    printf("+\n" RESET);

    printf("%s" MAGENTA "|" RESET BOLD "  %-*s" RESET MAGENTA "|\n" RESET, pad, width - 2, title);

    printf("%s" MAGENTA "+", pad);
    for (int i = 0; i < width; i++) printf("-");
    printf("+\n" RESET);

    for (int i = 0; i < count; i++) {
        printf("%s" MAGENTA "|  " RESET CYAN "[%d]" RESET " %-*s" MAGENTA " |\n" RESET,
               pad, i + 1, width - 7, options[i]);
    }

    if (backLabel) {
        printf("%s" MAGENTA "|  " RESET CYAN "[0]" RESET " %-*s" MAGENTA " |\n" RESET,
               pad, width - 7, backLabel);
    }

    printf("%s" MAGENTA "+", pad);
    for (int i = 0; i < width; i++) printf("-");
    printf("+\n" RESET);
}

void view_bookings()
{
    system("cls");
    print_header("LIST OF BOOKED SEATS");

    //Load buses first so we can show the plate/destination instead of just a raw bus index
    BUS allBuses[MAX_BUSES];
    int busCount = load_all_buses(allBuses, MAX_BUSES);

    int searchDate;
    printf("Enter travel date to filter (YYYYMMDD, or 0 for any date): ");
    if (!read_int_line(&searchDate)) {
        searchDate = 0;
    }

    printf("Enter destination to filter (leave blank for any): ");
    char destSearch[20];
    if (fgets(destSearch, sizeof(destSearch), stdin) == NULL) {
        destSearch[0] = '\0';
    }
    destSearch[strcspn(destSearch, "\n")] = 0;

    FILE *file = fopen("customers.txt", "r");
    if (file == NULL) {
        printf("No bookings found.\n");
        getch();
        return;
    }

    int ticketID;
    long long phone;
    char custName[64];
    int busID, seat, bookDate;
    int noFilter = (searchDate == 0 && strlen(destSearch) == 0);
    int shown = 0; // how many rows actually matched the filter

    printf("\n%-8s %-25s %-20s %-20s %-20s %-6s %-10s\n", "Ticket", "Passenger", "Phone", "Bus Plate", "Destination", "Seat", "Booked On");
    printf("-----------------------------------------------------------------------------------------------------------\n");

    while (fscanf(file, "%d|%lld|%[^|]|%d|%d|%d\n", &ticketID, &phone, custName, &busID, &seat, &bookDate) == 6) {
        if (busID >= 0 && busID < busCount) {
            //only show this booking if its bus matches the date/destination filter
            int dateOK = (searchDate == 0 || allBuses[busID].date == searchDate);
            int destOK = dest_matches(allBuses[busID].des, destSearch);
            if (dateOK && destOK) {
                Sleep(150);
                printf("T%06d %-25s %-20lld %-20s %-20s %-6d %-10d\n", ticketID, custName, phone, allBuses[busID].ID, allBuses[busID].des, seat, bookDate);
                shown++;
            }
        } else if (noFilter) {
            //Bus no longer exists (deleted or file mismatch) - only shown when no filter is applied,
            //since we have no date/destination to check these against
            printf("T%06d %-25s %-20lld %-20s %-20s %-6d %-10d\n", ticketID, custName, phone, "UNKNOWN", "UNKNOWN", seat, bookDate);
            shown++;
        }
    }

    if (shown == 0) {
        printf("No bookings found matching that date/destination.\n");
    }

    fclose(file);
    printf("\n" CYAN "Press any key to return..." RESET);
    getch();
}

void cancel_booking() 
{
    system("cls");
    print_header("CANCEL BOOKED SEAT");
    printf("\n");

    const char *searchOptions[] = {
        "Phone Number",
        "Ticket Number (PNR)"
    };
    print_menu_box("SEARCH BY", searchOptions, 2, "Back");
    printf("\n" CYAN "Your choice: " RESET);
    int searchMode;
    if (!read_int_line(&searchMode)) {
        searchMode = -1;
    }

    if (searchMode == 0) {
        return; // back to main menu, nothing to cancel
    }

    long long phone = 0;
    int searchTicketID = 0;

    if (searchMode == 2) {
        char tBuf[16];
        printf("Enter Ticket Number (e.g. T000012 or just 12): ");
        if (fgets(tBuf, sizeof(tBuf), stdin) == NULL) {
            printf("Invalid input!\n");
            getch();
            return;
        }
        tBuf[strcspn(tBuf, "\n")] = 0;
        //accept either "T000012" or a plain "12"
        const char *numPart = tBuf;
        if ((tBuf[0] == 'T' || tBuf[0] == 't')) {
            numPart = tBuf + 1;
        }
        searchTicketID = atoi(numPart);
        if (searchTicketID <= 0) {
            printf("Invalid ticket number!\n");
            getch();
            return;
        }
    } else if (searchMode == 1) {
        printf(CYAN "Enter phone number (10 digits): " RESET);
        if (!read_phone_line(&phone)) {
            print_error("Cancelled - no phone number entered.");
            getch();
            return;
        }
    } else {
        print_error("Invalid choice.");
        getch();
        return;
    }

    //Load every customer record into memory so we can filter, display, then rewrite the file afterwards
    FILE *file = fopen("customers.txt", "r");
    if (file == NULL) {
        printf("No bookings found.\n");
        getch();
        return;
    }

    Cus allCustomers[MAX_CUSTOMERS];
    int seatNums[MAX_CUSTOMERS]; //stored separately since Cus struct doesn't hold the seat number
    int ticketIDs[MAX_CUSTOMERS]; //keep each booking's ticket number so we can preserve it on ewrite
    int custCount = 0;
    int tID;
    long long p;
    char nm[64];
    int bID, s, d;

    while (custCount < MAX_CUSTOMERS && fscanf(file, "%d|%lld|%[^|]|%d|%d|%d\n", &tID, &p, nm, &bID, &s, &d) == 6) {
        ticketIDs[custCount] = tID;
        allCustomers[custCount].phon = p;
        strncpy(allCustomers[custCount].name, nm, sizeof(allCustomers[custCount].name) - 1);
        allCustomers[custCount].name[sizeof(allCustomers[custCount].name) - 1] = '\0';
        allCustomers[custCount].B_ID = bID;
        allCustomers[custCount].bookDate = d;
        seatNums[custCount] = s;
        custCount++;
    }
    fclose(file);

    BUS allBuses[MAX_BUSES];
    int busCount = load_all_buses(allBuses, MAX_BUSES);

    //Show only the bookings that belong to this phone number
    int matchIndexes[MAX_CUSTOMERS]; 
    int matchCount = 0;

    printf("\nYour bookings:\n");
    printf("%-5s %-8s %-15s %-20s %-6s\n", "No.", "Ticket", "Bus Plate", "Destination", "Seat"); 
    printf("----------------------------------------------------------------\n");

    for (int i = 0; i < custCount; i++) {
        int isMatch = (searchMode == 2) ? (ticketIDs[i] == searchTicketID) : (allCustomers[i].phon == phone);
        if (isMatch) {
            const char *plate = "UNKNOWN";
            const char *dest = "UNKNOWN";
            if (allCustomers[i].B_ID >= 0 && allCustomers[i].B_ID < busCount) {
                plate = allBuses[allCustomers[i].B_ID].ID;
                dest = allBuses[allCustomers[i].B_ID].des;
            }
            printf("%-5d T%06d %-15s %-20s %-6d\n", matchCount + 1, ticketIDs[i], plate, dest, seatNums[i]);
            matchIndexes[matchCount] = i;
            matchCount++;
        }
    }

    if (matchCount == 0) {
        if (searchMode == 2) {
            printf("\nNo booking found for that ticket number.\n");
        } else {
            printf("\nNo bookings found for this phone number.\n");
        }
        getch();
        return;
    }

    int choice;
    printf("\nEnter the number of the booking to cancel (0 to go back): ");
    if (!read_int_line(&choice)) {
        choice = 0;
    }

    if (choice <= 0 || choice > matchCount) {
        printf("Cancelled, nothing was changed.\n");
        getch();
        return;
    }

    int removeIndex = matchIndexes[choice - 1];
    int freedBusID = allCustomers[removeIndex].B_ID;
    int freedSeat = seatNums[removeIndex] - 1; // back to 0-based index for the seats[] array

    //Block the cancellation if the bus has already departed
    //Departure time is stored as HHMM (e.g. 1430 = 2:30pm), so we build the current time the same way to compare
    if (freedBusID >= 0 && freedBusID < busCount) {
        time_t now = time(NULL);
        struct tm *lt = localtime(&now);
        int currentTime = (lt->tm_hour * 100) + lt->tm_min; // e.g. 14:35 becomes 1435
        int today = ((lt->tm_year + 1900) * 10000) + ((lt->tm_mon + 1) * 100) + lt->tm_mday; // build today as YYYYMMDD so it can be compared to the bus date

        //only worry about "already departed" if the bus is actually travelling today -
        //a bus leaving next week shouldn't get blocked just because of the current time of day
        if (allBuses[freedBusID].date == today && currentTime >= allBuses[freedBusID].T) {
            printf("\nThis bus has already departed (departure time: %04d). Cancellation is not allowed.\n", allBuses[freedBusID].T);
            getch();
            return;
        }
    }

    //Free the seat in the buses array, then rewrite buses.txt so the seat becomes bookable again
    if (freedBusID >= 0 && freedBusID < busCount) {
        allBuses[freedBusID].seats[freedSeat] = 0;

        FILE *out = fopen("buses.txt", "w");
        if (out == NULL) {
            printf("\nError saving cancellation, please try again.\n");
            getch();
            return;
        }
        for (int i = 0; i < busCount; i++) {
            fprintf(out, "%s|%s|%d|%d|%.2f", allBuses[i].ID, allBuses[i].des, allBuses[i].date, allBuses[i].T, allBuses[i].fare); 
            for (int j = 0; j < TOTAL_SEAT; j++) {
                fprintf(out, "|%d", allBuses[i].seats[j]);
            }
            fprintf(out, "\n");
        }
        fclose(out);
    }

    //Rewrite customers.txt, skipping the cancelled booking
    FILE *out2 = fopen("customers.txt", "w");
    if (out2 == NULL) {
        printf("\nError saving cancellation, please try again.\n");
        getch();
        return;
    }
    for (int i = 0; i < custCount; i++) {
        if (i == removeIndex) continue;
        fprintf(out2, "%d|%lld|%s|%d|%d|%d\n", ticketIDs[i], allCustomers[i].phon, allCustomers[i].name, allCustomers[i].B_ID, seatNums[i], allCustomers[i].bookDate);
    }
    fclose(out2);

    if (freedBusID >= 0 && freedBusID < busCount) {
        printf("\nBooking cancelled successfully! Seat %d on bus %s is now free.\n", freedSeat + 1, allBuses[freedBusID].ID);
    } else {
        printf("\nBooking cancelled successfully! Seat %d is now free.\n", freedSeat + 1);
    }
    getch();
}
//----------------FUNCTION TO CLEAR OUT BUSES (AND THEIR BOOKINGS) THAT ALREADY DEPARTED-----------------||
// runs once at startup so buses.txt and customers.txt don't just keep growing with old, useless entries.
// any bus whose date is before today has 100% already departed, so it (and every booking on it) gets wiped.
void cleanup_expired_buses()
{
    BUS allBuses[MAX_BUSES];
    int busCount = load_all_buses(allBuses, MAX_BUSES);
    if (busCount == 0) {
        return; // nothing to clean
    }

    //get today's date the same way the rest of the program does (from time.h)
    time_t nowT = time(NULL);
    struct tm *nowLt = localtime(&nowT);
    int today = ((nowLt->tm_year + 1900) * 10000) + ((nowLt->tm_mon + 1) * 100) + nowLt->tm_mday;

    //Load every customer booking too, since deleting a bus means deleting its bookings as well
    Cus allCustomers[MAX_CUSTOMERS];
    int seatNums[MAX_CUSTOMERS];
    int ticketIDs[MAX_CUSTOMERS];
    int custCount = 0;
    FILE *cf = fopen("customers.txt", "r");
    if (cf != NULL) {
        int tID, bID, s, d;
        long long p;
        char nm[64];
        while (custCount < MAX_CUSTOMERS && fscanf(cf, "%d|%lld|%[^|]|%d|%d|%d\n", &tID, &p, nm, &bID, &s, &d) == 6) {
            ticketIDs[custCount] = tID;
            allCustomers[custCount].phon = p;
            strncpy(allCustomers[custCount].name, nm, sizeof(allCustomers[custCount].name) - 1);
            allCustomers[custCount].name[sizeof(allCustomers[custCount].name) - 1] = '\0';
            allCustomers[custCount].B_ID = bID;
            allCustomers[custCount].bookDate = d;
            seatNums[custCount] = s;
            custCount++;
        }
        fclose(cf);
    }

    //Keep only the buses that are today or later. newIndexOf[] remembers where each surviving
    //bus ends up so we can fix up every booking's B_ID afterwards (same idea as DELETE_BUS uses).
    BUS keptBuses[MAX_BUSES];
    int newIndexOf[MAX_BUSES];
    int keptCount = 0;
    int removedAny = 0;

    for (int i = 0; i < busCount; i++) {
        if (allBuses[i].date < today) {
            newIndexOf[i] = -1; // this bus's date is in the past, it already departed
            removedAny = 1;
        } else {
            keptBuses[keptCount] = allBuses[i];
            newIndexOf[i] = keptCount;
            keptCount++;
        }
    }

    if (!removedAny) {
        return; // every bus is still current, nothing needs to change
    }

    //Keep only the bookings whose bus survived, renumbering their B_ID to match the new bus list.
    //Any booking whose bus got removed above gets archived to booking_history.txt before it's dropped for good.
    Cus keptCustomers[MAX_CUSTOMERS];
    int keptSeatNums[MAX_CUSTOMERS];
    int keptTicketIDs[MAX_CUSTOMERS];
    int keptCustCount = 0;

    FILE *hist = fopen("booking_history.txt", "a"); // "a" so every run just adds on to the history, never wipes old history

    for (int i = 0; i < custCount; i++) {
        int oldBusID = allCustomers[i].B_ID;
        if (oldBusID >= 0 && oldBusID < busCount && newIndexOf[oldBusID] != -1) {
            keptCustomers[keptCustCount] = allCustomers[i];
            keptCustomers[keptCustCount].B_ID = newIndexOf[oldBusID];
            keptSeatNums[keptCustCount] = seatNums[i];
            keptTicketIDs[keptCustCount] = ticketIDs[i];
            keptCustCount++;
        } else if (hist != NULL) {
            //this booking's bus already departed - save a full snapshot (bus details included,
            //since buses.txt won't have this bus anymore after today) before it's gone for good
            const char *plate = "UNKNOWN";
            const char *dest = "UNKNOWN";
            int travelDate = 0, travelTime = 0;
            float fare = 0.0f;
            if (oldBusID >= 0 && oldBusID < busCount) {
                plate = allBuses[oldBusID].ID;
                dest = allBuses[oldBusID].des;
                travelDate = allBuses[oldBusID].date;
                travelTime = allBuses[oldBusID].T;
                fare = allBuses[oldBusID].fare;
            }
            fprintf(hist, "%d|%lld|%s|%s|%s|%d|%d|%d|%.2f|%d\n",
                    ticketIDs[i], allCustomers[i].phon, allCustomers[i].name,
                    plate, dest, travelDate, travelTime, seatNums[i], fare, allCustomers[i].bookDate);
        }
    }

    if (hist != NULL) {
        fclose(hist);
    }

    //Rewrite buses.txt with only the surviving buses
    FILE *out = fopen("buses.txt", "w");
    if (out != NULL) {
        for (int i = 0; i < keptCount; i++) {
            fprintf(out, "%s|%s|%d|%d|%.2f", keptBuses[i].ID, keptBuses[i].des, keptBuses[i].date, keptBuses[i].T, keptBuses[i].fare);
            for (int j = 0; j < TOTAL_SEAT; j++) {
                fprintf(out, "|%d", keptBuses[i].seats[j]);
            }
            fprintf(out, "\n");
        }
        fclose(out);
    }

    //Rewrite customers.txt with only the surviving bookings
    FILE *out2 = fopen("customers.txt", "w");
    if (out2 != NULL) {
        for (int i = 0; i < keptCustCount; i++) {
            fprintf(out2, "%d|%lld|%s|%d|%d|%d\n", keptTicketIDs[i], keptCustomers[i].phon, keptCustomers[i].name, keptCustomers[i].B_ID, keptSeatNums[i], keptCustomers[i].bookDate);
        }
        fclose(out2);
    }
}
//-------------------------------------------------------------------------------------------------------||


//------------------STRICTLY MADE FOR ONLY ATTEMPT FAILURE DONT USE OTHER PLACES------||
void show_lockout_page() 
{
    system("cls");
    int cols, rows;
    get_console_size(&cols, &rows);
    int topPad = (rows - 8) / 2;
    if (topPad < 0) topPad = 0;
    for (int i = 0; i < topPad; i++) printf("\n");

    print_header("SCREEN LOCKED");
    printf("\n");
    print_centered(cols, "Too many failed attempts. Your access has been temporarily restricted.", RED);
    printf("\n");

    for (int seconds_left = 120; seconds_left > 0; seconds_left--) {
        int minutes = seconds_left / 60;
        int seconds = seconds_left % 60;
        char timerMsg[64];
        sprintf(timerMsg, "Please sit tight. You can retry in: %02d:%02d ...", minutes, seconds);
        int len = (int)strlen(timerMsg);
        int pad = (cols - len) / 2;
        if (pad < 0) pad = 0;
        printf("\r");
        for (int i = 0; i < pad; i++) putchar(' ');
        printf(YELLOW "%s" RESET, timerMsg);
        Sleep(1000);
    }
    system("cls");
    get_console_size(&cols, &rows);
    int vPad = (rows - 2) / 2;
    if (vPad < 0) vPad = 0;
    for (int i = 0; i < vPad; i++) printf("\n");
    print_centered(cols, "Time is up! Redirecting back to the login screen...", CYAN);
    Sleep(2000);
}
//-----------------------------------------------------------------------------------------||




//----------------FUNCTION TO ADD BUSES NEW ONE--------------------------------------------||
void ADD_NEW_BUS() {
    system("cls");
    print_header("ADD NEW BUS");

    BUS existingBuses[MAX_BUSES];
    int existingCount = load_all_buses(existingBuses, MAX_BUSES);

    BUS newBus;

    //------------------RESETS THE VALUE OF SEATS SO WE CAN CREATE NEW BUS WITH EMPTY SEAT VALUES-------||
    for (int i = 0; i < TOTAL_SEAT; i++) {
        newBus.seats[i] = 0;
    }

    printf("Enter Bus Number Plate: ");
    fgets(newBus.ID, sizeof(newBus.ID), stdin);
    newBus.ID[strcspn(newBus.ID, "\n")] = 0; //rplaces the \n with null terminator giving us clean string

    //'|' would break our pipe-delimited file format, so we reject it here
    if (contains_pipe(newBus.ID)) {
        printf("Bus number plate cannot contain the '|' character. Bus not added.\n");
        getch();
        return;
    }

    printf("Enter Destination: ");
    fgets(newBus.des, sizeof(newBus.des), stdin);
    newBus.des[strcspn(newBus.des, "\n")] = 0;

   
    if (contains_pipe(newBus.des)) {
        printf("Destination cannot contain the '|' character. Bus not added.\n");
        getch();
        return;
    }

    printf("Enter Departure Time (e.g. 1430 for 2:30pm): ");
    if (!read_int_line(&newBus.T)) {
        printf("Invalid input! Bus not added.\n");
        getch();
        return;
    }

    if (!valid_time_format(newBus.T)) {
        printf("Invalid departure time. Hour must be 00-23 and minute 00-59. Bus not added.\n");
        getch();
        return;
    }

    printf("Enter Travel Date (YYYYMMDD, e.g. 20260708): ");
    if (!read_int_line(&newBus.date)) {
        printf("Invalid input! Bus not added.\n");
        getch();
        return;
    }

    {
        time_t nowT = time(NULL);
        struct tm *nowLt = localtime(&nowT);
        int today = ((nowLt->tm_year + 1900) * 10000) + ((nowLt->tm_mon + 1) * 100) + nowLt->tm_mday;
        if (newBus.date < today) {
            printf("Travel date cannot be in the past. Bus not added.\n");
            getch();
            return;
        }
    }

    printf("Enter Ticket Fare (e.g. 250.00): ");
    if (!read_float_line(&newBus.fare)) {
        printf("Invalid input! Bus not added.\n");
        getch();
        return;
    }

    if (newBus.fare <= 0) {
        printf("Fare must be greater than 0. Bus not added.\n");
        getch();
        return;
    }

    if (is_duplicate_bus(existingBuses, existingCount, newBus.ID, newBus.date, newBus.T)) {
        printf("This bus (same plate, date and time) already exists. Bus not added.\n");
        getch();
        return;
    }

    FILE *file = fopen("buses.txt", "a");
    if (file == NULL) {
        printf("Error opening file!\n");
        getch();
        return;
    }

    // Write all seat values too so we can load them back later
    fprintf(file, "%s|%s|%d|%d|%.2f", newBus.ID, newBus.des, newBus.date, newBus.T, newBus.fare);
    for (int i = 0; i < TOTAL_SEAT; i++) {
        fprintf(file, "|%d", newBus.seats[i]); // all 0s since bus is new
    }
    fprintf(file, "\n"); // end of this bus's line

    fclose(file);
    printf("\nBus added successfully!\n");
    getch();
}
//-------------------------------------------------------------------------------------------------------||


void EDIT_BUS()
{
    system("cls");
    print_header("EDIT BUS");

    BUS allBuses[MAX_BUSES];
    int busCount = load_all_buses(allBuses, MAX_BUSES);

    if (busCount == 0) {
        printf("No buses found.\n");
        getch();
        return;
    }

    printf("%-5s %-15s %-20s %-10s %-10s %-10s\n", "No.", "Number Plate", "Destination", "Date", "Time", "Fare");
    printf("------------------------------------------------------------------------------------\n");
    for (int i = 0; i < busCount; i++) {
        printf("%-5d %-15s %-20s %-10d %-10d %-10.2f\n", i + 1, allBuses[i].ID, allBuses[i].des, allBuses[i].date, allBuses[i].T, allBuses[i].fare);
    }

    int choice;
    printf("\nEnter bus number to edit (0 to cancel): ");
    if (!read_int_line(&choice) || choice <= 0 || choice > busCount) {
        printf("Cancelled.\n");
        getch();
        return;
    }
    choice--;

    printf("\nLeave a field blank / enter 0 to keep its current value.\n");

    printf("New Destination (blank = keep '%s'): ", allBuses[choice].des);
    char destBuf[64];
    if (fgets(destBuf, sizeof(destBuf), stdin) != NULL) {
        destBuf[strcspn(destBuf, "\n")] = 0;
        if (strlen(destBuf) > 0) {
            if (contains_pipe(destBuf)) {
                printf("Destination cannot contain '|'. Keeping old value.\n");
            } else {
                strncpy(allBuses[choice].des, destBuf, sizeof(allBuses[choice].des) - 1);
                allBuses[choice].des[sizeof(allBuses[choice].des) - 1] = '\0';
            }
        }
    }

    printf("New Travel Date (0 = keep %d): ", allBuses[choice].date);
    int newDate;
    if (read_int_line(&newDate) && newDate != 0) {
        allBuses[choice].date = newDate;
    }

    printf("New Departure Time (0 = keep %04d): ", allBuses[choice].T);
    int newTime;
    if (read_int_line(&newTime) && newTime != 0) {
        allBuses[choice].T = newTime;
    }

    printf("New Fare (0 = keep %.2f): ", allBuses[choice].fare);
    float newFare;
    if (read_float_line(&newFare) && newFare != 0) {
        allBuses[choice].fare = newFare;
    }

    FILE *out = fopen("buses.txt", "w");
    if (out == NULL) {
        printf("\nError saving changes, please try again.\n");
        getch();
        return;
    }
    for (int i = 0; i < busCount; i++) {
        fprintf(out, "%s|%s|%d|%d|%.2f", allBuses[i].ID, allBuses[i].des, allBuses[i].date, allBuses[i].T, allBuses[i].fare);
        for (int j = 0; j < TOTAL_SEAT; j++) {
            fprintf(out, "|%d", allBuses[i].seats[j]);
        }
        fprintf(out, "\n");
    }
    fclose(out);

    printf("\nBus updated successfully!\n");
    getch();
}
//-------------------------------------------------------------------------------------------------------||


//----------------FUNCTION TO DELETE A BUS-----------------------------------------------------------------||
// refuses to delete a bus that still has active bookings on it
// every remaining booking's B_ID since removing a bus shifts everyone after it down by one.
void DELETE_BUS()
{
    system("cls");
    print_header("DELETE BUS");

    BUS allBuses[MAX_BUSES];
    int busCount = load_all_buses(allBuses, MAX_BUSES);

    if (busCount == 0) {
        printf("No buses found.\n");
        getch();
        return;
    }

    printf("%-5s %-15s %-20s %-10s %-10s\n", "No.", "Number Plate", "Destination", "Date", "Time");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < busCount; i++) {
        printf("%-5d %-15s %-20s %-10d %-10d\n", i + 1, allBuses[i].ID, allBuses[i].des, allBuses[i].date, allBuses[i].T);
    }

    int choice;
    printf("\nEnter bus number to delete (0 to cancel): ");
    if (!read_int_line(&choice) || choice <= 0 || choice > busCount) {
        printf("Cancelled.\n");
        getch();
        return;
    }
    choice--; // 0-based index of the bus we want gone

    // Load customers so we can check for active bookings and fix up their B_ID afterwards
    FILE *cf = fopen("customers.txt", "r");
    Cus allCustomers[MAX_CUSTOMERS];
    int seatNums[MAX_CUSTOMERS];
    int ticketIDs[MAX_CUSTOMERS];
    int custCount = 0;
    if (cf != NULL) {
        int tID, bID, s, d;
        long long p;
        char nm[64];
        while (custCount < MAX_CUSTOMERS && fscanf(cf, "%d|%lld|%[^|]|%d|%d|%d\n", &tID, &p, nm, &bID, &s, &d) == 6) {
            ticketIDs[custCount] = tID;
            allCustomers[custCount].phon = p;
            strncpy(allCustomers[custCount].name, nm, sizeof(allCustomers[custCount].name) - 1);
            allCustomers[custCount].name[sizeof(allCustomers[custCount].name) - 1] = '\0';
            allCustomers[custCount].B_ID = bID;
            allCustomers[custCount].bookDate = d;
            seatNums[custCount] = s;
            custCount++;
        }
        fclose(cf);
    }

    for (int i = 0; i < custCount; i++) {
        if (allCustomers[i].B_ID == choice) {
            printf("\nCannot delete: this bus still has active bookings. Cancel them first.\n");
            getch();
            return;
        }
    }

    // Remove the bus from the array by shifting everything after it back by one slot
    for (int i = choice; i < busCount - 1; i++) {
        allBuses[i] = allBuses[i + 1];
    }
    busCount--;

    // Shift down the B_ID of every booking that pointed to a bus after the deleted one,
    // so their bookings still point at the correct (now renumbered) bus
    for (int i = 0; i < custCount; i++) {
        if (allCustomers[i].B_ID > choice) {
            allCustomers[i].B_ID--;
        }
    }

    FILE *out = fopen("buses.txt", "w");
    if (out == NULL) {
        printf("\nError saving changes, please try again.\n");
        getch();
        return;
    }
    for (int i = 0; i < busCount; i++) {
        fprintf(out, "%s|%s|%d|%d|%.2f", allBuses[i].ID, allBuses[i].des, allBuses[i].date, allBuses[i].T, allBuses[i].fare);
        for (int j = 0; j < TOTAL_SEAT; j++) {
            fprintf(out, "|%d", allBuses[i].seats[j]);
        }
        fprintf(out, "\n");
    }
    fclose(out);

    FILE *out2 = fopen("customers.txt", "w");
    if (out2 != NULL) {
        for (int i = 0; i < custCount; i++) {
            fprintf(out2, "%d|%lld|%s|%d|%d|%d\n", ticketIDs[i], allCustomers[i].phon, allCustomers[i].name, allCustomers[i].B_ID, seatNums[i], allCustomers[i].bookDate);
        }
        fclose(out2);
    }

    printf("\nBus deleted successfully!\n");
    getch();
}
//-------------------------------------------------------------------------------------------------------||


//----------------FUNCTION TO SHOW A SALES SUMMARY FOR THE COUNTER CLERK----------------------------------||
// quick summary so the clerk can reconcile tickets sold / revenue collected.
void SALES_REPORT()
{
    system("cls");
    print_header("SALES REPORT");

    BUS allBuses[MAX_BUSES];
    int busCount = load_all_buses(allBuses, MAX_BUSES);

    FILE *file = fopen("customers.txt", "r");
    if (file == NULL) {
        printf("No bookings found.\n");
        getch();
        return;
    }

    time_t nowT = time(NULL);
    struct tm *nowLt = localtime(&nowT);
    int today = ((nowLt->tm_year + 1900) * 10000) + ((nowLt->tm_mon + 1) * 100) + nowLt->tm_mday;

    int ticketID, busID, seat, bookDate;
    long long phone;
    char custName[64];
    int totalTickets = 0;
    float totalRevenue = 0.0f;
    int todayTickets = 0;
    float todayRevenue = 0.0f;

    while (fscanf(file, "%d|%lld|%[^|]|%d|%d|%d\n", &ticketID, &phone, custName, &busID, &seat, &bookDate) == 6) {
        totalTickets++;
        if (busID >= 0 && busID < busCount) {
            totalRevenue += allBuses[busID].fare;
        }
        if (bookDate == today) {
            todayTickets++;
            if (busID >= 0 && busID < busCount) {
                todayRevenue += allBuses[busID].fare;
            }
        }
    }
    fclose(file);

    printf("\n  " CYAN "Today (%04d-%02d-%02d)" RESET "\n", nowLt->tm_year + 1900, nowLt->tm_mon + 1, nowLt->tm_mday);
    printf("  Tickets sold today : %d\n", todayTickets);
    printf("  Revenue today      : %.2f\n", todayRevenue);
    printf("\n  " CYAN "All-time" RESET "\n");
    printf("  Total tickets sold : %d\n", totalTickets);
    printf("  Total revenue      : %.2f\n", totalRevenue);
    printf("\n" CYAN "Press any key to return..." RESET);
    getch();
}
//-------------------------------------------------------------------------------------------------------||


//----------------------FUNCTION TO DISPLAY ALL THE BOOKED CUSTOMERS-------------------------------------||
void DISPLAY_CUSTOMERS(){
    system("cls");

    //Load buses so we can check each booking's travel date/destination against the filter
    BUS allBuses[MAX_BUSES];
    int busCount = load_all_buses(allBuses, MAX_BUSES);

    int searchDate;
    printf("Enter date to filter (YYYYMMDD) or 0 for all: ");
    if (!read_int_line(&searchDate)) {
        searchDate = 0;
    }

    printf("Enter destination to filter (leave blank for all): ");
    char destSearch[20];
    if (fgets(destSearch, sizeof(destSearch), stdin) == NULL) {
        destSearch[0] = '\0';
    }
    destSearch[strcspn(destSearch, "\n")] = 0;

    print_header("CUSTOMER BOOKINGS");

    FILE *file = fopen("customers.txt", "r");
    if (file == NULL) {
        printf("No bookings found.\n");
        getch();
        return;
    }

    int ticketID;
    long long phone;// long long so it can catch long intiger strings other wise it get value oveflow||
    char custName[64];
    int busID, seat, bookDate;
    int noFilter = (searchDate == 0 && strlen(destSearch) == 0);
    int shown = 0;

    printf("%-8s %-20s %-25s %-10s %-10s %-12s %-10s\n", "Ticket", "Name", "Phone Number", "Bus ID", "Seat No.", "Travel Date", "Booked On"); 
    printf("---------------------------------------------------------------------------------------------------------\n");

    while (fscanf(file, "%d|%lld|%[^|]|%d|%d|%d\n", &ticketID, &phone, custName, &busID, &seat, &bookDate) == 6) {
        int matches = noFilter; // if no filter set, everything counts as a match
        int travelDate = 0; // date this bus actually travels on, pulled from allBuses via the booking's B_ID
        if (busID >= 0 && busID < busCount) {
            travelDate = allBuses[busID].date;
        }
        if (!matches && busID >= 0 && busID < busCount) {
            int dateOK = (searchDate == 0 || allBuses[busID].date == searchDate);
            int destOK = dest_matches(allBuses[busID].des, destSearch);
            matches = dateOK && destOK;
        }
        if (matches) {
            printf("T%06d %-20s %-25lld %-10d %-10d %-12d %-10d\n", ticketID, custName, phone, busID, seat, travelDate, bookDate);
            shown++;
        }
    }

    if (shown == 0) {
        printf("No bookings found matching that date/destination.\n");
    }

    fclose(file);
    printf("\n" CYAN "Press any key to return..." RESET);
    getch();
}
//-----------------------------------------------------------------------------------------------------------||


//----------------------FUNCTION TO DISPLAY ARCHIVED (PAST) BOOKINGS--------------------------------------||
// reads booking_history.txt, which cleanup_expired_buses() writes to right before it
// deletes a booking whose bus has already come and gone.
void DISPLAY_HISTORY()
{
    system("cls");
    print_header("BOOKING HISTORY (PAST TRAVEL DATES)");

    FILE *file = fopen("booking_history.txt", "r");
    if (file == NULL) {
        printf("No booking history yet.\n");
        getch();
        return;
    }

    int ticketID, travelDate, travelTime, seat, bookDate;
    long long phone;
    char custName[64], plate[20], dest[20];
    float fare;
    int count = 0;

    printf("%-8s %-20s %-15s %-15s %-20s %-12s %-6s %-6s %-8s %-10s\n",
           "Ticket", "Passenger", "Phone", "Bus Plate", "Destination", "Travel Date", "Time", "Seat", "Fare", "Booked On");
    printf("-------------------------------------------------------------------------------------------------------------------------------\n");

    while (fscanf(file, "%d|%lld|%[^|]|%[^|]|%[^|]|%d|%d|%d|%f|%d\n",
                  &ticketID, &phone, custName, plate, dest, &travelDate, &travelTime, &seat, &fare, &bookDate) == 10) {
        printf("T%06d %-20s %-15lld %-15s %-20s %-12d %-6d %-6d %-8.2f %-10d\n",
               ticketID, custName, phone, plate, dest, travelDate, travelTime, seat, fare, bookDate);
        count++;
    }

    if (count == 0) {
        printf("No booking history yet.\n");
    }

    fclose(file);
    printf("\n" CYAN "Press any key to return..." RESET);
    getch();
}
//-----------------------------------------------------------------------------------------------------------||





//--------------------------------FUNCTION TO DISPLAY ALL BUSES SAVED-----------------------------------------||
void DISPLAY_BUSES(){
    system("cls");

    BUS existingBuses[MAX_BUSES];
    int existingCount = load_all_buses(existingBuses, MAX_BUSES); // this already loads every bus's seat data too, so there's no need to reopen buses.txt below

    int searchDate;
    printf("Enter date to filter buses (YYYYMMDD) or 0 for all: ");
    if (!read_int_line(&searchDate)) {
        searchDate = 0;
    }

    printf("Enter destination to filter (leave blank for all): ");
    char destSearch[20];
    if (fgets(destSearch, sizeof(destSearch), stdin) == NULL) {
        destSearch[0] = '\0';
    }
    destSearch[strcspn(destSearch, "\n")] = 0;

    print_header("BUS LIST");

    if (existingCount == 0) {
        printf("No buses found. Add some first!\n");
        getch();
        return;
    }

    printf("%-5s %-15s %-20s %-10s %-10s %-10s %-8s\n", "No.", "Number Plate", "Destination", "Date", "Time", "Fare", "Available");
    printf("--------------------------------------------------------------------------------------------------\n");

    int count = 1;
    for (int i = 0; i < existingCount; i++) {
        int dateOK = (searchDate == 0 || existingBuses[i].date == searchDate);
        int destOK = dest_matches(existingBuses[i].des, destSearch);
        if (dateOK && destOK) {
            int bookedCount = 0;
            for (int j = 0; j < TOTAL_SEAT; j++) {
                if (existingBuses[i].seats[j] == 1) bookedCount++;
            }

            char availStr[16];
            sprintf(availStr, "%d/%d", TOTAL_SEAT - bookedCount, TOTAL_SEAT);
            printf("%-5d %-15s %-20s %-10d %-10d %-10.2f %-8s\n", count, existingBuses[i].ID, existingBuses[i].des, existingBuses[i].date, existingBuses[i].T, existingBuses[i].fare, availStr);
            Sleep(150);
            count++;
        }
    }

    if (count == 1) { // never incremented past 1, meaning nothing matched
        printf("No buses found matching that date/destination.\n");
    }

    printf("\n" CYAN "Press any key to return..." RESET);
    getch();
}
//---------------------------------------------------------------------------------------||

//---------------------------------------------------------------------------------------||
//Reads the ACTUAL size of the terminal window (not the scrollback buffer),
//so every screen in the program can size and center itself to fit whatever
//window the user has open, instead of assuming a fixed 80/132-column width.
void get_console_size(int *cols, int *rows)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        *cols = 100; // sane fallback if the handle/query ever fails
        *rows = 30;
    }
    if (*cols < 60) *cols = 60;   // never let layout collapse below a usable minimum
    if (*rows < 20) *rows = 20;
}

//Prints one line of text horizontally centered within "cols" columns.
//colorCode may be NULL for plain text.
void print_centered(int cols, const char *text, const char *colorCode)
{
    int len = (int)strlen(text);
    int pad = (cols - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) putchar(' ');
    if (colorCode) {
        printf("%s%s" RESET "\n", colorCode, text);
    } else {
        printf("%s\n", text);
    }
}

void print_header(const char *title)
{
    int cols, rows;
    get_console_size(&cols, &rows);
    if (cols > 160) cols = 160; // cap so a single line doesn't stretch absurdly on ultrawide terminals

    for (int i = 0; i < cols; i++) putchar('=');
    printf("\n");

    print_centered(cols, title, NULL);

    for (int i = 0; i < cols; i++) putchar('=');
    printf("\n");
}


//---------------------------------------------------------------------------------------||





//strictly for loading amiation and leaving animation only
void cat_loading_animation() 
{
    
    for (int loop = 0; loop < 5; loop++) {
        
        
        system("cls"); 
        printf("\n\t\t MR.Cat is loading you system ... Please Wait Patiently\n");
        printf("\t\t      /\\_/\\  \n");
        printf("\t\t     ( o.o ) \n");
        printf("\t\t      > ^ <  \n");
        printf("\t\t     /  |  \\ \n");
        printf("\t\t    (____)_) \n");
        Sleep(350); 

        
        system("cls"); 
        printf("\n\t\t MR.Cat is loading you system ... Please Wait Patiently\n");
        printf("\t\t      /\\_/\\  __\n"); 
        printf("\t\t     ( ^.^ )/ /\n");
        printf("\t\t      > ^ <  / \n");
        printf("\t\t     /  |   /  \n");
        printf("\t\t    (____)_)   \n");
        Sleep(350);
    }
    system("cls");
}
//------------------------------------------------------------------------------------------------|
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#ifdef _WIN32
#include <Windows.h>
#endif

#define N_ROOMS 6
#define N_TABLES 3
#define N_RAND_DIGITS 3
#define N_TIMESLOTS 2
#define INVALID_TABLE_ENTRY 0
#define TABLE_UNAVAILABLE -1
#define FILE_DOES_NOT_EXIST 2
#define LINE_LENGTH 256
#define NAME_LEN 100
#define MAX_BUFSIZE 6 * (3 * NAME_LEN + 64)
#define BOOKING_FILE "booking.txt"

// Booking struct
typedef struct {
    char *firstName, *lastName, *dob, *id, *boardType;
    int nDays, nAdults, nChildren, paper, roomNum, tableNum, tableSlot;
} Booking;

// The order of the fields in the CSV file
typedef enum {
    firstName,
    lastName,
    dob,
    id,
    boardType,
    nDays,
    nAdults,
    nChildren,
    paper,
    roomNum,
    tableNum,
    tableSlot
} BookingOrder;

// Tables
typedef enum {
    Endor    = 1,
    Naboo    = 2,
    Tatooine = 3
} Tables;

// Prices for each room
static int prices[N_ROOMS] = { 100, 100, 85, 75, 75, 50 };

// Utility functions for serialization and data conversion
void removeNewLine(char* s) {
    char* d = s;
    do {
        while (*d == '\n') {
            ++d;
        }
    } while (*s++ = *d++);
}

void print(int delay, const char* fmt, ...) 
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#ifdef _WIN32
    Sleep(delay);
#else
    sleep(delay);
#endif
}

char* getTableName(Tables table, int pad)
{
    if (table == Endor) return (pad ? "Endor   " : "Endor");
    if (table == Naboo) return (pad ? "Naboo   " : "Naboo");
    if (table == Tatooine) return "Tatooine";
    return NULL;
}

void checkInvalidChars(char* str, const char* chars)
{
    int containsInvalidChars = 0;
    do {
        char ch;
        for (int i = 0; i < strlen(chars); ++i) {
            ch = chars[i];
            for (int j = 0; j < strlen(str); ++j) {
                if (ch == str[j]) {
                    containsInvalidChars = 1;
                    break;
                }
            }
            if (containsInvalidChars) break;
        }
        if (containsInvalidChars) {
            printf("Sorry, the following characters are not allowed: ");
            for (int i = 0; i < strlen(chars); ++i) {
                printf("'%c'", chars[i]);
                if (i + 1 != strlen(chars)) printf(", ");
            }
            print(500, "\nPlease try again: ");
            scanf("%s", &str);
        }
    } while (containsInvalidChars);
}

void concatStr(char* buffer, char* str, int* idx) 
{
    size_t len = strlen(str);
    if (*idx + len > MAX_BUFSIZE) {
        print(500, "Error: cannot concatenate strings: exceeds max buffer size\n");
        exit(EXIT_FAILURE);
    }
    strcat(buffer + *idx, str);
    *idx += len;
}

char* intToString(int n)
{
    if (n < 0) {
        printf("cannot convert %d to string\n", n);
    }
    int nDigits = n ? ceil(log10(n)) + 1 : 1;
    char* buffer = malloc(nDigits); // free later to avoid memory leak
    itoa(n, buffer, 10);
    return buffer;
}

int parseCSV(char* data, Booking bookings[N_ROOMS])
{
    char* record = strtok(data, ";");
    int idx = 0;
    char* records[N_ROOMS];
    while (record != NULL) {
        records[idx] = record;
        record = strtok(NULL, ";");
        idx++;
    }
    for (int i = 0; i < idx; ++i) {
        Booking* booking = (bookings + i);
        // Optional fields with default values
        booking->tableNum = -1;
        booking->tableSlot = -1;
        char* record = strdup(records[i]);
        removeNewLine(record);
        int fieldIdx = 0;
        char* field = strtok(record, ",");
        while (field != NULL) {
            if (fieldIdx == firstName) booking->firstName = field;
            if (fieldIdx == lastName) booking->lastName = field;
            if (fieldIdx == dob) booking->dob = field;
            if (fieldIdx == id) booking->id = field;
            if (fieldIdx == boardType) booking->boardType = field;
            if (fieldIdx == nDays) booking->nDays = atoi(field);
            if (fieldIdx == nAdults) booking->nAdults = atoi(field);
            if (fieldIdx == nChildren) booking->nChildren = atoi(field);
            if (fieldIdx == paper) booking->paper = atoi(field);
            if (fieldIdx == roomNum) booking->roomNum = atoi(field);
            if (fieldIdx == tableNum) booking->tableNum = atoi(field);
            if (fieldIdx == tableSlot) booking->tableSlot = atoi(field);
            field = strtok(NULL, ",");
            fieldIdx++;
        }

    }
    return idx;
}

int loadBookingData(const char* filename, Booking bookings[N_ROOMS])
{
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        // If the file does not exist, create it
        if (errno == FILE_DOES_NOT_EXIST) {
            f = fopen(filename, "w");
            if (f == NULL) {
                print(500, "Error: could not create data file\n");
                exit(EXIT_FAILURE);
            }
            fclose(f);
            f = fopen(filename, "r");    
        } else {
            print(500, "Error: fopen() failed with code: %d\n", errno);
            return 0;
        }
    }
    // Calculate size of the file in bytes 
    fseek(f, 0, SEEK_END);
    size_t fSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    // Allocate buffer of size fSize bytes
    char* data = malloc(fSize);
    if (data == NULL) {
        printf("malloc() failed with error: %d\n", errno);
        exit(EXIT_FAILURE);   
    }
    char c = 0;
    int bytesRead = 0;
    while (bytesRead != fSize) {
        c = fgetc(f);
        data[bytesRead] = c;
        bytesRead++;
    }
    data[fSize - 1] = '\0';
    int nResults = parseCSV(data, bookings);
    fclose(f);
    return nResults;
}

void saveBookingData(const char* filename, Booking bookings[N_ROOMS], int nBookings)
{
    char buffer[MAX_BUFSIZE] = "";
    char *delim = ",", *lineBreak = ";\n";
    int charIdx = 0;
    for (int i = 0; i < nBookings; ++i) {
        Booking booking = bookings[i];
        concatStr(buffer, booking.firstName, &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, booking.lastName, &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, booking.dob, &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, booking.id, &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, booking.boardType, &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.nDays), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.nAdults), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.nChildren), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.paper), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.roomNum), &charIdx);
        if (booking.tableNum != INVALID_TABLE_ENTRY && booking.tableSlot != INVALID_TABLE_ENTRY 
            && booking.tableNum != TABLE_UNAVAILABLE && booking.tableSlot != TABLE_UNAVAILABLE) {
            concatStr(buffer, delim, &charIdx);
            concatStr(buffer, intToString(booking.tableNum), &charIdx);
            concatStr(buffer, delim, &charIdx);
            concatStr(buffer, intToString(booking.tableSlot), &charIdx);
        }
        if ((i + 1) != nBookings) {
            concatStr(buffer, lineBreak, &charIdx);
        }
    }
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        print(500, "error: could not write data to disk\n");
        exit(EXIT_FAILURE);
    }
    fputs(buffer, f);
    fclose(f);
}

// Check in function (Orin)
void checkIn()
{

    int room1Available = 1;
    int room2Available = 1;
    int room3Available = 1;
    int room4Available = 1;
    int room5Available = 1;
    int room6Available = 1;


    Booking bookings[N_ROOMS];
    Booking booking;

    int nBookings = loadBookingData("bookings.txt", bookings);
    if (nBookings == 6)
    {
        printf("Sorry the hotel is full");
        return;
    }

    printf("checking in...\n");
    printf("Please enter your first name...\n");
    scanf("%s", &booking.firstName);
            fflush(stdin);


    printf("Please enter you last name...\n");
    scanf("%s", &booking.lastName);
            fflush(stdin);

    sprintf(booking.id, "%s%d", booking.lastName, rand()%10);
    printf("Your ID is %s\n", booking.id);

    printf("Please enter your date of birth in the format DD/MM/YYY...\n");
    scanf("%s", &booking.dob);
            fflush(stdin);

    printf("How many days are you staying for...\n");
    scanf("%d", &booking.nDays);
            fflush(stdin);
    printf("__________________________\n");



    while((booking.nAdults + booking.nChildren) > 4 || (booking.nAdults + booking.nChildren) == 0)
    {
        printf("How many adults are staying...\n");
        scanf("%d", &booking.nAdults);
                fflush(stdin);


        printf("How many children are staying... \n(age 16 or below)\n");
        scanf("%d", &booking.nChildren);
        fflush(stdin);
        if((booking.nChildren + booking.nAdults) > 4)
        {
            printf("Sorry that is too many people in one room\n__________________________\n");

            
        }
    }

    printf("__________________________\n");

    printf("Would you like a daily newspaper... \n(1 for yes or 0 for no)\n");
    scanf("%s", &booking.paper);

    printf("__________________________\n");




    int roomChoice = 0;
    printf("What room would you like 1-6...\n");
    scanf("%d", &roomChoice);
    fflush(stdin);

    while(roomChoice < 1 || roomChoice > 6)
    {
        printf("Please enter a valid room number...\n");
        scanf("%d", &roomChoice);
        fflush(stdin);
    }

   if(roomChoice == 1 && room1Available == 1)
   {
    room1Available == 0;
    printf("You have booked room 1\n__________________________\n");
   }

   if(roomChoice == 2 && room2Available == 1)
   {
    room2Available == 0;
    printf("You have booked room 2\n__________________________\n");
   }
 
    if(roomChoice == 3 && room3Available == 1)
   {
    room3Available = 0;
    printf("You have booked room 3\n__________________________\n");
   }

   if(roomChoice == 4 && room4Available == 1)
   {
    room4Available == 0;
    printf("You have booked room 4\n__________________________\n");
   }

   if(roomChoice == 5 && room5Available == 1)
   {
    room5Available = 0;
    printf("You have booked room 5\n__________________________\n");
   }

   if(roomChoice == 6 && room6Available == 1)
   {
    room6Available = 0;
    printf("You have booked room 6\n__________________________\n");
   }

    booking.roomNum = roomChoice;

    bookings[nBookings] = booking;
    nBookings ++;
    
    saveBookingData("bookings.txt", bookings, nBookings);
}

// Check out function (Mikhail)
void checkOut()
{
    print(500, "checking out...\n");

    // Booking ID
    char bokid[10];
    printf("Enter your booking ID:");
    scanf("%s", bokid);


    // time of bill
    time_t s, val = 1;
    time_t s, val = 1;
    struct tm* current_time;
    s = time();
}

// Table booking function (Tom)
void bookTable()
{
    // Load booking data
    Booking bookings[N_ROOMS];
    int nBookings = loadBookingData(BOOKING_FILE, bookings), bookingIdx = -1;
    // Create a 2D array of all possible booking slots at each table
    int timeSlots[N_TIMESLOTS] = { 7, 9 };
    int tables[N_TABLES] = { Endor, Naboo, Tatooine };
    int tablesAvailable[N_TIMESLOTS][N_TABLES] = { { Endor, Naboo, Tatooine }, { Endor, Naboo, Tatooine } };
    // Check booking ID
    char bookingId[NAME_LEN + N_RAND_DIGITS];
    print(500, "In order to book a table, please enter your booking ID: ");
    scanf("%s", &bookingId);
    fflush(stdin);
    for (int i = 0; i < nBookings; ++i) {
        for (int timeSlotIdx = 0; timeSlotIdx < N_TIMESLOTS; ++timeSlotIdx) {
            for (int tableIdx = 0; tableIdx < N_TABLES; ++tableIdx) {
                if (timeSlots[timeSlotIdx] == bookings[i].tableSlot && tables[tableIdx] == bookings[i].tableNum) {
                    tablesAvailable[timeSlotIdx][tableIdx] = TABLE_UNAVAILABLE;
                }
            }
        } 
        if (strcmp(bookings[i].id, bookingId) == 0) {
            bookingIdx = i;
            break;
        }
    }
    if (bookingIdx == -1) {
        print(500, "Sorry, that is an invalid booking ID, you cannot book a table.\n");
        return;
    } else if (bookings[bookingIdx].boardType == "BB") {
        print(500, "Sorry, you are booked in for Bed & Breakfast, meaning you cannot book a dinner table.\n");
        return;
    } else if ((bookings[bookingIdx].tableNum != TABLE_UNAVAILABLE && bookings[bookingIdx].tableNum != INVALID_TABLE_ENTRY) || 
               (bookings[bookingIdx].tableSlot != TABLE_UNAVAILABLE && bookings[bookingIdx].tableSlot != INVALID_TABLE_ENTRY)) {
        print(
            500, 
            "You currently have a table booked: %s at %d:00pm\n", 
            getTableName(bookings[bookingIdx].tableNum, 0), 
            bookings[bookingIdx].tableSlot % 12 + 12
        );
        char choice;
        do {
            print(200, "Would you like to cancel your table booking? (Y/N) ");
            scanf("%c", &choice);
            fflush(stdin);
        } while (choice != 'Y' && choice != 'N' && choice != 'y' && choice != 'n');
        if (choice == 'N' || choice == 'n') return;
        bookings[bookingIdx].tableNum = INVALID_TABLE_ENTRY;
        bookings[bookingIdx].tableSlot = INVALID_TABLE_ENTRY;
        saveBookingData(BOOKING_FILE, bookings, nBookings);
        return;
    }
    int confirmChoice = 0;
    while (!confirmChoice) {
        // Display available tables
        print(200, "Available tables: \n-----------------\n");
        int idx = 0, newIdx = 0, tableChoice;
        for (int i = 0; i < N_TIMESLOTS; ++i) {
            for (int j = 0; j < N_TABLES; ++j) {
                if (tablesAvailable[i][j] == TABLE_UNAVAILABLE) continue;
                print(100, "%d: %s | %d:00pm | Serves 4\n", idx + 1, getTableName(tablesAvailable[i][j], 1), (timeSlots[i] + 12) % 24);
                idx++;
            }
        }
        printf("\n");
        do {
            print(200, "Please select the table you want (1-%d): ", idx);
            scanf("%d", &tableChoice);
            fflush(stdin);
        } while (1 > tableChoice || tableChoice > idx);
        
        int tempTableNum = 0, tempTableSlot = 0;
        for (int i = 0; i < N_TIMESLOTS; ++i) {
            for (int j = 0; j < N_TABLES; ++j) {
                if (tablesAvailable[i][j] == TABLE_UNAVAILABLE) continue;
                if (tableChoice == newIdx + 1) {
                    tempTableNum = tablesAvailable[i][j];
                    tempTableSlot = timeSlots[i];
                }
                newIdx++;
            }
        }
        printf(
            "You have selected: %s at %d:00pm\n", 
            getTableName(tempTableNum, 0), 
            (tempTableSlot + 12) % 24
        );
        char choice;
        do {
            print(200, "Would you like to confirm your booking? (Y/N) ");
            scanf("%c", &choice);
            fflush(stdin);
        } while (choice != 'Y' && choice != 'N' && choice != 'y' && choice != 'n');
        if (choice == 'Y' || choice == 'y') {
            bookings[bookingIdx].tableNum = tempTableNum;
            bookings[bookingIdx].tableSlot = tempTableSlot;
            confirmChoice = 1;
        }
    }
    saveBookingData(BOOKING_FILE, bookings, nBookings);
    print(
        500,
        "Successfully booked a table for %s at %d:00pm\n", 
        getTableName(bookings[bookingIdx].tableNum, 0), 
        (bookings[bookingIdx].tableSlot + 12) % 24
    );
}

// Main user interface
int main(const int argc, const char** argv) 
{
    int finished = 0;
    while (!finished) {
        char option[32];
        printf("\nWelcome to the Kashyyyk Hotel\n");
        for (int i = 0; i < 29; ++i) print(5, "-");
        print(500, "\nChoose an action (checkin, checkout, booktable, quit): ");
        scanf("%s", &option);
            
        if (strcmp((const char*)option, "checkin") == 0) {
            checkIn();
        } else if (strcmp((const char*)option, "checkout") == 0) {
            checkOut();
        } else if (strcmp((const char*)option, "booktable") == 0) {
            bookTable();    
        } else if (strcmp((const char*)option, "quit") == 0) {
            finished = 1;
        } else {
            print(500, "Action '%s' not recognised\n", option);
        }
    }
    return 0;
}
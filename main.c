#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define N_ROOMS 6
#define N_TABLES 3
#define N_RAND_DIGITS 3
#define N_TIMESLOTS 2
#define INVALID_TABLE_ENTRY 0
#define TABLE_UNAVAILABLE 0
#define FILE_DOES_NOT_EXIST 2
#define LINE_LENGTH 256
#define NAME_LEN 100
#define MAX_BUFSIZE 6 * (3 * NAME_LEN + 64)

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
            printf("\nPlease try again: ");
            scanf("%s", &str);
        }
    } while (containsInvalidChars);
}

void concatStr(char* buffer, char* str, int* idx) 
{
    size_t len = strlen(str);
    if (*idx + len > MAX_BUFSIZE) {
        printf("Error: cannot concatenate strings: exceeds max buffer size\n");
        exit(EXIT_FAILURE);
    }
    strcat(buffer + *idx, str);
    *idx += len;
}

char* intToString(int n)
{
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
    return idx > 1 ? idx - 1 : idx;
}

int loadBookingData(const char* filename, Booking bookings[N_ROOMS])
{
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        // If the file does not exist, create it
        if (errno == FILE_DOES_NOT_EXIST) {
            f = fopen(filename, "w");
            if (f == NULL) {
                printf("Error: could not create data file\n");
                exit(EXIT_FAILURE);
            }
            fclose(f);
            f = fopen(filename, "r");    
        } else {
            printf("error!\n");
            return 0;
        }
    }
    // Calculate size of the file in bytes 
    fseek(f, 0, SEEK_END);
    size_t fSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    // Allocate buffer of size fSize bytes
    char* data = malloc(fSize);
    int idx = 0, c = 0;
    while ((c = fgetc(f)) != EOF) {
        *(data + idx) = c;
        idx++;
    }  
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
        if (booking.tableNum != INVALID_TABLE_ENTRY && booking.tableSlot != INVALID_TABLE_ENTRY) {
            concatStr(buffer, delim, &charIdx);
            concatStr(buffer, intToString(booking.tableNum), &charIdx);
            concatStr(buffer, delim, &charIdx);
            concatStr(buffer, intToString(booking.tableSlot), &charIdx);
        }
        concatStr(buffer, lineBreak, &charIdx);
    }
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        printf("Error: could not write data to disk\n");
        exit(EXIT_FAILURE);
    }
    fputs(buffer, f);
    fclose(f);
}

// Check in function (Orin)
void checkIn()
{
    Booking bookings[N_ROOMS];
    int nResults = loadBookingData("bookings.csv", bookings);
    printf("%d\n", bookings[nResults - 1].tableNum);
    printf("checking in...\n");
}

// Check out function (Mikhail)
void checkOut()
{
    printf("checking out...\n");
}

// Table booking function (Tom)
void bookTable()
{
    // Load booking data
    Booking bookings[N_ROOMS];
    int nBookings = loadBookingData("bookings.csv", bookings), bookingIdx = -1;
    // Create a 2D array of all possible booking slots at each table
    int timeSlots[N_TIMESLOTS] = { 7, 9 };
    int tables[N_TABLES] = { Endor, Naboo, Tatooine };
    int tablesAvailable[N_TIMESLOTS][N_TABLES] = { { Endor, Naboo, Tatooine }, { Endor, Naboo, Tatooine } };
    // Check booking ID
    char bookingId[NAME_LEN + N_RAND_DIGITS];
    printf("In order to book a table, please enter your booking ID: ");
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
        printf("Sorry, that is an invalid booking ID, you cannot book a table.\n");
        return;
    } else if (bookings[bookingIdx].boardType == "BB") {
        printf("Sorry, you are booked in for Bed & Breakfast, meaning you cannot book a dinner table.\n");
        return;
    } else if (bookings[bookingIdx].tableNum != INVALID_TABLE_ENTRY || bookings[bookingIdx].tableSlot != INVALID_TABLE_ENTRY) {
        printf("Sorry, you already have a table booked. You cannot book another one.\n");
        return;
    }
    int confirmChoice = 0;
    while (!confirmChoice) {
        // Display available tables
        printf("Available tables: \n-----------------\n");
        int idx = 0, newIdx = 0, tableChoice;
        for (int i = 0; i < N_TIMESLOTS; ++i) {
            for (int j = 0; j < N_TABLES; ++j) {
                if (tablesAvailable[i][j] == TABLE_UNAVAILABLE) continue;
                printf("%d: %s | %d:00pm | Serves 4\n", idx + 1, getTableName(tablesAvailable[i][j], 1), (timeSlots[i] + 12) % 24);
                idx++;
            }
        }
        printf("\n");
        do {
            printf("Please select the table you want (1-%d): ", idx);
            scanf("%d", &tableChoice);
            fflush(stdin);
        } while (1 > tableChoice || tableChoice > idx);
        
        int tempTableNum = 0, tempTableSlot = 0;
        for (int i = 0; i < N_TIMESLOTS; ++i) {
            for (int j = 0; j < N_TABLES; ++j) {
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
            printf("Would you like to confirm your booking? (Y/N) ");
            scanf("%c", &choice);
            fflush(stdin);
        } while (choice != 'Y' && choice != 'N' && choice != 'y' && choice != 'n');
        if (choice == 'Y' || choice == 'y') {
            bookings[bookingIdx].tableNum = tempTableNum;
            bookings[bookingIdx].tableSlot = tempTableSlot;
            confirmChoice = 1;
        }
    }
    saveBookingData("bookings.csv", bookings, nBookings);
    printf(
        "Successfully booked a table for %s at %d:00pm\n", 
        getTableName(bookings[bookingIdx].tableNum, 0), 
        (bookings[bookingIdx].tableSlot + 12) % 24
    );
}

// Main user interface
int main(const int argc, const char** argv) 
{
    char option[32];
    printf("Choose an action (checkin, checkout, booktable): ");
    scanf("%s", &option);
        
    if (strcmp((const char*)option, "checkin") == 0) {
        checkIn();

    } else if (strcmp((const char*)option, "checkout") == 0) {
        checkOut();

    } else if (strcmp((const char*)option, "booktable") == 0) {
        bookTable();    

    }
    
    return 0;
}


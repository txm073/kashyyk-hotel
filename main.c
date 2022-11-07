#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define N_ROOMS 6
#define FILE_DOES_NOT_EXIST 2
#define LINE_LENGTH 256
#define NAME_LEN 100
#define NRAND_DIGITS 3
#define MAX_BUFSIZE 6 * (3 * NAME_LEN + 64)

// Booking struct
typedef struct {
    char *firstName, *lastName, *dob, *id, *boardType;
    int nDays, nAdults, nChildren, paper, roomNum;
} Booking;

// The order of the fields in the CSV file
typedef enum {
    firstName,
    lastName,
    dob,
    id,
    nDays,
    nAdults,
    nChildren,
    paper,
    roomNum,
    boardType
} BookingOrder;

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
        char* record = strdup(records[i]);
        removeNewLine(record);
        int fieldIdx = 0;
        char* field = strtok(record, ",");
        while (field != NULL) {
            if (fieldIdx == firstName) booking->firstName = field;
            if (fieldIdx == lastName) booking->lastName = field;
            if (fieldIdx == dob) booking->dob = field;
            if (fieldIdx == id) booking->id = field;
            if (fieldIdx == nDays) booking->nDays = atoi(field);
            if (fieldIdx == nAdults) booking->nAdults = atoi(field);
            if (fieldIdx == nChildren) booking->nChildren = atoi(field);
            if (fieldIdx == paper) booking->paper = atoi(field);
            if (fieldIdx == roomNum) booking->roomNum = atoi(field);
            if (fieldIdx == boardType) booking->boardType = field;
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
        concatStr(buffer, intToString(booking.nDays), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.nAdults), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.nChildren), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.paper), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, intToString(booking.roomNum), &charIdx);
        concatStr(buffer, delim, &charIdx);
        concatStr(buffer, booking.boardType, &charIdx);
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
    for (int i = 0; i < nResults; i++) {
        printf("%s\n", bookings[i].firstName);
        printf("%s\n", bookings[i].lastName);
        printf("%s\n", bookings[i].dob);
        printf("%s\n", bookings[i].id);
        printf("%d\n", bookings[i].nDays);
        printf("%d\n", bookings[i].nAdults);
        printf("%d\n", bookings[i].nChildren);
        printf("%d\n", bookings[i].paper);
        printf("%d\n", bookings[i].roomNum);
        printf("%s\n", bookings[i].boardType);
    }
    printf("checking in...\n");
}

// Check out function (Mikhail)
void checkOut(Booking booking)
{
    printf("checking out...\n");
}

// Table booking function (Tom)
void bookTable(Booking booking)
{
    printf("booking table...\n");
}

// Main user interface
int main(const int argc, const char** argv) 
{
    char option[32];
    printf("Choose an action: (checkin, checkout, booktable) ");
    scanf("%s", &option);
    Booking booking;
        
    if (strcmp((const char*)option, "checkin") == 0) {
        checkIn();

    } else if (strcmp((const char*)option, "checkout") == 0) {
        checkOut(booking);

    } else if (strcmp((const char*)option, "booktable") == 0) {
        bookTable(booking);    

    }
    
    return 0;
}
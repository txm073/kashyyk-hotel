/*
Kashyyyk Hotel Project
----------------------
Authors:
 - Tom Barnes
 - Orin Mockford
 - Mikhail Gray
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <locale.h>
#ifdef _WIN32
#include <Windows.h>
#endif

/* Constants */
#define N_ROOMS 6
#define N_TABLES 3
#define N_RAND_DIGITS 3
#define N_TIMESLOTS 2
#define INVALID_TABLE_ENTRY 0
#define TABLE_UNAVAILABLE -1
#define FILE_DOES_NOT_EXIST 2
#define MAX_DAYS 50
#define BOOKING_FILE "bookings.txt"
#define NAME_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ -"

/* Global variables */

// Booking structure containing information about each booking
typedef struct {
    char *firstName, *lastName, *dob, *id, *boardType;
    int nDays, nAdults, nChildren, paper, roomNum, tableNum, tableSlot, empty;
} Booking;

// The order of the fields in the data file
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

// Table names
typedef enum {
    Endor    = 1,
    Naboo    = 2,
    Tatooine = 3
} Tables;

// Prices for each room
static int roomPrices[N_ROOMS] = { 100, 100, 85, 75, 75, 50 };
// Days per month used to calculate difference between dates
static int daysPerMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* Utility functions */

// Removes any '\n' characters from a string
void removeNewLine(char* s) 
{
    char* d = s;
    do {
        while (*d == '\n') {
            ++d;
        }
    } while (*s++ = *d++);
}

// Trim any leading or trailing whitespace from a string
char* trim(char *str)
{
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
    return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

// Print data to the console then sleep for a period of time
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

// Maps a table enum value to a string, with optional formatting
char* getTableName(Tables table, int pad)
{
    if (table == Endor) return (pad ? "Endor   " : "Endor");
    if (table == Naboo) return (pad ? "Naboo   " : "Naboo");
    if (table == Tatooine) return "Tatooine";
    return NULL;
}

// Input validation
// Check to see if a string contains any invalid characters
// or make sure that it only contains valid characters
int containsInvalidChars(char* str, const char* invalidChars, const char* validChars)
{
    if ((invalidChars == NULL && validChars == NULL) || (invalidChars != NULL && validChars != NULL)) return 0;
    if (invalidChars == NULL) {
        for (int i = 0; i < strlen(str); ++i) {
            int validChar = 0;
            for (int j = 0; j < strlen(validChars); ++j) {
                if (validChars[j] == str[i]) {
                    validChar = 1;
                    break;
                }
            }
            if (!validChar) {
                printf("Only the following characters are allowed: %s\n", validChars);
                return 1;
            }
        }
    } else {
        for (int i = 0; i < strlen(invalidChars); ++i) {
            for (int j = 0; j < strlen(str); ++j) {
                if (invalidChars[i] == str[j]) {
                    printf("The following characters are not allowed: %s\n", invalidChars);
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Generate a pseudorandom integer inbetween to bounds (inclusive)
int randInt(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

// Concatenate two strings and dynamically resize if necessary
char* concatStr(char *s1, const char *s2)
{
    const size_t a = strlen(s1);
    const size_t b = strlen(s2);
    const size_t combinedSize = a + b + 1;

    s1 = realloc(s1, combinedSize);
    if (s1 == NULL) {
        printf("realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s1 + a, s2, b + 1);

    return s1;
}

// Convert an integer to string
// NOTE: this function does not accept negative integers
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

// Parse the .txt/.csv file and populate an array of Booking objects
// with the appropriate data, including handling data conversion
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

// Removes a booking from the booking array, and updates the array and 
// the booking counter variable accordingly
void removeBooking(Booking bookings[N_ROOMS], int idx, int* nBookings)
{
    for (int i = idx + 1; i < *nBookings; ++i) {
        bookings[i - 1] = bookings[i];
    }
    *nBookings = (*nBookings) - 1;
    Booking emptyBooking;
    bookings[*nBookings] = emptyBooking;
}

// High level function to load the booking data from a text file, returning
// the number of bookings that were loaded successfully
int loadBookingData(const char* filename, Booking bookings[N_ROOMS])
{
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        // If the file does not exist, create it
        if (errno == FILE_DOES_NOT_EXIST) {
            f = fopen(filename, "w");
            if (f == NULL) {
                print(500, "error: could not create file\n");
                exit(EXIT_FAILURE);
            }
            fclose(f);
            f = fopen(filename, "r");    
        } else {
            print(500, "error: fopen() failed with code: %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }
    // Calculate size of the file in bytes 
    fseek(f, 0, SEEK_END);
    size_t fSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    // Allocate buffer of size fSize bytes
    char* data = malloc(fSize);
    if (data == NULL) {
        printf("error: malloc() failed\n");
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

// High level function to serialize the booking array into a string and
// save it to a text file
void saveBookingData(const char* filename, Booking bookings[N_ROOMS], int nBookings)
{
    char *buffer = malloc(10), *delim = ",", *lineBreak = ";\n";
    *buffer = '\0';
    int charIdx = 0;
    for (int i = 0; i < nBookings; ++i) {
        Booking booking = bookings[i];
        buffer = concatStr(buffer, booking.firstName);
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, booking.lastName);
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, booking.dob);
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, booking.id);
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, booking.boardType);
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, intToString(booking.nDays));
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, intToString(booking.nAdults));
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, intToString(booking.nChildren));
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, intToString(booking.paper));
        buffer = concatStr(buffer, delim);
        buffer = concatStr(buffer, intToString(booking.roomNum));
        if (booking.tableNum != INVALID_TABLE_ENTRY && booking.tableSlot != INVALID_TABLE_ENTRY 
            && booking.tableNum != TABLE_UNAVAILABLE && booking.tableSlot != TABLE_UNAVAILABLE) {
            buffer = concatStr(buffer, delim);
            buffer = concatStr(buffer, intToString(booking.tableNum));
            buffer = concatStr(buffer, delim);
            buffer = concatStr(buffer, intToString(booking.tableSlot));
        }
        if ((i + 1) != nBookings) {
            buffer = concatStr(buffer, lineBreak);
        }
    }
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        printf("error: could not write data to disk\n");
        exit(EXIT_FAILURE);
    }
    fputs(buffer, f);
    fclose(f);
    free(buffer);
}

// Function that continously reads data from the stdin buffer until the user enters
// a new line character, automatically resizing the buffer and returning it
char* inputString()
{
    fflush(stdin);
    int size = 16, ch = 0;
    size_t len = 0;
    char* str = realloc(NULL, sizeof(char) * size);
    if (!str) {
        printf("error: realloc() failed");
        exit(EXIT_FAILURE);
    }
    while ((ch = fgetc(stdin)) != EOF && ch != '\n'){
        str[len++] = ch;
        if(len == size){
            str = realloc(str, sizeof(char) * (size += 16));
            if (!str) return str;
        }
    }
    str[len++] = '\0';
    return realloc(str, sizeof(char) * len);
}

// Parse a datetime string (DD/MM/YYYY) into integer components for the day, month and year
void parseDateTimeString(const char* str, int* birthDay, int* birthMonth, int* birthYear)
{
    // Parse string passed in
    char birthDayString[3], birthMonthString[3], birthYearString[5];
    strncpy(birthDayString, str, 2);
    birthDayString[2] = '\0';
    strncpy(birthMonthString, str + 3, 2);
    birthMonthString[2] = '\0';
    strncpy(birthYearString, str + 6, 4);
    birthYearString[4] = '\0';
    *birthDay = atoi(birthDayString);
    *birthMonth = atoi(birthMonthString);
    *birthYear = atoi(birthYearString);
}

// Get the current date as a string (DD/MM/YYYY)
char* currentDate()
{
    time_t seconds = time(NULL);
    const struct tm* t = localtime(&seconds);
    char* buffer = malloc(11);     
    strftime(buffer, 11, "%d/%m/%Y", t);
    return buffer;
}

// Calculate the number of days elapsed between two datetime strings
int daysElapsed(char* startDate, char* endDate)
{
    int d1, d2, m1, m2, y1, y2, days = 0;
    parseDateTimeString((const char*)startDate, &d1, &m1, &y1);
    parseDateTimeString((const char*)endDate, &d2, &m2, &y2);
    int doy1 = d1, doy2 = d2;
    for (int i = 0; i < m1 - 1; ++i) doy1 += ((i == 2 && y1 % 4 == 0) ? 29 : daysPerMonth[i]);
    int drem1 = y1 % 4 == 0 ? 366 - doy1 : 365 - doy1;
    for (int i = 0; i < m2 - 1; ++i) doy2 += ((i == 2 && y2 % 4 == 0) ? 29 : daysPerMonth[i]);
    if (y1 == y2) return doy2 - doy1;
    days = days + drem1 + doy2;
    if (y1 < y2) {
        for (int year = y1; year < y2 - 1; ++year) {
            days += (year % 4 == 0) ? 366 : 365;
        }
    } else {
        for (int year = y2; year < y1 + 1; ++year) {
            days -= (year % 4 == 0) ? 366 : 365;
        }
    }
    return days;
}

// Validate that the user has inputted a valid date of birth
int validDOB(char* str)
{
    if (strlen(str) != 10) {
        return 0;
    }
    for (int i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) {
            if (str[i] != '/') return 0;
        } else {
            if (!isdigit(str[i])) return 0;
        }
    } 
    float age = daysElapsed(str, currentDate()) / 365.25f;
    if (age < 0) {
        printf("Invalid age!\n");
        return 0;
    } else if (age < 18) {
        printf("You must be over 18 to book a room.\n");
        return 0;
    }

    // Parse string passed in
    int birthDay, birthMonth, birthYear;
    parseDateTimeString(str, &birthDay, &birthMonth, &birthYear);

    if (birthYear < 1900) {
        printf("Invalid date!\n");
        return 0;
    }
    // Day is within the month
    if (daysPerMonth[birthMonth - 1] < birthDay || birthDay < 1) {
        // Edge case to allow leap years
        int leapYear = birthYear % 4 == 0;
        if (leapYear && birthMonth == 2) {
            if (birthDay < 1 || birthDay > 29) {
                printf("Day must be between 1 and 29!\n");
                return 0;
            }
        } else {
            printf("Day must be between 1 and %d!\n", daysPerMonth[birthMonth - 1]);
            return 0;
        }
    }
    // Month is between 1 and 12
    if (birthMonth > 12 || birthMonth < 1) {
        printf("Month must be between 1 and 12!\n");
        return 0;
    }
    return 1;
}

/* Main program logic */

// Check in function (Orin)
void checkIn()
{
    Booking bookings[N_ROOMS];

    int nBookings = loadBookingData(BOOKING_FILE, bookings);
    if (nBookings == N_ROOMS) {
        printf("Sorry, there are no rooms available at the moment.\n");
        return;
    }
    int confirmedDetails = 0;
    while (!confirmedDetails) {
        Booking booking;
        do {
            printf("Please enter your first name: ");
            booking.firstName = trim(inputString());
        } while (containsInvalidChars(booking.firstName, NULL, NAME_CHARS) || strlen(booking.firstName) == 0);

        do {
            printf("Please enter your last name: ");
            booking.lastName = trim(inputString());
        } while (containsInvalidChars(booking.lastName, NULL, NAME_CHARS) || strlen(booking.lastName) == 0);

        printf("\n");
        booking.dob = "00/00/0000";
        do {
            printf("Please enter your date of birth (DD/MM/YYYY): ");
            booking.dob = trim(inputString());
        } while (!validDOB(booking.dob));

        int idSize = strlen(booking.lastName) + 2, idExists = 0;    
        do {
            idExists = 0;
            booking.id = realloc(booking.id, idSize);
            sprintf(booking.id, "%s%d", booking.lastName, rand() % 10);
            for (int i = 0; i < nBookings; ++i) {
                if (strcmp(booking.id, bookings[i].id) == 0) {
                    idExists = 1;
                }
            }
        } while (idExists);
        printf("\nHere is your booking id: %s\n", booking.id);
        printf("Do not forget it, you will need it to book a table and to check out\n");

        printf("\nAvailable board types:\n--------------------\n");
        wprintf(L"1: Full-Board (FB)      | £20 per person, per day\n");
        wprintf(L"2: Half-Board (HB)      | £15 per person, per day\n");
        wprintf(L"3: Bed & Breakfast (BB) | £5  per person, per day\n");
        int choice = 0;
        do {
            printf("Select a board type (1-3): ");
            scanf("%d", &choice);
            fflush(stdin);
        } while (choice > 3 || choice < 1);
        if (choice == 1) booking.boardType = "FB";
        else if (choice == 2) booking.boardType = "HB";
        else if (choice == 3) booking.boardType = "BB";
        printf("\n");

        do {
            printf("How many days will you be staying? (1-%d) ", MAX_DAYS);
            scanf("%d", &booking.nDays);
            fflush(stdin);
        } while (booking.nDays < 1 || booking.nDays > MAX_DAYS);

        do {
            printf("\nA room can have a maximum of 4 guests\n");
            printf("How may adults will be staying? ");
            scanf("%d", &booking.nAdults);
            fflush(stdin);
            printf("How may children will be staying? ");
            scanf("%d", &booking.nChildren);
            fflush(stdin);
        } while ((booking.nAdults + booking.nChildren) > 4 || (booking.nAdults + booking.nChildren) < 1);

        // Create array of all rooms
        int roomsAvailable[N_ROOMS];
        for (int i = 0; i < N_ROOMS; ++i) {
            roomsAvailable[i] = i + 1;
        }
        // Filter out unavailable rooms
        for (int i = 0; i < nBookings; ++i) {
            for (int j = 0; j < N_ROOMS; ++j) {
                if (roomsAvailable[j] == bookings[i].roomNum) {
                    roomsAvailable[j] = 0;
                }
            }
        }
        // Display available rooms and prompt user for selection
        printf("\nHere are the available rooms:\n-----------------------------\n");
        int nRoomsAvailable = 0;
        for (int i = 0; i < N_ROOMS; ++i) {
            if (roomsAvailable[i] == 0) continue;
            printf("Room %d | ", roomsAvailable[i]);
            wprintf(L"£");
            printf("%d per day\n", roomPrices[i]);
            nRoomsAvailable++;
        }
        int validRoom = 0;
        while (!validRoom) {
            printf("Please select a room [");
            char choiceBuffer[20] = "";
            for (int i = 0; i < N_ROOMS; i++) {
                if (roomsAvailable[i] == 0) continue;
                strcat(choiceBuffer, intToString(roomsAvailable[i]));
                strcat(choiceBuffer, ", "); 
            }
            choiceBuffer[3 * (N_ROOMS - nBookings) - 2] = '\0';
            printf(choiceBuffer);
            printf("]: ");
            scanf("%d", &booking.roomNum);
            fflush(stdin);
            for (int i = 0; i < N_ROOMS; i++) {
                if (roomsAvailable[i] == booking.roomNum) validRoom = 1;
            }
        }    
        printf("\n");
        char paperChoice;
        do {
            printf("Would you like to receive a newspaper each morning? (Y/N) ");
            scanf("%c", &paperChoice);
            fflush(stdin);
        } while (paperChoice != 'Y' && paperChoice != 'N' && paperChoice != 'y' && paperChoice != 'n');
        booking.paper = (paperChoice == 'Y' || paperChoice == 'y') ? 1 : 0;
        
        printf("\nYour information:\n");
        printf("-----------------\n");
        printf("First Name    | %s\n", booking.firstName);
        printf("Last Name     | %s\n", booking.lastName);
        printf("Date of Birth | %s\n", booking.dob);
        printf("Board Type    | %s\n", booking.boardType);
        printf("Room Number   | %d\n", booking.roomNum);
        printf("Stay length   | %d days\n", booking.nDays);
        printf("No. Adults    | %d\n", booking.nAdults);
        printf("No. Children  | %d\n", booking.nChildren);
        printf("Newspaper     | %s\n", booking.paper ? "Yes" : "No");

        printf("\n");
        char confirm;
        do {
            printf("Confirm your booking? (Y/N) ");
            scanf("%c", &confirm);
            fflush(stdin);
        } while (confirm != 'Y' && confirm != 'N' && confirm != 'y' && confirm != 'n');
        confirmedDetails = (confirm == 'Y' || confirm == 'y') ? 1 : 0;
        bookings[nBookings] = booking;
    }
    nBookings++;
    saveBookingData(BOOKING_FILE, bookings, nBookings);
    printf("Check-in was successful!\n");
}

// Check out function (Mikhail)
void checkOut() 
{
    // Load booking data
    Booking bookings[N_ROOMS];
    int nBookings = loadBookingData(BOOKING_FILE, bookings), bookingIdx = -1;
    printf("Please enter your booking ID: ");
    char* bookingId = inputString();
    for (int i = 0; i < nBookings; i++) {
        if (strcmp(bookings[i].id, bookingId) == 0) {
            bookingIdx = i;
            break;
        }
    }
    if (bookingIdx == -1) {
        printf("Invalid booking ID\n");
        return;
    }
    float roomCost = bookings[bookingIdx].nDays * roomPrices[bookings[bookingIdx].roomNum - 1];
    // Over 65's get 10% discount
    int discount = ((daysElapsed(bookings[bookingIdx].dob, currentDate()) - bookings[bookingIdx].nDays) / 365.25f) > 65.0f;
    if (discount) {
        roomCost *= 0.9f;
    }
    int nMeals = -1;
    do {
        printf("How many meals have you had? ");
        scanf("%d", &nMeals);
        fflush(stdin);
    } while (nMeals == -1);
    int costPerMeal;
    if (strcmp(bookings[bookingIdx].boardType, "BB") == 0) costPerMeal = 5;
    else if (strcmp(bookings[bookingIdx].boardType, "HB") == 0) costPerMeal = 15;
    else if (strcmp(bookings[bookingIdx].boardType, "FB") == 0) costPerMeal = 20;
    else {
        printf("error: invalid board type: %s\n", bookings[bookingIdx].boardType);
        printf("error: data has been corrupted\n");
        removeBooking(bookings, bookingIdx, &nBookings);
        saveBookingData(BOOKING_FILE, bookings, nBookings);
        exit(EXIT_FAILURE);
    }
    float mealCost = nMeals * costPerMeal * bookings[bookingIdx].nAdults + nMeals * costPerMeal * bookings[bookingIdx].nChildren / 2;
    float paperCost = bookings[bookingIdx].paper ? 5.5f : 0.0f;

    float total = roomCost + mealCost + paperCost;
    printf("\nBill:\n");
    for (int i = 0; i < (strlen(bookings[bookingIdx].firstName) + strlen(bookings[bookingIdx].lastName) + 13); ++i) printf("-");
    printf("\nName      | %s %s\n", bookings[bookingIdx].firstName, bookings[bookingIdx].lastName);
    printf("User ID   | %s\n", bookings[bookingIdx].id);
    printf("Guests    | %d adults, %d children\n", bookings[bookingIdx].nAdults, bookings[bookingIdx].nChildren);
    wprintf(L"Room      | £%.2f", roomCost);
    if (discount) printf(" (with 10%% discount for guests over 65)\n");
    else printf("\n");
    if (mealCost > 0) wprintf(L"Meals     | £%.2f\n", mealCost);
    if (paperCost > 0) wprintf(L"Newspaper | £%.2f\n", paperCost);
    wprintf(L"Total     | £%.2f\n", total);

    float amountPaid, truncatedTotal = floorf(total * 100) / 100;
    do {
        wprintf(L"Please enter the total bill (£%.2f): ", total);
        scanf("%f", &amountPaid);
        fflush(stdin);
    } while (amountPaid != truncatedTotal);

    removeBooking(bookings, bookingIdx, &nBookings);
    saveBookingData(BOOKING_FILE, bookings, nBookings);
    printf("\nThank you for staying at the Kashyyyk Hotel!\n");
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
    print(500, "In order to book a table, please enter your booking ID: ");
    char* bookingId = trim(inputString());
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
        }
    }
    if (bookingIdx == -1) {
        print(500, "Sorry, that is an invalid booking ID, you cannot book a table.\n");
        return;
    } else if (strcmp(bookings[bookingIdx].boardType, "BB") == 0) {
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
    setlocale(LC_CTYPE, ""); // enable utf-8 in the console
    srand(time(NULL)); // random seed 
    
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
        } else if (strcmp((const char*)option, "cls") == 0) {
            system("cls");
        } 
        else {
            print(500, "Action '%s' not recognised\n", option);
        }
    }
    return 0;
}

/* EOF */
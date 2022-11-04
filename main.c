#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* firstName, lastName, dob, id;
    int nDays, nAdults, nChildren, paper, roomNum;
    char boardType;
} Booking;

void checkIn()
{
    Booking booking;
    printf("checking in...\n");
}

void checkOut(Booking booking)
{
    printf("checking out...\n");
}

void bookTable(Booking booking)
{
    printf("booking table...\n");
}

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
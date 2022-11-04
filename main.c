#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char* firstName, lastName, dob, id;
    int nDays, nAdults, nChildren, paper, roomNum;
    char boardType;
} Booking;

void checkIn(Booking* booking)
{

}

void checkOut(Booking booking)
{

}

void bookTable(Booking booking)
{

}

int main(const int argc, const char** argv) 
{
    char option[32];
    printf("Choose an action: ");
    scanf("%s", &option);
    Booking booking;
        
    if (strcmp((const char*)option, "check in") == 0) {
        checkIn(&booking);

    } else if (strcmp((const char*)option, "check out") == 0) {
        checkOut(booking);

    } else if (strcmp((const char*)option, "book table") == 0) {
        bookTable(booking);    

    }
    
    return 0;
}


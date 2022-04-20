#include "area.h"
#include <stdio.h>

void check_area(unsigned char message[4]){
    unsigned char red_arr[2];
    unsigned char green_arr[2];
    unsigned char blue_arr[2];
    unsigned char orange_arr[2];
    unsigned char pink_arr[2];

    unsigned char area[2];

    area[0] = message[0] & 0xFF;
    area[1] = message[1] & 0xFF; // area of bounding box

    unsigned char red = 0x1;
    unsigned char green = 0x2;
    unsigned char blue = 0x3;
    unsigned char orange = 0x4;
    unsigned char pink = 0x5;

    int bigger = 0;

    unsigned char type = message[3]; // colour
    if(type == red){
        bigger = change_in_area(area, red_arr);
    }
    else if(type == green){
        bigger = change_in_area(area, green_arr);
    }
    else if(type == blue){
        bigger = change_in_area(area, blue_arr);
    }
    else if(type == orange){
        bigger = change_in_area(area, pink_arr);
    }
    else if(type == pink){
        bigger = change_in_area(area, orange_arr);
    }
    else{
        printf("Unknown colour code. Colour code: %x\n", type);
    }

    switch(bigger){
        // send bigger info to command
        case 1: 
        printf("Area gotten bigger; send info to command");
        break;
        case 0: 
        printf("Area stayed same/initialisation");
        break;
        case -1: 
        printf("Area gotten smaller; send info to command");
        break;
        default:
        printf("Unknown bigger case. Bigger is: %d", bigger);
        break;
    }
}

int change_in_area(unsigned char area[2], unsigned char arr[2]){
    // if arr[0] == -1, it is the first value i.e. initialisation
    if(arr[0] == -1 && arr[1] == -1){  
        arr[0] = area[0];
        arr[1] = area[1];
        return 0;
    }

    if(arr[0] > 0){
        if(arr[1] < area[1]){
            return 1; // area is getting bigger
        }
        else if(arr[1] == area[1]){
            if(arr[0] < area[0]){
                return 1; // area is getting bigger
            }
            if(arr[0] == area[0]){
                return 0; // area stayed the same
            }
        }
    }
    return -1;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>





void strToUpper(char str[]){
    for(int i=0; str[i]!='\0'; i++)
    {
        str[i] = toupper(str[i]);
    }
}

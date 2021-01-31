#include <ctype.h>


// funkcja zmieniająca stringi na uppercase
void str_upper(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = (char) toupper(str[i]);
    }
}

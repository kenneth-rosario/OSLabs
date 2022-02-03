#include "include/ex3.h"

void decode(char* cmd){
    int string_length = strlen(cmd);

    // Invalid check
    int potential_root_square = (int) sqrt((double)string_length);
    if (potential_root_square*potential_root_square != string_length) {
        printf("INVALID\n");
        return;
    }

    int i = 0;
    for (;i<potential_root_square;++i) {
        int offset = 0;
        while (i + offset < string_length) {
            printf("%c", cmd[i+offset]);
            offset += potential_root_square;
        }
    }
    printf("\n");
}

int main(void){
    // Tests Variables
    char cmd[MAX_CHARS];

    //Test 1
    strcpy(cmd, "WE OUE OUT LNGSAO H RWDN!");
    decode(cmd); // Expected: WE ARE LOW ON DOUGHNUTS !
    memset(cmd,0,MAX_CHARS);

    //Test 2
    strcpy(cmd, "S EFEM FNOCEDROE");
    decode(cmd); // Expected: SEND MORE COFFEE
    memset(cmd,0,MAX_CHARS);

    //Test 3
    strcpy(cmd, "CSERULES");
    decode(cmd); // Expected: INVALID
    memset(cmd,0,MAX_CHARS);

    return 0;
}
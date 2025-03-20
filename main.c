#include <stdio.h>
#include "include/ps1.h"

int main() 
{
    ps1* PS1 = ps1_create();
    ps1_init(PS1);
    ps1_load_bios(PS1);
    printf("Hello, World!\n");

    ps1_destroy(PS1);
    return 0;
}

#include "include/ps1.h"
#include <stdio.h>
#include <stdbool.h>

int main() 
{
    ps1* PS1 = ps1_create();
    ps1_init(PS1);
    ps1_load_bios(PS1);

    while(true)
        ps1_play(PS1);
    
    ps1_destroy(PS1);
    return 0;
}

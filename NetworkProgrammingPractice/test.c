#include <stdio.h>

int main()
{

    int a = 4;
#ifdef DEBUG
    printf("This is a debugging log" );
#endif

    for(int i = 0 ; i < 4; ++i)
    {
        printf("Using GCC!\n");
    }
}
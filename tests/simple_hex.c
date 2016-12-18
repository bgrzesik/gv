
#include <stdlib.h>
#include <stdio.h>


int main(int argc, const char **argv)
{
    static const char *hex_digits = "0123456789ABCDEF";

    int a;
    scanf("%d", &a);

    while (a != -1) {
        printf("%d %c\n", a, hex_digits[a % 16]);
        a /= 16;

        if (a < 16) {
            a = -1;
        }
    }

    printf("\n");

    return EXIT_SUCCESS;
}
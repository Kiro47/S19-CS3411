#include <stdio.h>
#include "arraysum.h"

int main() {
    int array[] = {15, 18, 14, 2, 13, 3, 19, 1, 10, 17};
    int arraysize = 10;
    int sum = dosum(array, arraysize);
    int max = domax(array, arraysize);

    printf("Sum is: %d Max is: %d\n", sum, max);

    return 0;
}

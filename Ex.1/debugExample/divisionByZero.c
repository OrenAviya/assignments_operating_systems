#include <stdio.h>

int main() {
    int numerator = 10;
    int denominator = 0;
    printf("numerator: %d\n" , numerator);
    printf("denominator: %d\n" , denominator);

    int res = numerator / denominator;

    // This line will not be reached if the division by zero occurs
    printf("Result of division: %d\n", res);

    return 0;
}

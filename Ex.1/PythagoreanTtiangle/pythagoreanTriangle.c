//
// Created by aviya on 1/23/24.
//

#include <stdio.h>
#include <math.h>

int main() {
    double a, b, c;

    // Get input from the user
    printf("Enter the lengths of the three sides of the triangle: ");
    scanf("%lf %lf %lf", &a, &b, &c);

    // Check if it's a Pythagorean triangle
    if (a > 0 && b > 0 && c > 0) {
        if (a * a + b * b == c * c || a * a + c * c == b * b || b * b + c * c == a * a) {
            // Calculate angles in radians
            double angleA = asin(b / c);
            double angleB = asin(a / c);
            double angleC = asin(a / b);

            // Print angles in radians
            printf("Angles in radians: A=%.4f, B=%.4f, C=%.4f\n", angleA, angleB, angleC);
        } else {
            // Not a Pythagorean triangle
            printf("Error\n");
            return 1; // Exit with an error code
        }
    } else {
        // Invalid side lengths
        printf("Error: Side lengths must be greater than zero\n");
        return 1; // Exit with an error code
    }

    return 0; // Exit successfully
}

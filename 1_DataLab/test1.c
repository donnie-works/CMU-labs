#include <stdlib.h>
#include <stdio.h>
// test return values of different logical operations 
// on binary input values

// Function to print binary representation of a number
void printBinary(int n) {
    if (n == 0) {
        printf("0");
        return;
    }

    int binary[32];
    int index = 0;

    // Handle negative numbers (two's complement)
    unsigned int num = (unsigned int)n;
    
    while (num > 0 && index < 32) {
        binary[index] = num % 2;
        num = num / 2;
        index++;
    }

    // Print in reverse order (most significant bit first)
    for (int i = index - 1; i >= 0; i--) {
        printf("%d", binary[i]);
    }
}

int main() {
    int x = -1;
    int y = 1;
    
    // Perform logical operation
    //Legal ops: ! ~ & ^ | + Max Ops: 10
    int chk = !(((x + 1) ^ (~x)) | !(x + 1));
    
    // Display the operation
    printf("Input values:\n");
    printf("x = %d (binary: ", x);
    printBinary(x);
    printf(")\n");
    
    printf("y = %d (binary: ", y);
    printBinary(y);
    printf(")\n\n");
    
    printf("Operation: !(((x + 1) ^ (~x)) | !(x + 1))\n");
    printf("Result: %d (binary: ", chk);
    printBinary(chk);
    printf(")\n");
    
    // Test with other values
    /*
    printf("\n--- Additional Tests ---\n");
    
    int test_cases[][2] = {
        {5, 3},
        {12, 7},
        {15, 15},
        {8, 4}
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int a = test_cases[i][0];
        int b = test_cases[i][1];
        int result = (~a & b) & (a & ~b);
        
        printf("\nx=%d, y=%d: result=%d (binary: ", a, b, result);
        printBinary(result);
        printf(")\n");
    }
    */
    return 0;
}

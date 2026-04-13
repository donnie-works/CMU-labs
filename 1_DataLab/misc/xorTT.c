#include <stdlib.h>
#include <stdio.h>

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
    // Test XOR truth table with single bits (0 and 1)
    printf("=== XOR TRUTH TABLE TEST (using only ~ and &) ===\n");
    printf("Formula: ~(~(x & ~y) & ~(~x & y))\n\n");
    
    int truth_table[][2] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };
    
    for (int i = 0; i < 4; i++) {
        int x = truth_table[i][0];
        int y = truth_table[i][1];
        
        // XOR using only NOT and AND
        int xor_result = ~(~(x & ~y) & ~(~x & y));
        
        // For comparison, actual XOR
        int actual_xor = x ^ y;
        
        printf("x=%d, y=%d: result=%d (actual XOR=%d) %s\n", 
               x, y, xor_result, actual_xor,
               (xor_result == actual_xor) ? "✓" : "✗");
    }
    
    // Test with multi-bit values
    printf("\n=== MULTI-BIT VALUE TESTS ===\n");
    
    int test_cases[][2] = {
        {5, 3},
        {12, 7},
        {15, 15},
        {8, 4},
        {255, 128}
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int a = test_cases[i][0];
        int b = test_cases[i][1];
        
        // XOR using only NOT and AND
        int result = ~(~(a & ~b) & ~(~a & b));
        
        // Actual XOR for comparison
        int actual = a ^ b;
        
        printf("\nx=%d (", a);
        printBinary(a);
        printf("), y=%d (", b);
        printBinary(b);
        printf(")\n");
        printf("Result: %d (", result);
        printBinary(result);
        printf(") %s\n", (result == actual) ? "✓ Matches XOR" : "✗ ERROR");
    }
    
    return 0;
}

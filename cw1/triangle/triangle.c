/* Classify a triangle according to the integer lengths of its sides. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

// Integer constants representing types of triangle.
enum { Equilateral, Isosceles, Right, Scalene, Flat, Impossible, Illegal };

bool checkDigit(char digit){
     if(digit >= '0' && digit <= '9') return true;
     return false;
}

// Convert a string into an integer.  Return -1 if it is not valid.
int convert(const char length[]) {
     //Reduce number of stlen calls
     int len = strlen(length);

     //If the argument has leading zeroes, reject.
     if(len > 1 && length[0] == '0') return -1;

     //For each character in the argument...
     for(int i = 0; i < len; i++) {
          //If not a digit (0-9), reject.
          if(checkDigit(length[i]) == false) return -1;
     }
     int value = atoi(length);
     //Negative lengths are illegal
     if(value > 0) return value;
     return -1;
}

//Sorts elements in list to ascending numerical order.
void bubbleSort(int n, int sides[]) {
     for(int i = 0; i < n; i++){
          //j starts on 1 (to avoid indexing error)
          //Ends on n - i, elements after are already sorted
          for(int j = 1; j < n - i; j++){
               //Checks if current element should be below previous - is less than
               if(sides[j] < sides[j-1]){
                    int temp = sides[j-1];
                    sides[j-1] = sides[j];
                    sides[j] = temp;
               }
          }
     }
}

bool isLegal(int sides[]){
     //Number of sides known, it's a triangle.
     for(int i = 0; i < 3; i++){
          if(sides[i] < 0) return false;
     }
     return true;
}

int isPossible(int sides[]){
     //Cast to long to prevent overflow
     long delta = (long)sides[2] - ((long)sides[0] + (long)sides[1]);
     return delta;
}

//Checks how many sides are the same
int similar(int sides[]){
     int matches = 1;
     for(int i = 1; i < 3; i++){
          //If two sides are equal, they match
          if(sides[i] == sides[i - 1]) matches++;
     }
     return matches;
}

//Finds if a^2 + b^2 = c^2
bool pythagoras(int sides[]){
     //Squares all of the sides. Element 2 is the largest (list is sorted), so is c
     //Make long so overflow doesn't occur
     long cSquared = (long)sides[2] * (long)sides[2];
     long aSquared = (long)sides[0] * (long)sides[0];
     long bSquared = (long)sides[1] * (long)sides[1];
     if(cSquared == aSquared + bSquared) return true;
     return false;
}

// Classify a triangle, given side lengths as strings:
int triangle(int a, int b, int c) {
    int sides[3] = {a, b, c};
    bubbleSort(3, sides);

    if(isLegal(sides) == false) return Illegal;

    //Difference between hypotenuse and shorter sides
    int delta = isPossible(sides);
    if(delta > 0) return Impossible;
    if(delta == 0) return Flat;

    //Number of sides that are equal to each other (one if no matches)
    int similarSides = similar(sides);
    if(similarSides == 3) return Equilateral;
    if(similarSides == 2) return Isosceles;

    bool isRightAngled = pythagoras(sides);
    if(isRightAngled) return Right;
    else return Scalene;
}

// -----------------------------------------------------------------------------
// User interface and testing.

void print(int type) {
    switch (type) {
        case Equilateral: printf("Equilateral"); break;
        case Isosceles: printf("Isosceles"); break;
        case Right: printf("Right"); break;
        case Scalene: printf("Scalene"); break;
        case Flat: printf("Flat"); break;
        case Impossible: printf("Impossible"); break;
        case Illegal: printf("Illegal"); break;
    }
    printf("\n");
}

// A replacement for the library assert function.
void assert(int line, bool b) {
    if (b) return;
    printf("The test on line %d fails.\n", line);
    exit(1);
}

// Check that you haven't changed the triangle type constants.  (If you do, it
// spoils automatic marking, when your program is linked with a test program.)
void checkConstants() {
    assert(__LINE__, Equilateral==0 && Isosceles==1 && Right==2 && Scalene==3);
    assert(__LINE__, Flat==4 && Impossible==5 && Illegal==6);
}

// Tests 1 to 2: check equilateral
void testEquilateral() {
    assert(__LINE__, triangle(8, 8, 8) == Equilateral);
    assert(__LINE__, triangle(1073, 1073, 1073) == Equilateral);
}

// Tests 3 to 5: check isosceles
void testIsosceles() {
    assert(__LINE__, triangle(25, 25, 27) == Isosceles);
    assert(__LINE__, triangle(25, 27, 25) == Isosceles);
    assert(__LINE__, triangle(27, 25, 25) == Isosceles);
}

// Tests 6 to 14: check right angled
void testRight() {
    assert(__LINE__, triangle(3, 4, 5) == Right);
    assert(__LINE__, triangle(3, 5, 4) == Right);
    assert(__LINE__, triangle(5, 3, 4) == Right);
    assert(__LINE__, triangle(5, 12, 13) == Right);
    assert(__LINE__, triangle(5, 13, 12) == Right);
    assert(__LINE__, triangle(12, 5, 13) == Right);
    assert(__LINE__, triangle(12, 13, 5) == Right);
    assert(__LINE__, triangle(13, 5, 12) == Right);
    assert(__LINE__, triangle(13, 12, 5) == Right);
}

// Tests 15 to 20: check scalene
void testScalene() {
    assert(__LINE__, triangle(12, 14, 15) == Scalene);
    assert(__LINE__, triangle(12, 15, 14) == Scalene);
    assert(__LINE__, triangle(14, 12, 15) == Scalene);
    assert(__LINE__, triangle(14, 15, 12) == Scalene);
    assert(__LINE__, triangle(15, 12, 14) == Scalene);
    assert(__LINE__, triangle(15, 14, 12) == Scalene);
}

// Tests 21 to 25: check flat
void testFlat() {
    assert(__LINE__, triangle(7, 9, 16) == Flat);
    assert(__LINE__, triangle(7, 16, 9) == Flat);
    assert(__LINE__, triangle(9, 16, 7) == Flat);
    assert(__LINE__, triangle(16, 7, 9) == Flat);
    assert(__LINE__, triangle(16, 9, 7) == Flat);
}

// Tests 26 to 31: check impossible
void testImpossible() {
    assert(__LINE__, triangle(2, 3, 13) == Impossible);
    assert(__LINE__, triangle(2, 13, 3) == Impossible);
    assert(__LINE__, triangle(3, 2, 13) == Impossible);
    assert(__LINE__, triangle(3, 13, 2) == Impossible);
    assert(__LINE__, triangle(13, 2, 3) == Impossible);
    assert(__LINE__, triangle(13, 3, 2) == Impossible);
}

// Tests 32 to 44: check conversion.
// Leading zeros are disallowed because thy might be mistaken for octal.
void testConvert() {
    assert(__LINE__, convert("1") == 1);
    assert(__LINE__, convert("12345678") == 12345678);
    assert(__LINE__, convert("2147483647") == 2147483647);
    assert(__LINE__, convert("2147483648") == -1);
    assert(__LINE__, convert("2147483649") == -1);
    assert(__LINE__, convert("0") == -1);
    assert(__LINE__, convert("-1") == -1);
    assert(__LINE__, convert("-2") == -1);
    assert(__LINE__, convert("-2147483648") == -1);
    assert(__LINE__, convert("x") == -1);
    assert(__LINE__, convert("4y") == -1);
    assert(__LINE__, convert("13.4") == -1);
    assert(__LINE__, convert("03") == -1);
}

// Tests 45 to 50: check for correct handling of overflow
void testOverflow() {
    assert(__LINE__, triangle(2147483647,2147483647,2147483646) == Isosceles);
    assert(__LINE__, triangle(2147483645,2147483646,2147483647) == Scalene);
    assert(__LINE__, triangle(2147483647,2147483647,2147483647) == Equilateral);
    assert(__LINE__, triangle(1100000000,1705032704,1805032704) == Scalene);
    assert(__LINE__, triangle(2000000001,2000000002,2000000003) == Scalene);
    assert(__LINE__, triangle(150000002,666666671,683333338) == Scalene);
}

// Run tests on the triangle function.
void test() {
    checkConstants();
    testEquilateral();
    testIsosceles();
    testRight();
    testScalene();
    testFlat();
    testImpossible();
    testConvert();
    testOverflow();
    printf("All tests passed\n");
}

// Run the program or, if there are no arguments, test it.
int main(int n, char *args[n]) {
    setbuf(stdout, NULL);
    if (n == 1) {
        test();
    }
    else if (n == 4) {
        int a = convert(args[1]), b = convert(args[2]), c = convert(args[3]);
        int result = triangle(a, b, c);
        print(result);
    }
    else {
        fprintf(stderr, "Use e.g.: ./triangle 3 4 5\n");
        exit(1);
    }
    return 0;
}

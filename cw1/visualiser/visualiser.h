#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>
#include <float.h>

#define bitsof(x) ( sizeof(x) * 8 )

long double powerOf(int n, int e){
     if(n == 0) return 1;
     return (long double)e * powerOf(n - 1, e);
}

unsigned long absVal(long n){
     if(n < 0) return (unsigned long)(n * (unsigned long)-1);
     return n;
}

double absValF(double n){
     if(n < 0) return n * - 1;
     return n;
}

void trimFront(char string[], int x){
     int len = strlen(string);
     for(int i = x; i < len; i++)
     {
          string[i - x] = string[i];
     }
     for(int i = 1; i <= x; i++){
          string[len - i] = 0;
     }
}

int getTypeLength(int type){
     switch(type){
          case 0: return bitsof(char);
          case 1: return bitsof(short);
          case 2: return bitsof(int);
          case 3: return bitsof(long);
          case 4: return bitsof(double);
          case 5: return bitsof(float);
          case 6: return 64;
          default: return 0;
     }
}

long getLimits(int type, bool isSigned, bool maxOrMin){
     switch(type){
          case 0:
               if(maxOrMin){
                    if(isSigned) return CHAR_MAX;
                    return (long)CHAR_MAX - (long)CHAR_MIN;
               }
               if(isSigned) return CHAR_MIN;
                    return 0;
          case 1:
               if(maxOrMin){
                    if(isSigned) return SHRT_MAX;
                    return (long)SHRT_MAX - (long)SHRT_MIN;
               }
               if(isSigned) return SHRT_MIN;
                    return 0;
          case 2:
               if(maxOrMin){
                    if(isSigned) return INT_MAX;
                    return (long)INT_MAX - (long)INT_MIN;
               }
               if(isSigned) return INT_MIN;
                    return 0;
          case 3:
               if(maxOrMin){
                    if(isSigned) return LONG_MAX;
                    return (unsigned long)LONG_MAX - (long)LONG_MIN;
               }
               if(isSigned) return LONG_MIN;
                    return 0;
     }
     return 0;
}

int getTypeLengthFloat(int type, bool mantissa){
     switch(type){
          case 4: if(mantissa) return DBL_MANT_DIG;
               return (int)bitsof(double) - (DBL_MANT_DIG);
          case 5: if(mantissa) return (FLT_MANT_DIG);
               return (int)bitsof(float) - (FLT_MANT_DIG);
          default: return 0;
     }
}

long getLimitsFloat(int type, bool maxOrMin, bool mantissa){
     if(mantissa){
          if(maxOrMin) { return powerOf(getTypeLengthFloat(type, true) - 1, 2) - 1;}
          return -1 * powerOf(getTypeLengthFloat(type, true) - 1, 2);
     }
     if(maxOrMin) return powerOf(getTypeLengthFloat(type, false) - 1, 2) - 1;
     return -1 * powerOf(getTypeLengthFloat(type, false) - 1, 2);
}

bool checkValue(long calculated, char *input){
     char check[30];
     sprintf(check, "%ld", calculated);
     if(strcmp(check, input) != 0) return false;
     return true;
}

bool checkValueULong(unsigned long calculated, char *input){
     char check[30];
     sprintf(check, "%lu", calculated);
     if(strcmp(check, input) != 0) return false;
     return true;
}

bool checkDecDigit(char digit)
{
     if(digit >= '0' && digit <= '9') return true;
     if(digit == '-') return true;
     if(digit == '.') return true;
     return false;
}

bool checkSyntax(char digit, int i, int len, bool isSigned, int *numberOfPoints){
     if(digit == '-' && !isSigned) { return false; }
     //No leading zeroes in a non-zero value
     if(digit == '0' && i == 0 && len > 1) { return false;; }
     //Cannot have a negative sign anywhere but in the first character
     if(digit == '-' && i != 0) { return false; }
     //No decimal points at the front or end of the value
     if(digit == '.' && (i == 0 || i == len - 1)) { return false; }
     if(digit == '.' && *numberOfPoints <= 1) { *numberOfPoints = *numberOfPoints + 1; }
     if(digit == '.' && *numberOfPoints > 1) { return false; }
     if(!checkDecDigit(digit)) { return false; }
     return true;
}

bool checkFloat(long mantissa, long exponent, int type){
     printf("m = %li, e = %li", mantissa, exponent);
     if(mantissa > getLimitsFloat(type, true, true)) return false;
     if(mantissa < getLimitsFloat(type, false, true)) return false;

     if(exponent > getLimitsFloat(type, true, false)) return false;
     if(exponent < getLimitsFloat(type, false, false)) return false;

     return true;
}

void negate(int x, bool binary[x]){
     //First, NOT all bits
     for(int i = 0; i < x; i++) { binary[i] = !binary[i]; }
     int j = x - 1;
     //Then add one
     while(binary[j]) { binary[j] = 0; j--; }
     binary[j] = 1;
}

int getRounding(char string[]){
     int x = 0;
     bool isAfterPoint = false;
     int len = strlen(string);
     for(int i = 0; i < len; i++){
          if(string[i] != '.' && isAfterPoint) { x++; }
          else if (string[i] == '.'){ isAfterPoint = true; }
     }
     return x;
}

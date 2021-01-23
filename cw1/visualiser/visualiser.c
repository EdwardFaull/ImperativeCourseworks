#include "visualiser.h"

enum datatype {CHAR, SHRT, INT, LONG, DOUBLE, FLOAT, INVALID};

const char separator[] = "\n-------------------------------------------------------------------------------------\n";

int readType(const char type[])
{
     char copy[15];
     strcpy(copy, type);
     if(copy[0] == 'u') trimFront(copy, 1);
     if(strcmp(copy, "char") == 0) return CHAR;
     if(strcmp(copy, "shrt") == 0) return SHRT;
     if(strcmp(copy, "short") == 0) return SHRT;
     if(strcmp(copy, "int") == 0) return INT;
     if(strcmp(copy, "long") == 0) return LONG;
     if(strcmp(copy, "double") == 0 && type[0] != 'u') { return DOUBLE; }
     if(strcmp(copy, "float") == 0 && type[0] != 'u') { return FLOAT; }

     return INVALID;
}

bool readSign(const char type[])
{
     if(type[0] == 'u') return false;
     return true;
}

//Reads in values for integer values
long readValue(char value[], int type, bool isSigned, bool *isValid)
{
     int len = strlen(value);
     int numberOfPoints = 0;
     for(int i = 0; i < len; i++){
          //Check if syntax of current digit is correct
          *isValid = checkSyntax(value[i], i, len, isSigned, &numberOfPoints);
          if(!*isValid) { return 0; }
     }
     if(numberOfPoints == 1) {
          //Valid for floating point types, return early to read using readFloat
          if(type == DOUBLE || type == FLOAT) { *isValid = true; return 0; }
          //Invalid for integer types
          else { *isValid = false; return 0; }
     }
     //Ulong types need to be read using readLong, return early without invalidating
     //Stay in the function long enough to validate value's syntax
     if(type == LONG && !isSigned) { *isValid = true; return 0; }
     long dec = atol(value);
     //If a 'ceiling' error occurs (if dec > LONG_MAX)
     if(!checkValue(dec, value)) { *isValid = false; return 0; }
     //If check fails, value is out of bounds of the chosen datatype
     if(dec > getLimits(type, isSigned, true))  { *isValid = false; return 0; }
     if(dec < getLimits(type, isSigned, false)) { *isValid = false; return 0; }
     return dec;
}

//If a ulong value > LONG_MAX or < LONG_MIN is passed to readValue,
//the closest of those two values is returned due to readValue's return type
//readLong is needed to properly overflow to give the correct binary sequence
unsigned long readLong(char value[], bool *isValid)
{
     unsigned long dec;
     sscanf(value, "%lu", &dec);

     if(checkValueULong(dec, value)) return dec;
     *isValid = false;
     return 0;
}

//Increases size of mantissa so that the least number of digits are discarded from the RHS
void resizeFloat(double *mantissa, long *exponent, int type)
{
     //Increase until mantissa is a whole number
     while(*mantissa != (long)*mantissa){
          *mantissa = *mantissa * 2;
          *exponent = *exponent - 1;
     }
     //If too large to be held by the mantissa (> 2^52 - 1 or < -(2^52)), decrease until within the correct range
     long upperLimit = getLimitsFloat(type, true, true);
     long lowerLimit = getLimitsFloat(type, false, true);
     if(*mantissa > upperLimit || *mantissa < lowerLimit){
          bool condition = true;
          while(condition){
               *mantissa = *mantissa / 2;
               *exponent = *exponent + 1;
               if(*mantissa > 0) condition = (long)(*mantissa) > upperLimit;
               else condition = (long)*mantissa < lowerLimit;
          }
     }
}

//Produces an un-normalised mantissa and exponent for a real decimal number
//ALTERNATIVE
//uint x = *(uint*)&floatingValue; to directly access binary data, then mask to get exp and mant
//REASON FOR NOT CHOOSING
//Not theoretically accurate, accessing the memory location has nothing to do with converting a float to binary
//Knowledge of using pointers already shown throughout code,
//my method shows that I know about binary representation better than the alternative method would have.
void readFloat(char string[], long floatingDec[2], int type, bool isSigned)
{
     double number = 0;

     long exponent = getTypeLengthFloat(type, true) - 1;
     sscanf(string, "%lf", &number);
     number = strtod(string, &string);

     resizeFloat(&number, &exponent, type);

     floatingDec[0] = (long)number; floatingDec[1] = exponent;
}

//Produces a binary sequence from the integer dec
void makeBinary(long dec, int length, bool binary[length])
{
     unsigned long absoluteDec = absVal(dec);
     for(int i = 0; i < length; i++){

          unsigned long bit = powerOf(length - i - 1, 2);
          if((long)(absoluteDec - bit) >= 0){
               binary[i] = true;
               absoluteDec -= bit;
          }
          else{
               binary[i] = false;
          }
     }
     if(dec < 0) { negate(length, binary); }
}

//Normalises the floating point data passed in
//Ensures that the two most significant bits of the mantissa are opposite
void normaliseFloat(long *mantissa, long *exponent, int type)
{
     //If zero, don't store anything in either the mantissa or the exponent
     if(*mantissa == 0) { *exponent = 0; return; }
     int typeLength = getTypeLengthFloat(type, true);
     bool mantissaBinary[typeLength];
     for(int i = 0; i < typeLength; i++) {mantissaBinary[i] = false;}

     //Create the binary sequence to check the two most significant bits.
     makeBinary(*mantissa, typeLength, mantissaBinary);

     //If normalised and positive:
     bool isNormalPositive = mantissaBinary[0] == 0 && mantissaBinary[1] == 1;
     //If normalised and negative:
     bool isNormalNegative = mantissaBinary[0] == 1 && mantissaBinary[1] == 0;

     if(!isNormalPositive && !isNormalNegative){
          //Shift mantissa left one bit
          *mantissa *= 2;
          *exponent -= 1;
          //Repeat until fully normalised
          normaliseFloat(mantissa, exponent, type);
     }
}

//Converts a binary sequence into a hexadecimal sequence
void BinaryToHex(int len, bool binary[len], char hex[len / 4]){
     int nibbleTotal = 0;
     int j = len / 4 - 1;
     for(int i = len - 1; i >= 0; i--){
          //Get value of current bit in the nibble being read
          int bit = powerOf(3 - i % 4, 2);
          if(binary[i]) { nibbleTotal += bit; }
          //If 4 bits have been added up, create the next hex character
          if(i % 4 == 0){
               if(nibbleTotal <= 9) { hex[j] = '0' + nibbleTotal; }
               else { hex[j] = 'A' + (nibbleTotal - 10); }
               //Reset current nibble value and shift string incrementer
               j--; nibbleTotal = 0;
          }
     }
}

//Converts a floating point sequence into a real decimal number
double BinaryToFloat(int mLength, bool mantissa[mLength], int eLength, bool exponent[eLength]){
     double value = 0; int exp = 0;

     for(int i = 0; i < mLength; i++){
          if(mantissa[i]){
               //MSB is -1
               if(i == 0){ value -= 1; }
               //Powers are < 0, so invert the power of i
               else { value += (double)((double)1 / (double)powerOf(i, 2)); }
          }
     }
     for(int i = 0; i < eLength; i++){
          if(exponent[i]){
               if(i == 0){ exp -= powerOf(eLength - 1 - i, 2); }
               else { exp += powerOf(eLength - 1 - i, 2); }
          }
     }
     if(exp >= 0) return value * powerOf(exp, 2);
     //If power is negative, invert the result of 2^(|exp|)
     long double multiplier = powerOf(-1 * exp, 2);
     //If multiplier is so small it can't be divided by, return 0.
     if(multiplier != 0) return value * (1 / multiplier);
     return 0;
}

//Displays information about the value entered if it is an integer type
void printInteger(char typeName[], bool isSigned, int typeLength, bool binary[typeLength])
{
     printf("\nType %s has %d bits.", typeName, typeLength);
     printf("%s", separator);

     if(!isSigned) { printf("0 sign bits.\n"); }
     else {
          printf("1 sign bit: %d", binary[0]);
          if(!binary[0]) printf(" (positive)\n");
          else printf(" (negative)\n");
     }

     printf("%d binary digits: ", typeLength - 1 * isSigned);
     for(int i = 1 * isSigned; i < typeLength; i++) { printf("%d ", binary[i]); }
     printf("%s", separator);

     char hexadecimalString[typeLength / 4 + 1];
     BinaryToHex(typeLength, binary, hexadecimalString);
     hexadecimalString[typeLength / 4] = 0;
     printf("This can be given as the hexadecimal (base 16) value: 0x%s\n\n", hexadecimalString);
}

//Displays information about the value entered if it is a floating point type
void printFloat(char typeName[], char originalFloat[], int type, int typeLength, bool mantissa[], bool exponent[])
{
     //Print mantissa info
     printf("\nType %s has %d bits.%s", typeName, typeLength, separator);
     printf("1 mantissa sign bit: %d", mantissa[0]);
     if(!mantissa[0]) printf(" (positive)\n");
     else printf(" (negative)\n");
     int mantissaLength = getTypeLengthFloat(type, true);
     printf("%d binary mantissa digits: ", mantissaLength - 1);
     for(int i = 1; i < mantissaLength; i++) { printf("%d ", mantissa[i]); }
     printf("%s", separator);

     //Print exponent info
     printf("1 exponent sign bit: %d", exponent[0]);
     if(!exponent[0]) printf(" (positive)\n");
     else printf(" (negative)\n");
     int exponentLength = getTypeLengthFloat(type, false);
     printf("%d binary exponent digits: ", exponentLength - 1);
     for(int i = 1; i < exponentLength; i++) { printf("%d ", exponent[i]); }
     printf("%s", separator);

     //Find rounding error, if there is any
     double stored = BinaryToFloat(mantissaLength, mantissa, exponentLength, exponent);
     int decimalPlaces = getRounding(originalFloat);

     printf("The value you entered was: %s\n", originalFloat);
     printf("The value stored, rounded to the same number of decimal places, was: %.*f\n", decimalPlaces, stored);

     //String needs to be at least as large as the original, add one more for the \0
     char storedString[strlen(originalFloat) + 1]; sprintf(storedString, "%0.*f", decimalPlaces, stored);

     if(strcmp(storedString, originalFloat) == 0) { printf("The two are identical - no rounding has occurred."); }
     else{ printf("Rounding has occured.");} printf("\n\n");
}

//Test that datatype is read correctly
void testType(){
     assert(readType("char") == CHAR);
     assert(readType("uchar") == CHAR);

     assert(readType("shrt") == SHRT);
     assert(readType("ushrt") == SHRT);

     assert(readType("int") == INT);
     assert(readType("uint") == INT);

     assert(readType("long") == LONG);
     assert(readType("ulong") == LONG);

     assert(readType("double") == DOUBLE);
     assert(readType("udouble") == INVALID);

     assert(readType("float") == FLOAT);
     assert(readType("ufloat") == INVALID);

     assert(readType("whatever") == INVALID);
}

//Test that the 'signedness' of the type is read correctly
void testSign(){
     assert(readSign("char") == true);
     assert(readSign("uchar") == false);
}

//Tests parsing of char and uchar types
void testChar(){
     bool isValid = true;
     //Signed over/underflow tests
     assert(readValue("127", CHAR, true, &isValid) == 127);
     assert(readValue("-128", CHAR, true, &isValid) == -128);
     assert(readValue("128", CHAR, true, &isValid) == 0 && isValid == false);
     assert(readValue("-129", CHAR, true, &isValid) == 0 && isValid == false);

     //Unsigned over/underflow tests
     assert(readValue("128", CHAR, false, &isValid) == 128);
     assert(readValue("255", CHAR, false, &isValid) == 255);
     assert(readValue("-1", CHAR, false, &isValid) == 0 && isValid == false);
     assert(readValue("256", CHAR, false, &isValid) == 0 && isValid == false);

     //Syntax tests: no floating points, no invalid numbers.
     assert(readValue("0.2", CHAR, false, &isValid) == 0 && isValid == false);
     assert(readValue(".2", CHAR, false, &isValid) == 0 && isValid == false);
     assert(readValue("2.", CHAR, false, &isValid) == 0 && isValid == false);
     assert(readValue("001", CHAR, false, &isValid) == 0 && isValid == false);
     assert(readValue("0y6", CHAR, false, &isValid) == 0 && isValid == false);
}

//Tests parsing of shrt and ushrt types
void testShrt(){
     bool isValid = true;
     //Signed over/underflow tests
     assert(readValue("32767", SHRT, true, &isValid) == 32767);
     assert(readValue("-32768", SHRT, true, &isValid) == -32768);
     assert(readValue("32768", SHRT, true, &isValid) == 0 && isValid == false);
     assert(readValue("-32769", SHRT, true, &isValid) == 0 && isValid == false);

     //Unsigned over/underflow tests
     assert(readValue("32768", SHRT, false, &isValid) == 32768);
     assert(readValue("65535", SHRT, false, &isValid) == 65535);
     assert(readValue("-1", SHRT, false, &isValid) == 0 && isValid == false);
     assert(readValue("65536", SHRT, false, &isValid) == 0 && isValid == false);

     //Syntax tests: no floating points, no invalid numbers.
     assert(readValue("0.2", SHRT, false, &isValid) == 0 && isValid == false);
     assert(readValue(".2", SHRT, false, &isValid) == 0 && isValid == false);
     assert(readValue("2.", SHRT, false, &isValid) == 0 && isValid == false);
     assert(readValue("001", SHRT, false, &isValid) == 0 && isValid == false);
     assert(readValue("0y6", SHRT, false, &isValid) == 0 && isValid == false);
}

//Tests parsing of int and uint types
void testInt(){
     bool isValid = true;
     //Signed over/underflow tests
     assert(readValue("2147483647", INT, true, &isValid) == 2147483647);
     assert(readValue("-2147483648", INT, true, &isValid) == -2147483648);
     assert(readValue("2147483648", INT, true, &isValid) == 0 && isValid == false);
     assert(readValue("-2147483649", INT, true, &isValid) == 0 && isValid == false);

     //Unsigned over/underflow tests
     assert(readValue("2147483648", INT, false, &isValid) == 2147483648);
     assert(readValue("4294967295", INT, false, &isValid) == 4294967295);
     assert(readValue("-1", INT, false, &isValid) == 0 && isValid == false);
     assert(readValue("4294967296", INT, false, &isValid) == 0 && isValid == false);

     //Syntax tests: no floating points, no invalid numbers.
     assert(readValue("0.2", INT, false, &isValid) == 0 && isValid == false);
     assert(readValue(".2", INT, false, &isValid) == 0 && isValid == false);
     assert(readValue("2.", INT, false, &isValid) == 0 && isValid == false);
     assert(readValue("001", INT, false, &isValid) == 0 && isValid == false);
     assert(readValue("0y6", INT, false, &isValid) == 0 && isValid == false);
}

//Tests parsing of long and ulong types
void testLong(){
     bool isValid = true;
     //Signed over/underflow tests
     assert(readValue("9223372036854775807", LONG, true, &isValid) == 9223372036854775807);
     assert(readValue("-9223372036854775808", LONG, true, &isValid) == LONG_MIN);
     assert(readValue("9223372036854775808", LONG, true, &isValid) == 0 && isValid == false);
     assert(readValue("-9223372036854775809", LONG, true, &isValid) == 0 && isValid == false);

     //Unsigned tests: looks to see if it skips over normal parsing correctly
     assert(readValue("9223372036854775808", LONG, false, &isValid) == 0 && isValid == true);
     assert(readValue("18446744073709551615", LONG, false, &isValid) == 0 && isValid == true);
     assert(readValue("-1", LONG, false, &isValid) == 0 && isValid == false);
     assert(readValue("18446744073709551616", LONG, false, &isValid) == 0 && isValid == true);

     //Unsigned parsing tests:
     assert(readLong("9223372036854775808", &isValid) == LONG_MIN);
     assert(readLong("18446744073709551615", &isValid) == ULONG_MAX);
     assert(readLong("-1", &isValid) == 0 && isValid == false);
     assert(readLong("18446744073709551616", &isValid) == 0 && isValid == false);

     //Syntax tests: no floating points, no invalid numbers.
     assert(readValue("0.2", LONG, false, &isValid) == 0 && isValid == false);
     assert(readValue("2.", LONG, false, &isValid) == 0 && isValid == false);
     assert(readValue(".2", LONG, false, &isValid) == 0 && isValid == false);
     assert(readValue("001", LONG, false, &isValid) == 0 && isValid == false);
     assert(readValue("0y6", LONG, false, &isValid) == 0 && isValid == false);
}

void test(){
     testType();
     testSign();
     testChar();
     testShrt();
     testInt();
     testLong();
     printf("All tests passed.\n");
}

int main(int n, char *args[n])
{
     setbuf(stdout, NULL);
     if(n == 1){
          test(); exit(1);
     }
     if(n == 3){
          int dataType = readType(args[1]); int typeLength = getTypeLength(dataType);

          if(dataType != INVALID){
               bool isSigned = readSign(args[1]); bool isValid = true;
               long valueDec = readValue(args[2], dataType, isSigned, &isValid);
               //If ulong, get a properly overflowed value for valueDec
               if(dataType == LONG && !isSigned) { valueDec = readLong(args[2], &isValid); }

               if(isValid){
                    if(dataType != DOUBLE && dataType != FLOAT){
                         bool binary[typeLength];
                         makeBinary(valueDec, typeLength, binary);
                         printInteger(args[1], isSigned, typeLength, binary);
                    }
                    else if(dataType != INVALID){
                         long floatingDec[2] = {0, 0};
                         readFloat(args[2], floatingDec, dataType, isSigned);
                         isValid = checkFloat(floatingDec[0], floatingDec[1], dataType);
                         if(isValid){
                              int mLength = getTypeLengthFloat(dataType, true);
                              int eLength = getTypeLengthFloat(dataType, false);
                              bool mantissa[mLength]; bool exponent[eLength];
                              normaliseFloat(&floatingDec[0], &floatingDec[1], dataType);

                              makeBinary(floatingDec[0], mLength, mantissa); makeBinary(floatingDec[1], eLength, exponent);
                              printFloat(args[1], args[2], dataType, typeLength, mantissa, exponent);
                         }
                         else { fprintf(stderr, "Value %s is not valid for type %s.\n", args[2], args[1]); exit(1); }
                    }
               }
               else { fprintf(stderr, "Value %s is not valid for type %s.\n", args[2], args[1]); exit(1); }
          }
          else { fprintf(stderr, "Type %s is not valid.\n", args[1]); exit(1); }
     }
     else { fprintf(stderr, "Use ./visualiser <Type> <value>.\n"); exit(1); }
     return 0;
}

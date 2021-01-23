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
     int valueType = 0;
     bool isValid = true;
     //Signed over/underflow tests
     assert(readValue("127", CHAR, true, &valueType, &isValid) == 127);
     assert(readValue("-128", CHAR, true, &valueType, &isValid) == -128);
     assert(readValue("128", CHAR, true, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("-129", CHAR, true, &valueType, &isValid) == 0 && isValid == false);

     //Unsigned over/underflow tests
     assert(readValue("128", CHAR, false, &valueType, &isValid) == 128);
     assert(readValue("255", CHAR, false, &valueType, &isValid) == 255);
     assert(readValue("-1", CHAR, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("256", CHAR, false, &valueType, &isValid) == 0 && isValid == false);

     //Syntax tests: no floating points, no invalid numbers.
     assert(readValue("0.2", CHAR, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue(".2", CHAR, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("001", CHAR, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("0y6", CHAR, false, &valueType, &isValid) == 0 && isValid == false);
}

//Tests parsing of shrt and ushrt types
void testShrt(){
     int valueType = 0;
     bool isValid = true;
     //Signed over/underflow tests
     assert(readValue("32767", SHRT, true, &valueType, &isValid) == 32767);
     assert(readValue("-32768", SHRT, true, &valueType, &isValid) == -32768);
     assert(readValue("32768", SHRT, true, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("-32769", SHRT, true, &valueType, &isValid) == 0 && isValid == false);

     //Unsigned over/underflow tests
     assert(readValue("32768", SHRT, false, &valueType, &isValid) == 32768);
     assert(readValue("65535", SHRT, false, &valueType, &isValid) == 65535);
     assert(readValue("-1", SHRT, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("65536", SHRT, false, &valueType, &isValid) == 0 && isValid == false);

     //Syntax tests: no floating points, no invalid numbers.
     assert(readValue("0.2", SHRT, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue(".2", SHRT, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("001", SHRT, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("0y6", SHRT, false, &valueType, &isValid) == 0 && isValid == false);
}

//Tests parsing of int and uint types
void testInt(){
     int valueType = 0;
     bool isValid = true;
     //Signed over/underflow tests
     assert(readValue("2147483647", INT, true, &valueType, &isValid) == 2147483647);
     assert(readValue("-2147483648", INT, true, &valueType, &isValid) == -2147483648);
     assert(readValue("2147483648", INT, true, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("-2147483649", INT, true, &valueType, &isValid) == 0 && isValid == false);

     //Unsigned over/underflow tests
     assert(readValue("2147483648", INT, false, &valueType, &isValid) == 2147483648);
     assert(readValue("4294967295", INT, false, &valueType, &isValid) == 4294967295);
     assert(readValue("-1", INT, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("4294967296", INT, false, &valueType, &isValid) == 0 && isValid == false);

     //Syntax tests: no floating points, no invalid numbers.
     assert(readValue("0.2", INT, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue(".2", INT, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("001", INT, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("0y6", INT, false, &valueType, &isValid) == 0 && isValid == false);
}

//Tests parsing of long and ulong types
void testLong(){
     int valueType = 0;
     bool isValid = true;
     //Signed over/underflow tests
     assert(readValue("9223372036854775807", LONG, true, &valueType, &isValid) == 9223372036854775807);
     assert(readValue("-9223372036854775808", LONG, true, &valueType, &isValid) == LONG_MIN);
     assert(readValue("9223372036854775808", LONG, true, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("-9223372036854775809", LONG, true, &valueType, &isValid) == 0 && isValid == false);

     //Unsigned tests: looks to see if it skips over normal parsing correctly
     assert(readValue("9223372036854775808", LONG, false, &valueType, &isValid) == 0 && isValid == true);
     assert(readValue("18446744073709551615", LONG, false, &valueType, &isValid) == 0 && isValid == true);
     assert(readValue("-1", LONG, false, &valueType, &isValid) == 0 && isValid == true);
     assert(readValue("18446744073709551616", LONG, false, &valueType, &isValid) == 0 && isValid == true);

     //Unsigned parsing tests:
     assert(readLong("9223372036854775808", &isValid) == LONG_MIN);
     assert(readLong("18446744073709551615", &isValid) == ULONG_MAX);
     assert(readLong("-1", &isValid) == 0 && isValid == false);
     assert(readLong("18446744073709551616", &isValid) == 0 && isValid == false);

     //Syntax tests: no floating points, no invalid numbers.
     assert(readValue("0.2", LONG, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("2.", LONG, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue(".2", LONG, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("001", LONG, false, &valueType, &isValid) == 0 && isValid == false);
     assert(readValue("0y6", LONG, false, &valueType, &isValid) == 0 && isValid == false);
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

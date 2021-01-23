Edward Faull (pb19639)'s Visualiser Notes:

Usage: ./visualiser <typename> <value>.
-Unsigned types can be used with 'u': uchar. Not for double or float.

Reasoning for methods used
-One alternative for floating point was to use pointers. Eg:

	uint x = *(uint*)&floatingValue;

 I used a different approach - constructing the mantissa from the value, incrementing the 
 exponent, then normalising the result. This shows the theory of floating point representation
 better than just accessing the binary data directly.

-ulong uses function readLong to stop value 'locking' at LONG_MAX/LONG_MIN. While interpreted as
 negative by the long it returns to, its binary data is still correct, and is represented correctly.
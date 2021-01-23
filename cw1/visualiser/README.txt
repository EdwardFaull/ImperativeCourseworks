Edward Faull (pb19639) Visualiser Notes:

-Usage: ./visualiser <type> <value>
-Takes char, shrt/short, int, long, float, double types - long double would be next addition
-All integer types can be signed or unsigned, prefix with 'u'.
-Takes base 10 input, hex, ASCII (for type char) and UNICODE as additions.
-Displays hex version of integer types for readability.
-Identifies if data has been lost from the original floating-point value entered by rounding.
-Does not use pointers to get floating-point value, constructs a mantissa and exponent to show knowledge
 of how the data is represented abstractly rather than physically in the computer. (Line93 for more)
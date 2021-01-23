Converter notes:
Currently:
-NOTE: converter.c flagged as security risk, so placed in .zip file.
-Usage: ./converter filein fileout
-Uses RLE to get maximum width and height of block
-Then finds longest width for which whole selection is a block
-Uses up to 39B/rectangle (max 6 loads for each of the 5 variables plus tool selection and drawing)
-Optimises loading of position variables (x,y,tx,ty), lowering average to ~23B/rectangle
-Giving a .pgm as filein makes a .sk file
-Giving a .sk as filein makes a .pgm file

Future:
-Investigate other ways to find rectangles in image to decrease fileout size more
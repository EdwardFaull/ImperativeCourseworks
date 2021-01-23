#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct point { int x, y; } point;
typedef struct object { unsigned char colour; point* p1; point* p2; } object;
typedef struct objects { int capacity; int k; object **o; } objects;
typedef struct commands { int capacity; int k; char *values; } commands;
typedef struct state { int x, y, tx, ty, w, h; unsigned char tool; unsigned int colour, data; } state;

const double ARRAY_GROW_RATE = 1.5;
const int INITIAL_OBJ_CAPACITY = 100;
const int INITIAL_COM_CAPACITY = 1000;
const int INITIAL_IMG_SIZE = 200;
const int MAX_LOAD = 6;

//Operations
enum { DX = 0, DY = 1, TOOL = 2,
       DATA = 3
     };

// Tool Types
enum { NONE = 0, BLOCK = 2, COLOUR = 3, TARGETX = 4, TARGETY = 5
     };

//Program Types
enum { PGM_SK = 1, SK_PGM = 2
     };

//Make an empty commands structure
commands *newCommands(){
     commands *c = malloc(sizeof(commands));
     c->k = 0;
     c->capacity = INITIAL_COM_CAPACITY;
     c->values = malloc(c->capacity * sizeof(char));

     return c;
}

//Make a new rectangle from (x1, y1) to (x2, y2) with colour c
object *newObject(const unsigned char c,
                  const int x1, const int y1, const int x2, const int y2){
     point *p1 = malloc(sizeof(point));
     *p1 = (point) {x1, y1};
     point *p2 = malloc(sizeof(point));
     *p2 = (point) {x1 + x2, y1 + y2};

     object *o = malloc(sizeof(object));
     *o = (object) {c, p1, p2};

     return o;
}

objects *newObjects(){
     objects *os = malloc(sizeof(objects));
     os->k = 0;
     os->capacity = INITIAL_OBJ_CAPACITY;
     os->o = malloc(os->capacity * sizeof(object*));
     return os;
}

//Frees the list of objects and commands
void freeStructs(objects *o, commands *c){
     for(int i = 0; i < o->k; i++){
          object *current = o->o[i];
          free(current->p1);
          free(current->p2);
          free(current);
     }
     free(o->o);
     free(o);
     free(c->values);
     free(c);
}

//Frees memory allocated to the image
void freeImage(int h, unsigned char **image){
     for(int y = 0; y < h; y++){
          free(image[y]);
     }
     free(image);
}

//Reallocate memory to the structure
void resizeObjects(objects *objs){
     if(objs->k == objs->capacity){
          objs->capacity *= ARRAY_GROW_RATE;
          objs->o = realloc(objs->o, objs->capacity * sizeof(object*));
     }
}

void resizeCommands(commands *c){
     if(c->k == c->capacity){
          c->capacity *= ARRAY_GROW_RATE;
          c->values = realloc(c->values, c->capacity * sizeof(char));
     }
}

//Increase the size of img to match the new dimensions w and h
void resizeImage(unsigned char **img, const int w, const int h){
     for(int i = 0; i < h; i++){
          img[i] = realloc(img[i], w * sizeof(char));
     }
     img = realloc(img, h * sizeof(char*));
}

//Open safely - if name not valid, end the program.
FILE *safeOpen(const char *filename, const char* mode){
     FILE *f = fopen(filename, mode);
     if(f != NULL) return f;
     else{
          fprintf(stderr, "Cannot open %s\n", filename);
          exit(1);
     }
}

//Set all pixels spanned by the object to true in the done array.
void fill(const object *o, const int w, const int h, bool done[h][w]){
     for(int x = o->p1->x; x < o->p2->x; x++){
          for(int y = o->p1->y; y < o->p2->y; y++) {
               done[y][x] = true;
          }
     }
}

//Find the width for which a rectangle of height endY is a solid block
int checkBlock(const int startX, const int startY, const int endX, const int endY,
               const int w, const int h, const unsigned char image[h][w], const bool done[h][w]){
     unsigned char colour = image[startY][startX];
     for(int x = startX; x < startX + endX; x++){
          for(int y = startY; y < startY + endY; y++){
               if(image[y][x] != colour){
                    return x - startX;
               }
          }
     }
     //If a solid block for all, return y
     return endX;
}

//Get a list of all rectangles in the image
objects *getBlocks(const int w, const int h, const unsigned char image[h][w], bool done[h][w]){
     objects *blocks = newObjects();
     for(int y = 0; y < h; y++){
          for(int x = 0; x < w; x++){
               if(!done[y][x]){
                    unsigned char current = image[y][x];
                    //Get longest uninterruped distance in x
                    int endX = 0;
                    while(x + endX < w && current == image[y][x + endX]) { endX++; }

                    //Get longest uninterruped distance in x
                    int endY = 0;
                    while(y + endY < h && current == image[y + endY][x]) { endY++; }

                    int newX = checkBlock(x, y, endX, endY, w, h, image, done);
                    object *obj = newObject(current, x, y, newX, endY);

                    fill(obj, w, h, done);
                    blocks->o[blocks->k] = obj;
                    blocks->k += 1;
                    resizeObjects(blocks);
               }
          }
     }
     return blocks;
}

//Get the colour to be loaded by the .sk file
unsigned int getRGBA(const object *o){
     unsigned int c = 0xFF;
     c += (unsigned int)o->colour << 8;
     c += (unsigned int)o->colour << 16;
     c += (unsigned int)o->colour << 24;
     return c;
}

//Get the 6 values to be pushed into the data value in the .sk file for a variable
void getCoordinate(int c, char data[6]){
     for(int i = 5; i >= 0; i--){
          data[i] = c & 63;
          c = c >> 6;
     }
}

//Adds a single command
void addCommand(commands *c, const char value, const char opcode){
     char op = opcode << 6;
     op = op | (value & 63);
     c->values[c->k] = op;
     c->k++;
     resizeCommands(c);
}

//Adds 'len' number of commands
void addCommandList(commands *c, const int len, const char *values, const char opcode){
     for(int i = 6-len; i < 6; i++){
          char op = opcode << 6;
          op += (values[i] & 63);
          c->values[c->k] = op;
          c->k++;
          resizeCommands(c);
     }
}

//Optimise number of loads needed for variables
int getLength(const char *values){
     int i = 0;
     while(i < MAX_LOAD && values[i] == 0) i++;
     return MAX_LOAD - i;
}

//Adds all of the instructions needed to represent a single rectangle in the image
void addInstruction(const object *o, commands *c){
     //Declare arrays. Always 6 loads of values needed
     //Ints have 32-bits, so 32/6 commands are needed to load in the data (5.33... so 6)
     char data[MAX_LOAD];
     int len = MAX_LOAD;
     unsigned int colour = getRGBA(o);

     addCommand(c, NONE, TOOL); //Set TOOL to NONE
     getCoordinate(colour, data); //Get colour operands
     addCommandList(c, len, data, DATA); //Add colours to data buffer with DATA
     addCommand(c, COLOUR, TOOL); //Set colour value with COLOUR

     getCoordinate(o->p1->x, data); //Get x values
     len = getLength(data);
     addCommandList(c, len, data, DATA); //Load x coordinate into data
     addCommand(c, TARGETX, TOOL); //Sets tx to data

     getCoordinate(o->p1->y, data); //Get y values
     len = getLength(data);
     addCommandList(c, len, data, DATA); //Load y coordinate into data
     addCommand(c, TARGETY, TOOL); //Sets ty to data
     addCommand(c, 0, DY); //Move by nothing to set x and y to tx and ty

     getCoordinate(o->p2->x, data); //Get tx values
     len = getLength(data);
     addCommandList(c, len, data, DATA); //Add targetx to data buffer with DATA
     addCommand(c, TARGETX, TOOL); //Set tx coordinate with TARGETX

     getCoordinate(o->p2->y, data); //Get ty values
     len = getLength(data);
     addCommandList(c, len, data, DATA); //Add targety to data buffer with DATA
     addCommand(c, TARGETY, TOOL); //Set ty coordinate with TARGETY

     addCommand(c, BLOCK, TOOL); //Set TOOL to BLOCK
     addCommand(c, 0, DY); //Draw block with DY
}

//Reads the pixels of a .pgm file and initialises the bool array to false
void initialiseArrays(FILE *f, const int h, const int w,
                      unsigned char image[h][w], bool done[h][w]){
     for(int i = 0; i < h; i++){
          for(int j = 0; j < w; j++){
               image[i][j] = fgetc(f);
               done[i][j] = false;
          }
     }
}

//Write the contents of 'commands' to an .sk file.
void writeSKFile(const char *filename, const commands *c){
     FILE *f = safeOpen(filename, "wb");
     for(int i = 0; i < c->k; i++){
          fputc(c->values[i], f);
     }
     fclose(f);
}

//Write the pixel info in 'image' to a .pgm file
void writePGMFile(const char *filename, const int w, const int h, unsigned char **image){
     FILE *f = safeOpen(filename, "wb");
     fprintf(f, "P5 %d %d %d\n", w, h, 255);
     for(int y = 0; y < h; y++){
          for(int x = 0; x < w; x++){
               fputc(image[y][x], f);
          }
     }
     fclose(f);
}

// Extract an opcode from a byte (two most significant bits).
int getOpcode(const char b) {
     int c = b >> 6;
     return c & 3;
}

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(const char b) {
     int d = b & 63;
     if(d > 31) d -= 64;
     return d;
}

//Fill the image with the set colour from (s->x, s->y) to (s->tx, s->ty)
void drawBlock(unsigned char **image, state *s){
     if(s->tx > s->w){
          resizeImage(image, s->tx, s->h);
          s->w = s->tx;
     }
     if(s->ty > s->h){
          resizeImage(image, s->w, s->ty);
          s->h = s->ty;
     }
     for(int x = s->x; x < s->tx; x++){
          for(int y = s->y; y < s->ty; y++){
               image[y][x] = s->colour;
          }
     }
}

//Execute the instructions when opcode = DY
void dy(unsigned char **image, state *s, int operand){
     s->ty += operand;
     if(s->tool == BLOCK){
          drawBlock(image, s);
     }
     s->x = s->tx;
     s->y = s->ty;
}

//Execute the instructions when opcode = TOOL
void tool(unsigned char **image, state *s, int operand){
     switch(operand){
          case COLOUR:
               s->colour = (s->data >> 24) & 255;
               break;
          case TARGETX:
               s->tx = s->data;
               break;
          case TARGETY:
               s->ty = s->data;
               break;
          default:
               s->tool = operand;
               break;
     }
     s->data = 0;
}

// Execute the next byte of the command sequence.
void obey(unsigned char **image, state *s, unsigned char op) {
     int opcode = getOpcode(op);
     int operand = getOperand(op);
     switch(opcode){
          case DX:
               s->tx += operand;
               break;
          case DY:
               dy(image, s, operand);
               break;
          case TOOL:
               tool(image, s, operand);
               break;
          case DATA:
               s->data = s->data << 6;
               s->data += (operand & 63);
               break;
     }
}

//Decide which file is being read in - .sk or .pgm
int operationMode(const char *filein){
     int len = strlen(filein);
     if(filein[len - 4] == '.' && filein[len - 3] == 'p' && filein[len - 2] == 'g' && filein[len - 1] == 'm'){
             return PGM_SK;
     }
     if(filein[len - 3] == '.' && filein[len - 2] == 's' && filein[len - 1] == 'k'){
             return SK_PGM;
     }
     return NONE;

}

//Convert a PGM file to an SK file
void PGMtoSK(const char *filein, const char *fileout){
     FILE *f = safeOpen(filein, "rb");
     int h, w;
     char s[20];
     fgets(s, 20, f);
     if(sscanf(s, "P5 %u %u 255 ", &w, &h) != 2){
          fprintf(stderr, "Error: PGM file not formatted correctly.\n");
          exit(1);
     }
     unsigned char image[h][w];
     bool done[h][w];
     initialiseArrays(f, h, w, image, done);
     fclose(f);

     objects *objects = getBlocks(w, h, image, done);

     commands *c = newCommands();
     for(int i = 0; i < objects->k; i++){
          addInstruction(objects->o[i], c);
     }

     writeSKFile(fileout, c);
     printf("Written a SK file to %s\n", fileout);
     freeStructs(objects, c);
}

//Convert an SK file to a PGM
void SKtoPGM(const char *filein, const char *fileout){
     FILE *f = safeOpen(filein, "rb");
     state *s = malloc(sizeof(state));
     *s = (state) { 0, 0, 0, 0, INITIAL_IMG_SIZE, INITIAL_IMG_SIZE, 0, 0, 0 };
     unsigned char **image = calloc(INITIAL_IMG_SIZE, sizeof(char*));
     for(int i = 0; i < INITIAL_IMG_SIZE; i++){
          image[i] = calloc(INITIAL_IMG_SIZE, sizeof(char));
     }

     char op = fgetc(f);
     while(!feof(f)){
          obey(image, s, op & 255);
          op = fgetc(f);
     }

     writePGMFile(fileout, s->w, s->h, image);
     freeImage(s->h, image);
     printf("Written a PGM file to %s\n", fileout);
     free(s);
     fclose(f);
}

int main(const int n, const char **args){
     if(n == 3){
          int operation = operationMode(args[1]);
          switch(operation){
               case PGM_SK:
                    PGMtoSK(args[1], args[2]);
                    break;
               case SK_PGM:
                    SKtoPGM(args[1], args[2]);
                    break;
          }
     }
     else{
          fprintf(stderr, "Usage: ./converter filein.pgm fileout.sk\n");
          exit(1);
     }
     return 0;
}

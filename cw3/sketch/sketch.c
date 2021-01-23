// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Allocate memory for a drawing state and initialise it
state *newState() {
  //TO DO
  state *s = malloc(sizeof(state));
  *s = (state) {0, 0, 0, 0, 0, LINE, 0, 0, false};
  return s;
  //return NULL; // this is a placeholder only
}

// Release all memory associated with the drawing state
void freeState(state *s) {
     free(s);
     //TO DO
}

// Extract an opcode from a byte (two most significant bits).
int getOpcode(byte b) {
     int c = b >> 6;
     return c;
}

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) {
     int d = b & 63;
     if(d > 31) d -= 64;
     return d;
}

void dy(display *d, state *s, int operand){
     s->ty += operand;
     if(s->tool == LINE) {
          line(d, s->x, s->y, s->tx, s->ty);
     }
     else if(s->tool == BLOCK){
          block(d, s->x, s->y, s->tx - s->x, s->ty - s->y);
     }
     s->x = s->tx;
     s->y = s->ty;
}

void tool(display *d, state *s, int operand){
     switch(operand){
          case COLOUR:
               colour(d, s->data);
               break;
          case TARGETX:
               s->tx = s->data;
               break;
          case TARGETY:
               s->ty = s->data;
               break;
          case SHOW:
               show(d);
               break;
          case PAUSE:
               pause(d, s->data);
               break;
          case NEXTFRAME:
               s->start = s->i;
               s->end = true;
          default:
               s->tool = operand;
               break;
     }
     s->data = 0;
}

// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {
     s->i++;
     int opcode = getOpcode(op);
     int operand = getOperand(op);
     switch(opcode){
          case DX:
               s->tx += operand;
               break;
          case DY:
               dy(d, s, operand);
               break;
          case TOOL:
               tool(d, s, operand);
               break;
          case DATA:
               s->data = s->data << 6;
               s->data += (operand & 63);
               break;
     }
}

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.
bool processSketch(display *d, void *data, const char pressedKey) {
     if(data == NULL) return (pressedKey == 27);
     state *s = (state*) data;
     char *filename = getName(d);
     FILE *f = fopen(filename, "rb");
     if(f == NULL){ return (pressedKey == 27); }
     byte b = fgetc(f);
     for(int i = 0; i < s->start; i++){
          b = fgetc(f);
          s->i++;
     }
     while(!feof(f) && !s->end){
          obey(d, s, b);
          b = fgetc(f);
          if(feof(f)) s->start = 0;
     }
     fclose(f);
     show(d);
     *s = (state) {0, 0, 0, 0, 0, LINE, s->start, 0, false};
     data = (void*) s;
     return (pressedKey == 27);
}

// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { // return usage hint if not exactly one argument
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); // otherwise view sketch file in argument
  return 0;
}
#endif

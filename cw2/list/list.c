// Implementation of list module.

// ---------- Add headers here ----------
#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// Each node in a doubly linked list is stored in this structure. The user of the
// module does not have any knowledge of nodes.
struct node { struct node *back; item x; struct node *next; };
typedef struct node node;

// A list is to be represented as a circular list. It has to contain a doubly linked list
// of item nodes and one special sentinel node that links to the two ends of the item list.
// The none field holds this sentinel node. It has a default item, must always be present for
// a list and always sits before the first item node and after the last item node. For a list
// that holds no item nodes the back and next fields of the sentinel node point to the sentinel
// node itself. The current node refers to the currently selected item node in the list. If
// the current node is the none node then no item is selected.
struct list { node *none, *current; };
typedef struct list list;

// ---------- Add functions here ----------

list *newList(item e){
     list *l = (list*) malloc(sizeof(list));
     node *n = (node*) malloc(sizeof(node));
     n->x = e;
     n->back = n; n->next = n;
     l->none = n;
     l->current = l->none;

     return l;
}

void last(list *xs){
     //xs->none->next = none, so if xs->current = xs->none the loop won't be used
     while(xs->current->next != xs->none){
          xs->current = xs->current->next;
     }
}

void first(list *xs){
     //xs->none->back = none, so if xs->current = xs->none the loop won't be used
     while(xs->current->back != xs->none){
          xs->current = xs->current->back;
     }
}

void freeList(list *xs){
     //Traverse to the end of the list
     last(xs);
     //Frees xs->none first
     //If list is empty, iteration will not happen as xs->none->back = xs->none
     while(xs->current->back != xs->none){
          free(xs->current->next);
     }
     //Final element is the first element, or xs->none if list is empty
     free(xs->current);
     //Frees list struct
     free(xs);
}

bool none(list *xs){
     if(xs->current == xs->none) return true;
     return false;
}

bool after(list *xs){
     if(none(xs)) return false;
     xs->current = xs->current->next; return true;
}

bool before(list *xs){
     if(none(xs)) return false;
     xs->current = xs->current->back; return true;
}

item get(list *xs){
     return xs->current->x;
}

bool set(list *xs, item x){
     if(none(xs)) return false;
     xs->current->x = x; return true;
}

void insertAfter(list *xs, item x){
     node *curr = xs->current;
     node *next = curr->next;
     node *n = (node*) malloc(sizeof(node));
     n->x = x;
     //back points to the (now) previous element
     n->back = curr;
     //next points to the next element of the previously current element
     n->next = next;

     //make the previously next element point backwards to n
     next->back = n;
     //make the previously current element point to n
     curr->next = n;

     //make list point to n
     xs->current = n;
}

void insertBefore(list *xs, item x){
     node *curr = xs->current;
     node *back = curr->back;
     node *n = (node*) malloc(sizeof(node));

     n->x = x;
     n->next = xs->current;
     n->back = xs->current->back;

     back->next = n;
     curr->back = n;

     xs->current = n;

}

bool deleteToAfter(list *xs){
     if(none(xs)) return false;

     node *newBack = xs->current->back;
     after(xs);
     free(xs->current->back);

     xs->current->back = newBack;
     xs->current->back->next = xs->current;

     return true;
}

bool deleteToBefore(list *xs){
     if(none(xs)) return false;

     node *newNext = xs->current->next;
     before(xs);
     free(xs->current->next);

     xs->current->next = newNext;
     xs->current->next->back = xs->current;

     return true;
}

// Test the list module, using int as the item type. Strings are used as
// 'pictograms' to describe lists. Single digits represent items and the '|' symbol
// in front of a digit indicates that this is the current item. If the '|' symbol
// is at the end of the string then 'none' of the items is selected. The strings
// "|37", "3|7", "37|" represent a list of two items, with the current position
// at the first item, the last item, and a situation where 'none' of the items
// is selected.
#ifdef test_list

// Build a list from a pictogram, with -1 as the default item.
list *build(char *s) {
  list *xs = malloc(sizeof(list));
    int n = strlen(s);
    node *nodes[n];
    for (int i = 0; i < n; i++) nodes[i] = malloc(sizeof(node));
    for (int i = 0; i < n; i++) nodes[i]->next = nodes[(i + 1) % n];
    for (int i = 1; i < n + 1; i++) nodes[i % n]->back = nodes[i - 1];
    xs->none = nodes[0];
    xs->none->x = -1;
    node *p = xs->none->next;
    for (int i = 0; i < strlen(s); i++) {
      if (s[i] == '|') xs->current = p;
      else {
        p->x = s[i] - '0';
        p = p->next;
      }
    }
  return xs;
}

// Destroy a list which was created with the build function and which matches a
// pictogram.
void destroy(list *xs, char *s) {
  int n = strlen(s);
  node *nodes[n];
  nodes[0] = xs->none;
  for (int i = 1; i < n; i++) nodes[i] = nodes[i-1]->next;
  for (int i = 0; i < n; i++) free(nodes[i]);
  free(xs);
}

// Check that a list matches a pictogram.
bool match(list *xs, char *s) {
  int n = strlen(s);
  node *nodes[n];
  nodes[0] = xs->none;
  for (int i = 1; i < n; i++) nodes[i] = nodes[i - 1]->next;
  //for(int i = 1; i < n; i++){
//       printf("%d, ", nodes[i]->x);
// }printf("\n");
  if (nodes[n - 1]->next != xs->none){ //printf("1\n");
                                             return false;}
    for (int i = 1; i < n; i++) {
         //printf("node[%d]->back = %p, node[%d] = %p\n", i, nodes[i]->back, i-1, nodes[i-1]);
      if (nodes[i]->back != nodes[i - 1]) { //printf("2\n");
                                             return false;}
    }
  node *p = xs->none->next;
  for (int i = 0; i < strlen(s); i++) {
    if (s[i] == '|') {
      if (p != xs->current) { //printf("3\n");
                                             return false;}
    }
    else {
      if (p->x != s[i] - '0') { //printf("4\n");
                                             return false;}
      p = p->next;
    }
  }
  return true;
}

// Use constants to say which function to call.
enum { First, Last, None, After, Before, Get, Set, InsertAfter, InsertBefore, DeleteToAfter, DeleteToBefore};
typedef int function;

// A replacement for the library assert function.
void assert(int line, bool b) {
  if (b) return;
  printf("The test on line %d fails.\n", line);
  exit(1);
}

// Call a given function with a possible integer argument, returning a possible
// integer or boolean result (or -1).
int call(function f, list *xs, int arg) {
  int result = -1;
  switch (f) {
    case None: result = none(xs); break;
    case First: first(xs); break;
    case Last: last(xs); break;
    case After: result = after(xs); break;
    case Before: result = before(xs); break;
    case Get: result = get(xs); break;
    case Set: result = set(xs, arg); break;
    case InsertAfter: insertAfter(xs, arg); break;
    case InsertBefore: insertBefore(xs, arg); break;
    case DeleteToBefore: result = deleteToBefore(xs); break;
    case DeleteToAfter: result = deleteToAfter(xs); break;
    default: assert(__LINE__, false);
  }
  return result;
}

// Check that a given function does the right thing. The 'in' value is passed to
// the function or is -1. The 'out' value is the expected result, or -1.
bool check(function f, int in, char *before, char *after, int out) {
  list *xs = build(before);
  int result = call(f, xs, in);
  //check circular list structure
  assert(__LINE__, (xs->none == xs->none->next->back));
  assert(__LINE__, (xs->none == xs->none->back->next));
  //check correct workings of the function
  bool ok = (match(xs, after) && (result == out));
  destroy(xs, after);
  return ok;
}

// Test newList, and call freeList. The test for freeList is that the memory
// leak detector in the -fsanitize=address or -fsanitize=leak compiler option
// reports no problems.
void testNewList() {
    list *xs = newList(-1);
    //check circular list structure
    assert(__LINE__, (xs->none == xs->none->next));
    assert(__LINE__, (xs->none == xs->none->back));
    //check that an empty list is produced with a sentinel correctly
    assert(__LINE__, match(xs, "|"));
    freeList(xs);
}

// Test the various functions.
void testFirst() {
    assert(__LINE__, check(First, -1, "|", "|", -1));
    assert(__LINE__, check(First, -1, "|37", "|37", -1));
    assert(__LINE__, check(First, -1, "3|7", "|37", -1));
    assert(__LINE__, check(First, -1, "37|", "|37", -1));
}

void testLast() {
    assert(__LINE__, check(Last, -1, "|", "|", -1));
    assert(__LINE__, check(Last, -1, "|37", "3|7", -1));
    assert(__LINE__, check(Last, -1, "3|7", "3|7", -1));
    assert(__LINE__, check(Last, -1, "37|", "3|7", -1));
}

void testNone() {
    assert(__LINE__, check(None, -1, "|", "|", true));
    assert(__LINE__, check(None, -1, "|37", "|37", false));
    assert(__LINE__, check(None, -1, "3|7", "3|7", false));
    assert(__LINE__, check(None, -1, "37|", "37|", true));
}

void testAfter() {
    assert(__LINE__, check(After, -1, "|", "|", false));
    assert(__LINE__, check(After, -1, "|37", "3|7", true));
    assert(__LINE__, check(After, -1, "3|7", "37|", true));
    assert(__LINE__, check(After, -1, "37|", "37|", false));
}

void testBefore() {
    assert(__LINE__, check(Before, -1, "|", "|", false));
    assert(__LINE__, check(Before, -1, "|37", "37|", true));
    assert(__LINE__, check(Before, -1, "3|7", "|37", true));
    assert(__LINE__, check(Before, -1, "37|", "37|", false));
}

void testGet() {
    assert(__LINE__, check(Get, -1, "|", "|", -1));
    assert(__LINE__, check(Get, -1, "|37", "|37", 3));
    assert(__LINE__, check(Get, -1, "3|7", "3|7", 7));
    assert(__LINE__, check(Get, -1, "37|", "37|", -1));
}

void testSet() {
    assert(__LINE__, check(Set, 5, "|", "|", false));
    assert(__LINE__, check(Set, 5, "|37", "|57", true));
    assert(__LINE__, check(Set, 5, "3|7", "3|5", true));
    assert(__LINE__, check(Set, 5, "37|", "37|", false));
}

void testInsertAfter() {
    assert(__LINE__, check(InsertAfter, 5, "|", "|5", -1));
    assert(__LINE__, check(InsertAfter, 5, "|37", "3|57", -1));
    assert(__LINE__, check(InsertAfter, 5, "3|7", "37|5", -1));
    assert(__LINE__, check(InsertAfter, 5, "37|", "|537", -1));
}

void testInsertBefore() {
    assert(__LINE__, check(InsertBefore, 5, "|", "|5", -1));
    assert(__LINE__, check(InsertBefore, 5, "|37", "|537", -1));
    assert(__LINE__, check(InsertBefore, 5, "3|7", "3|57", -1));
    assert(__LINE__, check(InsertBefore, 5, "37|", "37|5", -1));
}

void testDeleteToAfter() {
    assert(__LINE__, check(DeleteToAfter, -1, "|", "|", false));
    assert(__LINE__, check(DeleteToAfter, -1, "|37", "|7", true));
    assert(__LINE__, check(DeleteToAfter, -1, "3|7", "3|", true));
    assert(__LINE__, check(DeleteToAfter, -1, "37|", "37|", false));
    assert(__LINE__, check(DeleteToAfter, -1, "|5", "|", true));
}

void testDeleteToBefore() {
    assert(__LINE__, check(DeleteToBefore, -1, "|", "|", false));
    assert(__LINE__, check(DeleteToBefore, -1, "|37", "7|", true));
    assert(__LINE__, check(DeleteToBefore, -1, "3|7", "|3", true));
    assert(__LINE__, check(DeleteToBefore, -1, "37|", "37|", false));
    assert(__LINE__, check(DeleteToBefore, -1, "|5", "|", true));
}

int main() {
    testNewList();
    testFirst();
    testLast();
    testNone();
    testAfter();
    testBefore();
    testGet();
    testSet();
    testInsertAfter();
    testInsertBefore();
    testDeleteToAfter();
    testDeleteToBefore();
    printf("List module tests run OK.\n");
    return 0;
}
#endif
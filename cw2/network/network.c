#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

     //Constants
const int INITIAL_EDGES = 5;
const int INITIAL_NODES = 10;
const double GROWTH_RATE = 1.5;

     //Struct definitions
//node declared here, user has no knowledge of it
typedef struct node{
     item x;
     int capacity;
     int links;
     //Stores all of the edge that begin at that node
     struct node **edge;
     //Stores the 'cost' of traversing a particular edge
     //Corresponds to the edge array
          //ie: the weight of edge[0] is stored in weight[0].
     double *weight;
} node;

//network, while not defined here, is opaque to the user - its attributes are hidden
typedef struct network{
     int size;
     int capacity;
     //The value to return when the list is empty
     item null;
     //Points to the currently selected node in the network
     node *current;
     //Points to the node identified as the root
     //Can be quickly returned to with reset, to help navigating directed edges
     //Used in the tree, dijkstra, DFS functions as starting points for the algorithm
     node *root;
     //Stores a pointer to every node in the network for ease of searching
     node **inventory;
} network;

//Private function declarations

//   MEMORY HANDLING

//Frees memory allocated to a node
void freeNode(node *n);
//Frees memory allocated to a network and all of its nodes
void freeNetwork(network *n);

//   STRING HANDLING

//Returns all of a string starting from i before the separator character appears
//i passed as a pointer, as it then will become the index of the char after the separator
char *sliceTerm(char *s, int *i, char separator);

//Checks if a value is a valid integer or floating point number (floating = false for integers)
//Checks from i until the terminator character reached or end of string
//Only non-numerical char (0-9) allowed is '.', only if value is floating point
//Not allowed to end on the terminator value - 'x-' and 'x-y/' are invalid, for example
bool checkValue(char *s, int *i, char terminator, bool floating);

//Makes sure the term entered is valid. Acceptable syntaxes are:
     //x-y/w   - where x has an edge to y with weight w.
     //x-y     - where x has an edge to y with default weight 1
     //x       - where x is a node with no edges
bool checkTerm(char *s);

//Used to reduce repetition in getValues
//Gets a substring of s from i to the limit (or end of string) character
//Converts value found using passed specifier, stores in x
void readValue(char *s, void *x, int *i, char limit, char *specifier);

//Parses the values of x, y, and w from string s
bool getValues(char *s, item *x, item *y, double *w, item n);

//Adds the nodes and edges in the term to the network.
//If some of the nodes and edges already exist, the addition of them is ignored
     //This uses the existing checking of the header-defined functions.
void addTerm(network *n, char *s);

//   SUPPORTING FUNCTIONS

//Gets the address of the node containing the item x
//If x is not in the network, NULL is returned
node *find(network *n, item x);

//Swaps the values of elements arr[i] and arr[j]
void swap(int i, int j, node **arr);

//Selection sort algorithm - size not expected to be huge so O(n^2) acceptable for gained simplicity
//Used for 'neatness' when printing networks
void sort(int size, node **arr);

//Removes the specified item from the array,
//then shifts all items above it down to fill the gap
bool deleteFromArr(int len, node **arr, item x);
//Removes the weight in the specified index from the array,
//then shifts all items above it down to fill the gap
bool deleteWeight(int len, double *arr, int index);

//Returns true if the passed list of node pointers points to a node with the value x.
bool contains(int len, node **arr, item x);

//Returns the index of the node with the value x in the array of nodes
//If no node contains x, -1 is returned
int getIndex(int len, node **arr, item x);

//   RECURSION FUNCTIONS

//Checks if a node has already been visited, then recurses into its child nodes
//Acts in a depth-first style
bool isNodeCyclic(node *n, int k, item visited[]);

//Checks the current node, then recurses into each child node
//The whole subtree of each child node is searched before the next one is
node *depthSearchNode(node *n, item x, int k, item visited[]);

//In a depth-first style, recurses into each of its child nodes
//The largest result of the recursion is returned, + 1
int depthNode(node *current);

//Uses a depth first approach - calculates the depth to the child, then recurses into it
//If a depth found is shorter than the currently stored one, it is replaced by the new one
void dijkstraNode(network *n, node *current, double *d, item *p, int k, item visited[]);

//   PRINTING FUNCITONS

//Prints the list for the passed node in an adjacency list
void printNodeList(node *n);
//Prints a row of an adjacency matrix - all of the nodes that current has edges to
void printNodeMatrix(network *n, node *current);

//Function definitions

char *sliceTerm(char *s, int *i, char separator){
     int len = 0;
     for(int j = *i; s[j] != separator && s[j] != 0; j++){
          len++;
     }
     int j = *i;
     *i += len + 1;
     char *substring = malloc((len + 1) * sizeof(char));

     for(int k = 0; k < len; k++){
          substring[k] = s[k + j];
     }
     substring[len] = 0;

     return substring;
}

bool checkValue(char *s, int *i, char terminator, bool floating){
     int len = strlen(s);
     while(s[*i] != terminator && s[*i] != 0){
          //If it's a number
          if(s[*i] > '9' || s[*i] < '0'){
               //If not a floating point number, will always return false
               //If a floating point number not equal to '.', return false
               if((floating && s[*i] == '.') == false) return false;
          }
          *i += 1;
     }
     if(s[*i] == terminator && *i == len - 1) return false;
     return true;
}

bool checkTerm(char *s){
     int i = 0;
     //Checks the first value
     if(!checkValue(s, &i, '-', false)) return false;
     if(s[i] == 0) return true;
     i++;

     //Check second value
     if(!checkValue(s, &i, '/', false)) return false;
     if(s[i] == 0) return true;
     i++;

     //Check weight
     if(!checkValue(s, &i, '\0', true)) return false;
     return true;
}

void readValue(char *s, void *x, int *i, char limit, char *specifier){
     //Get substring
     char *X = sliceTerm(s, i, limit);
     //Convert to numerical type
     sscanf(X, specifier, x);
     free(X);
}

bool getValues(char *s, item *x, item *y, double *w, item n){
     int i = 0;
     int len = strlen(s);

     //Read in 'x' node, need an integer
     readValue(s, (void*)x, &i, '-', "%d");
     if(*x == n) return false;
     if(i >= len) return true;

     //Read in 'y' node
     readValue(s, (void*)y, &i, '/', "%d");
     if(*y == n) return false;
     if(i >= len) return true;

     //Weight is floating, so %lf used
     readValue(s, (void*)w, &i, '\0', "%lf");
     return true;
}

void addTerm(network *n, char *s){
     item x = n->null, y = n->null; double w = 1;

     bool isValid = getValues(s, &x, &y, &w, n->null);
     if(isValid == false) return;

     addNode(n, x);
     addNode(n, y);
     n->current = find(n, x);
     link(n, y, w);
}

network *newNetwork(item d){
     network *n = malloc(sizeof(network));
     n->null = d;
     n->current = NULL;
     n->root = NULL;
     n->size = 0;
     n->capacity = INITIAL_NODES;
     n->inventory = malloc(INITIAL_NODES * sizeof(node*));
     return n;
}

network *newNetworkFromString(char *s, item d){
     network *n = newNetwork(d);
     int i = 0, len = strlen(s);
     item r = d;
     while(i < len){
          char *term = sliceTerm(s, &i, ',');
          if(checkTerm(term) == false){
               fprintf(stderr, "%s is not a valid term.\nUse:\n   %s   %s   %s", term,
                         "x-y/w where x has an edge to y with weight w.\n",
                         "x-y where x has an edge to y with weight 1.\n",
                         "x where x has no edges.\n");
               free(term);
               exit(1);
          }
          addTerm(n, term);
          if(getRoot(n) == n->null) { r = get(n); setRoot(n, r); }
          free(term);
     }
     if(n->root != NULL) { reset(n); }
     else { freeNetwork(n); return NULL; }
     sort(n->size, n->inventory);
     return n;
}

bool empty(network *n){
     if(n->size == 0) return true;
     return false;
}

bool addNode(network *n, item x){
     //Prevents two nodes of the same value
     if(find(n, x) != NULL) return false;
     if(x == n->null) return false;

     //Define new node
     node *v = malloc(sizeof(node));
     v->x = x;
     v->capacity = INITIAL_EDGES;
     v->edge = malloc(INITIAL_EDGES * sizeof(node*));
     v->weight = malloc(INITIAL_EDGES * sizeof(double));
     v->links = 0;

     //Update network to point to new node
     n->current = v;
     if(n->size == 0) n->root = v;
     if(n->size == n->capacity){
          n->capacity *= GROWTH_RATE;
          n->inventory = realloc(n->inventory, n->capacity * sizeof(node*));
     }
     n->inventory[n->size] = v;
     n->size = n->size + 1;

     return true;
}

bool deleteNode(network *n, item x){
     if(empty(n)) return false;
     int index = getIndex(n->size, n->inventory, x);
     if(index == -1) return false;
     node *itemToRemove = n->inventory[index];
     //Remove the node from the network
     deleteFromArr(n->size, n->inventory, x);
     n->size = n->size - 1;
     //Remove all edges leading to the node
     for(int i = 0; i < n->size; i++){
          node *m = n->inventory[i];
          if(deleteFromArr(m->links, m->edge, x)) m->links = m->links - 1;
     }
     //If the node to be deleted is the current node
     if(itemToRemove == n->current){
          //If there exists a root, make that the current node
          if(getRoot(n) != n->null && n->root != itemToRemove) n->current = n->root;
          //If the network became empty after removing the item, make current NULL
          else if(empty(n)) n->current = NULL;
          //If not, select the first item in the network
          else n->current = n->inventory[0];
          //If the root was the removed item, make the root NULL - manual setting required afterwards
          if(n->root == itemToRemove) n->root = NULL;
     }
     freeNode(itemToRemove);
     return true;
}

void freeNode(node *n){
     free(n->edge);
     free(n->weight);
     free(n);
}

void freeNetwork(network *n){
     for(int i = 0; i < n->size; i++){
          freeNode(n->inventory[i]);
     }
     free(n->inventory);
     free(n);
}

item get(network *n){
     if(empty(n)) return n->null;
     return n->current->x;
}

item getRoot(network *n){
     if(empty(n)) return n->null;
     if(n->root == NULL) return n->null;
     return n->root->x;
}

double getWeight(network *n, item y){
     if(empty(n)) return -1;
     node *x = n->current;
     for(int i = 0; i < x->links; i++){
          if(x->edge[i]->x == y){
               return x->weight[i];
          }
     }
     return -1;
}

bool set(network *n, item x){
     //If network is empty, return false
     if(empty(n)) return false;
     //If attempting to set the node to the null value
     if(x == n->null) return false;
     //If attempting to set the node to an already existing value
     if(find(n, x) != NULL) return false;
     n->current->x = x;
     return true;
}

bool setRoot(network *n, item r){
     if(empty(n)) return false;
     node *newRoot = find(n, r);
     if(newRoot == NULL) return false;

     n->root = newRoot;
     return true;
}

bool setWeight(network *n, item y, double w){
     if(empty(n)) return false;
     if(w < 0) return false;
     node *x = n->current;
     for(int i = 0; i < x->links; i++){
          if(x->edge[i]->x == y){
               x->weight[i] = w; return true;
          }
     }
     return false;
}

int nodes(network *n){
     return n->size;
}

int edges(network *n){
     return n->current->links;
}

bool traverse(network *n, item x){
     if(n->current == NULL) return false;
     node *v = n->current;
     int length = v->links;
     for(int i = 0; i < length; i++){
          if(v->edge[i]->x == x){
               n->current = v->edge[i];
               return true;
          }
     }
     return false;
}

bool reset(network *n){
     if(empty(n)) return false;
     n->current = n->root;
     return true;
}

node *find(network *n, item x){
     if(n->current == NULL) return NULL;

     int len = n->size;
     for(int i = 0; i < len; i++){
          node *v = n->inventory[i];
          if(v->x == x) return v;
     }
     return NULL;
}

bool contains(int len, node **arr, item x){
     for(int i = 0; i < len; i++){
          if(arr[i]->x == x){
               return true;
          }
     }
     return false;
}

bool link(network *n, item y, double w){
     if(empty(n)) return false;
     if(w < 0) return false;

     node *nodeX = n->current;
     if(contains(nodeX->links, nodeX->edge, y)) return false;
     node *nodeY = find(n, y);

     if(nodeY == NULL) return false;

     if(nodeX->links == nodeX->capacity){
          nodeX->capacity *= GROWTH_RATE;
          nodeX->edge = realloc(nodeX->edge, nodeX->capacity * sizeof(node));
          nodeX->weight = realloc(nodeX->edge, nodeX->capacity * sizeof(double));
     }

     nodeX->edge[nodeX->links] = nodeY;
     nodeX->weight[nodeX->links] = w;
     nodeX->links++;
     return true;
}

bool deleteFromArr(int len, node **arr, item x){
     bool isDone = false;
     for(int i = 0; i < len && !isDone; i++){
          if(arr[i]->x == x){
               isDone = true;
               for(int j = i; j < len - 1; j++){
                    arr[j] = arr[j + 1];
               }
               arr[len - 1] = NULL;
          }
     }
     return isDone;
}

bool deleteWeight(int len, double *arr, int index){
     bool isDone = false;
     for(int i = 0; i < len && !isDone; i++){
          if(i == index){
               isDone = true;
               for(int j = i; j < len - 1; j++){
                    arr[j] = arr[j + 1];
               }
               arr[len - 1] = -1;
          }
     }
     return isDone;
}

bool unlink(network *n, item y){
     //If network is empty, return false and do nothing.
     if(empty(n)) return false;

     node *nodeX = n->current;

     int index = getIndex(nodeX->links, nodeX->edge, y);
     if(index == -1) return false;

     deleteFromArr(nodeX->links, nodeX->edge, y);
     deleteWeight(nodeX->links, nodeX->weight, index);

     nodeX->links--;
     return true;
}

bool isNodeCyclic(node *n, int k, item visited[]){
     for(int i = 0; i < k; i++){
          if(visited[i] == n->x) return true;
     }
     visited[k] = n->x; k++;

     for(int i = 0; i < n->links; i++){
          if(isNodeCyclic(n->edge[i], k, visited)){
               return true;
          }
     }
     return false;
}

bool isCyclic(network *n){
     if(empty(n)) return false;

     item visited[n->size];
     return isNodeCyclic(n->current, 0, visited);
}

bool isTree(network *n){
     //If the network is cyclic, it's not a tree
     if(isCyclic(n)) return false;
     //Find how many parents each node has
     int parents[n->size]; for(int i = 0; i < n->size; i++) { parents[i] = 0; }
     for(int i = 0; i < n->size; i++){
          node *current = n->inventory[i];
          for(int j = 0; j < current->links; j++){
               int index = getIndex(n->size, n->inventory, current->edge[j]->x);
               if(parents[index] >= 1) return false;
               parents[index]++;
          }
     }
     //If each node doesn't have exactly one parent, it's not a tree
     for(int i = 0; i < n->size; i++){
          if(n->inventory[i] == n->root){
               if(parents[i] != 0) return false;
          }
          else{
               if(parents[i] != 1) return false;
          }
     }
     return true;
}

int getIndex(int len, node **arr, item x){
     for(int i = 0; i < len; i++){
          if(arr[i]->x == x) return i;
     }
     return -1;
}

int depthNode(node *current){
     int depth = 1;
     for(int i = 0; i < current->links; i++){
          node *neighbour = current->edge[i];
          int x = 1 + depthNode(neighbour);
          if(x > depth) depth = x;
     }
     return depth;
}

int depth(network *n){
     if(empty(n)) return 0;
     if(isTree(n) == false) return -1;

     return depthNode(n->current);
}

node *depthSearchNode(node *n, item x, int k, item visited[]){
     if(n->x == x) return n;
     for(int i = 0; i < k; i++){
          if(visited[i] == n->x) return NULL;
     }
     visited[k] = n->x; k++;
     for(int i = 0; i < n->links; i++){
          node *m = depthSearchNode(n->edge[i], x, k, visited);
          if(m != NULL) return m;
     }
     return NULL;
}

bool depthFirstSearch(network *n, item x, bool goTo){
     if(empty(n)) return false;
     item visited[n->size];
     node *m = depthSearchNode(n->root, x, 0, visited);
     if(m == NULL) return false;
     if(goTo) n->current = m;
     return true;
}

bool breadthFirstSearch(network *n, item x, bool goTo){
     //Initialise 'queue' array
     node *q[n->size]; int back = 0;
     item visited[n->size]; int k = 0;
     for(int i = 0; i < n->size; i++){ q[i] = NULL; }
     //Enqueue root
     q[0] = n->root; back++;
     while(q[0] != NULL){
          //Dequeue
          node *v = q[0];
          deleteFromArr(n->size, q, v->x);
          back--;
          //If v is the node to find
          if(v->x == x){
               if(goTo) n->current = v;
               return true;
          }
          for(int i = 0; i < v->links; i++){
               node *w = v->edge[i];
               bool isWDiscovered = false;
               //Check if w is already visited
               for(int j = 0; j < k && !isWDiscovered; j++){
                    if(visited[j] == w->x) isWDiscovered = true;
               }
               //If not, enqueue w and make visited
               if(!isWDiscovered) {
                    visited[k] = w->x; k++;
                    q[back] = w; back++;
               }
          }
     }
     return false;
}

bool isSubNet(network *n, network *m){
     if(empty(n) || empty(m)) return false;
     for(int i = 0; i < m->size; i++){
          node *mNode = m->inventory[i];
          item x = mNode->x;
          node *nNode = find(n, x);

          if(nNode == NULL) return false;
          for(int j = 0; j < mNode->links; j++){
               if(contains(nNode->links, nNode->edge, mNode->edge[j]->x) == false)
                    return false;
          }
     }
     return true;
}

void dijkstraNode(network *n, node *current, double *d, item *p, int k, item visited[]){
     for(int i = 0; i < k; i++){
          if(visited[i] == current->x) return;
     }
     visited[k] = current->x; k++;

     int currentIndex = getIndex(n->size, n->inventory, current->x);
     for(int i = 0; i < current->links; i++){
          node *neighbour = current->edge[i];
          int index = getIndex(n->size, n->inventory, neighbour->x);

          double w = current->weight[i];
          if(d[currentIndex] != -1) w += d[currentIndex];
          if(d[index] > w || d[index] == -1){
               d[index] = w;
               p[index] = current->x;
               dijkstraNode(n, neighbour, d, p, k, visited);
          }
     }
}

void dijkstra(network *n, double *d, item *p){
     //Set initial distance and previous value
     for(int i = 0; i < n->size; i++){
          if(n->inventory[i]->x == getRoot(n)) d[i] = 0;
          else d[i] = -1;
          p[i] = n->null;
     }
     item visited[n->size];
     dijkstraNode(n, n->root, d, p, 0, visited);
}

double getShortestDistance(network *n, item y, double *d){
     for(int i = 0; i < n->size; i++){
          if(n->inventory[i]->x == y){
               return d[i];
          }
     }
     return -1;
}

void getShortestPath(network *n, item y, item *p, item *path){
     for(int i = 0; i < n->size; i++) { path[i] = n->null; }
     int index = getIndex(n->size, n->inventory, y);
     if(index == -1) return;

     int i = 0;
     while(i < n->size){
          if(i != 0 && n->inventory[index]->x == path[i - 1]) return;
          path[i] = n->inventory[index]->x; i++;

          index = getIndex(n->size, n->inventory, p[index]);
          if(index == -1) return;
     }
}

void swap(int i, int j, node **arr){
     node *temp = arr[i];
     arr[i] = arr[j];
     arr[j] = temp;
}

void sort(int size, node **arr){
     if(size <= 1) return;
     for(int i = 0; i < size - 1; i++){
          if(arr[i]->x > arr[i + 1]->x) swap(i, i + 1, arr);
     }
     sort(size - 1, arr);
}

void printDijkstra(network *n, double *d, item *p){
     for(int i = 0; i < n->size; i++){
          node *m = n->inventory[i];
          printf("%d: ", m->x);
          printf("d = %.2f, ", d[i]);
          item path[n->size];
          getShortestPath(n, m->x, p, path);

          int j = 0;
          while(j < n->size){
               if(path[j] != n->null){
                    printf("%d", path[j]);
               }
               if(j == n->size - 1) { printf("\n"); j = n->size; }
               else if(path[j + 1] == n->null) { printf("\n"); j = n->size; }
               else printf(" <- ");
               j++;
          }
     }
}

void printNodeList(node *n){
     int len = n->links;
     //Name of current list
     printf("%d: {", n->x);
     if(len == 0) printf("}\n");

     for(int i = 0; i < len; i++){
          //Prints destination node followed by edge weight
          printf("%d : %.2f", (n->edge[i])->x, n->weight[i]);
          if(i == len - 1) printf("}\n");
          else printf(", ");
     }
}

void printList(network *n){
     if(empty(n)) return;
     sort(n->size, n->inventory);
     for(int i = 0; i < n->size; i++){
          printNodeList(n->inventory[i]);
     }
}

void printNodeMatrix(network *n, node *current){
     //Row name
     printf("%d |", current->x);
     for(int i = 0; i < n->size; i++){
          node *neighbour = n->inventory[i];
          //If current points to neighbour
          if(contains(current->links, current->edge, neighbour->x)){
               int weightIndex = getIndex(current->links, current->edge, neighbour->x);
               double w = current->weight[weightIndex];
               //Print weight of connection
               printf("%.2f|", w);
          }
          //If neighbour not linked to current
          else{
               printf(" -- |");
          }
     }
     printf("\n");
}

void printMatrix(network *n){
     if(empty(n)) return;
     sort(n->size, n->inventory);
     int len = n->size;
     //Print the line of column names
     printf("__|");
     for(int i = 0; i < len; i++){
          printf("  %d |", n->inventory[i]->x);
     }
     printf("\n");
     //Print each row
     for(int i = 0; i < len; i++){
          printNodeMatrix(n, n->inventory[i]);
     }
}

//Testing and main function
//Not read when using network as an API
#ifdef test_network

void printInformation(network *n){
     if(empty(n)) { printf("This network is empty.\n"); return; }
     printf("Number of nodes: %d\n", nodes(n));
     printf("Root node: %d\n", getRoot(n));
     printf("Network type: ");
     if(isTree(n)){ printf("tree\n"); printf("Tree depth: %d\n", depth(n)); }
     else if(isCyclic(n)) { printf("cyclic\n"); }
     else { printf("acyclic\n"); }

     //Sort before printing to make it easier to read - values in ascending order
     sort(n->size, n->inventory);

     printf("\nAs an adjacency list: \n");
     printList(n); printf("\n");

     printf("As an adjacency matrix: \n");
     printMatrix(n); printf("\n");

     printf("Shortest path from root:\n");
     double d[n->size];
     item p[n->size];
     dijkstra(n, d, p);
     printDijkstra(n, d, p);
}

void testNewNetwork(){
     network *n = newNetwork(-1);
     assert(n->current == NULL);
     assert(n->null == -1);
     freeNetwork(n);
}

void testAddNode(){
     network *n = newNetwork(-1);
     //Add to empty list
     assert(addNode(n, 5));
     assert(n->current->x == 5);
     //Add to non-empty list
     assert(addNode(n, 4));
     assert(n->current->x == 4);
     //Add invalid value
     assert(addNode(n, 4) == false);
     //Add null value
     assert(addNode(n, -1) == false);

     freeNetwork(n);
}

void testDeleteNode(){
     network *n = newNetworkFromString("1-2,2-3,3-1", -1);
     assert(get(n) == 1 && getRoot(n) == 1);
     //Doesn't delete nodes not in n
     assert(!deleteNode(n, 4));
     //Does delete nodes inside n
     assert(deleteNode(n, 1));
     //New node is the first one in inventory (n->inventory[0])
     //Root has been deleted, so getRoot returns null
     assert(get(n) == 2 && getRoot(n) == -1);
     //Make sure edges to 1 are deleted
     assert(find(n, 3)->links == 0);
     //Make sure size is updated
     assert(nodes(n) == 2);
     //Make sure network can be emptied
     assert(deleteNode(n, 2));
     assert(deleteNode(n, 3));
     assert(empty(n));

     freeNetwork(n);
}

void testEmpty(){
     network *n = newNetwork(-1);
     assert(empty(n));
     addNode(n, 5);
     assert(empty(n) == false);
     freeNetwork(n);
}

void testGet(){
     network *n = newNetwork(-1);
     assert(get(n) == -1);
     addNode(n, 5);
     assert(get(n) == 5);
     addNode(n, 3);
     assert(get(n) == 3);
     freeNetwork(n);
}

void testSet(){
     network *n = newNetwork(-1);
     assert(set(n, 0) == false);
     addNode(n, 5);
     assert(set(n, 3) == true && get(n) == 3);
     addNode(n, 5);
     assert(set(n, 3) == false && get(n) == 5);
     freeNetwork(n);
}

void testNodes(){
     network *n = newNetwork(-1);
     assert(nodes(n) == 0);
     addNode(n, 1);
     assert(nodes(n) == 1);
     addNode(n, 2);
     assert(nodes(n) == 2);

     freeNetwork(n);
}

void testTraverse(){
     network *n = newNetwork(-1);
     addNode(n, 3);
     addNode(n, 5);
     assert(get(n) == 5);
     assert(link(n, 3, 0));
     assert(link(n, 3, 0) == false);
     assert(traverse(n, 3));
     assert(get(n) == 3);
     freeNetwork(n);
}

void testFind(){
     network *n = newNetwork(-1);
     addNode(n, 3);
     node *nodeOne = n->current;
     addNode(n, 5);
     assert(find(n, 5) == n->current);
     assert(find(n, 3) == nodeOne);
     freeNetwork(n);
}

void testLink(){
     network *n = newNetwork(-1);
     addNode(n, 3);
     addNode(n, 5);
     assert(n->current->links == 0);
     assert(link(n, 3, 2.5));
     assert(n->current->links == 1);
     assert(n->current->weight[0] == 2.5);
     freeNetwork(n);
}

void testUnlink(){
     network *n = newNetwork(-1);
     addNode(n, 3);
     addNode(n, 5);
     assert(n->current->links == 0);
     link(n, 3, 0);
     assert(n->current->links == 1);
     assert(unlink(n, 3));
     assert(n->current->links == 0);
     assert(unlink(n, 3) == false);
     freeNetwork(n);
}

void testGetWeight(){
     network *n = newNetwork(-1);
     //Doesn't work on empty networks
     assert(getWeight(n, 1) == -1);
     addNode(n, 5);
     addNode(n, 3);
     //Weight set to 2
     link(n, 5, 2);
     assert(getWeight(n, 5) == 2);
     //Weight given is illegal
     assert(link(n, 3, -2) == false);
     //Make sure it is not assigned
     assert(getWeight(n, 3) == -1);
     link(n, 3, 3.3);
     assert(getWeight(n, 3) == 3.3);
     freeNetwork(n);
}

void testSetWeight(){
     network *n = newNetwork(-1);
     //Doesn't work on empty networks
     assert(setWeight(n, 1, 2.3) == false);
     addNode(n, 5);
     addNode(n, 3);
     //Weight set to 2
     link(n, 5, 2);
     assert(getWeight(n, 5) == 2);
     assert(setWeight(n, 5, 3));
     assert(getWeight(n, 5) == 3);
     //Weight given is illegal
     assert(setWeight(n, 5, -2) == false);
     //Make sure it is not assigned
     assert(getWeight(n, 5) == 3);

     freeNetwork(n);
}

void testCyclic(){
     network *n = newNetworkFromString("1-2,2-3,3-4,4-1", -1);
     assert(isCyclic(n));
     freeNetwork(n);

     n = newNetworkFromString("1-2,1-3,2-3,3-4", -1);
     assert(isCyclic(n) == false);
     freeNetwork(n);
}

void testTree(){
     network *n = newNetworkFromString("1-2,1-4,2-3,4-5,4-7,5-6", -1);
     assert(isTree(n));
     freeNetwork(n);

     n = newNetworkFromString("1-2,1-4,5-1", -1);
     assert(isTree(n) == false);
     freeNetwork(n);
}

void testDepth(){
     //Test on tree
     network *n = newNetworkFromString("1-2,1-4,2-3,4-5,4-7,5-6", -1);
     assert(depth(n) == 4);
     freeNetwork(n);

     //Test on acyclic non-tree
     n = newNetworkFromString("1-3,1-2,2-4,3-2", -1);
     assert(depth(n) == -1);
     freeNetwork(n);

     //Test on cyclic network
     n = newNetworkFromString("1-2,2-3,3-4,4-1", -1);
     assert(depth(n) == -1);
     freeNetwork(n);

     n = newNetwork(-1);
     assert(depth(n) == 0);
     freeNetwork(n);
}

void testDepthFirstSearch(){
     network *n = newNetwork(-1);

     //Can't find anything in an empty list
     assert(depthFirstSearch(n, -1, false) == false);
     addNode(n, 5);

     //Can find current node
     assert(depthFirstSearch(n, 5, false) == true);
     addNode(n, 3); link(n, 5, 0);

     //Can find an an adjacent node
     assert(depthFirstSearch(n, 5, false) == true);

     //Can find a non-adjacent node
     addNode(n, 4); link(n, 3, 0);
     assert(depthFirstSearch(n, 5, false) == true);

     //Can traverse to a non-adjacent node
     assert(depthFirstSearch(n, 5, true) == true);
     assert(n->current->x == 5);

     //Can't find a node the current node doesn't point to
     traverse(n, 3);
     assert(depthFirstSearch(n, 4, false) == false);

     //Can traverse to an adjacent node
     assert(depthFirstSearch(n, 5, true) == true);
     assert(n->current->x == 5);

     freeNetwork(n);
}

void testBreadthFirstSearch(){
     network *n = newNetworkFromString("1-2,1-3,2-3,3-4,5", -1);

     assert(breadthFirstSearch(n, 1, false));
     assert(breadthFirstSearch(n, 2, false));
     assert(breadthFirstSearch(n, 3, false));
     assert(breadthFirstSearch(n, 4, false));
     assert(!breadthFirstSearch(n, 5, false));
     assert(!breadthFirstSearch(n, 6, false));

     assert(breadthFirstSearch(n, 3, true));
     assert(get(n) == 3);

     freeNetwork(n);
}

void testIsSubNet(){
     network *n = newNetwork(-1); addNode(n, 3);
     network *m = newNetwork(-1);

     assert(isSubNet(n, m) == false);

     //n = 3, m = 3
     //m is a subnet of n here, as it contains all of the nodes of m (3)
     //and all of its connections (none).
     addNode(m, 3);
     assert(isSubNet(n, m));

     //n = 5-3, m = 3
     //m is a subnet of m here, as n contains all of the nodes (just 3)
     //and all of the edges (in this instance, none) in m.
     addNode(n, 5); link(n, 3, 1.5);
     assert(isSubNet(n, m));

     //n = 5-3, m = 3-3
     //m is not a subnet of n here, as 3 in n does not point to 3
     link(m, 3, 1);
     assert(isSubNet(n, m) == false);

     //n = 5-3,3-3, m = 3-3
     //m is a subnet of n here, as 3 does point to 3 in n
     traverse(n, 3);
     link(n, 3, 2);
     assert(isSubNet(n, m));

     freeNetwork(n);
     freeNetwork(m);
}

void testDijkstra(){
     network *n = newNetworkFromString("1-2,1-3/4,2-3/5,3-4/1.5,5", -1);
     double d[5];
     item p[5];
     dijkstra(n, d, p);

     assert(d[0] == 0 && d[1] == 1 && d[2] == 4 && d[3] == 5.5 && d[4] == -1);
     assert(p[0] == -1 && p[1] == 1 && p[2] == 1 && p[3] == 3 && p[4] == -1);

     freeNetwork(n);
}

void testGetDistance(){
     network *n = newNetworkFromString("1-2,1-3/4,2-3/5,3-4/1.5,5", -1);
     double d[5];
     item p[5];
     dijkstra(n, d, p);

     assert(getShortestDistance(n, 1, d) == 0);
     assert(getShortestDistance(n, 2, d) == 1);
     assert(getShortestDistance(n, 3, d) == 4);
     assert(getShortestDistance(n, 4, d) == 5.5);
     assert(getShortestDistance(n, 5, d) == -1);

     freeNetwork(n);
}

void testGetPath(){
     network *n = newNetworkFromString("1-2,1-3/4,2-3/5,3-4/1.5,5", -1);
     double d[5];
     item p[5];
     item path[5];
     dijkstra(n, d, p);

     getShortestPath(n, 1, p, path);
     assert(path[0] == 1 && path[1] == n->null);

     getShortestPath(n, 2, p, path);
     assert(path[0] == 2 && path[1] == 1 && path[2] == -1);

     getShortestPath(n, 3, p, path);
     assert(path[0] == 3 && path[1] == 1 && path[2] == -1);

     getShortestPath(n, 4, p, path);
     assert(path[0] == 4 && path[1] == 3 && path[2] == 1 && path[3] == -1);

     getShortestPath(n, 5, p, path);
     assert(path[0] == 5 && path[1] == n->null);

     freeNetwork(n);
}

void testCheckTerm(){
     assert(checkTerm("1-2"));
     assert(checkTerm("1"));
     assert(checkTerm("1-2/3"));
     assert(checkTerm("1-2/3.4"));

     assert(checkTerm("y") == false);
     assert(checkTerm("1-y") == false);
     assert(checkTerm("1-2/y") == false);
     assert(checkTerm("1-") == false);
     assert(checkTerm("1-2/") == false);
     assert(checkTerm("1/3") == false);
}

void testAddTerm(){
     network *n = newNetwork(-1);
     addTerm(n, "1");
     assert(n->current->x == 1);
     assert(n->current->links == 0);
     freeNetwork(n);

     n = newNetwork(-1);
     addTerm(n, "1-2");
     assert(n->current->x == 1);
     assert(n->current->links == 1);
     assert(n->current->edge[0]->x == 2);
     freeNetwork(n);

     n = newNetwork(-1);
     addTerm(n, "1-2/3");
     assert(n->current->x == 1);
     assert(n->current->links == 1);
     assert(n->current->edge[0]->x == 2);
     assert(n->current->weight[0] == 3);
     freeNetwork(n);

     n = newNetwork(1);
     addTerm(n, "1-2");
     assert(n->current == NULL);
     freeNetwork(n);
}

void test(){
     testNewNetwork();
     testAddNode();
     testDeleteNode();
     testEmpty();
     testGet();
     testSet();
     testNodes();
     testTraverse();
     testFind();
     testLink();
     testUnlink();
     testGetWeight();
     testSetWeight();
     testCyclic();
     testTree();
     testDepth();
     testDepthFirstSearch();
     testBreadthFirstSearch();
     testIsSubNet();
     testDijkstra();
     testGetDistance();
     testGetPath();
     testCheckTerm();
     testAddTerm();

     printf("Network module tests run OK.\n");
}

int main(int argC, char **argV){
     if(argC == 1) test();
     else if(argC == 3 || argC == 2){
          item root = -1;
          if(argC == 3){ sscanf(argV[2], "%d", &root); }
          network *n = newNetworkFromString(argV[1], -1);
          if(root != -1) setRoot(n, root);

          printInformation(n);

          freeNetwork(n);
     }
     return 0;
}
#endif

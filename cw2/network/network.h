//Library references needed
#include <stdbool.h>

//Change type used here
typedef int item;

//Structs - network is opaque
struct network;
typedef struct network network;

//Function delcarations

//Creates a new, empty network with a defined default value
network *newNetwork(item d);

//Creates a new network with null value d in the format specified by s
//s is composed of a series of connections.
     //eg: 1-2,3-2,2-2/1.5
     //The nodes in this string are {1,2,3}
     //Node 1 has an edge that traverses to Node 2.
     //Node 2 has an edge that traverses in a loop to Node 2.
          //The '/1.5' means that the edge has a weight of 1.5.
          //The weight when unspecified is 1.
     //Node 3 has an edge that traverses in a loop to Node 3.
//The first node created is set to the root and current nodes.
//If x is not in the list, then a NULL pointer is returned.
network *newNetworkFromString(char *s, item d);

//Creates a new node with an initial value
//Sets the current node in the network to the new node
bool addNode(network *n, item x);

//Removes the node containing item x and any edges that point to it, then returns true
//If x is not in the network, false is returned
bool deleteNode(network *n, item x);

void freeNetwork(network *n);

//Checks if the network is empty
bool empty(network *n);

//Returns the value of the currently selected node
//If node is empty, returns the default value
item get(network *n);

//Returns the value of the root node
//If list is empty, n->null returned
item getRoot(network *n);

//Gets the weight of the edge from the current node that points to the node
//containing item y.
//If the current node does not point to the y-node, -1 is returned.
double getWeight(network *n, item y);

//Sets the value of the currently selected node and returns true
//If network is empty, nothing happens and false is returned
bool set(network *n, item x);

//Makes n->root point to the node containing item r
//True/false returned if the change is successful
bool setRoot(network *n, item r);

//Sets the weight of the edge pointing from the current node to y to w.
//If a weight less than 0 is given, nothing happens and false returns.
//If the current node does not point to one containing y, nothing happens and false returns.
bool setWeight(network *n, item y, double w);

//Returns the number of nodes in the network
int nodes(network *n);

//Returns the number of edges attached the the current vertex
//If no vertex is selected, 0 is returned as a precautionary measure
int edges(network *n);

//Moves from the current node to the linked node containing x, then returns true
//If no neighbouring node contains x, return false
bool traverse(network *n, item x);

//Moves the current node back to the root node
bool reset(network *n);

//Links the current node with the node containing item y and returns true
//The edge formed travels from the current node to the node containing item y
//If y is not in the network, nothing happens and false is returned
//If y is already linked, nothing happens and false is returned
//If the weight w is illegal (<0) nothing happens and false is returned
bool link(network *n, item y, double w);

//Removes the link between the currently selected node and the node containing item y, then returns true
//If n->current does not point to y, false is returned and nothing is done
//If n is empty, false is returned and nothing is done
bool unlink(network *n, item y);

//If there are cycles in the network, true is returned
bool isCyclic(network *n);

//Returns true if the network is a tree.
//Uses the node that n->root points to as the root node (should have 0 parents)
//True is returned if:
     //The graph is not cyclic
     //Each node has exactly one parent node
     //excepting the root node, which should have 0.
bool isTree(network *n);

//Returns the depth of the network
//Uses the node that n->root points to as the starting point
//0 is returned for an empty network
//-1 is returned if the network is not a tree
int depth(network *n);

//Returns true if the item x exists as a node in the network n
//If goTo is set to true, the network then traverses to that node and returns true
//If goTo is false, nothing will happen and it will simply return true
//If the item cannot be found or the network is empty, nothing happens and false returns
//The algorithm recursively checks each child of the root node until x is found, or not in the network
bool depthFirstSearch(network *n, item x, bool goTo);
//Same principle as depthFirstSearch, but a different algorithm is used
//Checks every node at each depth before moving on to the next
bool breadthFirstSearch(network *n, item x, bool goTo);

//Checks if the network m is a sub-network of network n.
//m is a subnet if n contains all of the nodes in m,
//and if each of those nodes point to all of the nodes that they do in m.
//Nodes in n that match nodes in m can also point to nodes that their corresponding m nodes do not point to,
//but the converse cannot be true.
//If either n or m are empty, false is returned automatically.
bool isSubNet(network *n, network *m);

//Calculates the shortest distance from the root node to every node in the network
//If a node can't be traversed to from the root node, its distance is returned as -1
//and its previous value is given as the null value of the network.
void dijkstra(network *n, double *d, item *p);

//Returns the numerical value of the shortest distance to item y.
//If y is not in n, -1 is returned.
double getShortestDistance(network *n, item y, double *d);

//Finds the full path from the root value
//to the node containing item y. This is put in the 'path' array.
//If y is not in n, path is left unchanged.
void getShortestPath(network *n, item y, item *p, item *path);

//Prints all of the information calculated by the running of dijkstra(n,d,p).
//Includes the shortest distance as well as the full path from the root node.
void printDijkstra(network *n, double *d, item *p);

//Prints the entire network in the form of an adjacency list
//If the network is empty, nothing is printed
void printList(network *n);

//Prints the entire network in the form of an adjacency matrix
//If the network is empty, nothing is printed
void printMatrix(network *n);

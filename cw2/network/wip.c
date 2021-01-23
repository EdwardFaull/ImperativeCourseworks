bool unlink(network *n, item y){
     if(empty(n)) return false;
     node *nodeX = n->current;
     node *nodeY = find(n, y);
     int length = edgeCount(n);
     int nLength = neighbourCount(nodeX);

     if(contains(length, nodeX->edges, y) == false) return false;

     deleteFromNodeArr(length, nodeX->edges, y);
     //If node Y does not point to node X, deleteFromNodeArr node Y from node X's neighbours list
     //No longer an edge from node X to Y or from node Y to X, so does not belong.
     if(!contains(edgeCountN(nodeY), nodeY->edges, nodeX->x)){
          deleteFromNodeArr(nLength, nodeX->neighbours, y);
          deleteFromNodeArr(neighbourCount(nodeY), nodeY->neighbours, nodeX->x);
     }

     return true;
}

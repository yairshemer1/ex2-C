#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define MAX_LINE_LEN 1024

#define INVALID_INPUT fprintf(stderr, "Invalid input\n"); return EXIT_FAILURE;

#define MEM_FAILURE fprintf(stderr, "Memory allocation failed\n"); return EXIT_FAILURE;

#define INVALID_TREE fprintf(stderr, "The given graph is not a tree\n"); return EXIT_FAILURE;

#define INVALID_ARG fprintf(stderr, \
"Usage: TreeAnalyzer <Graph File Path> <First Vertex> <Second Vertex>\n"); return EXIT_FAILURE;

static const int MEM_ERR = -2;

static const int ERROR = -1;

static const int FIRST_NODE_ARG = 2;

static const int SECOND_NODE_ARG = 3;

static const int TXT_LOC_ARG = 1;

static char *const NO_CHILDREN = "-";

static char *const NO_CHILDREN_EOF = "-\r\n";

static char *const DELIMS = " \r\n";

static const int CLI_ARGS = 4;

/*
 * ************************* NODE *************************************
 */

/**
 * Node struct will represent a Node in a graph, holding it's key, number of children and parents,
 * distance field for BFS run, pointer to pointers to it's children (other nodes),
 * previous node and parent node.
 */
typedef struct Node
{
    unsigned int key;
    int numOfChildren;
    int numOfParents;
    int distance;
    struct Node **children;
    struct Node *parent;
    struct Node *prev;
} Node;


/*
 * ************************* END-NODE *********************************
 */

/*
 * ************************* GRAPH ************************************
 */
/**
 * Graph holds a group of Nodes, it's root and number of nodes and edges in it.
 */
typedef struct Graph
{
    int numOfVertex;
    int numOfEdges;
    Node *nodes;
    Node *root;
} Graph;

/**
 * Allocates space for a new graph
 * @return the new graph or null if there's no memory
 */
Graph *allocGraph()
{
    Graph *g = (Graph *) malloc(sizeof(Graph));
    if (g == NULL)
    {
        return NULL;
    }
    g->numOfVertex = 0;
    g->numOfEdges = 0;
    g->nodes = NULL;
    g->root = NULL;
    return g;
}

/**
 * Frees the graph and it's nodes
 * @param g the graph to free
 */
void freeGraph(Graph **g)
{
    if (*g == NULL)
    {
        return;
    }
    for (int i = 0; i < (*g)->numOfVertex; i++)
    {
        free((*g)->nodes[i].children);
    }
    free((*g)->nodes);
    free(*g);
    *g = NULL;
}

/**
 * Adds a child to a given node. changing respectively the parent
 * @param g the graph of which we work on, to add an edge
 * @param node the node representing the parent
 * @param child a new child to the given node
 * @return 0 upon success, -1 upon invalid input and -2 upon memory failure
 */
int addChild(Graph *g, Node *node, Node *child)
{
    if (node == NULL || child == NULL)
    {
        return ERROR;
    }

    node->children = (Node **) realloc(node->children, (node->numOfChildren + 1) * sizeof(Node *));

    if (node->children == NULL)
    {
        return MEM_ERR;
    }
    node->children[node->numOfChildren] = child;
    child->parent = node;
    node->numOfChildren++;
    child->numOfParents++;
    g->numOfEdges++;
    return EXIT_SUCCESS;
}

/*
 * ************************* END-GRAPH ********************************
 */
/**
 * Checks if we are in the end ofa given file, keeping the information on it
 * @param fp file
 * @return -1 if EOF, 0 if there's still text
 */
int fileCheckEOF(FILE *fp)
{
    int c = fgetc(fp);
    if (c == EOF)
    {
        return ERROR;
    }
    //put back in stream
    ungetc(c, fp);
    return EXIT_SUCCESS;
}

/**
 * Creates an integer out of a given string, if possible
 * @param str the string to analyze
 * @return the int, or -1 if it's not a number
 */
int strToInt(char *str)
{
    char *strEnd;
    int numToRet = (int) strtol(str, &strEnd, 0);
    if (numToRet < 0 || strlen(strEnd) != 0)
    {
        return ERROR;
    }
    return numToRet;
}

/**
 * Given the line, if the line means "-" as for no children, it will be found out.
 * @param line the line was read
 * @return 0 if the line represents "-", -1 otherwise
 */
int checkNoChildren(char *line)
{
    if (strcmp(line, NO_CHILDREN_EOF) == 0 || strcmp(line, NO_CHILDREN) == 0)
    {
        return EXIT_SUCCESS;
    }
    return ERROR;
}

/**
 * Read a line from the folder and treating the nodes from the file, according to instructions,
 * to create a graph in the end
 * @param fp the file
 * @param g the graph
 * @param nodeIndex the line of which the node was read from, representing it's key
 * @return 0 upon success, -1 upon invalid input and -2 upon memory failure
 */
int readLine(FILE *fp, Graph *g, int nodeIndex)
{
    //init node's key
    g->nodes[nodeIndex].key = nodeIndex;
    char line[MAX_LINE_LEN];
    if (fileCheckEOF(fp) == ERROR)
    {
        return ERROR;
    }

    fgets(line, MAX_LINE_LEN, fp);
    if (!checkNoChildren(line))
    {
        return EXIT_SUCCESS;
    }
    char *value = strtok(line, DELIMS);
    if (value == NULL)
    {
        return ERROR;
    }
    while (value)
    {
        int num = strToInt(value);
        // if it's not a number or the number is bigger then number of vertex in the graph,
        //failure
        if (num == ERROR || num >= g->numOfVertex)
        {
            return ERROR;
        }

        int addRes = addChild(g, &(g->nodes[nodeIndex]), &(g->nodes[num]));
        if (addRes == ERROR)
        {
            return ERROR;
        }
        else if (addRes == MEM_ERR)
        {
            return MEM_ERR;
        }
        value = strtok(NULL, DELIMS);
    }
    return EXIT_SUCCESS;
}

/**
 * Function to initialize the graph - create all nodes according to the first line, and after
 * that using readLine one by one in order to create the edges between the nodes, as mentioned
 * in the given file
 * @param g the graph of which we work on
 * @param fp the file
 * @return 0 upon success, -1 upon invalid input and -2 upon memory failure
 */
int initializeGraph(Graph **g, FILE *fp)
{
    //reading first line
    char firstLine[MAX_LINE_LEN];
    fgets(firstLine, MAX_LINE_LEN, fp);
    char *line;
    line = strtok(firstLine, DELIMS);
    int vertexNum = strToInt(line);
    if (vertexNum == ERROR || strtok(NULL, DELIMS))
    {
        return ERROR;
    }
    //init the nodes in the right amount
    (*g)->nodes = (Node *) calloc(vertexNum, sizeof(Node));
    (*g)->numOfVertex = vertexNum;
    if ((*g)->nodes == NULL)
    {
        return MEM_ERR;
    }
    //graph is initialized with nodes and first line was read
    //next, read line and determine children for each node using readLine
    for (int i = 0; i < vertexNum; i++)
    {
        int readRes = readLine(fp, *g, i);
        if (readRes == ERROR)
        {
            return ERROR;
        }
        else if (readRes == MEM_ERR)
        {
            return MEM_ERR;
        }
    }
    //try to read another line, if it's not EOF, failure
    if (fileCheckEOF(fp) != ERROR)
    {
        return ERROR;
    }
    return EXIT_SUCCESS;

}

/**
 * Checks if the graph is connected, starting from the root
 * @param counter holds the number of nodes we're going through the "explore"
 * @param root the node to start with
 */
void isConnected(int *counter, Node *root)
{
    (*counter)++;
    for (int i = 0; i < root->numOfChildren; i++)
    {
        isConnected(counter, root->children[i]);
    }
}

/**
 * Checks if the graph is a Tree. If a node has 0 parents then it's the root, if 1 parent, its
 * just one of the nodes and if more than 1 parent, it's not a tree. Using isConnected we make sure
 * the graph is connected and stands the parent's number rule.
 * @param g the graph of which we work on
 * @return 0 upon success, -1 upon invalid input and -2 upon memory failure
 */
int isTree(Graph *g)
{
    if (g->numOfVertex - g->numOfEdges != 1)
    {
        return ERROR;
    }
    for (int i = 0; i < g->numOfVertex; i++)
    {
        if (g->root == NULL && g->nodes[i].numOfParents == 0)
        {
            g->root = &(g->nodes[i]);
        }
            //if another vertex with 0 parents
        else if (g->root != NULL && g->nodes[i].numOfParents == 0)
        {
            return ERROR;
        }
            //if has more then 1 parent
        else if (g->nodes[i].numOfParents > 1)
        {
            return ERROR;
        }
    }
    int counter = 0;
    //check if the graph is connected
    isConnected(&counter, g->root);
    if (counter != g->numOfVertex)
    {
        return ERROR;
    }
    return EXIT_SUCCESS;
}

/**
 * BFS as written in the ex. instructions. Making sure that the parent is visited as well as
 * the children, because once it's a tree, the directions of the edges has no meaning.
 * @param g the graph of which we work on
 * @param s the node to start
 */
void BFS(Graph *g, Node *s)
{
    for (int i = 0; i < g->numOfVertex; i++)
    {
        g->nodes[i].distance = ERROR;
    }
    s->distance = 0;
    Queue *q = allocQueue();
    enqueue(q, s->key);
    //we will pay attention that we will treat the parent and the children the same
    // way - it is a non cyclic graph once it's a tree
    while (!queueIsEmpty(q))
    {
        //take a node from the queue
        Node *cur = &(g->nodes[dequeue(q)]);
        //checking children
        for (int i = 0; i < cur->numOfChildren; i++)
        {
            //if it's child's distance is -1, update it and put it in the queue
            if (cur->children[i]->distance == ERROR)
            {
                enqueue(q, cur->children[i]->key);
                cur->children[i]->prev = cur;
                cur->children[i]->distance = cur->distance + 1;
            }
        }
        //checking parent
        if (cur->parent != NULL && cur->parent->distance == ERROR)
        {
            enqueue(q, cur->parent->key);
            cur->parent->prev = cur;
            cur->parent->distance = cur->distance + 1;
        }

    }
    freeQueue(&q);
}

/**
 * Finds the maximum height of the graph.
 * @param g the graph of which we work on
 * @return the max height in g
 */
int minHeight(Graph *g)
{
    int minimum = g->numOfEdges;
    for (int i = 0; i < g->numOfVertex; i++)
    {
        if (g->nodes[i].numOfChildren == 0 && g->nodes[i].distance < minimum)
        {
            minimum = g->nodes[i].distance;
        }
    }
    return minimum;
}

/**
 * Finds the minimum height of the graph.
 * @param g the graph of which we work on
 * @return the min height in g
 */
int maxHeight(Graph *g)
{
    int maximum = 0;
    for (int i = 0; i < g->numOfVertex; i++)
    {
        if (g->nodes[i].numOfChildren == 0 && g->nodes[i].distance > maximum)
        {
            maximum = g->nodes[i].distance;
        }
    }
    return maximum;
}

/**
 * Finds the diameter of the graph
 * @param g the graph of which we work on
 * @return the diameter of g
 */
int findDiameter(Graph *g)
{
    //0 vertex will not go into the for loop and therefore let diameter -1, and if
    //the graph contains 1 vertex, BFS will make it's only node's distance 0, which will be
    //the maxHeight, as expected
    int diameter = ERROR, currentMax;
    //start BFS from all nodes and find longest height
    for (int i = 0; i < g->numOfVertex; i++)
    {
        BFS(g, &(g->nodes[i]));
        //now if it's a leaf, maxHeight will return the longest way to another leaf
        currentMax = maxHeight(g);
        if (currentMax > diameter)
        {
            diameter = currentMax;
        }
    }
    return diameter;
}

/**
 * Main function to analyze the tree and print it's information
 * @param argc amout of args from cli
 * @param argv list of args from cli
 * @return 0 upon success, 1 upon failure
 */
int main(int argc, char *argv[])
{
    if (argc != CLI_ARGS)
    {
        INVALID_ARG
    }
    FILE *fp = fopen(argv[TXT_LOC_ARG], "r");
    if (fp == NULL)
    {
        INVALID_INPUT
    }
    if (fileCheckEOF(fp))
    {
        INVALID_INPUT
    }
    Graph *graph = allocGraph();
    if (graph == NULL)
    {
        fclose(fp);
        MEM_FAILURE
    }
    int initRes = initializeGraph(&graph, fp);
    if (initRes == ERROR)
    {
        fclose(fp);
        freeGraph(&graph);
        INVALID_INPUT
    }
    else if (initRes == MEM_ERR)
    {
        fclose(fp);
        freeGraph(&graph);
        MEM_FAILURE
    }
    char *n1 = argv[FIRST_NODE_ARG], *n2 = argv[SECOND_NODE_ARG];
    int v = strToInt(n1), u = strToInt(n2);
    if (v == ERROR || u == ERROR || v >= graph->numOfVertex || u >= graph->numOfVertex)
    {
        INVALID_INPUT
    }
    //check if the graph is a tree and set the root
    if (isTree(graph) == ERROR)
    {
        INVALID_TREE
    }

    //printing after checks are made
    printf("Root Vertex: %d\n", graph->root->key);
    printf("Vertices Count: %d\n", graph->numOfVertex);
    printf("Edges Count: %d\n", graph->numOfEdges);
    BFS(graph, graph->root);
    printf("Length of Minimal Branch: %d\n", minHeight(graph));
    printf("Length of Maximal Branch: %d\n", maxHeight(graph));
    printf("Diameter Length: %d\n", findDiameter(graph));
    printf("Shortest Path Between %d and %d: ", v, u);
    Node *vNode = &graph->nodes[v], *uNode = &graph->nodes[u];
    BFS(graph, uNode);
    //after BFS, if there's a way from v to u, and I assume there is by instructions
    // (undirected once it's a tree), it will be by going backwards from v to u using prev field
    while (vNode->key != (unsigned int) u)
    {
        printf("%d ", vNode->key);
        vNode = vNode->prev;
    }
    printf("%d\n", u);
    freeGraph(&graph);
    fclose(fp);
    return EXIT_SUCCESS;
}


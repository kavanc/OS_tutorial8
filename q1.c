#include <stddef.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <unistd.h> 
#include <signal.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <string.h>

// process struct
typedef struct proc
{
    char parent[256];
    char name[256];
    int priority;
    int memory;

} proc;

// node struct
typedef struct node
{
    proc process;
    struct node *left;
    struct node *right;
} node_t;


// inserts element into tree
node_t* insert(node_t *tree, proc process)
{
    // instantiate a bunch of stuff
    node_t *new_node = (node_t*) malloc(sizeof(node_t));
    new_node->process = process;
    node_t *root = tree;
    node_t *tmp = NULL;

    // if its the first node, set root as new node
    if(!strcmp(process.parent, "NULL")) {
        root = new_node;
        return root;
    }

    // find the parent node, assume left if parent node can't be found
    while(root) {
        tmp = root;

        if(strcmp(process.parent, root->process.name) == 0) {
            break;
        } else if(root->left && strcmp(process.parent, root->left->process.name) == 0) {
            root = root->left;
        } else if(root->right && strcmp(process.parent, root->right->process.name) == 0) {
            root = root->right;
        } else {
            root = root->left;
        }
    }   

    // set left child as new_node unless left exists then set right
    if(!tmp->left) tmp->left = new_node;
    else tmp->right = new_node;

    return tmp;
}

// pre-order traversal
void pre_order(node_t *current)
{
    // if no node return
    if (!current) return;

    // print value of the node and its children
    printf("Val: %s\n", current->process.name);
    if(current->left) printf("Left: %s\n", current->left->process.name);
    if(current->right) printf("Right: %s\n", current->right->process.name);
    printf("\n");

    // traverse tree left
    pre_order(current->left);
    // traverse tree right
    pre_order(current->right);
}


int main(void) 
{
    FILE *fp;
    char s[3] = ", "; // this needs a space for proper tokenization
    char line[256];
    node_t *tree = (node_t*) malloc(sizeof(node_t));
    
    // open file and validate success
    fp = fopen("processes_tree.txt", "r");

    if(!fp){
        printf("Error!");   
        return -1;
    }

    // iterate over file and read in each line as tokens and assign to process struct
    int i = 0;
    while(fgets(line, sizeof(line), fp)){
        proc p;
        strcpy(p.parent, strtok(line, s));
        strcpy(p.name, strtok(NULL, s));
        p.priority = atoi(strtok(NULL, s));
        p.memory = atoi(strtok(NULL, s));
        // set root node value
        if(i++ == 0) {
            tree = insert(tree, p);
            continue;
        }

        // create new node
        insert(tree, p);
    }

    // perform pre-order traversal
    pre_order(tree);

    // close file
    fclose(fp);
} 
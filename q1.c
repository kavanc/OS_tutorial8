#include <stddef.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <unistd.h> 
#include <signal.h> 
#include <sys/types.h> 
#include <sys/wait.h> 

typedef struct proc
{
    char parent[256];
    char name[256];
    int priority;
    int memory;

}proc;

typedef struct node
{
    proc process;
    struct node *left;
    struct node *right;
} node_t;


void insert(node_t *tree, proc process){
    

}


int main(void) {
    FILE *fp;
    char s[2] = ",";
    char line[256];
    node_t *tree = (node_t*) malloc(sizeof(node_t));
    
    int p_pid;

    fp = fopen("processes_tree.txt", "r");

    if(!fp){
        printf("Error!");   
        exit(1);      
    }

    while(fgets(line, sizeof(line), fp)){
        proc p;
        strcpy(p.parent, strtok(line, s));
        strcpy(p.name, strtok(NULL,s));
        p.priority = atoi(strtok(NULL,s));
        p.memory = atoi(strtok(NULL, s));
        insert(tree, p);


    }
   
} 
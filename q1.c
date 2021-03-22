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

};

struct node
{
    int key_value;
    struct node *left;
    struct node *right;
};


int main(void) {
    FILE *fp;
    fp = fopen("processes_tree.txt", "r");
    if(fp == NULL){
        printf("Error!");   
        exit(1);      
    }
} 
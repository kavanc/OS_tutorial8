#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MEMORY 1024

// process struct
typedef struct {
    char name[256];
    int priority;
    int pid;
    int address;
    int memory;
    int runtime;
    bool suspended;
} proc;

// node struct
typedef struct node {
    proc process;
    struct node *next;
} node_t;

// push node to linked list
void push(proc process, node_t *head)
{
    node_t *current = head;

    while(current->next) current = current->next;

    current->next = (node_t*) malloc(sizeof(node_t));
    current->next->process = process;
    current->next->next = NULL;
}

// remove first node from linked list
proc pop(node_t **head)
{
    proc p = (*head)->process;
    node_t *next_node = NULL;

    if(!*head) return p;

    next_node = (*head)->next;
    p = (*head)->process;
    free(*head);
    *head = next_node;

    return p;
}

int main()
{
    FILE *fp;
    char s[2] = ","; 
    char line[256];
    node_t *priority = (node_t*) malloc(sizeof(node_t));
    node_t *secondary = (node_t*) malloc(sizeof(node_t));
    int p_pid, s_pid;

    int avail_mem[MEMORY] = {0};

    // open file and validate
    fp = fopen("processes_q2.txt", "r");

    if (!fp) {
        printf("Failed to open file...");
        return -1;
    }

    // read in contents of file to process structs and add it to the two linked list
    int p_count = 0;
    int s_count = 0;
    while(fgets(line, sizeof(line), fp)) {
        proc p;
        strcpy(p.name, strtok(line, s));
        p.priority = atoi(strtok(NULL, s));
        p.memory = atoi(strtok(NULL, s));
        p.runtime = atoi(strtok(NULL, s));
        p.pid = 0;
        p.address = 0;
        p.suspended = false;

        // add to priority queue if priority is zero
        if(p.priority == 0) {
            if(p_count++ == 0) {
                priority->process = p;
            } else {
                push(p, priority);
            }
        // add to secondary queue
        } else {
            if(s_count++ == 0) {
                secondary->process = p;
            } else {
                push(p, secondary);
            }
        }
    }

    // close file
    fclose(fp);

    // iterate over priority queue and run program
    while(priority) {
        // pop off process
        proc p = pop(&priority);

        // get memory start index
        int s_ind = 0;
        while(1) {
            if(avail_mem[s_ind] == 0) break;
            s_ind++;
        }

        // allocated memory in avail_mem
        for(int i = s_ind; i < s_ind + p.memory; i++) {
            avail_mem[i] = 1;
        }

        p.address = s_ind;

        // fork process
        p_pid = fork();

        if(!p_pid) {
            // print data on process and run it
            p.pid = getpid();
            printf("%s, %d, %d, %d, %d\n", p.name, p.priority, p.pid, p.memory, p.runtime);
            execl(p.name, NULL);
            break;
        } else {
            // wait for process to run, send it termination signal and wait for it to be complete
            sleep(p.runtime);
            kill(p_pid, SIGTSTP);
            waitpid(p_pid, NULL, 0);

            // free the memory
            for(int i = p.address; i < p.address + p.memory; i++) {
                avail_mem[i] = 0;
            }
        }
    }

    // iterate over secondary queue
    while(secondary && p_pid) {
        // pop off head
        proc p = pop(&secondary);

        // if the program hasn't been suspended, check for enough memory and if so allocate it
        if(!p.suspended) {
            int a = 0;
            while(true) {
                if(avail_mem[a] == 0) {
                    for(int i = a; i < a + p.memory; i++) {
                        if(avail_mem[i] == 1) {
                            a = i + 1;
                            break;
                        }
                    }
                    break;
                } else {
                    a++;
                }
            } 

            // if theres not enough memory, push the process back onto the queue
            if(a + p.memory > MEMORY) {
                push(p, secondary);
                continue;
            }
            // set the memory address
            p.address = a;

            // allocate memory
            for(int i = p.address; i < p.address + p.memory; i++) {
                avail_mem[i] = 1;
            }
        }

        // if the process doesn't have an idea, create a fork
        if(!p.pid) s_pid = fork();

        // set the process id, print info on the process and execute it
        if(!s_pid) {
            p.pid = getpid();
            printf("%s, %d, %d, %d, %d\n", p.name, p.priority, p.pid, p.memory, p.runtime);
            execl(p.name, NULL);
            break;
        } else {
            // if the process has only 1 second remaining, run it, kill it, wait for it and deallocate memory
            if(p.runtime == 1) {
                kill(p.pid, SIGCONT);
                sleep(1);
                kill(p.pid, SIGINT);
                waitpid(p.pid, NULL, 0);
                for(int i = p.address; i < p.address + p.memory; i++) {
                    avail_mem[i] = 0;
                }
                continue;
            }

            // if the process was suspended, continue it
            if(p.suspended && p.pid) {
                kill(p.pid, SIGCONT);
            }

            // wait for process to run for 1 sec then suspend it
            sleep(1);
            kill(s_pid, SIGTSTP);

            // set process id
            if(!p.pid) {
                p.pid = s_pid;
            }

            // decrement runtime and set suspended to true
            p.runtime--;
            p.suspended = true;

            // if there is only one process left, allocate memory for the node and re add it to the head of the stack
            if(!secondary) {
                secondary = (node_t*) malloc(sizeof(node_t));
                secondary->process = p;
                secondary->next = NULL;
                continue;
            }
            // push suspended process to the queue
            push(p, secondary);
        }
    }

    return 0;
}
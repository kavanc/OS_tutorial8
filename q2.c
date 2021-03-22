#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MEMORY 1024

typedef struct {
    char name[256];
    int priority;
    int pid;
    int address;
    int memory;
    int runtime;
    bool suspended
} proc;

typedef struct node {
    proc process;
    struct node *next;
} node_t;

void push(proc process, node_t *head)
{
    node_t *current = head;

    while(current->next) current = current->next;

    current->next = (node_t*) malloc(sizeof(node_t));
    current->next->process = process;
    current->next->next = NULL;
}

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

    fp = fopen("processes_q2.txt", "r");

    if (!fp) {
        printf("Failed to open file...");
        return -1;
    }

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

        if(p.priority == 0) {
            if(p_count++ == 0) {
                priority->process = p;
            } else {
                push(p, priority);
            }
        } else {
            if(s_count++ == 0) {
                secondary->process = p;
            } else {
                push(p, secondary);
            }
        }
    }

    while(priority) {
        proc p = pop(&priority);

        int s_ind = 0;
        while(1) {
            if(avail_mem[s_ind] == 0) break;
            s_ind++;
        }

        for(int i = s_ind; i < s_ind + p.memory; i++) {
            avail_mem[i] = 1;
        }

        p.address = s_ind;
        p_pid = fork();

        if(!p_pid) {
            p.pid = getpid();
            printf("%s, %d, %d, %d, %d\n", p.name, p.priority, p.pid, p.memory, p.runtime);
            execl(p.name, NULL);
            break;
        } else {
            sleep(p.runtime);
            kill(p_pid, SIGTSTP);
            waitpid(p_pid, NULL, 0);

            for(int i = p.address; i < p.address + p.memory; i++) {
                avail_mem[i] = 0;
            }
        }
    }

    while(secondary && p_pid) {
        proc p = pop(&secondary);

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

            if(a + p.memory > MEMORY && p.suspended == false) {
                push(p, secondary);
                continue;
            }
            p.address = a;

            for(int i = p.address; i < p.address + p.memory; i++) {
                avail_mem[i] = 1;
            }
        }

        if(!p.pid) s_pid = fork();

        if(!s_pid) {
            p.pid = getpid();
            printf("%s, %d, %d, %d, %d\n", p.name, p.priority, p.pid, p.memory, p.runtime);
            execl(p.name, NULL);
            break;
        } else {
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

            if(p.suspended && p.pid) {
                kill(p.pid, SIGCONT);
            }

            sleep(1);
            kill(s_pid, SIGTSTP);

            if(!p.pid) {
                p.pid = s_pid;
            }

            p.runtime--;
            p.suspended = true;
            if(!secondary) {
                secondary = (node_t*) malloc(sizeof(node_t));
                secondary->process = p;
                secondary->next = NULL;
                continue;
            }
            push(p, secondary);
        }
    }

    return 0;
}
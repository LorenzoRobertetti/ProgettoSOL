#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "./util.h"

void* manage_cust(void* arg) {
    manage_arg argmanage = (*((manage_arg*) arg));
    pthread_t* cust_th = argmanage.cust_th;
    arg_cust* custargv = argmanage.custargv;
    core* supm = argmanage.smdata;
    pthread_mutex_t* director_mutex = supm->director_mutex;
    pthread_cond_t* director_cond = supm->director_cond;
    queue* director_queue = supm->director_queue;
    int bound = c - e;
    int d = c;
    //int count = 1;
    //Preparo il cliente di terminazione da mandare al DIRETTORE
    cust_data dataclose;
    dataclose.cust_id = TERM_ID;
    //Preparo gli argomenti dei c thread CLIENTI
    for(int i = 0; i < c; i++) {
        custargv[i].id = i;
        custargv[i].smdata = supm;
    }
    //Faccio partire i c thread CLIENTI
    if(pthread_mutex_lock(&mutex_tot_cust) != 0) {
        perror("Error mutex_lock gestore\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < c; i++) {
        tot_cust++;
        if(pthread_create(&cust_th[i], NULL, cliente, &custargv[i]) != 0) {
            perror("Error pthread_create gestore\n");
            exit(EXIT_FAILURE);
        }
    }
    if(pthread_mutex_unlock(&mutex_tot_cust) != 0) {
        perror("Error mutex_unlock gestore\n");
        exit(EXIT_FAILURE);
    }
    //Ora entro nel ciclo di gestione vera e propria
    while(1) {
        if(sm_emerg == 1) {
            if(pthread_mutex_lock(director_mutex) != 0) {
                perror("Error mutex_lock gestore\n");
                exit(EXIT_FAILURE);
            }
            if(director_queue->head == NULL) {
                if(pthread_cond_signal(director_cond) != 0) {
                    perror("Error cond_signal gestore\n");
                    exit(EXIT_FAILURE);
                }
            }
            (*director_queue) = pushf(*director_queue, dataclose);
            if(pthread_mutex_unlock(director_mutex) != 0) {
                perror("Error mutex_unlock gestore\n");
                exit(EXIT_FAILURE);
            }
            return NULL;
        }
        if(pthread_mutex_lock(&mutex_tot_cust) != 0) {
            perror("Error mutex_lock gestore\n");
            exit(EXIT_FAILURE);
        }
        while(tot_cust > bound) {
            if(pthread_cond_wait(&cond_tot_cust, &mutex_tot_cust) != 0) {
                perror("Error cond_wait gestore\n");
                exit(EXIT_FAILURE);
            }
        }
        //Se il supermercato e' chiuso non faccio entrare altri clienti;
        if(sm_open == 0) {
            if(pthread_cond_wait(&cond_tot_cust, &mutex_tot_cust) != 0) {
                perror("Error cond_wait gestore\n");
                exit(EXIT_FAILURE);
            }
            if(pthread_mutex_unlock(&mutex_tot_cust) != 0) {
                perror("Error mutex_unlock gestore\n");
                exit(EXIT_FAILURE);
            }
            if(pthread_mutex_lock(director_mutex) != 0) {
                perror("Error mutex_lock gestore\n");
                exit(EXIT_FAILURE);
            }
            if(director_queue->head == NULL) {
                if(pthread_cond_signal(director_cond) != 0) {
                    perror("Error cond_signal gestore\n");
                    exit(EXIT_FAILURE);
                }
            }
            (*director_queue) = pushf(*director_queue, dataclose);
            if(pthread_mutex_unlock(director_mutex) != 0) {
                perror("Error mutex_unlock gestore\n");
                exit(EXIT_FAILURE);
            }
            return NULL;
        }
        int limit = d + e;
        int i = 0;
        while(d < limit) {
            custargv[i].id = d;
            custargv[i].smdata = supm;
            tot_cust++;
            if(pthread_create(&cust_th[i], NULL, cliente, &custargv[i]) != 0) {
                perror("Error pthread_create gestore\n");
                exit(EXIT_FAILURE);
            }
            d++;
            i++;
        }
        if(pthread_mutex_unlock(&mutex_tot_cust) != 0) {
            perror("Error mutex_unlock gestore\n");
            exit(EXIT_FAILURE);
        }
    }
}

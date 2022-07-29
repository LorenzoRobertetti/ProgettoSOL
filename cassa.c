#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/timeb.h>
#include "./util.h"

void* timer_check(void* arg) {
    timer_check_arg timerarg = (*(timer_check_arg*) arg);
    int* isopen = timerarg.isopen;
    int* numcust = timerarg.num_cust;
    queue* q = timerarg.q;
    pthread_mutex_t* mutex_check = timerarg.mutex_check;
    int len = 0;

    while(1) {
        if((*isopen) == 0) {
            return NULL;
        }
        if(sm_emerg == 1) {
            return NULL;
        }
        long msec = z;
        struct timespec ts;
        ts.tv_sec = msec / 1000;
        ts.tv_nsec = (msec % 1000) * 1000000;
        if(nanosleep(&ts, NULL) == -1) {
            perror("Error nanosleep\n");
            exit(EXIT_FAILURE);
        }
        //Leggo quanti clienti ci sono in coda
        if(pthread_mutex_lock(mutex_check) != 0) {
            perror("Error mutex_lock time_check\n");
            exit(EXIT_FAILURE);
        }
        len = (q->len);
        if(pthread_mutex_unlock(mutex_check) != 0) {
            perror("Error mutex_unlock cliente\n");
            exit(EXIT_FAILURE);
        }
        //Aggiorno il numero di clienti in coda
        if(pthread_mutex_lock(&mutex_num) != 0) {
            perror("Error mutex_lock time_check\n");
            exit(EXIT_FAILURE);
        }
        (*numcust) = len;
        if(pthread_mutex_unlock(&mutex_num) != 0) {
            perror("Error mutex_unlock cliente\n");
            exit(EXIT_FAILURE);
        }

    }
}

void* cassa(void* arg) {
    //Parsing argomenti thread cassiere
    arg_checkout argcheck = (*(arg_checkout*) arg);
    int i = argcheck.checkout_number;
    int* isopen = (argcheck.smdata->vec_open) + i;
    int* numcust = (argcheck.smdata->vec_cnum) + i;
    int* tot_prod = (argcheck.smdata->tot_prod_check) + i;
    int* tot_cust_rel = (argcheck.smdata->cust_num_check) + i;
    int* tot_close_i = (argcheck.smdata->tot_close) + i;
    FILE* log_fp = argcheck.smdata->log_fp;
    queue* q = (argcheck.smdata->vec_queue) + i;
    pthread_mutex_t* mutex_check = (argcheck.smdata->vec_mutex_queue) + i;
    pthread_cond_t* check_cond = argcheck.check_cond;
    //Struct per misurare il tempo di apertura
    struct timeb time_check_2;
    struct timeb time_check_1;
    //Prima misura
    if(ftime(&time_check_1) == -1) {
        perror("Error ftime\n");
        exit(EXIT_FAILURE);
    }
    //**********THREAD TIME_CHECK CASSA************
    pthread_t timer;
    //Inizializzo l'argomento del thread
    timer_check_arg timerarg;
    timerarg.i = i;
    timerarg.isopen = isopen;
    timerarg.num_cust = numcust;
    timerarg.mutex_check = mutex_check;
    timerarg.q = q;
    //Faccio partire il thread;
    if(pthread_create(&timer, NULL, timer_check, &timerarg) != 0) {
        perror("Error pthread_create cassa\n");
        exit(EXIT_FAILURE);
    }
    //Calcolo il tempo fisso iniziale
    //Aggiungo pthread_self per assicurarmi la diversita' del seme successivamente ad una riapertura
    unsigned int seedp = time(NULL) + i + pthread_self();
    int r = (rand_r(&seedp) % 60) + 20;
    //struct per i dati del cliente;
    cust_data data;

    while(1) {
        //Routine tipica di attesa e servizio
        if(pthread_mutex_lock(mutex_check) != 0) {
            perror("Error mutex_lock time_check\n");
            exit(EXIT_FAILURE);
        }
        while(q->head == NULL) {
            if(pthread_cond_wait(check_cond, mutex_check) != 0) {
                perror("Error cond_wait cassa\n");
                exit(EXIT_FAILURE);
            }
        }
        (*q) = pop((*q), &data);
        if(pthread_mutex_unlock(mutex_check) != 0) {
            perror("Error mutex_unlock cassa\n");
            exit(EXIT_FAILURE);
        }
        //Caso chiusura cassa (cliente di terminazione)
        if(data.cust_id == (TERM_ID)) {
            if(pthread_mutex_lock(mutex_check) != 0) {
                perror("Error mutex_lock time_check\n");
                exit(EXIT_FAILURE);
            }
            (*q) = del_queue((*q));
            if(pthread_mutex_unlock(mutex_check) != 0) {
                perror("Error mutex_unlock cassa\n");
                exit(EXIT_FAILURE);
            }
            if(pthread_join(timer, NULL) != 0) {
                perror("Error mutex_unlock cassa\n");
                exit(EXIT_FAILURE);
            }
            if(ftime(&time_check_2) == -1) {
                perror("Error ftime\n");
                exit(EXIT_FAILURE);
            }
            long int t1  = ((long int) (time_check_1.time)) * (1000) + (time_check_1.millitm);
            long int t2 = ((long int) (time_check_2.time)) * (1000) + (time_check_2.millitm);
            t1 = t2 - t1;
            float tot_time = (float) t1/1000;
            //Prima di chiudere scrivo sul file di log i dati relativi alla finestra di apertura
            if(pthread_mutex_lock(&file_mutex) != 0) {
                perror("Error mutex_lock time_check\n");
                exit(EXIT_FAILURE);
            }
            fprintf(log_fp, "cassachiusura %d %.3f\n", i, tot_time);
            if(pthread_mutex_unlock(&file_mutex) != 0) {
                perror("Error mutex_unlock cassa\n");
                exit(EXIT_FAILURE);
            }
            (*tot_close_i) ++;
            return NULL;
        } else {
            //Caso cliente vero
            //Tutti i dati per aspettare prelevati dalla struct dei dati del cliente
            int products = data.products;
            pthread_cond_t* cust_cond = data.cust_cond;
            long msec = r + (l * products);
            struct timespec ts;
            ts.tv_sec = msec / 1000;
            ts.tv_nsec = (msec % 1000) * 1000000;
            if(nanosleep(&ts, NULL) == -1) {
                perror("Error nanosleep\n");
                exit(EXIT_FAILURE);
            }
            (*tot_prod) += products;
            (*tot_cust_rel) ++;
            *(data.time_serve) = msec;
            //Dico al cliente che e' stato servito
            *(data.flag_serve) = 1;
            if(pthread_cond_signal(cust_cond) != 0) {
                perror("Error cond_signal cassa\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

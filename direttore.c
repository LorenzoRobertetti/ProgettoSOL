#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/timeb.h>
#include "./util.h"

void* timer_manager(void* arg) {
    //parsing
    timer_manager_arg timanarg = (*(timer_manager_arg*) arg);
    pthread_t* check_th = timanarg.check_th;
    arg_checkout* chargv = timanarg.chargv;
    pthread_cond_t* vec_cond = timanarg.vec_cond;
    int* flag_term = timanarg.flag_term;
    core* supm = timanarg.smdata;
    int* open_checkouts = supm->vec_open;
    int* cust_queue = supm->vec_cnum;
    queue* vec_queue = supm->vec_queue;
    pthread_mutex_t* vec_mutex_queue = supm->vec_mutex_queue;
    unsigned int seedp = time(NULL) + pthread_self();
    //Inizializzo le variabili per il ciclo (alg decisione)
    int count1;
    int count2;
    int to_open;
    int to_close;
    int diff;
    int i;
    int temp;
    //struct cliente per la chiusura
    cust_data dataclose;
    dataclose.cust_id = TERM_ID;

    while(1) {
        if((*flag_term) == 0) {
            cust_data dataterm;
            dataterm.cust_id = TERM_ID;
            for(int i = 0; i < k; i++) {
                if(open_checkouts[i] == 1) {
                    open_checkouts[i] = 0;
                    if(pthread_mutex_lock(vec_mutex_queue + i) != 0) {
                        perror("Error mutex_lock director\n");
                        exit(EXIT_FAILURE);
                    }
                    queue* q = (vec_queue + i);
                    if((vec_queue+i)->len == 0) {
                        if(pthread_cond_signal(vec_cond + i) != 0) {
                            perror("Error cond_signal directore\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    (*q) = pushf(*q, dataterm);
                    if(pthread_mutex_unlock(vec_mutex_queue + i) != 0) {
                        perror("Error mutex_unlock director\n");
                        exit(EXIT_FAILURE);
                    }
                    if(pthread_join(check_th[i], NULL) != 0) {
                        perror("Error mutex_unlock director\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            return NULL;
        }
        count1 = 0;
        count2 = 0;
        to_open = 0;
        to_close = 0;
        diff = 0;
        i = 0;
        //Aspetto un intervallo
        long msec = z;
        struct timespec ts;
        ts.tv_sec = msec / 1000;
        ts.tv_nsec = (msec % 1000) * 1000000;
        if(nanosleep(&ts, NULL) == -1) {
            perror("Error nanosleep\n");
            exit(EXIT_FAILURE);
        }
        if(pthread_mutex_lock(&mutex_open) != 0) {
            perror("Error mutex_lock director\n");
            exit(EXIT_FAILURE);
        }
        if(pthread_mutex_lock(&mutex_num) != 0) {
            perror("Error mutex_lock director\n");
            exit(EXIT_FAILURE);
        }
        //Algoritmo di decisione
        for(int j = 0; j < k; j++) {
            if(open_checkouts[j] == 1) {
                if(cust_queue[j] <= 1) {
                    count1++;
                }
                if(cust_queue[j] > y) {
                    count2++;
                }
            }
        }
        if(count1 >= x) {
            to_close = count1 - (x - 1);
        }
        to_open = count2;
        diff = to_open - to_close;
        if(diff > 0) {
            temp = rand_r(&seedp) % k;
            //Devo aprire diff casse
            while(i < k && diff > 0) {
                if(open_checkouts[i] == 0) {
                    //Prepara l'i-esimo argomento
                    open_checkouts[i] = 1;
                    chargv[i].checkout_number = i;
                    chargv[i].check_cond = vec_cond + i;
                    chargv[i].smdata = supm;
                    //Fa partire l'i-esima cassa
                    if(pthread_create(&check_th[i], NULL, cassa, &chargv[i]) != 0) {
                        perror("Error pthread_create director\n");
                        exit(EXIT_FAILURE);
                    }
                    diff--;
                }
                i++;
                temp = (temp + 1) % k;
            }
        } else {
            if(diff < 0) {
                diff = (-diff);
                temp = rand_r(&seedp) % k;
                //Devo chiudere diff casse
                while(i < k && diff > 0) {
                    if(open_checkouts[temp] == 1) {
                        if(pthread_mutex_lock(vec_mutex_queue + temp) != 0) {
                            perror("Error mutex_lock director\n");
                            exit(EXIT_FAILURE);
                        }
                        if(pthread_detach(check_th[temp]) != 0) {
                            perror("Error detach\n");
                            exit(EXIT_FAILURE);
                        }
                        open_checkouts[temp] = 0;
                        if((vec_queue + temp) -> head == NULL) {
                            if(pthread_cond_signal(vec_cond + temp) != 0) {
                                perror("Error cond_signal director\n");
                                exit(EXIT_FAILURE);
                            }
                        }
                        (*(vec_queue + temp)) = pushf((*(vec_queue + temp)), dataclose);
                        if(pthread_mutex_unlock(vec_mutex_queue + temp) != 0) {
                            perror("Error mutex_unlock director\n");
                            exit(EXIT_FAILURE);
                        }
                        diff--;
                    }
                    i++;
                    temp = (temp + 1) % k;
                }
            }
        }
        if(pthread_mutex_unlock(&mutex_num) != 0) {
            perror("Error mutex_unlock director\n");
            exit(EXIT_FAILURE);
        }
        if(pthread_mutex_unlock(&mutex_open) != 0) {
            perror("Error mutex_unlock director\n");
            exit(EXIT_FAILURE);
        }
    }
}


void* director_fun(void* arg) {
    //Parsing struct
    director_arg dirarg = (*(director_arg*) arg);
    pthread_t* check_th = dirarg.check_th;
    arg_checkout* chargv = dirarg.chargv;
    pthread_cond_t* vec_cond = dirarg.vec_cond;
    pthread_t* gest_cust = dirarg.gest_cust;
    int* cust_zerop = dirarg.cust_zerop;
    core* supm = dirarg.smdata;
    pthread_cond_t* director_cond = dirarg.director_cond;
    pthread_mutex_t* director_mutex = supm->director_mutex;
    queue* director_queue = supm->director_queue;
    //Struct per i dati del cliente
    cust_data data;
    //Preparo gli argomenti per il timer manage
    int flag_term = 1;
    timer_manager_arg timanarg;
    timanarg.check_th = check_th;
    timanarg.chargv = chargv;
    timanarg.vec_cond = vec_cond;
    timanarg.flag_term = &flag_term;
    timanarg.smdata = supm;

    //Creo il thread;
    pthread_t timermanager;
    //Faccio partire il thread;
    if(pthread_create(&timermanager, NULL, timer_manager, &timanarg) != 0) {
        perror("Error pthread_create cassa\n");
        exit(EXIT_FAILURE);
    }
    //Gestisto la coda dei clienti con 0 prodotti
    while(1) {
        if(pthread_mutex_lock(director_mutex) != 0) {
            perror("Error mutex_lock director\n");
            exit(EXIT_FAILURE);
        }
        while(director_queue->head == NULL) {
            if(pthread_cond_wait(director_cond, director_mutex) != 0) {
                perror("Error cond_wait director\n");
                exit(EXIT_FAILURE);
            }
        }

        (*director_queue) = pop((*director_queue), &data);
        if(pthread_mutex_unlock(director_mutex) != 0) {
            perror("Error mutex_unlock director\n");
            exit(EXIT_FAILURE);
        }
        if(data.cust_id == (TERM_ID)) {
            if(pthread_mutex_lock(director_mutex) != 0) {
                perror("Error mutex_lock director\n");
                exit(EXIT_FAILURE);
            }
            (*director_queue) = del_queue((*director_queue));
            if(pthread_mutex_unlock(director_mutex) != 0) {
                perror("Error mutex_unlock director\n");
                exit(EXIT_FAILURE);
            }
            break;
        } else {
            pthread_cond_t* cust_cond = data.cust_cond;
            *(data.flag_serve) = 1;
            (*cust_zerop) ++;
            if(pthread_cond_signal(cust_cond) != 0) {
                perror("Error cond_signal director\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    // Aspetto gestore
    if(pthread_join((*gest_cust), NULL) != 0) {
        perror("Error mutex_unlock director\n");
        exit(EXIT_FAILURE);
    }

    flag_term = 0;
    //Aspetto il timer_manager
    if(pthread_join(timermanager, NULL) != 0) {
        perror("Error mutex_unlock director\n");
        exit(EXIT_FAILURE);
    }
    return NULL;
}

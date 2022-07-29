#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/timeb.h>
#include "./util.h"

void* cliente(void* arg) {
    //Imposto il cliente in modalita' detach
    if(pthread_detach(pthread_self()) != 0) {
        perror("Error detach\n");
        exit(EXIT_FAILURE);
    }
    //Parsing argomenti cliente
    arg_cust argcust = (*((arg_cust*) arg));
    int flag_serve = 0;
    long time_serve = 0;
    int id = argcust.id;
    int* vec_open = argcust.smdata->vec_open;
    pthread_mutex_t* vec_mutex_queue = argcust.smdata->vec_mutex_queue;
    FILE* log_fp = argcust.smdata->log_fp;
    //Inizializzo i dati da mandare al cassiere
    cust_data data;
    pthread_cond_t cond;
    if(pthread_cond_init(&cond, NULL) != 0) {
        perror("Error cond_init cliente\n");
        exit(EXIT_FAILURE);
    }
    data.cust_id = id;
    data.flag_serve = &flag_serve;
    data.cust_cond = &cond;
    data.time_serve = &time_serve;
    //Seme per la rand_r
    unsigned int seedp = time(NULL) + id;
    //Struct per misurare il tempo totale nel supermercato
    struct timeb time_sm_2;
    struct timeb time_sm_1;
    //Struct per misurare il tempo in coda
    struct timeb time_queue_2;
    struct timeb time_queue_1;
    //variebili per scegliere la cassa
    int num_open;
    int rand_num;
    int i;
    int count = 0;
    //Calcolo quanto devo aspettare per la fase di acquisti
    long time_to_buy = (rand_r(&seedp) % (t - 10)) + 10;
    struct timespec ts;
    ts.tv_sec = time_to_buy / 1000;
    ts.tv_nsec = (time_to_buy % 1000) * 1000000;
    //Calcolo quanti prodotti acquista
    int products = rand_r(&seedp) % p;
    //Assegno products in data
    data.products = products;
    //Misuro il tempo all'"ingresso" del supermercato
    if(ftime(&time_sm_1) == -1) {
        perror("Error ftime\n");
        exit(EXIT_FAILURE);
    }
    //Controllo se per caso sm_emerg = -1
    if(sm_emerg == 1) {
        return NULL;
    }
    //Aspetto il tempo per gli acquisti
    if(nanosleep(&ts, NULL) == -1) {
        perror("Error nanosleep\n");
        exit(EXIT_FAILURE);
    }
    //Caso products = 0
    if(products == 0) {
        pthread_mutex_t* director_mutex = argcust.smdata->director_mutex;
        pthread_cond_t* director_cond = argcust.smdata->director_cond;
        queue* director_queue = argcust.smdata->director_queue;
        //Prendo la lock sulla mutex del direttore;
        if(pthread_mutex_lock(director_mutex) != 0) {
            perror("Error mutex_lock cliente\n");
            exit(EXIT_FAILURE);
        }
        //Misuro il tempo iniziale per la coda
        if(ftime(&time_queue_1) == -1) {
            perror("Error ftime\n");
            exit(EXIT_FAILURE);
        }
        //Mando il segnale nel caso in cui la coda fosse vuota
        if(director_queue->head == NULL) {
            if(pthread_cond_signal(director_cond) != 0) {
                perror("Error cond_signal cliente\n");
                exit(EXIT_FAILURE);
            }
        }
        (*director_queue) = pushb(*director_queue, data);
        //Aspetto il segnale da parte del direttore
        if(pthread_cond_wait(&cond, director_mutex) != 0) {
            perror("Error cond_wait cliente\n");
            exit(EXIT_FAILURE);
        }
        //Misuro il tempo finale per la coda
        if(ftime(&time_queue_2) == -1) {
            perror("Error ftime\n");
            exit(EXIT_FAILURE);
        }
        if(pthread_mutex_unlock(director_mutex) != 0) {
            perror("Error mutex_unlock cliente\n");
            exit(EXIT_FAILURE);
        }
        //Prendo tutti i dati necessari per calcolare il tempo
        long int t1  = ((long int) (time_queue_1.time)) * (1000) + (time_queue_1.millitm);
        long int t2 = ((long int) (time_queue_2.time)) * (1000) + (time_queue_2.millitm);
        t1 = t2 - t1;
        float tot_time_queue = (float) t1/1000;
        //USCITA DI EMERGENZA
        if(flag_serve == 0) {
            return NULL;
        }
        //Aggiorno il numero di clienti prendendo la relativa mutex
        if(pthread_mutex_lock(&mutex_tot_cust) != 0) {
            perror("Error mutex_lock cliente\n");
            exit(EXIT_FAILURE);
        }
        tot_cust--;
        //Se i clienti sono esattamente c-e avviso il gestore
        if(tot_cust == (c-e)) {
            if(pthread_cond_signal(&cond_tot_cust) != 0) {
                perror("Error cond_signal cliente\n");
                exit(EXIT_FAILURE);
            }
        }
        //Se i clienti sono esattamente 0 avviso il gestore
        if(tot_cust == 0) {
            if(pthread_cond_signal(&cond_tot_cust) != 0) {
                perror("Error cond_signal cliente\n");
                exit(EXIT_FAILURE);
            }
        }
        if(pthread_mutex_unlock(&mutex_tot_cust) != 0) {
            perror("Error mutex_unlock cliente\n");
            exit(EXIT_FAILURE);
        }
        //Tempo totale nel supermercato
        if(ftime(&time_sm_2) == -1) {
            perror("Error ftime\n");
            exit(EXIT_FAILURE);
        }
        long int t3  = ((long int) (time_sm_1.time)) * (1000) + (time_sm_1.millitm);
        long int t4 = ((long int) (time_sm_2.time)) * (1000) + (time_sm_2.millitm);
        t3 = t4 - t3;
        float tot_time_sm = (float) t3/1000;
        //Scrivo sul file di log i risultati
        if(pthread_mutex_lock(&file_mutex) != 0) {
            perror("Error mutex_lock cliente\n");
            exit(EXIT_FAILURE);
        }

        fprintf(log_fp, "clientepuro %d -1 %.3f %d %.3f 0\n", id, tot_time_queue, products, tot_time_sm);
        if(pthread_mutex_unlock(&file_mutex) != 0) {
            perror("Error mutex_unlock cliente\n");
            exit(EXIT_FAILURE);
        }
        return NULL;
    }
    //Prendo il tempo iniziale per l'attesa in coda
    if(ftime(&time_queue_1) == -1) {
        perror("Error ftime\n");
        exit(EXIT_FAILURE);
    }
    while(1) {
        if(sm_emerg == 1) {
            return NULL;
        }
        num_open = 0;
        count++;
        //Scelgo la cassa
        if(pthread_mutex_lock(&mutex_open) != 0) {
            perror("Error mutex_lock cliente\n");
            exit(EXIT_FAILURE);
        }

        for(i = 0; i < k; i++) {
            if(vec_open[i] == 1) {
                num_open++;
            }
        }

        rand_num = (rand_r(&seedp) % num_open) + 1;
        for(i = 0; i < k; i++) {
            if(vec_open[i] == 1) {
                rand_num--;
                if(rand_num == 0) break;
            }
        }
        //Prendo i valori relativi a quella cassa
        pthread_cond_t* check_cond = (argcust.smdata->vec_cond) + i;
        queue* q = (argcust.smdata->vec_queue) + i;
        pthread_mutex_t* mutex_check = vec_mutex_queue + i;
        //Mi metto in coda nella cassa i;
        if(pthread_mutex_lock(mutex_check) != 0) {
            perror("Error mutex_lock cliente\n");
            exit(EXIT_FAILURE);
        }
        if(q->head == NULL) {
            if(pthread_cond_signal(check_cond) != 0) {
                perror("Error cond_signal cliente\n");
                exit(EXIT_FAILURE);
            }
        }
        (*q) = pushb(*q, data);
        if(pthread_mutex_unlock(&mutex_open) != 0) {
            perror("Error mutex_unlock cliente\n");
            exit(EXIT_FAILURE);
        }
        if(pthread_cond_wait(&cond, mutex_check) != 0) {
            perror("Error cond_wait cliente\n");
            exit(EXIT_FAILURE);
        }
        if(ftime(&time_queue_2) == -1) {
            perror("Error ftime\n");
            exit(EXIT_FAILURE);
        }
        if(pthread_mutex_unlock(mutex_check) != 0) {
            perror("Error mutex_unlock cliente\n");
            exit(EXIT_FAILURE);
        }
        if(flag_serve == 1) {
            long int t1  = ((long int) (time_queue_1.time)) * (1000) + (time_queue_1.millitm);
            long int t2 = ((long int) (time_queue_2.time)) * (1000) + (time_queue_2.millitm);
            t1 = t2 - t1;
            float tot_time_queue = (float) t1/1000;
            //Modifico il numero totale dei clienti nel sm
            if(pthread_mutex_lock(&mutex_tot_cust) != 0) {
                perror("Error mutex_lock cliente\n");
                exit(EXIT_FAILURE);
            }
            tot_cust--;
            if(tot_cust == (c-e)) {
                if(pthread_cond_signal(&cond_tot_cust) != 0) {
                    perror("Error cond_signal cliente\n");
                    exit(EXIT_FAILURE);
                }
            }
            if(tot_cust == 0) {
                if(pthread_cond_signal(&cond_tot_cust) != 0) {
                    perror("Error cond_signal cliente\n");
                    exit(EXIT_FAILURE);
                }

            }
            if(pthread_mutex_unlock(&mutex_tot_cust) != 0) {
                perror("Error mutex_unlock cliente\n");
                exit(EXIT_FAILURE);
            }
            //Tempo totale nel supermercato
            if(ftime(&time_sm_2) == -1) {
                perror("Error ftime\n");
                exit(EXIT_FAILURE);
            }

            long int t3  = ((long int) (time_sm_1.time)) * (1000) + (time_sm_1.millitm);
            long int t4 = ((long int) (time_sm_2.time)) * (1000) + (time_sm_2.millitm);
            t3 = t4 - t3;
            float tot_time_sm = (float) t3/1000;
            float time_serve = ((float) *(data.time_serve))/1000;

            //scrivo sul file di log i risultati
            if(pthread_mutex_lock(&file_mutex) != 0) {
                perror("Error mutex_lock cliente\n");
                exit(EXIT_FAILURE);
            }
            fprintf(log_fp, "clientepuro %d %d %.3f %d %.3f %d\n", id, i, tot_time_queue, products, tot_time_sm, count);
            fprintf(log_fp, "clientecassa %d %d %.3f\n", id, i, time_serve);
            if(pthread_mutex_unlock(&file_mutex) != 0) {
                perror("Error mutex_unlock cliente\n");
                exit(EXIT_FAILURE);
            }
            return NULL;
        }
    }
}

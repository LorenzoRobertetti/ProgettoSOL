#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "./util.h"

void* signal_manager(void* arg){
    //Modalita' pthread_detach
    if(pthread_detach(pthread_self()) != 0) {
        perror("Error detach\n");
        exit(EXIT_FAILURE);
    }
    //Parsing arg
    sigset_t* set = (sigset_t *) arg;
    //Devo distinguere il segnale
    int sig;
    //Mi metto in attesa
    if(sigwait(set, &sig) != 0) {
        perror("Error sigwait\n");
        exit(EXIT_FAILURE);
    }
    switch(sig) {
        case SIGHUP:
            sm_open = 0;
            return NULL;
        case SIGQUIT:
            sm_emerg = 1;
            return NULL;
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        printf("Use: %s pathconfig\n", argv[0]);
        exit(0);
    }

    //Creo la stringa per il path del file log;
    char log_path[MAX_PATH_LEN];
    parsing(argv[1], log_path);

    //Setto la maschera per i segnali SIGHUP e SIGQUIT
    sigset_t set;
    sigset_t oldset;
    //Aggiungo alla maschera SIGHUP e SIGQUIT
    if(sigemptyset(&set) != 0) {
        perror("Error sigset\n");
        exit(EXIT_FAILURE);
    }
    if(sigaddset(&set, SIGHUP) != 0) {
        perror("Error sigset\n");
        exit(EXIT_FAILURE);
    }
    if(sigaddset(&set,SIGQUIT) != 0) {
        perror("Error sigset\n");
        exit(EXIT_FAILURE);
    }
    //Blocco i 2 segnali
    if(pthread_sigmask(SIG_SETMASK, &set, &oldset) != 0) {
        perror("Error sigset\n");
        exit(EXIT_FAILURE);
    }
    //Creo il thread per la gestione dei segnali
    pthread_t handler;
    //Faccio partire il thread
    if(pthread_create(&handler, NULL, signal_manager, &set) != 0) {
        perror("Error pthread_create cassa\n");
        exit(EXIT_FAILURE);
    }


    if(pthread_mutex_init (&mutex_open, NULL) != 0) {
        perror("Error mutex_init\n");
        exit(EXIT_FAILURE);
    } //mutex per vedere le casse aperte
    if(pthread_mutex_init (&mutex_num, NULL) != 0) {
        perror("Error mutex_init\n");
        exit(EXIT_FAILURE);
    }//mutex per aggiornare il numero di elementi in coda
    if(pthread_mutex_init (&mutex_queue, NULL) != 0) {
        perror("Error mutex_init\n");
        exit(EXIT_FAILURE);
    } //mutex per le code
    if(pthread_mutex_init (&mutex_tot_cust, NULL) != 0) {
        perror("Error mutex_init\n");
        exit(EXIT_FAILURE);
    }// mutex per il numero totale di clienti
    if(pthread_mutex_init (&director_mutex, NULL) != 0) {
        perror("Error mutex_init\n");
        exit(EXIT_FAILURE);
    }//mutex per il direttore;
    if(pthread_mutex_init (&file_mutex, NULL) != 0) {
        perror("Error mutex_init\n");
        exit(EXIT_FAILURE);
    } //mutex per scrivere sul file di log


    //////////////////////////mutex concorrente///////////////////////
    pthread_mutex_t* vec_mutex_queue;
    if((vec_mutex_queue = malloc(sizeof(pthread_mutex_t) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //inizializzo le variabili di mutex queue
    for(int i = 0; i < k; i++) {
        if(pthread_mutex_init((vec_mutex_queue + i), NULL) != 0) {
            perror("Error mutex_init\n");
            exit(EXIT_FAILURE);
        }
    }
    //inizializzo numero totale di CLIENTI
    tot_cust = 0;
    //"Apro" il supermercato
    sm_open = 1;
    sm_emerg = 0;
    //Inizializzo vettore casse aperte/chiuse
    int* open_checkouts;

    if((open_checkouts = malloc(sizeof(int) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < k; i++) {
        open_checkouts[i] = 0;
    }
    //inizializzo il vettore del numero dei clienti in coda ad ogni cassa
    int* cust_queue;
    if((cust_queue = malloc(sizeof(int) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < k; i++) {
        cust_queue[i] = 0;
    }
    //Inizializzo il vettore delle code
    queue* vec_queue;
    if((vec_queue = malloc(sizeof(queue) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < k; i++) {
        vec_queue[i] = init_queue(vec_queue[i]);
    }
    int* cust_num_check;
    //Creo il vettore dei clienti serviti per le casse
    if((cust_num_check = malloc(sizeof(int) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //Lo inizializzo a zero
    for(int i = 0; i < k; i++) {
        cust_num_check[i] = 0;
    }
    int* tot_prod_check;
    //Creo il vettore dei prodotti per le casse
    if((tot_prod_check = malloc(sizeof(int) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //Lo inizializzo a zero
    for(int i = 0; i < k; i++) {
        tot_prod_check[i] = 0;
    }
    //Creo il vettore per contare le chiusure delle casse
    int* tot_close;
    if((tot_close = malloc(sizeof(int) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //Lo inizializzo a zero
    for(int i = 0; i < k; i++) {
        tot_close[i] = 0;
    }
    //Apro il file di log_path
    FILE* log_fp;
    if((log_fp = fopen(log_path, "wt")) == NULL) {
        perror("Errore apertura file");
        exit(EXIT_FAILURE);
    }
    //Creo la coda per il direttore;
    queue director_queue = init_queue(director_queue);
    pthread_cond_t director_cond;
    if(pthread_cond_init(&director_cond, NULL) != 0) {
        perror("Error cond_init sm\n");
        exit(EXIT_FAILURE);
    }
    //Assegno i valori alla struct di gestione
    core supm;
    supm.vec_open = open_checkouts;
    supm.vec_cnum = cust_queue;
    supm.vec_queue = vec_queue;
    supm.log_fp = log_fp;
    supm.tot_close = tot_close;
    supm.log_fp = log_fp;
    supm.tot_prod_check = tot_prod_check;
    supm.cust_num_check = cust_num_check;
    supm.vec_mutex_queue = vec_mutex_queue;
    supm.director_queue = &director_queue;
    supm.director_cond = &director_cond;
    supm.director_mutex = &director_mutex;
    //*********************************************
    //******************CASSIERI*******************
    //*********************************************
    //Creo il vettore dei thread cassieri
    pthread_t* check_th;
    if((check_th = malloc(sizeof(pthread_t) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //Creo il vettore degli argomenti dei thread cassieri
    arg_checkout* chargv;
    if((chargv = malloc(sizeof(arg_checkout) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //Creo il vettore delle variabili di condizione dei cassieri
    pthread_cond_t* vec_cond;
    if((vec_cond = malloc(sizeof(pthread_cond_t) * k)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //Inizializzo le variabili di condizione dei CASSIERI
    for(int i = 0; i < k; i++) {
        if(pthread_cond_init((vec_cond + i), NULL) != 0) {
            perror("Error cond_init sm\n");
            exit(EXIT_FAILURE);
        }
    }
    //Assegno a supm il vettore delle variabili di condizione
    supm.vec_cond = vec_cond;
    //Faccio partire le prime due casse
    //Preparo gli argomenti per i thread cassieri iniziali
    for(int i = 0; i < m; i++) {
        open_checkouts[i] = 1;
        chargv[i].checkout_number = i;
        chargv[i].check_cond = vec_cond + i;
        chargv[i].smdata = &supm;
    }
    //Faccio partire i thread cassieri iniziali
    for(int i = 0; i < 2; i++) {
        if(pthread_create(&check_th[i], NULL, cassa, &chargv[i]) != 0) {
            perror("Error pthread_create sm\n");
            exit(EXIT_FAILURE);
        }
    }
    //*****************DIRETTORE********************
    pthread_t director;
    int cust_zerop = 0;
    //Preparo l'argomento per il DIRETTORE
    director_arg dirarg;
    dirarg.director_cond = &director_cond;
    dirarg.check_th = check_th;
    dirarg.chargv = chargv;
    dirarg.cust_zerop = &cust_zerop;
    dirarg.vec_cond = vec_cond;
    dirarg.smdata = &supm;

    //****************GESTORE***********************
    //Creo i primi c clienti
    pthread_t* cust_th;
    if((cust_th = malloc(sizeof(pthread_t) * c)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //Creo il vettore degli argomenti dei c thread CLIENTI
    arg_cust* custargv;
    if((custargv = malloc(sizeof(arg_cust) * c)) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    //Preparo argomento per il thread GESTORE
    manage_arg argmanage;
    argmanage.cust_th = cust_th;
    argmanage.custargv = custargv;
    argmanage.smdata = &supm;
    //Creo il thread di gestione
    pthread_t gest_cust;
    //Faccio partire il DIRETTORE
    dirarg.gest_cust = &gest_cust;
    if(pthread_create(&director, NULL, director_fun, &dirarg) != 0) {
        perror("Error pthread_create sm\n");
        exit(EXIT_FAILURE);
    }
    //Faccio partire il thread gestore
    if(pthread_create(&gest_cust, NULL, manage_cust, &argmanage) != 0) {
        perror("Error pthread_create sm\n");
        exit(EXIT_FAILURE);
    }
    //Aspetto direttore
    if(pthread_join(director, NULL) != 0) {
        perror("Error mutex_unlock sm\n");
        exit(EXIT_FAILURE);
    }
    //Stampo i risultati generali
    int def_tot_cust = 0;
    int def_tot_prod = 0;
    for(int i = 0; i < k; i++) {
        def_tot_cust += cust_num_check[i];
    }
    def_tot_cust += cust_zerop;
    for(int i = 0; i < k; i++) {
        def_tot_prod += tot_prod_check[i];
    }
    if(pthread_mutex_lock(&file_mutex) != 0) {
        perror("Error mutex_lock sm\n");
        exit(EXIT_FAILURE);
    }
    fprintf(log_fp, "numerototaleclientiserviti %d\n", def_tot_cust);
    fprintf(log_fp, "numerototaleprodottiacquistati %d\n", def_tot_prod);
    for(int i = 0; i < k; i++) {
        fprintf(log_fp, "cassafinale %d %d %d\n", i, tot_close[i], cust_num_check[i]);
    }
    if(pthread_mutex_unlock(&file_mutex) != 0) {
        perror("Error mutex_unlock sm\n");
        exit(EXIT_FAILURE);
    }
    if(sm_emerg == 1) {
        sleep(SEC_SLEEP);
    }
    //Chiudo il file
    fclose(log_fp);
    //Libero la memoria
    free(tot_close);
    free(vec_mutex_queue);
    free(cust_th);
    free(custargv);
    free(open_checkouts);
    free(cust_queue);
    free(vec_queue);
    free(check_th);
    free(chargv);
    free(vec_cond);
    free(cust_num_check);
    free(tot_prod_check);

    return 0;
}

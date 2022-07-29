#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include "./util.h"

queue init_queue(queue q) {
    q.head = NULL;
    q.tail = NULL;
    q.len = 0;
    return q;
}

queue del_queue(queue q) {
    while(q.head != NULL) {
        cust_node* temp = q.head;
        if(pthread_cond_signal(q.head->cust_cond) != 0) {
            perror("Error mutex_unlock queue\n");
            exit(EXIT_FAILURE);
        }
        q.head = q.head->next;
        q.len--;
        free(temp);
    }
    q.tail = NULL;
    return q;
}

queue pushb(queue q, cust_data cust) {
    cust_node* toadd;
    if((toadd = malloc(sizeof(cust_node))) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    toadd->cust_id = cust.cust_id;
    toadd->flag_serve = cust.flag_serve;
    toadd->time_serve = cust.time_serve;
    toadd->products = cust.products;
    toadd->cust_cond = cust.cust_cond;
    toadd->next = NULL;

    if(q.head == NULL) {
        q.head = toadd;
    } else {
        if(q.tail == NULL) {
            q.head->next = toadd;
            q.tail = toadd;
        } else {
            q.tail->next = toadd;
            q.tail = toadd;
        }
    }
    q.len++;
    return q;
}

queue pushf(queue q, cust_data cust) {
    cust_node* toadd;
    if((toadd = malloc(sizeof(cust_node))) == NULL) {
        perror("Errore malloc\n");
        exit(EXIT_FAILURE);
    }
    toadd->cust_id = cust.cust_id;
    toadd->flag_serve = cust.flag_serve;
    toadd->time_serve = cust.time_serve;
    toadd->products = cust.products;
    toadd->cust_cond = cust.cust_cond;
    toadd->next = q.head;
    q.head = toadd;
    if(q.tail == NULL && q.head != NULL && q.head->next != NULL) {
        q.tail = q.head->next;
    }
    q.len++;
    return q;
}

queue pop(queue q, cust_data* data) {
    if(q.head == NULL) {
        data->cust_id = -1;
        return q;
    } else {
        data->cust_id = q.head->cust_id;
        data->flag_serve = q.head->flag_serve;
        data->time_serve = q.head->time_serve;
        data->products = q.head->products;
        data->cust_cond = q.head->cust_cond;
        cust_node* temp = q.head;
        if(q.head->next == q.tail) {
            q.tail = NULL;
        }
        q.head = q.head->next;
        free(temp);
    }
    q.len--;
    return q;
}

void stampaq(queue q) {
    int i = 0;
    printf("La coda ha %d elementi\n", q.len);
    while(q.head != NULL) {
        i++;
        printf("Elem %d: %d\n", i, q.head->cust_id);
        q.head = q.head->next;
    }
}

pthread_mutex_t mutex_open;
pthread_mutex_t mutex_num;
pthread_mutex_t mutex_queue;
pthread_mutex_t mutex_tot_cust;
pthread_mutex_t director_mutex;
pthread_mutex_t file_mutex;
#define MAX_ARGS 256
#define MAX_FILE_LEN 4096
#define MAX_PATH_LEN 256
#define TERM_ID -7
#define TIME_TIMER_CHECK 10
#define TIME_TIMER_MANAGER 10
#define SEC_SLEEP 3

int tot_cust;
int sm_open;
int sm_emerg;

pthread_cond_t cond_tot_cust;

int k, c, e, t, p , s, x, y, m, l, z;


typedef struct cust_data {
    int cust_id;
    int* flag_serve;
    long* time_serve;
    int products;
    pthread_cond_t* cust_cond;
} cust_data;

typedef struct cust_node {
    int cust_id;
    int* flag_serve;
    long* time_serve;
    int products;
    pthread_cond_t* cust_cond;
    struct cust_node* next;
} cust_node;

typedef struct queue {
    cust_node* head;
    cust_node* tail;
    int len;
} queue;

typedef struct core {
    int* vec_open;
    int* vec_cnum;
    int* cust_num_check;
    int* tot_prod_check;
    int* tot_close;
    FILE* log_fp;
    pthread_cond_t* vec_cond;
    pthread_mutex_t* vec_mutex_queue;
    queue* vec_queue;
    queue* director_queue;
    pthread_cond_t* director_cond;
    pthread_mutex_t* director_mutex;
} core;


typedef struct arg_checkout {
    int checkout_number;
    pthread_cond_t* check_cond;
    core* smdata;
} arg_checkout;

typedef struct arg_cust {
    int id;
    core* smdata;
} arg_cust;

typedef struct manage_arg {
    pthread_t* cust_th;
    arg_cust* custargv;
    core* smdata;
} manage_arg;

typedef struct director_arg {
    pthread_t* check_th;
    pthread_cond_t* director_cond;
    arg_checkout* chargv;
    int* cust_zerop;
    pthread_cond_t* vec_cond;
    pthread_t* gest_cust;
    core* smdata;
} director_arg;

typedef struct timer_check_arg {
    int i;
    int* isopen;
    int* num_cust;
    pthread_cond_t* director_cond;
    pthread_mutex_t* mutex_check;
    queue* q;
} timer_check_arg;

typedef struct timer_manager_arg {
    pthread_t* check_th;
    arg_checkout* chargv;
    pthread_cond_t* vec_cond;
    int* flag_term;
    core* smdata;
} timer_manager_arg;

void* director_fun(void* arg);
void* manage_cust(void* arg);
void parsing(char*, char*);
queue init_queue(queue q);
queue del_queue(queue q);
queue pushb(queue q, cust_data cust);
queue pop(queue q, cust_data* data);
queue pushf(queue q, cust_data cust);
void stampaq(queue q);
void* cliente(void* arg);
void* cassa(void* arg);
void usage(char *pname);

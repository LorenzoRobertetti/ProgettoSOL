#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "./util.h"

void usage(char *pname) {
    fprintf(stderr, "\nScrivi: %s -k <value> -c <value> -e <value> -t <value> -p <value> -s <value> -f <logpath> -x <value> -y <value> -m <value> -l <value> -z <value>\n\n", pname);
    exit(EXIT_FAILURE);
}

void parsing(char* pname, char* log_path) {
    //Classico parsing degli argomenti tramite getopt
    FILE* f;
    if((f = fopen(pname, "r")) == NULL) {
        perror("Error fopen\n");
        exit(EXIT_FAILURE);
    }
    char fargs[MAX_FILE_LEN] = "";
    fread(fargs, sizeof(char), MAX_FILE_LEN, f); 
    fclose(f);

    char* fargv[MAX_ARGS];
    fargv[0] = pname;
    int fargc = 1;

    k = -1, c = -1, e = -1, t = -1, p = -1, s = -1;

    fargv[fargc] = strtok(fargs, " \n");
    while(fargc < MAX_ARGS && fargv[fargc] != NULL) {
        fargc++;
        fargv[fargc] = strtok(NULL, " \n");
    }
    extern char* optarg;
    int opt;
    //Cerco nel file
    while((opt = getopt(fargc, fargv, "k:c:e:t:p:s:f:x:y:m:l:z:")) != -1) {
    	switch(opt) {
    	case 'k':
    	    k=atoi(optarg);
    	    break;
    	case 'c':
    	    c=atoi(optarg);
    	    break;
    	case 'e':
    	    e=atoi(optarg);
    	    break;
        case 't':
    	    t=atoi(optarg);
    	    break;
        case 'p':
    	    p=atoi(optarg);
    	    break;
        case 's':
    	    s=atoi(optarg);
    	    break;
        case 'x':
    	    x=atoi(optarg);
    	    break;
        case 'y':
    	    y=atoi(optarg);
    	    break;
        case 'm':
    	    m=atoi(optarg);
    	    break;
        case 'l':
    	    l=atoi(optarg);
    	    break;
        case 'z':
    	    z=atoi(optarg);
    	    break;
        case 'f':
            strcpy(log_path, optarg);
            break;
    	default:
    	    usage(pname);
    	    break;
    	}
    }
    //Casi particolari
    if (k <= 0 || c <= 0 || e <= 0 || t <= 0 || p <= 0 || s <= 0 || x <= 1 || y <= 0 || m <= 0 || l <= 0 || z <= 0 ) usage(pname);
}

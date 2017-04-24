#include "utils.h"



char *time_stamp(){
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return asctime(timeinfo);
}

char *create_capital(char *msg){
    char *res = malloc((strlen(msg)) * sizeof(char));
    for(int i = 0; i < strlen(msg); i++){
        res[i] = msg[i] - 32;
    }
    return res;
}

char* get_echo_message(char* msg, int pref_len){
    pref_len++;
    char *res = malloc((strlen(msg)-pref_len) * sizeof(char));
    for(int i = pref_len; i < strlen(msg); i++){
        res[i-pref_len] = msg[i];
    }
    return res;
}

key_t generate_key(){
    char *pathname = getenv("HOME");
    char global_key = 'a';
    return ftok(pathname, global_key);
}


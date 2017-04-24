
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <signal.h>
#include <stdlib.h>
#include "utils.h"
#include <time.h>
#include <unistd.h>
#define MAX_REQUESTS 65536

static mqd_t server_mq;
static mqd_t client_mq;
char *my_queue_name;
int runtime = 1;

void initialize_attr(struct mq_attr *attr){
    attr->mq_flags = 0;
    attr->mq_maxmsg = 10;
    attr->mq_msgsize = MSG_MAX_SIZE;
    attr->mq_curmsgs = 0;
}



char *generate_raw_name(){
    char *name = malloc(10);
    name[0] = '/';
    for(int i = 1 ; i < 10; i++){
        name[i] = (char)rand()%25 + 'a';
    }
    return name;
}

char *generate_hello_req(char *name){

    char *res = malloc(MSG_MAX_SIZE);
    strcpy(res,"HELLO ");
    strcat(res, name);
    return res;
}


void sig_handler(int signum) {
    if(signum == SIGINT){
        printf("Served sigint\n");
        mq_close(server_mq);
        mq_close(client_mq);
        mq_unlink(my_queue_name);
        exit(0);
    }
}

void handle_client_message(char *msg){
    if(strcmp(msg, "SHUTDOWN") == 0){
        mq_close(server_mq);
        mq_close(client_mq);
        mq_unlink(my_queue_name);
        runtime = 0;
    }else{
         printf("%s\n",msg);
    }
}


Req parse_request(char *request, char *name){
    Req req;
    if(!strncmp(request,"ECHO",4)){
        req.type = ECHO;
        req.message = get_echo_message(request, 4);
    } else if(!strncmp(request,"CAPITALIZE",10)){
        req.type = CAPITALIZE;
        req.message = get_echo_message(request, 10);
    } else if(!strncmp(request,"TIMESTAMP",9)){
        req.type = TIMESTAMP;
        req.message = "NULL";
    } else if(!strncmp(request,"SHUTDOWN",8)){
        req.type = SHUTDOWN;
        req.message = "NULL";
    } else if(!strncmp(request,"HELLO",5)){
        req.type = WELCOME;
        req.message = get_echo_message(request, 5);
    } else {
        req.type = UNKNOWN;
        req.message = "NULL";
    }
    req.desc = client_mq;
    req.name = name;

    return req;
}





int main(int argc, char **argv){
    srand(time(NULL));
    ssize_t bytes_read;

    struct mq_attr attr;
    initialize_attr(&attr);

    char write_buff[MSG_MAX_SIZE+1];
    char read_buff[MSG_MAX_SIZE+1];


    my_queue_name = generate_raw_name();
    printf("My name is : %s\n", my_queue_name);

    /* Otwarcie kolejek */
    server_mq = mq_open(SRV_Q_NAME, O_WRONLY);
    client_mq = mq_open(my_queue_name, O_CREAT | O_RDONLY, 0644, &attr);

    if(server_mq == -1){
        perror("Can't open the server queue");
        exit(EXIT_FAILURE);
    }

    if(client_mq == -1){
        perror("Can't open the client queue");
        exit(EXIT_FAILURE);
    }

    /* Inicjalizacja handshake'a */
    char temp_buff[MSG_MAX_SIZE];
    char *request = generate_hello_req(my_queue_name);
    Req handshake = parse_request(request, my_queue_name);
    sprintf(write_buff, "%d %s %d %s", handshake.desc, handshake.message, handshake.type, handshake.name);
    printf("Write buff: %s\n", write_buff);
    if(mq_send(server_mq, write_buff, MSG_MAX_SIZE, 0) == -1){
        perror("Can't send hello request to server queue");
        exit(EXIT_FAILURE);
    }

    printf("Send to server (enter \"exit\" to stop it):\n");


    ssize_t byte_reads;
    int req_c = 0;

    do {
        signal(SIGINT, sig_handler);
        printf("> ");
        fflush(stdout);
        memset(write_buff, 0, MSG_MAX_SIZE);
        fgets(write_buff, MSG_MAX_SIZE, stdin);
        Req r = parse_request(write_buff, my_queue_name);
        sprintf(write_buff,"%d %s %d %s", r.desc, r.message, r.type, r.name);
        //printf("%s\n", write_buff);
        if(mq_send(server_mq, write_buff, MSG_MAX_SIZE, 0) == -1){
            perror("Can't send message to server queue");
            exit(EXIT_FAILURE);
        }


        bytes_read = mq_receive(client_mq, read_buff, MSG_MAX_SIZE, NULL);
        if(bytes_read < 0){
            perror("Can't read from client queue");
            exit(EXIT_FAILURE);
        }else if(bytes_read != 0){
            read_buff[bytes_read] = '\0';
            handle_client_message(read_buff);
        }


    } while(runtime);


    return 0;
}

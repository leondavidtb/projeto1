#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

enum estados
{
    iniciando,
    comunicando,
    finalizando,
    encerrado
};

static const int BUFSIZE = 512;

int sockDestino, sockOrigem;

struct addrinfo *destinoAddr;
struct addrinfo *origemAddr;

struct sockaddr_storage destinoRecv_addr;
socklen_t destinoAddr_len = sizeof destinoRecv_addr;

struct sockaddr_storage origemRecv_addr;
socklen_t origemAddr_len = sizeof origemRecv_addr;


void comutador(){
    int sendMsg,recvMsg;
    char buffer[BUFSIZE];
    int i, cont = 0;  
    
    puts("Aguardando destino e origem...");
    
    while(cont < 2){
        cont++;
        for ( i = 0; i < 10; i++){
            buffer[i]='\0';
        }
        recvMsg = recvfrom(sockDestino, buffer, BUFSIZE, 0, (struct sockaddr*)&destinoRecv_addr, &destinoAddr_len);
        sendMsg = sendto(sockOrigem, buffer, BUFSIZE, 0, origemAddr->ai_addr, origemAddr->ai_addrlen);
        
        for ( i = 0; i < 10; i++){
            buffer[i]='\0';
        }
        recvMsg = recvfrom(sockOrigem, buffer, BUFSIZE, 0, (struct sockaddr*)&origemRecv_addr, &origemAddr_len);
        sendMsg = sendto(sockDestino, buffer, BUFSIZE, 0, (struct sockaddr*)&destinoRecv_addr, destinoAddr_len);
    }
}


int socketOrigem(const char *ip, const char *porta){

    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(ip, porta, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    {
        {
            perror("getaddrinfo() falhou\n");
            exit(EXIT_FAILURE);
        }
    }

    int sock = -1;
    for (origemAddr = servAddr; origemAddr != NULL; origemAddr = origemAddr->ai_next){
        sock = socket(origemAddr->ai_family, origemAddr->ai_socktype, origemAddr->ai_protocol);
        if (sock < 0)
            continue;
        break;
        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}

int socketDestino(const char *porta){
    
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria)); 
    addrCriteria.ai_family = AF_UNSPEC;             
    addrCriteria.ai_flags = AI_PASSIVE;             
    addrCriteria.ai_socktype = SOCK_DGRAM;         
    addrCriteria.ai_protocol = IPPROTO_UDP;        

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, porta, &addrCriteria, &servAddr);
    if (rtnVal != 0){
        perror("getaddrinfo() falhou\n");
        exit(EXIT_FAILURE);
    }

    int servSock = -1;
    for (destinoAddr = servAddr; destinoAddr != NULL; destinoAddr = destinoAddr->ai_next){
        servSock = socket(destinoAddr->ai_family, destinoAddr->ai_socktype, destinoAddr->ai_protocol);
        if (servSock < 0)
            continue;

        if ((bind(servSock, destinoAddr->ai_addr, destinoAddr->ai_addrlen) == 0)){
            break;
        }

        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}

int main(int argc, char const *argv[]){
    if (argc != 4){
        perror("Execucao correta: ./comutador <porta de origem> <ip> <porta de destino>\n");
        return EXIT_FAILURE;
    }

    int estado_atual = iniciando;
    
    while (estado_atual != encerrado){
        switch (estado_atual){
            case iniciando:
                sockDestino = socketDestino(argv[1]);
                sockOrigem = socketOrigem(argv[2],argv[3]);
                estado_atual = comunicando;
                break;
            case comunicando:
                comutador();
                estado_atual = finalizando;
                break;
            case finalizando:
                estado_atual = encerrado;
                close(sockDestino);
                close(sockOrigem);
                break;
            default:
                break;
        }
    }
    puts("Comunicação encerrada.");
    return EXIT_SUCCESS;
} 
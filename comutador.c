#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

enum estados{
    iniciando,
    comunicando,
    finalizando,
    encerrado
};

#define host localhost;
#define service_origem 5001;
#define service_destino 5002;
 
int meuListen() {
    fputs("Iniciando escuta...", stdout);
    return 1;
}

int meuSocketCliente() {
    
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DEGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(host, service_destino, &addrCriteria, &servAddr);
    if (rtnVal != 0){
        {
            perror("getaddrinfo() falhou\n");
            exit(EXIT_FAILURE);
        }
    }

    int sock = -1;
    for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next){
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0)
            continue;

        if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
            break;

        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}

int meuSocketServidor() {
    struct addrinfo addrCriteria;                   
    memset(&addrCriteria, 0, sizeof(addrCriteria)); 
    addrCriteria.ai_family = AF_UNSPEC;             
    addrCriteria.ai_flags = AI_PASSIVE;             
    addrCriteria.ai_socktype = SOCK_DGRAM;         
    addrCriteria.ai_protocol = IPPROTO_UDP;         

    struct addrinfo *servAddr; 
    int rtnVal = getaddrinfo(host, service_origem, &addrCriteria, &servAddr);
    if (rtnVal != 0){
        perror("getaddrinfo() falhou\n");
        exit(EXIT_FAILURE);
    }

    int servSock = -1; 
    for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next){

        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (servSock < 0)
            continue;

        if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) && (meuListen())){
            break;
        }

        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}


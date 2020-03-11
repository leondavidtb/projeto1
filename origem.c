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

const char *host;
const char *service;

static const int BUFSIZE = 512;

int meuConnect(int socket, struct addrinfo addr) {
    char buffer[BUFSIZE];
    char *msg = "SYN";
    int msgRecv, len;
    
    puts("Pedindo conexão...");
    
    sendto(socket, (const char *)msg, strlen(msg), 0, 
            (const struct addrinfo *) &addr, sizeof(addr));
    
    msgRecv = recvfrom(socket, buffer, BUFSIZE, 0, 
            (const struct addrinfo *) &addr, &len);
    
    if (msgRecv < 0){
        perror("recv() falhou\n");
        exit(EXIT_FAILURE);
    }

    puts("Conexão estabelecida.");

    return 0;
}

int meuSocket(){

    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
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

        if(meuConnect(sock, addrCriteria) == 0) {
            break;
        }

        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}

int main(int argc, char const *argv[]){
    if (argc < 2 || argc > 3){
        perror("Execução correta: ./origem <ip> [<porta>]\n");
        return EXIT_FAILURE;
    }

    host = argv[1];
    service = (argc == 3) ? argv[2] : "5001";

    int estado_atual = iniciando;
    int sock;

    while (estado_atual != encerrado){
        switch (estado_atual){
        case iniciando:
            sock = meuSocket();
            if (sock < 0){
                perror("meuSocket() falhou.\n");
                return EXIT_FAILURE;
            }
            estado_atual = comunicando;
            break;
        case comunicando:
            estado_atual = finalizando;
            break;
        case finalizando:
            close(sock);
            estado_atual = encerrado;
            break;
        default:
            break;
        }
    }

    return EXIT_SUCCESS;
}
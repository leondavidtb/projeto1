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

const char *service;

static const int BUFSIZE = 512;

struct addrinfo addrCriteria;

int meuListen() {
    fputs("Iniciando escuta...", stdout);
    fputs("\n", stdout);
    
    return 0;
}

int meuAccept(int socket) {
    fputs("Esperando conexão...", stdout);
    fputs("\n", stdout);
    char buffer[BUFSIZE];
    char *msg = "SYNACK";
    int msgRecv, len;

    msgRecv = recvfrom(socket, buffer, BUFSIZE, 0, (const struct addrinfo *) &addrCriteria, &len);

    if(strcmp(buffer, "SYN") == 0) {
        fputs("Conexão pedida.", stdout);
        fputs("\n", stdout);
    }

    sendto(socket, (const char *)msg, strlen(msg), 0, (const struct sockaddr *) &addrCriteria, sizeof(addrCriteria));

    fputs("Conexão aceita.", stdout);
    fputs("\n", stdout);

    return 0;
}

int meuSend(int socket) {

}

int meuRecv() {

}

int meuSocket(){
                   
    memset(&addrCriteria, 0, sizeof(addrCriteria)); 
    addrCriteria.ai_family = AF_UNSPEC;             
    addrCriteria.ai_flags = AI_PASSIVE;             
    addrCriteria.ai_socktype = SOCK_DGRAM;         
    addrCriteria.ai_protocol = IPPROTO_UDP;         

    struct addrinfo *servAddr; 
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if (rtnVal != 0){
        perror("getaddrinfo() falhou\n");
        exit(EXIT_FAILURE);
    }

    int servSock = -1; 
    for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next){

        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (servSock < 0)
            continue;

        if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) && (meuListen() == 0)){
            break;
        }

        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}

int aceitarConexaoUDP(int servSock){
    struct sockaddr_storage clntAddr; 

    socklen_t clntAddrLen = sizeof(clntAddr);

    int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    if (clntSock < 0){
        perror("accept() falhou\n");
        exit(EXIT_FAILURE);
    }

    return clntSock;
}


void trocaMensagem(int clntSocket){
    char buffer[BUFSIZE];
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    ssize_t numBytesSent, numBytesRcvd;

    while (true){
        numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
        if (numBytesRcvd < 0){
            perror("recv() falhou\n");
            exit(EXIT_FAILURE);
        }

        fputs("Client: ", stdout);
        fputs(buffer, stdout);
        fputc('\n', stdout);

        fputs("Server: ", stdout);
        read = getline(&line, &len, stdin);
        numBytesSent = send(clntSocket, line, len, 0);
        
        if (numBytesSent < 0){
            perror("send() falhou\n");
            exit(EXIT_FAILURE);
        }
        if (strcmp(line, "exit\n") == 0){
            free(line);
            break;
        }
    }
}

int main(int argc, char const *argv[]){
    if (argc != 2){
        perror("Execução correta: ./destino <porta>\n");
        return EXIT_FAILURE;
    }

    service = argv[1];

    int estado_atual = iniciando;
    int servSock, clntSock;

    while (estado_atual != encerrado){
        switch (estado_atual){
        case iniciando:
            servSock = meuSocket();
            estado_atual = comunicando;
            break;
        case comunicando:
            clntSock = aceitarConexaoUDP(servSock);
            meuAccept(clntSock);
            estado_atual = finalizando;
            break;
        case finalizando:
            estado_atual = encerrado;
            close(servSock);
            close(clntSock);
            break;
        default:
            break;
        }
    }

    return EXIT_SUCCESS;
}

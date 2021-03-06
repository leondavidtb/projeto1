#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

const char *host;
const char *service;

static const int BUFFER = 512;

struct addrinfo *addr;

struct sockaddr_storage recv_addr;
socklen_t addr_len = sizeof recv_addr;

enum estados
{
    iniciando,
    comunicando,
    finalizando,
    encerrado
};

static const int BUFSIZE = 512;

int meuSocket(){

    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    
    struct addrinfo *servAddr;
    
    int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
    if (rtnVal != 0){

        perror("getaddrinfo() falhou\n");
        exit(EXIT_FAILURE);
    }
    
    int sock = -1;
    for (addr = servAddr; addr != NULL; addr = addr->ai_next){

        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0)
            continue;
        
        if (meuConnect(sock, addr) == 0){
            
            break;
        }
        
        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}

int meuConnect(int sock, struct addrinfo *addr){
    int sendMsg, recvMsg;
    const char *msgSend = "SYN";
    const char *msgTest = "TESTE";
    char buffer[BUFFER];
    
    puts("Pedindo conexão...");
    printf("Enviei %s\n", msgSend);
    sendMsg = sendto(sock, msgSend, sizeof(&msgSend), 0, addr->ai_addr, addr->ai_addrlen);
    if(sendMsg < 0){
        perror("Erro ao enviar mensagem\n");
        return EXIT_FAILURE;
    }

    recvMsg = recvfrom(sock, buffer, BUFFER, 0, (struct sockaddr *)&recv_addr, &addr_len);
    if(recvMsg < 0){
        perror("Erro ao receber mensagem\n");
        return EXIT_FAILURE;
    }

    if (strcmp(buffer, "SYNACK") == 0){
        printf("\nRecebi ");
        fputs(buffer, stdout);
        puts("\nConexão estabelecida.\n");
    }

    printf("Enviei %s\n", msgTest);
    meuSend(sock, msgTest, sizeof(msgTest));
    meuRecv(sock, buffer, sizeof(buffer));
    printf("Recebi ");
    fputs(buffer, stdout);
    puts("\n");
    
    return 0;
}

int meuSend(int sock, char *msg, int msgLen){   
    
    int sendMsg = sendto(sock, msg, msgLen, 0, addr->ai_addr, addr->ai_addrlen);
    
    return sendMsg;
}
int meuRecv(int sock, char *buffer, int BUFSIZE){
    
    int recvMsg = recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *)&recv_addr, &addr_len);
    
    return recvMsg;
}

int main(int argc, char const *argv[]){

    if (argc != 3){
        perror("Execucao correta: ./origem <ip> [<porta>]\n");
        return EXIT_FAILURE;
    }

    host = argv[1];
    service = argv[2];

    int estado_atual = iniciando;
    int sock;

    while (estado_atual != encerrado){
        
        switch (estado_atual){

        case iniciando:
            sock = meuSocket();
            estado_atual = comunicando;
            break;
        case comunicando:
            estado_atual = finalizando;
            break;
        case finalizando:
            estado_atual = encerrado;
            break;
        default:
            break;
        }
    }

    return EXIT_SUCCESS;
}

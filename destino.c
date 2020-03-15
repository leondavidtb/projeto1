#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

const char *service;

static const int BUFFER = 512;

struct addrinfo addrCriteria;

enum estados{
    iniciando,
    comunicando,
    finalizando,
    encerrado
};

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
    
    int sock = -1;
    for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next){
        
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        
        if (sock < 0)
            continue;
        
        if ((bind(sock, addr->ai_addr, addr->ai_addrlen) == 0) && meuListen() == 0){
            
            int sendMsg, recvMsg;
            const char *msgSend = "SYNACK";
            char buffer[BUFFER];

            struct sockaddr_storage recv_addr;
            socklen_t addr_len = sizeof recv_addr;

            puts("Esperando conexão");
            recvMsg = recvfrom(sock, buffer, BUFFER, 0, (struct sockaddr *)&recv_addr, &addr_len);
            
            if(recvMsg < 0){
                perror("Erro ao receber mensagem\n");
                return EXIT_FAILURE;
            }
            
            if (strcmp(buffer, "SYN") == 0){
                sendMsg = sendto(sock, msgSend, sizeof(&msgSend), 0, (struct sockaddr *)&recv_addr, addr_len);
                if(sendMsg < 0){
                    perror("Erro ao enviar mensagem\n");
                    return EXIT_FAILURE;
                }
                puts("Conexão aceita");
                fputs(buffer, stdout);
                puts("\n");

            }

            break;
        }
        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}

int meuListen(){
    puts("Iniciando escuta");
    return 0;
}

int meuAccept(int sock, struct addrinfo *addr){
    int sendMsg, recvMsg;
    int len;
    char *msgSend = "SYNACK";
    char buffer[BUFFER];

    struct sockaddr_storage recv_addr;
    socklen_t addr_len = sizeof recv_addr;

    puts("Esperando conexão");
    recvMsg = recvfrom(sock, buffer, BUFFER, 0, (struct sockaddr *)&recv_addr, &addr_len);
    if(recvMsg < 0){
        perror("Erro ao receber mensagem da origem\n");
        return EXIT_FAILURE;
    }
    sendMsg = sendto(sock, (const char *)msgSend, strlen(msgSend), 0, addr->ai_addr, addr->ai_addrlen);
    if(sendMsg < 0){
        perror("Erro ao enviar mensagem para origem\n");
        return EXIT_FAILURE;
    }
    puts("Conexão aceita");
    return 0;
}

//TODO Envio de mensagens

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        perror("Execução correta: ./destino <porta>\n");
        return EXIT_FAILURE;
    }

    service = argv[1];

    int estado_atual = iniciando;
    int servSock, clntSock;

    while (estado_atual != encerrado)
    {
        switch (estado_atual)
        {
        case iniciando:
            servSock = meuSocket();
            estado_atual = comunicando;
            break;
        case comunicando:
            // meuAccept(clntSock);
            estado_atual = finalizando;
            break;
        case finalizando:
            estado_atual = encerrado;
            close(servSock);
            break;
        default:
            break;
        }
    }

    return EXIT_SUCCESS;
}

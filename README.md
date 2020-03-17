# Objetivo
Comunicação entre origem e destino simulando o protocolo TCP, via UDP.
Origem envia mensagem para um intermediador (comutador) que, por sua vez, encaminha para o destino. O envio dessa mensagem só é feito após a origem e o destino estabelecerem conexão, feito atraves da troca de mensagens "SYN" e "SYNACK".

# para executar
1. gcc comutador.c -o comutador
2. gcc origem.c -o origem
3. gcc destino.c -o destino
4. ./comutador 5001 localhost 5002
5. ./destino 5002
6. ./origem 5001

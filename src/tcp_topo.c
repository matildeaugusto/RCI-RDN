#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "tcp_topo.h"

void tcp_topo_accept_entry(int listen_fd, TopologyInfo *topo) {
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    char buffer[256];

    int conn_fd = accept(listen_fd, (struct sockaddr *)&cliaddr, &clilen);
    if (conn_fd < 0) {
        perror("Erro no accept TCP");
        return;
    }

    int n = read(conn_fd, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        perror("Erro ao ler mensagem TCP");
        close(conn_fd);
        return;
    }

    buffer[n] = '\0';
    printf(">> Recebido: %s", buffer);

    if (strncmp(buffer, "ENTRY", 5) == 0) {
        char ip[50];
        int port;
        if (sscanf(buffer, "ENTRY %s %d", ip, &port) != 2) {
            close(conn_fd);
            return;
        }

        // Enviar SAFE com o vizinho externo atual (antes de atualizar)
        char safe_msg[100];
        snprintf(safe_msg, sizeof(safe_msg), "SAFE %s %d\n", topo->vzext_ip, topo->vzext_tcp);
        write(conn_fd, safe_msg, strlen(safe_msg));
        printf(">> Enviado: %s", safe_msg);

        // Agora sim: atualizar vizinho externo se estava sozinho
        if (strcmp(topo->vzext_ip, topo->id_ip) == 0 && topo->vzext_tcp == topo->id_tcp) {
            strcpy(topo->vzext_ip, ip);
            topo->vzext_tcp = port;
        }

        // Adicionar como vizinho interno
        if (topo->num_intr < MAX_NODES) {
            strcpy(topo->intr[topo->num_intr].ip, ip);
            topo->intr[topo->num_intr].tcp_port = port;
            topo->num_intr++;
        }
    }

    close(conn_fd);
}

int tcp_topo_djoin(char *vzext_ip, int vzext_tcp, char *my_ip, int my_tcp_port, TopologyInfo *topo) {
    int sockfd;
    struct sockaddr_in servaddr;
    char msg[100];
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket TCP");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(vzext_tcp);
    servaddr.sin_addr.s_addr = inet_addr(vzext_ip);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        close(sockfd);
        return -1;
    }

    // Enviar ENTRY
    snprintf(msg, sizeof(msg), "ENTRY %s %d\n", my_ip, my_tcp_port);
    if (write(sockfd, msg, strlen(msg)) < 0) {
        close(sockfd);
        return -1;
    }

    printf(">> Enviado: %s", msg);

    // Esperar resposta SAFE
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        close(sockfd);
        return -1;
    }

    buffer[n] = '\0';
    printf(">> Recebido: %s", buffer);

    // Atualizar vizinho externo com quem fez djoin
    strcpy(topo->vzext_ip, vzext_ip);
    topo->vzext_tcp = vzext_tcp;

    // Interpretar SAFE
    char safe_ip[50];
    int safe_tcp;

    if (sscanf(buffer, "SAFE %s %d", safe_ip, &safe_tcp) != 2) {
        close(sockfd);
        return -1;
    }

    // Verifica se o SAFE corresponde ao nó a quem fiz djoin
    if (strcmp(safe_ip, vzext_ip) == 0 && safe_tcp == vzext_tcp) {
        // Caso especial: dupla ligação
        if (topo->num_intr < MAX_NODES) {
            strcpy(topo->intr[topo->num_intr].ip, vzext_ip);
            topo->intr[topo->num_intr].tcp_port = vzext_tcp;
            topo->num_intr++;

        }
    } 

    close(sockfd);
    return 0;
}

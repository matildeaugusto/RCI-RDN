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

        dprintf(conn_fd, "SAFE %s %d\n", topo->vzext_ip, topo->vzext_tcp);
        shutdown(conn_fd, SHUT_WR);
        printf(">> Enviado SAFE para %s:%d\n", ip, port);

        if (strcmp(topo->vzext_ip, topo->id_ip) == 0 && topo->vzext_tcp == topo->id_tcp) {
            strcpy(topo->vzext_ip, ip);
            topo->vzext_tcp = port;
        }

        if (topo->num_intr < MAX_NODES) {
            strcpy(topo->intr[topo->num_intr].ip, ip);
            topo->intr[topo->num_intr].tcp_port = port;
            topo->num_intr++;
        }

    } else if (strncmp(buffer, "LEAVE", 5) == 0) {
        char ip[50];
        int port;

        if (sscanf(buffer, "LEAVE %s %d", ip, &port) == 2) {
            printf(">> Recebido LEAVE %s:%d\n", ip, port);

            int found = 0;

            for (int i = 0; i < topo->num_intr; i++) {
                if (strcmp(topo->intr[i].ip, ip) == 0 && topo->intr[i].tcp_port == port) {
                    printf(">> MEU - Vou remover o vizinho interno %s:%d\n", ip, port);

                    // Remover deslocando o array
                    for (int j = i; j < topo->num_intr - 1; j++) {
                        topo->intr[j] = topo->intr[j + 1];
                    }
                    topo->num_intr--;

                    printf(">> Vizinho interno %s:%d removido.\n", ip, port);

                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf(">> NOT MY\n");

                printf(">> Removendo vizinho externo %s:%d\n", topo->vzext_ip, topo->vzext_tcp);
                strcpy(topo->vzext_ip, topo->id_ip);
                topo->vzext_tcp = topo->id_tcp;

                printf(">> Fazendo djoin para novo vizinho externo %s:%d\n", ip, port);
                tcp_topo_djoin(ip, port, topo->id_ip, topo->id_tcp, topo);
            }
        }
    }

    close(conn_fd);
}

int tcp_topo_djoin(char *vzext_ip, int vzext_tcp, char *my_ip, int my_tcp_port, TopologyInfo *topo) {
    int sockfd;
    struct sockaddr_in servaddr;
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

    // Enviar ENTRY diretamente sem buffer
    dprintf(sockfd, "ENTRY %s %d\n", my_ip, my_tcp_port);
    shutdown(sockfd, SHUT_WR);
    printf(">> Enviado ENTRY para %s:%d\n", vzext_ip, vzext_tcp);

    // Esperar resposta SAFE
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        close(sockfd);
        return -1;
    }

    buffer[n] = '\0';
    printf(">> Recebido: %s", buffer);

    // Atualizar vizinho externo
    strcpy(topo->vzext_ip, vzext_ip);
    topo->vzext_tcp = vzext_tcp;

    // Interpretar SAFE
    char safe_ip[50];
    int safe_tcp;

    if (sscanf(buffer, "SAFE %s %d", safe_ip, &safe_tcp) != 2) {
        close(sockfd);
        return -1;
    }

    // Caso especial: dupla ligação
    if (strcmp(safe_ip, vzext_ip) == 0 && safe_tcp == vzext_tcp) {
        if (topo->num_intr < MAX_NODES) {
            strcpy(topo->intr[topo->num_intr].ip, vzext_ip);
            topo->intr[topo->num_intr].tcp_port = vzext_tcp;
            topo->num_intr++;
        }
    }

    close(sockfd);
    return 0;
}

void send_leave_message(char *my_ip, int my_port, char *dest_ip, int dest_port) {
    int sockfd;
    struct sockaddr_in dest_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
    inet_pton(AF_INET, dest_ip, &dest_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return;
    }

    printf(">> Enviando LEAVE para %s:%d\n", dest_ip, dest_port);

    // Enviar LEAVE diretamente sem buffer
    dprintf(sockfd, "LEAVE %s %d\n", my_ip, my_port);
    shutdown(sockfd, SHUT_WR);
    close(sockfd);
}

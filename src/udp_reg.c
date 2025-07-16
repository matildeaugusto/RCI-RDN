#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "udp_reg.h"
#include "topology.h"

int udp_reg_join(int sockfd, char *reg_ip, int reg_udp_port, int net, char *my_ip, int my_tcp_port, NodeInfo nodes[], int *num_nodes, TopologyInfo *topo) {
    struct sockaddr_in servaddr;
    char sendline[MAX_LINE];
    char recvline[MAX_LINE];
    socklen_t len = sizeof(servaddr);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(reg_udp_port);
    servaddr.sin_addr.s_addr = inet_addr(reg_ip);

    // Enviar NODES
    snprintf(sendline, sizeof(sendline), "NODES %03d\n", net);
    sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, len);
    printf(">> Enviado: %s", sendline);

    // Receber NODESLIST
    int n = recvfrom(sockfd, recvline, MAX_LINE - 1, 0, (struct sockaddr *)&servaddr, &len);
    if (n < 0) {
        return -1;
    }
    recvline[n] = '\0';
    printf(">> Recebido: %s\n", recvline);

    if (strncmp(recvline, "NODESLIST", 9) != 0) {
        return -1;
    }

    // Preencher a lista de nós
    *num_nodes = 0;
    char *line = strtok(recvline + 10, "\n");
    while (line != NULL && *num_nodes < MAX_NODES) {
        sscanf(line, "%s %d", nodes[*num_nodes].ip, &nodes[*num_nodes].tcp_port);
        (*num_nodes)++;
        line = strtok(NULL, "\n");
    }

    // Configurar a topologia inicial
    strcpy(topo->id_ip, my_ip);
    topo->id_tcp = my_tcp_port;

    if (*num_nodes == 0) {
        // Nó é o primeiro da rede
        strcpy(topo->vzext_ip, my_ip);
        topo->vzext_tcp = my_tcp_port;

        topo->num_intr = 0;

    } else {
        // Preencher topologia parcial (vizinho externo será escolhido depois)
        topo->num_intr = 0; // Ainda não liga a ninguém
    }

    topology_init(topo, my_ip, my_tcp_port, *num_nodes);

    // Enviar REG
    snprintf(sendline, sizeof(sendline), "REG %03d %s %d\n", net, my_ip, my_tcp_port);
    sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, len);
    printf(">> Enviado: %s", sendline);

    // Receber OKREG
    n = recvfrom(sockfd, recvline, MAX_LINE - 1, 0, (struct sockaddr *)&servaddr, &len);
    if (n < 0) {
        return -1;
    }
    recvline[n] = '\0';
    printf(">> Recebido: %s\n", recvline);

    if (strncmp(recvline, "OKREG", 5) != 0) {
        return -1;
    }

    return 0;
}

int udp_reg_leave(int sockfd, char *reg_ip, int reg_udp_port, int net, char *my_ip, int my_tcp_port) {
    struct sockaddr_in servaddr;
    char sendline[MAX_LINE];
    char recvline[MAX_LINE];
    socklen_t len = sizeof(servaddr);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(reg_udp_port);
    servaddr.sin_addr.s_addr = inet_addr(reg_ip);

    // Enviar UNREG
    snprintf(sendline, sizeof(sendline), "UNREG %03d %s %d\n", net, my_ip, my_tcp_port);
    sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, len);
    printf(">> Enviado: %s", sendline);

    // Receber OKUNREG
    int n = recvfrom(sockfd, recvline, MAX_LINE - 1, 0, (struct sockaddr *)&servaddr, &len);
    if (n < 0) {
        return -1;
    }

    recvline[n] = '\0';
    printf(">> Recebido: %s\n", recvline);

    if (strncmp(recvline, "OKUNREG", 7) != 0) {
        return -1;
    }

    return 0;
}


int udp_reg_show_nodes(int sockfd, char *reg_ip, int reg_udp_port, int net, NodeInfo nodes[], int *num_nodes) {
    struct sockaddr_in servaddr;
    char sendline[MAX_LINE];
    char recvline[MAX_LINE];
    socklen_t len = sizeof(servaddr);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(reg_udp_port);
    servaddr.sin_addr.s_addr = inet_addr(reg_ip);

    // Enviar NODES
    snprintf(sendline, sizeof(sendline), "NODES %03d\n", net);
    sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *)&servaddr, len);

    // Receber NODESLIST
    recvfrom(sockfd, recvline, MAX_LINE, 0, (struct sockaddr *)&servaddr, &len);
    if (strncmp(recvline, "NODESLIST", 9) != 0) {
        return -1;
    }

    // Ler lista de nós
    *num_nodes = 0;
    char *line = strtok(recvline + 10, "\n");
    printf("╔══════════════════════════════════╗\n");
    printf("║ Lista de Nós da rede %03d         ║\n", net);
    printf("╠══════════════════════════════════╣\n");
    while (line != NULL && *num_nodes < MAX_NODES) {
        sscanf(line, "%s %d", nodes[*num_nodes].ip, &nodes[*num_nodes].tcp_port);
        printf("║ Nó %d: %s %d\n", *num_nodes + 1, nodes[*num_nodes].ip, nodes[*num_nodes].tcp_port);
        (*num_nodes)++;
        line = strtok(NULL, "\n");
    }
    printf("╚══════════════════════════════════╝\n");

    return 0;
}

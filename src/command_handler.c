#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "udp_reg.h"
#include "command_handler.h"
#include "topology.h"
#include "tcp_topo.h"
#include "utils.h"   // Para is_valid_ip e is_valid_port



void handle_command(char *command, int udp_fd, char *reg_ip, int reg_udp_port, char *my_ip, int my_tcp_port, TopologyInfo *topo) {
    static int net = -1;
    static NodeInfo nodes[MAX_NODES];
    static int num_nodes = 0;

    char cmd[50];
    sscanf(command, "%s", cmd);

    if (strncmp(command, "join ", 5) == 0 || strncmp(command, "j ", 2) == 0) {
        char net_str[10];
        int net_input;

        if (strncmp(command, "join ", 5) == 0) {
            if (sscanf(command + 5, "%s", net_str) != 1) {
                printf("Uso correto: join <net> (exatamente 3 dígitos)\n");
                return;
            }
        } else {
            if (sscanf(command + 2, "%s", net_str) != 1) {
                printf("Uso correto: j <net> (exatamente 3 dígitos)\n");
                return;
            }
        }

        if (strlen(net_str) != 3 || strspn(net_str, "0123456789") != 3) {
            printf("O identificador da rede deve ser exatamente 3 dígitos (000 a 999).\n");
            return;
        }

        net_input = atoi(net_str);
        if (net_input < 0 || net_input > 999) {
            printf("Valor inválido para a rede (deve ser entre 000 e 999).\n");
            return;
        }

        net = net_input;

        if (udp_reg_join(udp_fd, reg_ip, reg_udp_port, net, my_ip, my_tcp_port, nodes, &num_nodes, topo) == 0) {
        }

    } else if (strcmp(cmd, "leave") == 0 || strcmp(cmd, "l") == 0) {

        if (topo->num_intr > 0 &&
            strcmp(topo->vzext_ip, topo->intr[0].ip) == 0 &&
            topo->vzext_tcp == topo->intr[0].tcp_port) {

            printf(">> Vizinho externo é igual ao primeiro vizinho interno. Removendo...\n");

            // Remove o primeiro vizinho interno deslocando o array
            for (int j = 0; j < topo->num_intr - 1; j++) {
                topo->intr[j] = topo->intr[j + 1];
            }
            topo->num_intr--;

            strcpy(topo->vzext_ip, topo->id_ip);
            topo->vzext_tcp = topo->id_tcp;

            printf(">> Vizinho interno duplicado removido.\n");
        }

        if (strcmp(topo->vzext_ip, topo->id_ip) != 0 || topo->vzext_tcp != topo->id_tcp) {
            printf("AAA\n");
            send_leave_message(topo->id_ip, topo->id_tcp, topo->vzext_ip, topo->vzext_tcp);
        }

        if (topo->num_intr > 0) {
            printf("BBB\n");
            // Notifica o primeiro vizinho interno normalmente
            send_leave_message(topo->vzext_ip, topo->vzext_tcp, topo->intr[0].ip, topo->intr[0].tcp_port);

            // Se houver mais vizinhos internos, envia LEAVE aos restantes com o IP/PORT do primeiro vizinho interno
            for (int i = 1; i < topo->num_intr; i++) {
                printf(">> Enviando LEAVE ao vizinho interno %s:%d com novo vizinho externo %s:%d\n",
                    topo->intr[i].ip, topo->intr[i].tcp_port,
                    topo->intr[0].ip, topo->intr[0].tcp_port);

                send_leave_message(topo->intr[0].ip, topo->intr[0].tcp_port,
                                topo->intr[i].ip, topo->intr[i].tcp_port);
            }
        }

        if (udp_reg_leave(udp_fd, reg_ip, reg_udp_port, net, my_ip, my_tcp_port) == 0) {
            net = -1;
        }

        printf(">> Limpando todos os vizinhos.\n");
        strcpy(topo->vzext_ip, topo->id_ip);
        topo->vzext_tcp = topo->id_tcp;
        topo->num_intr = 0;

    } else if (strncmp(command, "show topology", 13) == 0 || strcmp(command, "st") == 0) {
        topology_print(topo);

    } else if (strncmp(command, "djoin", 5) == 0 || strncmp(command, "direct join", 11) == 0) {
        char ip[50];
        int port;
        char *token;

        if (strncmp(command, "djoin", 5) == 0) {
            // Caso djoin
            token = strtok(command, " ");
            token = strtok(NULL, " "); // ip
        } else {
            // Caso direct join
            token = strtok(command, " ");
            token = strtok(NULL, " "); // "join" (ignorar)
            token = strtok(NULL, " "); // ip
        }

        if (token == NULL) {
            printf("Uso correto: djoin <ip> <port>\n");
            return;
        }
        strcpy(ip, token);

        token = strtok(NULL, " "); // port
        if (token == NULL) {
            printf("Uso correto: djoin <ip> <port>\n");
            return;
        }

        if (!is_valid_ip(ip)) {
            printf("IP inválido: %s\n", ip);
            return;
        }

        port = atoi(token);
        if (!is_valid_port(port)) {
            printf("Porto inválido: %d (deve estar entre 1024 e 65535)\n", port);
            return;
        }

        if (tcp_topo_djoin(ip, port, my_ip, my_tcp_port, topo) == 0) {
        }
    } else {
            printf("Comando não reconhecido: %s\n", cmd);
    }
}

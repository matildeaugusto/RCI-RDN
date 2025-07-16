#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils.h"

int tcp_listen_fd;
int udp_fd;

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1;
}

int is_valid_port(int port) {
    return port >= 1024 && port <= 65535;
}

void show_usage(char *progname) {
    printf("\nUso correto:\n");
    printf("%s IP TCP [regIP regUDP]\n", progname);
    printf("Se não fornecer regIP e regUDP, serão usados os valores por omissão:\n");
    printf("regIP = %s\n", DEFAULT_REG_IP);
    printf("regUDP = %d\n", DEFAULT_REG_PORT);
}

void show_commands() {
    printf("\n╔════════════════════════════════════════════════╗\n");
    printf("║            Comandos Disponíveis                ║\n");
    printf("╠════════════════════════════════════════════════╣\n");
    printf("║ join (j) <net>               - Entrar na rede  ║\n");
    printf("║ direct join (djoin) <IP> <P> - Ligação direta  ║\n");
    printf("║ create <name>                - Criar objeto    ║\n");
    printf("║ delete <name>                - Apagar objeto   ║\n");
    printf("║ retrieve <name>              - Obter objeto    ║\n");
    printf("║ show topology                - Ver topologia   ║\n");
    printf("║ show names                   - Ver objetos     ║\n");
    printf("║ show interest table          - Ver interesses  ║\n");
    printf("║ leave (l)                    - Sair da rede    ║\n");
    printf("║ exit (x)                     - Terminar        ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
}

void setup_sockets(char *my_ip, int my_tcp_port, char *reg_ip, int reg_udp_port) {
    struct sockaddr_in tcp_addr, udp_addr;

    tcp_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_listen_fd < 0) {
        exit(1);
    }

    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(my_tcp_port);
    tcp_addr.sin_addr.s_addr = inet_addr(my_ip);

    if (bind(tcp_listen_fd, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0) {
        close(tcp_listen_fd);
        exit(1);
    }

    if (listen(tcp_listen_fd, 10) < 0) {
        close(tcp_listen_fd);
        exit(1);
    }


    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        close(tcp_listen_fd);
        exit(1);
    }

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(reg_udp_port);
    udp_addr.sin_addr.s_addr = inet_addr(reg_ip);

}

void close_sockets() {
    close(tcp_listen_fd);
    close(udp_fd);
}


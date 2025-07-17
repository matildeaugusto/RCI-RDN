#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "utils.h"
#include "command_handler.h"
#include "tcp_topo.h"

TopologyInfo topo;

int main(int argc, char *argv[]) {
    char my_ip[50];
    int my_tcp_port;
    char reg_ip[50];
    int reg_udp_port;

    if (argc != 3 && argc != 5) {
        show_usage(argv[0]);
        return 1;
    }

    strcpy(my_ip, argv[1]);
    my_tcp_port = atoi(argv[2]);

    if (argc == 5) {
        strcpy(reg_ip, argv[3]);
        reg_udp_port = atoi(argv[4]);
    } else {
        strcpy(reg_ip, DEFAULT_REG_IP);
        reg_udp_port = DEFAULT_REG_PORT;
    }

    if (!is_valid_ip(my_ip)) {
        printf("IP local inválido: %s\n", my_ip);
        show_usage(argv[0]);
        return 1;
    }

    if (!is_valid_port(my_tcp_port)) {
        printf("Porta TCP inválida (%d). Deve estar entre 1024 e 65535.\n", my_tcp_port);
        show_usage(argv[0]);
        return 1;
    }

    if (!is_valid_ip(reg_ip)) {
        printf("IP do servidor inválido: %s\n", reg_ip);
        show_usage(argv[0]);
        return 1;
    }

    if (!is_valid_port(reg_udp_port)) {
        printf("Porta UDP inválida (%d). Deve estar entre 1024 e 65535.\n", reg_udp_port);
        show_usage(argv[0]);
        return 1;
    }

    // Criar socket UDP
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("Erro ao criar socket UDP");
        return 1;
    }

    // Criar socket TCP de escuta
    int tcp_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_listen_fd < 0) {
        perror("Erro ao criar socket TCP");
        return 1;
    }

    struct sockaddr_in tcp_addr;
    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(my_tcp_port);
    tcp_addr.sin_addr.s_addr = inet_addr(my_ip);

    if (bind(tcp_listen_fd, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0) {
        perror("Erro no bind TCP");
        close(tcp_listen_fd);
        return 1;
    }

    if (listen(tcp_listen_fd, 10) < 0) {
        perror("Erro no listen TCP");
        close(tcp_listen_fd);
        return 1;
    }

    strcpy(topo.id_ip, my_ip);
    topo.id_tcp = my_tcp_port;
    
    show_commands();

    char buffer[256];

    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);

        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(udp_fd, &read_fds);
        FD_SET(tcp_listen_fd, &read_fds);

        int max_fd = udp_fd > tcp_listen_fd ? udp_fd : tcp_listen_fd;
        max_fd = STDIN_FILENO > max_fd ? STDIN_FILENO : max_fd;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Erro no select");
            break;
        }

        // Comando do utilizador
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                printf("Erro ao ler comando.\n");
                continue;
            }

            buffer[strcspn(buffer, "\n")] = 0;

            if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "x") == 0) {
                break;
            }

            handle_command(buffer, udp_fd, reg_ip, reg_udp_port, my_ip, my_tcp_port, &topo);

        }

        // Receção de TCP ENTRY
        if (FD_ISSET(tcp_listen_fd, &read_fds)) {
            tcp_topo_accept_entry(tcp_listen_fd, &topo);
        }

        // (Futuro) Aqui podes tratar UDP ou outros eventos
    }

    close(udp_fd);
    close(tcp_listen_fd);

    return 0;
}

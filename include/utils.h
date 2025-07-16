#ifndef UTILS_H
#define UTILS_H

#define DEFAULT_REG_IP "193.136.138.142"
#define DEFAULT_REG_PORT 59000

int is_valid_ip(const char *ip);
int is_valid_port(int port);
void show_usage(char *progname);
void show_commands();
void setup_sockets(char *my_ip, int my_tcp_port, char *reg_ip, int reg_udp_port);
void close_sockets();
#endif

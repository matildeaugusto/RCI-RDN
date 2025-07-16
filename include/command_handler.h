#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "topology_struct.h"

void handle_command(char *command, int udp_fd, char *reg_ip, int reg_udp_port, char *my_ip, int my_tcp_port, TopologyInfo *topo);

#endif

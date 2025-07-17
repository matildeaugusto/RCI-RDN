#ifndef TCP_TOPO_H
#define TCP_TOPO_H

#include "topology_struct.h"

int tcp_topo_djoin(char *vzext_ip, int vzext_tcp, char *my_ip, int my_tcp_port, TopologyInfo *topo);
void tcp_topo_accept_entry(int listen_fd, TopologyInfo *topo);
void send_leave_message(char *my_ip, int my_port, char *dest_ip, int dest_port);

#endif
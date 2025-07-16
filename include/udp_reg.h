#ifndef UDP_REG_H
#define UDP_REG_H

#define MAX_NODES 100
#define MAX_LINE 256

#include "topology_struct.h"
#include "nodeinfo.h"



// Funções principais
int udp_reg_join(int sockfd, char *reg_ip, int reg_udp_port, int net, char *my_ip, int my_tcp_port, NodeInfo nodes[], int *num_nodes, TopologyInfo *topo);
int udp_reg_leave(int sockfd, char *reg_ip, int reg_udp_port, int net, char *my_ip, int my_tcp_port);
int udp_reg_show_nodes(int sockfd, char *reg_ip, int reg_udp_port, int net, NodeInfo nodes[], int *num_nodes);

#endif

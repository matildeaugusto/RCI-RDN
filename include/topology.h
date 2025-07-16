#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "topology_struct.h"
#include "nodeinfo.h"

void topology_init(TopologyInfo *topo, char *my_ip, int my_tcp_port, int num_nodes);
void topology_print(TopologyInfo *topo);

#endif

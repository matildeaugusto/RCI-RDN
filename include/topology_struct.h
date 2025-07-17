#ifndef TOPOLOGY_STRUCT_H
#define TOPOLOGY_STRUCT_H

#include "nodeinfo.h"  // NodeInfo deve estar num header separado

typedef struct {
    char id_ip[50];
    int id_tcp;

    char vzext_ip[50];
    int vzext_tcp;

    char vzsalv_ip[50];
    int vzsalv_tcp;

    NodeInfo intr[MAX_NODES];
    int num_intr;

} TopologyInfo;



#endif

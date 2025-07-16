#include <stdio.h>
#include <string.h>
#include "topology.h"

void topology_init(TopologyInfo *topo, char *my_ip, int my_tcp_port, int num_nodes) {
    strcpy(topo->id_ip, my_ip);
    topo->id_tcp = my_tcp_port;

    if (num_nodes == 1) {
        // Nó sozinho na rede
        strcpy(topo->vzext_ip, my_ip);
        topo->vzext_tcp = my_tcp_port;

        topo->num_intr = 0;

    } else {
        // Rede com outros nós
        strcpy(topo->vzext_ip, "N/A");
        topo->vzext_tcp = -1;

        topo->num_intr = 0;
    }
}


void topology_print(TopologyInfo *topo) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║ Topologia atual do nó                ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║ Vizinho externo: %s %d\n", topo->vzext_ip, topo->vzext_tcp);
    if (topo->num_intr == 0) {
        printf("║ Vizinhos internos: Nenhum\n");
    } else {
        printf("║ Vizinhos internos:\n");
        for (int i = 0; i < topo->num_intr; i++) {
            printf("║   -> %s %d\n", topo->intr[i].ip, topo->intr[i].tcp_port);
        }
    }
    printf("╚══════════════════════════════════════╝\n");
}

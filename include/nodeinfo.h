// nodeinfo.h
#ifndef NODEINFO_H
#define NODEINFO_H

#define MAX_NODES 100

typedef struct {
    char ip[50];
    int tcp_port;
} NodeInfo;

#endif

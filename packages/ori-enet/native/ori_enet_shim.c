#include <enet/enet.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
/* Pull Winsock when this staticlib is linked into the Ori binary (MSVC). */
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#endif

/* Thin slot-based ENet boundary for Ori (int handles, C strings via ptr). */

#define MAX_HOSTS 8
#define MAX_PEERS 64
#define MAX_PACKET_BYTES (1024 * 1024)

static int g_inited = 0;
static ENetHost *g_hosts[MAX_HOSTS];
static ENetPeer *g_peers[MAX_PEERS];
static int g_peer_host[MAX_PEERS]; /* host slot that owns peer, or -1 */

static ENetEvent g_event;
static ENetPacket *g_last_packet = NULL;
static int g_last_peer_slot = -1;
static int g_last_channel = 0;
static enet_uint32 g_last_data = 0;
static char g_last_err[256];

static void set_err(const char *msg) {
    if (!msg) {
        g_last_err[0] = '\0';
        return;
    }
    strncpy(g_last_err, msg, sizeof(g_last_err) - 1);
    g_last_err[sizeof(g_last_err) - 1] = '\0';
}

static int alloc_host(void) {
    for (int i = 0; i < MAX_HOSTS; ++i) {
        if (!g_hosts[i]) return i;
    }
    return -1;
}

static int alloc_peer_slot(void) {
    for (int i = 0; i < MAX_PEERS; ++i) {
        if (!g_peers[i]) return i;
    }
    return -1;
}

static int peer_slot_of(ENetPeer *p) {
    if (!p) return -1;
    for (int i = 0; i < MAX_PEERS; ++i) {
        if (g_peers[i] == p) return i;
    }
    return -1;
}

static int ensure_peer_slot(ENetPeer *p, int host_slot) {
    int existing = peer_slot_of(p);
    if (existing >= 0) return existing;
    int slot = alloc_peer_slot();
    if (slot < 0) {
        set_err("too many peers");
        return -1;
    }
    g_peers[slot] = p;
    g_peer_host[slot] = host_slot;
    return slot;
}

static void free_peer_slot(int slot) {
    if (slot < 0 || slot >= MAX_PEERS) return;
    g_peers[slot] = NULL;
    g_peer_host[slot] = -1;
}

static void free_peers_for_host(int host_slot) {
    for (int i = 0; i < MAX_PEERS; ++i) {
        if (g_peer_host[i] == host_slot) {
            free_peer_slot(i);
        }
    }
}

static void clear_last_packet(void) {
    if (g_last_packet) {
        enet_packet_destroy(g_last_packet);
        g_last_packet = NULL;
    }
}

int ori_enet_initialize(void) {
    if (g_inited) return 0;
    if (enet_initialize() != 0) {
        set_err("enet_initialize failed");
        return 1;
    }
    memset(g_hosts, 0, sizeof(g_hosts));
    memset(g_peers, 0, sizeof(g_peers));
    for (int i = 0; i < MAX_PEERS; ++i) g_peer_host[i] = -1;
    g_inited = 1;
    set_err(NULL);
    return 0;
}

void ori_enet_deinitialize(void) {
    if (!g_inited) return;
    clear_last_packet();
    for (int i = 0; i < MAX_HOSTS; ++i) {
        if (g_hosts[i]) {
            enet_host_destroy(g_hosts[i]);
            g_hosts[i] = NULL;
        }
    }
    for (int i = 0; i < MAX_PEERS; ++i) free_peer_slot(i);
    enet_deinitialize();
    g_inited = 0;
}

/* Create bound host (server). port in host order. max_clients/channels >= 1. */
int ori_enet_host_create(int port, int max_clients, int channels) {
    if (!g_inited) {
        set_err("not initialized");
        return -1;
    }
    if (max_clients < 1) max_clients = 1;
    if (channels < 1) channels = 1;
    int slot = alloc_host();
    if (slot < 0) {
        set_err("too many hosts");
        return -1;
    }
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = (enet_uint16)(port & 0xFFFF);
    ENetHost *host = enet_host_create(&address, (size_t)max_clients, (size_t)channels, 0, 0);
    if (!host) {
        set_err("host_create failed");
        return -1;
    }
    g_hosts[slot] = host;
    set_err(NULL);
    return slot;
}

/* Client-side host (no bind port). */
int ori_enet_host_create_client(int max_peers, int channels) {
    if (!g_inited) {
        set_err("not initialized");
        return -1;
    }
    if (max_peers < 1) max_peers = 1;
    if (channels < 1) channels = 1;
    int slot = alloc_host();
    if (slot < 0) {
        set_err("too many hosts");
        return -1;
    }
    ENetHost *host = enet_host_create(NULL, (size_t)max_peers, (size_t)channels, 0, 0);
    if (!host) {
        set_err("host_create_client failed");
        return -1;
    }
    g_hosts[slot] = host;
    set_err(NULL);
    return slot;
}

void ori_enet_host_destroy(int host) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) return;
    free_peers_for_host(host);
    enet_host_destroy(g_hosts[host]);
    g_hosts[host] = NULL;
}

int ori_enet_host_is_valid(int host) {
    return (host >= 0 && host < MAX_HOSTS && g_hosts[host] != NULL) ? 1 : 0;
}

/* Connect to host:port. Returns peer slot (>=0) or -1. */
int ori_enet_host_connect(int host, int host_name_ptr, int port, int channels) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) {
        set_err("invalid host");
        return -1;
    }
    const char *name = (const char *)(intptr_t)host_name_ptr;
    if (!name || !name[0]) {
        set_err("null host name");
        return -1;
    }
    if (channels < 1) channels = 1;
    ENetAddress address;
    if (enet_address_set_host(&address, name) != 0) {
        set_err("address_set_host failed");
        return -1;
    }
    address.port = (enet_uint16)(port & 0xFFFF);
    ENetPeer *peer = enet_host_connect(g_hosts[host], &address, (size_t)channels, 0);
    if (!peer) {
        set_err("host_connect failed");
        return -1;
    }
    int slot = ensure_peer_slot(peer, host);
    if (slot < 0) return -1;
    set_err(NULL);
    return slot;
}

/* Returns event type: 0 none, 1 connect, 2 disconnect, 3 receive. */
int ori_enet_service(int host, int timeout_ms) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) {
        set_err("invalid host");
        return 0;
    }
    clear_last_packet();
    g_last_peer_slot = -1;
    g_last_channel = 0;
    g_last_data = 0;

    int rc = enet_host_service(g_hosts[host], &g_event, (enet_uint32)(timeout_ms < 0 ? 0 : timeout_ms));
    if (rc < 0) {
        set_err("host_service error");
        return 0;
    }
    if (rc == 0) {
        return 0; /* NONE */
    }

    if (g_event.peer) {
        g_last_peer_slot = ensure_peer_slot(g_event.peer, host);
    }
    g_last_data = g_event.data;
    g_last_channel = (int)g_event.channelID;

    switch (g_event.type) {
    case ENET_EVENT_TYPE_CONNECT:
        set_err(NULL);
        return 1;
    case ENET_EVENT_TYPE_DISCONNECT:
        if (g_last_peer_slot >= 0) free_peer_slot(g_last_peer_slot);
        g_last_peer_slot = -1;
        set_err(NULL);
        return 2;
    case ENET_EVENT_TYPE_RECEIVE:
        g_last_packet = g_event.packet;
        g_event.packet = NULL; /* ownership transferred */
        set_err(NULL);
        return 3;
    default:
        return 0;
    }
}

void ori_enet_host_flush(int host) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) return;
    enet_host_flush(g_hosts[host]);
}

int ori_enet_event_peer(void) {
    return g_last_peer_slot;
}

int ori_enet_event_channel(void) {
    return g_last_channel;
}

int ori_enet_event_data(void) {
    return (int)g_last_data;
}

int ori_enet_packet_size(void) {
    if (!g_last_packet) return 0;
    return (int)g_last_packet->dataLength;
}

int ori_enet_packet_to_path(int path_ptr) {
    if (!g_last_packet) {
        set_err("no packet");
        return 1;
    }
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        set_err("null path");
        return 1;
    }
    FILE *f = fopen(path, "wb");
    if (!f) {
        set_err("open path failed");
        return 1;
    }
    size_t n = g_last_packet->dataLength;
    if (n > 0 && fwrite(g_last_packet->data, 1, n, f) != n) {
        fclose(f);
        set_err("write failed");
        return 1;
    }
    fclose(f);
    set_err(NULL);
    return 0;
}

void ori_enet_packet_destroy(void) {
    clear_last_packet();
}

/* reliable != 0 → ENET_PACKET_FLAG_RELIABLE */
int ori_enet_send(int peer, int data_ptr, int len, int channel, int reliable) {
    if (peer < 0 || peer >= MAX_PEERS || !g_peers[peer]) {
        set_err("invalid peer");
        return 1;
    }
    if (len < 0) {
        set_err("negative len");
        return 1;
    }
    if (len > MAX_PACKET_BYTES) {
        set_err("packet too large");
        return 1;
    }
    const void *data = (const void *)(intptr_t)data_ptr;
    if (len > 0 && !data) {
        set_err("null data");
        return 1;
    }
    enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket *packet = enet_packet_create(data, (size_t)len, flags);
    if (!packet) {
        set_err("packet_create failed");
        return 1;
    }
    if (enet_peer_send(g_peers[peer], (enet_uint8)(channel & 0xFF), packet) < 0) {
        enet_packet_destroy(packet);
        set_err("peer_send failed");
        return 1;
    }
    set_err(NULL);
    return 0;
}

int ori_enet_broadcast(int host, int data_ptr, int len, int channel, int reliable) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) {
        set_err("invalid host");
        return 1;
    }
    if (len < 0 || len > MAX_PACKET_BYTES) {
        set_err("bad len");
        return 1;
    }
    const void *data = (const void *)(intptr_t)data_ptr;
    if (len > 0 && !data) {
        set_err("null data");
        return 1;
    }
    enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    ENetPacket *packet = enet_packet_create(data, (size_t)len, flags);
    if (!packet) {
        set_err("packet_create failed");
        return 1;
    }
    enet_host_broadcast(g_hosts[host], (enet_uint8)(channel & 0xFF), packet);
    set_err(NULL);
    return 0;
}

int ori_enet_peer_is_valid(int peer) {
    return (peer >= 0 && peer < MAX_PEERS && g_peers[peer] != NULL) ? 1 : 0;
}

void ori_enet_peer_disconnect(int peer) {
    if (peer < 0 || peer >= MAX_PEERS || !g_peers[peer]) return;
    enet_peer_disconnect(g_peers[peer], 0);
}

void ori_enet_peer_disconnect_now(int peer) {
    if (peer < 0 || peer >= MAX_PEERS || !g_peers[peer]) return;
    enet_peer_disconnect_now(g_peers[peer], 0);
    free_peer_slot(peer);
}

int ori_enet_peer_rtt_ms(int peer) {
    if (peer < 0 || peer >= MAX_PEERS || !g_peers[peer]) return -1;
    return (int)g_peers[peer]->roundTripTime;
}

int ori_enet_peer_count_live(void) {
    int n = 0;
    for (int i = 0; i < MAX_PEERS; ++i) {
        if (g_peers[i]) n++;
    }
    return n;
}

int ori_enet_errmsg_to_path(int path_ptr) {
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) return 1;
    FILE *f = fopen(path, "wb");
    if (!f) return 1;
    size_t n = strlen(g_last_err);
    if (n > 0) fwrite(g_last_err, 1, n, f);
    fclose(f);
    return 0;
}

/* Host traffic counters (ENet host totals). */
int ori_enet_host_total_sent(int host) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) return -1;
    return (int)g_hosts[host]->totalSentData;
}

int ori_enet_host_total_received(int host) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) return -1;
    return (int)g_hosts[host]->totalReceivedData;
}

int ori_enet_host_peer_count(int host) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) return -1;
    return (int)g_hosts[host]->connectedPeers;
}

/* Peer state: maps ENetPeerState enum as int (0 disconnected … 5 connected). */
int ori_enet_peer_state(int peer) {
    if (peer < 0 || peer >= MAX_PEERS || !g_peers[peer]) return -1;
    return (int)g_peers[peer]->state;
}

/* "a.b.c.d:port" into path. */
int ori_enet_peer_address_to_path(int peer, int path_ptr) {
    if (peer < 0 || peer >= MAX_PEERS || !g_peers[peer]) {
        set_err("invalid peer");
        return 1;
    }
    const char *path = (const char *)(intptr_t)path_ptr;
    if (!path) {
        set_err("null path");
        return 1;
    }
    char hostbuf[64];
    if (enet_address_get_host_ip(&g_peers[peer]->address, hostbuf, sizeof(hostbuf)) != 0) {
        set_err("address_get_host_ip failed");
        return 1;
    }
    char line[96];
    snprintf(line, sizeof(line), "%s:%u", hostbuf, (unsigned)g_peers[peer]->address.port);
    FILE *f = fopen(path, "wb");
    if (!f) {
        set_err("open path failed");
        return 1;
    }
    fputs(line, f);
    fclose(f);
    set_err(NULL);
    return 0;
}

/* Drain up to max_events with 0ms timeout; returns number of non-NONE events processed.
 * Last event metadata remains in event_* / packet_* getters. */
int ori_enet_service_drain(int host, int max_events) {
    if (host < 0 || host >= MAX_HOSTS || !g_hosts[host]) {
        set_err("invalid host");
        return -1;
    }
    if (max_events < 1) max_events = 1;
    int count = 0;
    for (int i = 0; i < max_events; ++i) {
        int ev = ori_enet_service(host, 0);
        if (ev == 0) break;
        count++;
        /* RECEIVE packets must be destroyed by the caller before next service
         * if they care about the payload; for drain we free automatically. */
        if (ev == 3) clear_last_packet();
    }
    set_err(NULL);
    return count;
}

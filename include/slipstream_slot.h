#ifndef SLIPSTREAM_SLOT
#define SLIPSTREAM_SLOT

#include "SPCDNS/src/dns.h"
#include "picoquic.h"

typedef struct st_slot_t {
    dns_decoded_t dns_decoded[DNS_DECODEBUF_4K];
    dns_rcode_t error;
    struct sockaddr_storage peer_addr;
    struct sockaddr_storage local_addr;
    picoquic_cnx_t* cnx;
    int path_id;
    bool is_poll_packet;
} slot_t;

#endif // SLIPSTREAM_SLOT

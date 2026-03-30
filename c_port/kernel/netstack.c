#include "types.h"

extern int rtl8139_send_packet(const u8* packet, u32 len);
extern u32 rtl8139_poll_receive(u8* dst, u32 max_len);

#define NET_MTU 1500u
#define ETH_HDR_LEN 14u
#define IP_HDR_LEN 20u
#define UDP_HDR_LEN 8u

u8 net_local_mac[6] = {0x02,0x00,0x00,0x00,0x00,0x01};
u32 net_local_ip = 0x6401A8C0u;
u16 udp_last_len;
u32 udp_last_src_ip;
u16 udp_last_src_port;
u8 udp_last_payload[NET_MTU];

static u8 net_rx_frame[ETH_HDR_LEN + NET_MTU];
static u8 net_tx_frame[ETH_HDR_LEN + NET_MTU];

void net_stack_init(void) { udp_last_len = 0; udp_last_src_ip = 0; udp_last_src_port = 0; }

static u16 ip_checksum_20(const u8* ip) {
    u32 sum = 0;
    for (u32 i = 0; i < 20; i += 2) sum += ((u16)ip[i] << 8) | ip[i + 1];
    while (sum >> 16) sum = (sum & 0xFFFFu) + (sum >> 16);
    return (u16)(~sum);
}

void net_poll(void) {
    u32 n = rtl8139_poll_receive(net_rx_frame, sizeof(net_rx_frame));
    if (n < ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN) return;
    if (net_rx_frame[12] != 0x08 || net_rx_frame[13] != 0x00) return;
    const u8* ip = net_rx_frame + ETH_HDR_LEN;
    if ((ip[0] & 0xF0) != 0x40 || ip[9] != 17) return;
    u32 dst_ip = *(u32*)&ip[16];
    if (dst_ip != net_local_ip && dst_ip != 0xFFFFFFFFu) return;
    u8 ihl = (ip[0] & 0x0F) * 4u;
    const u8* udp = ip + ihl;
    u16 udp_len = ((u16)udp[4] << 8) | udp[5];
    if (udp_len < UDP_HDR_LEN) return;
    udp_last_src_port = ((u16)udp[0] << 8) | udp[1];
    udp_last_src_ip = *(u32*)&ip[12];
    udp_last_len = udp_len - UDP_HDR_LEN;
    if (udp_last_len > NET_MTU) udp_last_len = NET_MTU;
    for (u32 i = 0; i < udp_last_len; ++i) udp_last_payload[i] = udp[UDP_HDR_LEN + i];
}

int udp_send(const u8* payload, u32 len, u32 dst_ip, u16 src_port, u16 dst_port) {
    if (len > NET_MTU - IP_HDR_LEN - UDP_HDR_LEN) return 0;
    u8* p = net_tx_frame;
    for (u32 i = 0; i < 6; ++i) p[i] = 0xFF;
    for (u32 i = 0; i < 6; ++i) p[6 + i] = net_local_mac[i];
    p[12] = 0x08; p[13] = 0x00;

    u8* ip = p + ETH_HDR_LEN;
    ip[0] = 0x45; ip[1] = 0;
    u16 tlen = IP_HDR_LEN + UDP_HDR_LEN + len;
    ip[2] = tlen >> 8; ip[3] = tlen & 0xFF;
    ip[4] = 0; ip[5] = 1; ip[6] = 0; ip[7] = 0;
    ip[8] = 64; ip[9] = 17; ip[10] = 0; ip[11] = 0;
    *(u32*)&ip[12] = net_local_ip;
    *(u32*)&ip[16] = dst_ip;
    u16 sum = ip_checksum_20(ip);
    ip[10] = sum >> 8; ip[11] = sum & 0xFF;

    u8* udp = ip + IP_HDR_LEN;
    udp[0] = src_port >> 8; udp[1] = src_port & 0xFF;
    udp[2] = dst_port >> 8; udp[3] = dst_port & 0xFF;
    u16 ulen = UDP_HDR_LEN + len;
    udp[4] = ulen >> 8; udp[5] = ulen & 0xFF;
    udp[6] = 0; udp[7] = 0;
    for (u32 i = 0; i < len; ++i) udp[8 + i] = payload[i];

    return rtl8139_send_packet(net_tx_frame, ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN + len);
}

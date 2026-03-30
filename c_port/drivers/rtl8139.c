#include "types.h"
#include "io.h"
#include "api.h"

u8 rtl8139_found;
static u16 rtl_iobase;
static u8 rtl_tx_index;
static u16 rtl_rx_offset;

#define RTL_RX_BUF_SIZE 8192u
#define RTL_TX_BUF_SIZE 2048u
static u8 rtl_rx_buffer[RTL_RX_BUF_SIZE + 16 + 1500] __attribute__((aligned(16)));
static u8 rtl_tx_buffer[4][RTL_TX_BUF_SIZE] __attribute__((aligned(16)));

static int rtl_detect(void) {
    for (u32 bus = 0; bus < 256; ++bus) for (u32 dev = 0; dev < 32; ++dev) for (u32 fn = 0; fn < 8; ++fn) {
        outl(0xCF8, 0x80000000u | (bus << 16) | (dev << 11) | (fn << 8));
        u32 vd = inl(0xCFC);
        if ((vd & 0xFFFFu) != 0x10ECu || ((vd >> 16) & 0xFFFFu) != 0x8139u) continue;
        outl(0xCF8, 0x80000000u | (bus << 16) | (dev << 11) | (fn << 8) | 0x10);
        rtl_iobase = (u16)(inl(0xCFC) & 0xFFFFFFFCu);
        return 1;
    }
    return 0;
}

static void rtl_hw_init(void) {
    outb(rtl_iobase + 0x52, 0);
    outb(rtl_iobase + 0x37, 0x10);
    while (inb(rtl_iobase + 0x37) & 0x10) {}
    outl(rtl_iobase + 0x30, (u32)rtl_rx_buffer);
    outb(rtl_iobase + 0x37, 0x0C);
    outl(rtl_iobase + 0x44, 0x0000E70F);
    outl(rtl_iobase + 0x40, 0x03000700);
    outw(rtl_iobase + 0x3C, 0x0005);
    outw(rtl_iobase + 0x3E, 0xFFFF);
    rtl_tx_index = 0;
    rtl_rx_offset = 0;
}

void net_init(void) {
    rtl8139_found = 0;
    if (!rtl_detect()) { api_print_string("RTL8139 not found.\n"); return; }
    rtl8139_found = 1;
    api_print_string("RTL8139 detected.\n");
    rtl_hw_init();
    api_print_string("RTL8139 initialized.\n");
}

int rtl8139_send_packet(const u8* packet, u32 len) {
    if (!rtl8139_found || !len || len > RTL_TX_BUF_SIZE) return 0;
    u8 idx = rtl_tx_index;
    for (u32 i = 0; i < len; ++i) rtl_tx_buffer[idx][i] = packet[i];
    outl(rtl_iobase + 0x20 + (idx * 4u), (u32)rtl_tx_buffer[idx]);
    outl(rtl_iobase + 0x10 + (idx * 4u), len);
    rtl_tx_index = (idx + 1u) & 3u;
    return 1;
}

u32 rtl8139_poll_receive(u8* dst, u32 max_len) {
    if (!rtl8139_found) return 0;
    if (!(inw(rtl_iobase + 0x3E) & 0x0001u)) return 0;
    outw(rtl_iobase + 0x3E, 0x0001u);

    u8* p = rtl_rx_buffer + rtl_rx_offset;
    u16 status = *(u16*)&p[0];
    if (!(status & 1u)) return 0;
    u32 pkt_len = *(u16*)&p[2];
    if (pkt_len < 4u) return 0;
    pkt_len -= 4u;
    if (pkt_len > max_len) pkt_len = max_len;
    for (u32 i = 0; i < pkt_len; ++i) dst[i] = p[4 + i];

    u32 adv = (*(u16*)&p[2]) + 4u;
    rtl_rx_offset = (u16)((rtl_rx_offset + ((adv + 3u) & ~3u)) & (RTL_RX_BUF_SIZE - 1u));
    outw(rtl_iobase + 0x38, (u16)(rtl_rx_offset - 16u));
    return pkt_len;
}

#include <rte_eal.h>
#include <rte_kvargs.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#define RX_RING_SIZE 256
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

bool continue_polling = true;
static const struct rte_eth_conf port_conf_default = {
	.rxmode = {
		.max_rx_pkt_len = ETHER_MAX_LEN,
	},
};

/*
 * Validate whether the current user has root privileges.
 */
bool is_user_root()
{
	if (getuid())
	{
		printf("Root privileges are required to run this program\nUsage: sudo ./capture\n");
		return false;
	}

	return true;
}

/*
 * Signal handler that stops the packet capture loop.
 */
void stop_polling()
{
	printf("\rInterrupt received, stopping live capture\n");
	continue_polling = false;
}

/* 
 * Initialize the receive descriptors for the given DPDK port.
 */
int port_init(uint8_t port, struct rte_mempool *mbuf_pool)
{
	int retval;
	uint16_t nb_txd = 0;
	uint16_t nb_rxd = RX_RING_SIZE;
	struct rte_eth_conf port_conf = port_conf_default;

	if (port >= rte_eth_dev_count_avail())
		return -1;

	retval = rte_eth_dev_configure(port, 1, 0, &port_conf);
	if (retval != 0)
		return retval;

	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if (retval != 0)
		return retval;

	printf("EAL: Using RX ring size of %u packets\n", nb_rxd);

	retval = rte_eth_rx_queue_setup(port, 0, nb_rxd, rte_eth_dev_socket_id(port), NULL, mbuf_pool);
	if (retval < 0)
		return retval;

	retval = rte_eth_dev_start(port);
	if (retval < 0)
		return retval;

	rte_eth_promiscuous_enable(port);
	return 0;
}

/*
 * Function to initialize the DPDK framework.
 */
struct rte_mempool *initialize_dpdk(int argc, char **argv, uint8_t port)
{
	// Initialize abstraction layer
	int ret = rte_eal_init(argc, argv);

	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	// Enumerate available ports
	uint nb_ports = rte_eth_dev_count_avail();

	if (nb_ports == 0)
		rte_exit(EXIT_FAILURE, "No assigned network interfaces found\n  Run \"sudo ./bind-nic-to-dpdk.sh\" to assign the host-only adapter to DPDK\n");

	// Allocate memory pool for packets
	struct rte_mempool *mbuf_pool;
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports, MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	// Initialize NIC receive port
	if (port_init(port, mbuf_pool) != 0)
		rte_exit(EXIT_FAILURE, "Cannot init port %u\n", port);

	return mbuf_pool;
}

/*
 * Display some basic information about the packet, like MAC and IP addresses.
 */
void process_packet(struct rte_mbuf *mbuf)
{
	u_char *p = rte_pktmbuf_mtod(mbuf, u_char *);
	uint16_t length = rte_pktmbuf_data_len(mbuf);
	printf("---------------------------\n");

	// Process MAC layer
	if (length < 14)
		return;

	printf("MAC src = %02x:%02x:%02x:%02x:%02x:%02x\n", p[6], p[7], p[8], p[9], p[10], p[11]);
	printf("MAC dst = %02x:%02x:%02x:%02x:%02x:%02x\n", p[0], p[1], p[2], p[3], p[4], p[5]);

	// Process IP layer, if any
	if (length < 34 || p[12] != 0x08 || p[13] != 0x00)
		return;

	printf("IP src = %u.%u.%u.%u\n", p[26], p[27], p[28], p[29]);
	printf("IP dst = %u.%u.%u.%u\n", p[30], p[31], p[32], p[33]);

	// Get IP payload protocol
	switch (p[23])
	{
	case 0x01:
		printf("ICMP payload\n");
		break;
	case 0x06:
		printf("TCP payload\n");
		break;
	case 0x11:
		printf("UDP payload\n");
		break;
	default:
		printf("Unknown payload\n");
	}
}

/*
 * Initialize the DPDK framework and start polling for received packets.
 */
int main(int argc, char **argv)
{
	// Set handler for catching CTRL+C interrupt
	signal(SIGINT, stop_polling);

	// Check privileges
	if (!is_user_root())
		return EXIT_FAILURE;

	// Initialize DPDK
	uint8_t port = 0;
	struct rte_mempool *mbuf_pool = initialize_dpdk(argc, argv, port);
	struct rte_mbuf *bufs[BURST_SIZE];

	// Main packet capture loop
	while (continue_polling)
	{
		// Poll for a new burst of packets
		uint16_t packet_count = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);

		for (uint buf = 0; buf < packet_count; buf++)
		{
			process_packet(bufs[buf]);

			// Free packet buffer after processing packet
			rte_pktmbuf_free(bufs[buf]);
		}
	}

	// Print capture statistics
	struct rte_eth_stats stats;
	if (rte_eth_stats_get(0, &stats) == 0)
		printf("Capture statistics: %lu received, %lu dropped\n", stats.ipackets, stats.imissed);

	return 0;
}

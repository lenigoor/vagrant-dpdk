#include <rte_eal.h>
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
	.rxmode = { .max_rx_pkt_len = ETHER_MAX_LEN, },
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

	if (port >= rte_eth_dev_count())
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

	retval  = rte_eth_dev_start(port);
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
	uint nb_ports = rte_eth_dev_count();

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
		uint16_t nb_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);

		if (nb_rx)
		{
			printf("Received %u packets\n", nb_rx);

			// Free packet buffer after processing packet
			for (uint buf = 0; buf < nb_rx; buf++)
				rte_pktmbuf_free(bufs[buf]);
		}
	}

	// Print capture statistics
	struct rte_eth_stats stats;
	if (rte_eth_stats_get(0, &stats) == 0)
		printf("Capture statistics: %lu received, %lu dropped\n", stats.ipackets, stats.imissed);

	return 0;
}

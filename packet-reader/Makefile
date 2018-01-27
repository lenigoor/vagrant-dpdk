LDFLAGS = -march=native -O2
INCLUDE = -I ${RTE_SDK}/build/include -L ${RTE_SDK}/build/lib
LIBS = -Wl,--whole-archive -lrte_pmd_e1000 -lrte_ethdev -lrte_eal -lrte_mbuf -lrte_mempool -lrte_mempool_ring -lrte_ring -lrte_hash -lrt -lm -ldl -lnuma -lpthread -Wl,--no-whole-archive

capture: main.c
	gcc $^ -o $@ $(LDFLAGS) $(INCLUDE) $(LIBS)

.PHONY: clean
clean:
	rm -f $(obj) capture
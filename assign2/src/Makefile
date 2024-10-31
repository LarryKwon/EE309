CC = gcc209
CFLAGS = -O3 -D NDEBUG -D_GNU_SOURCE

all: testheapmgrgnu testheapmgrkr testheapmgrbase testheapmgr1 testheapmgr1_gpt testheapmgr2

# Targets for building different versions of the heap manager
testheapmgrgnu: testheapmgr.c heapmgrgnu.c chunk.c
	$(CC) $(CFLAGS) $^ -o $@

testheapmgrkr: testheapmgr.c heapmgrkr.c chunk.c
	$(CC) $(CFLAGS) $^ -o $@

testheapmgrbase: testheapmgr.c heapmgrbase.c chunkbase.c
	$(CC) $(CFLAGS) $^ -o $@

testheapmgr1: testheapmgr.c heapmgr1.c chunk.c
	$(CC) $(CFLAGS) $^ -o $@

testheapmgr1_gpt: testheapmgr.c heapmgr1_gpt.c chunkbase.c
	$(CC) $(CFLAGS) $^ -o $@

testheapmgr2: testheapmgr.c heapmgr2.c chunk.c
	$(CC) $(CFLAGS) $^ -o $@

# Clean up generated files
clean:
	rm -f testheapmgrgnu testheapmgrkr testheapmgrbase testheapmgr1 testheapmgr2 *.o

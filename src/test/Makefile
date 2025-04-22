LEVEL1_DCACHE_SIZE = $(shell getconf -a | awk '/LEVEL1_DCACHE_SIZE/{print $$2;}')
LEVEL2_DCACHE_SIZE = $(shell getconf -a | awk '/LEVEL2_CACHE_SIZE/{print $$2;}')

CXXFLAGS=-O3 -march=native -std=c++17 -g -DNDEBUG -DLEVEL1_DCACHE_SIZE=$(LEVEL1_DCACHE_SIZE) -DLEVEL2_DCACHE_SIZE=$(LEVEL2_DCACHE_SIZE) -Wall -Wextra -I../include
LDFLAGS=-O3

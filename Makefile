SRCDIR := .
BLDDIR := build

all: $(BLDDIR)/simulate

$(BLDDIR):
	mkdir -p $(BLDDIR)

dSFMT-src-2.2.2.tar.gz:
	wget -N http://www.math.sci.hiroshima-u.ac.jp/~m-mat/bin/dl/dl.cgi?SFMT:dSFMT-src-2.2.2.tar.gz

dSFMT: dSFMT-src-2.2.2.tar.gz
	tar xzvf dSFMT-src-2.2.2.tar.gz
	mv dSFMT-src-2.2.2 dSFMT

$(BLDDIR)/simulate: dSFMT simulate.c
	$(CC) -Wall -O3 -std=gnu99 -msse2 -DHAVE_SSE2 -DDSFMT_MEXP=19937 -o simulate dSFMT/dSFMT.c simulate.c

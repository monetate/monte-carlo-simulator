dSFMT := dSFMT-src-2.2.2

all: simulate

$(dSFMT).tar.gz:
	wget -N http://www.math.sci.hiroshima-u.ac.jp/~m-mat/bin/dl/dl.cgi?SFMT:$(dSFMT).tar.gz

dSFMT: $(dSFMT).tar.gz
	tar xzvf $(dSFMT).tar.gz
	mv $(dSFMT) dSFMT

simulate: dSFMT simulate.c
	$(CC) -Wall -O3 -std=gnu99 -msse2 -DHAVE_SSE2 -DDSFMT_MEXP=19937 -o simulate dSFMT/dSFMT.c simulate.c

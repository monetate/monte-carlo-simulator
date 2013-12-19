dSFMT := dSFMT-src-2.2.2

all: simulate

$(dSFMT).tar.gz:
	wget http://www.math.sci.hiroshima-u.ac.jp/~m-mat/bin/dl/dl.cgi?SFMT:$(dSFMT).tar.gz -O $(dSFMT).tar.gz

dSFMT: $(dSFMT).tar.gz
	tar xzvf $(dSFMT).tar.gz
	mv $(dSFMT) dSFMT

simulate: dSFMT simulate.c
	$(CC) -Wall -O3 -std=gnu99 -msse2 -DHAVE_SSE2 -DDSFMT_MEXP=19937 -o simulate dSFMT/dSFMT.c simulate.c

functional_tests: all
	cd tests/; ./functional_tests.sh

test: functional_tests

sync:
	rsync -avt --delete --exclude=".git" --exclude="dSFMT*" . $(DEVBOX)

.PHONY : all test function_tests sync

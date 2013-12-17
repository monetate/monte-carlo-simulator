
```
rsync -avt -e "ssh -i /Users/chrisconley/.ssh/chrisconley-io.pem" --delete --exclude=".git" . core@$core_ip:/tmp/projects/coreos-test/
gcc -Wall -O3 -std=gnu99 -msse2 -DHAVE_SSE2 -DDSFMT_MEXP=19937 -o simulate dSFMT-src-2.2.2/dSFMT.c simulate.c
```

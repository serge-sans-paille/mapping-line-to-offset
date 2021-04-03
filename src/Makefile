PROG= ref seq seq_memchr bithack sse_align sse sse_memchr
CPPFLAGS=-std=c++14
CXXFLAGS=-O2

all:$(PROG)

check:$(PROG) ../test/sqlite3.c
	for p in $(PROG) ; do \
		printf "$$p: " 1>&2 ;\
		./$$p ../test/sqlite3.c > $$p.data ; \
		cmp $$p.data ref.data || echo "$$p invalid" ; \
	done

perf: $(PROG)
	for p in $(PROG) ; do \
		printf "$$p: " 1>&2 ;\
		for _ in `seq 100` ; do \
		  ./$$p ../test/sqlite3.c > /dev/null ;\
		done 2>&1 | awk 'BEGIN { s=0; } { s+=$$1; } END { print(s/NR); }' ; \
	done

clean:
	$(RM) *.data
	$(RM) $(PROG)

PROG= ref seq seq_memchr bithack bithack_scan sse_align sse sse_memchr
CPPFLAGS=-std=c++14
CXXFLAGS=-O2

all:$(PROG)

check:$(PROG) ../test/sqlite3.c ../test/sqlite3_r.c ../test/sqlite3_rn.c
	for t in ../test/sqlite3.c ../test/sqlite3_r.c ../test/sqlite3_rn.c; do \
		echo "= $$t" ;\
		for p in $(PROG) ; do \
			printf "$$p: " 1>&2 ;\
			./$$p $$t > $$p.`basename $$t .c`.data ; \
			cmp $$p.`basename $$t .c`.data ref.`basename $$t .c`.data || echo "$$p invalid" ; \
		done \
	done

perf: $(PROG)
	for t in ../test/sqlite3.c ../test/sqlite3_r.c ../test/sqlite3_rn.c; do \
		echo "= $$t" ;\
		for p in $(PROG) ; do \
			printf "$$p: " 1>&2 ;\
			for _ in `seq 100` ; do \
			  ./$$p ../test/sqlite3.c > /dev/null ;\
			done 2>&1 | awk 'BEGIN { s=0; } { s+=$$1; } END { print(s/NR); }' ; \
		done \
	done

clean:
	$(RM) *.data
	$(RM) $(PROG)

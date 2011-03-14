OBJECTS=patch_point.o
CFLAGS=-g -O0 -DWNAZI

all: libpatch_point.a

test: libpatch_point.a
	cd tests; ./test-suite

libpatch_point.a: ${OBJECTS}
	ar r $@ ${OBJECTS}

clean:
	rm -f ${OBJECTS} libpatch_point.a

.PHONY: test clean
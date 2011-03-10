OBJECTS=patch_point.o

all: libpatch_point.a

tests: libpatch_point.a
	cd tests; ./test-suite

libpatch_point.a: ${OBJECTS}
	ar r $@ ${OBJECTS}


.PHONY: tests
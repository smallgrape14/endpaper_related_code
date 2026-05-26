.PHONY: clean

CFLAGS  := -Wall -g
LDLIBS  := ${LDLIBS} -lrdmacm -libverbs -lpthread

APPS    := client

all: ${APPS}


clean:
	rm -f ${APPS}


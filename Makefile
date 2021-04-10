all: myping sniffer
myping: myping.o
	gcc -o myping myping.o
myping.o: myping.cpp
	gcc -Wall -g -c myping.cpp
sniffer: sniffer.o
	gcc -o sniffer sniffer.o
sniffer.o: sniffer.c
	gcc -Wall -g -c sniffer.c
.PHONY: all clean

clean:
	rm -f *.o *.a sniffer myping

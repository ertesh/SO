all: gather sim-cat map-cat mpx-blocks mpx-nonblock mpx-select nodes
 
gather.o: gather.c 
	gcc -Wall -c gather.c

map-cat.o: map-cat.c 
	gcc -Wall -c map-cat.c

sim-cat.o: sim-cat.c 
	gcc -Wall -c sim-cat.c

mpx-blocks.o: mpx-blocks.c 
	gcc -Wall -c mpx-blocks.c

mpx-nonblock.o: mpx-nonblock.c 
	gcc -Wall -c mpx-nonblock.c

mpx-select.o: mpx-select.c 
	gcc -Wall -c mpx-select.c

gather: gather.o 
	gcc -Wall -o gather gather.o 

map-cat: map-cat.o 
	gcc -Wall -o map-cat map-cat.o 

sim-cat: sim-cat.o 
	gcc -Wall -o sim-cat sim-cat.o 

mpx-blocks: mpx-blocks.o 
	gcc -Wall -o mpx-blocks mpx-blocks.o 

mpx-nonblock: mpx-nonblock.o 
	gcc -Wall -o mpx-nonblock mpx-nonblock.o 

mpx-select: mpx-select.o 
	gcc -Wall -o mpx-select mpx-select.o 

nodes:
	[ -e p1 ] || mknod p1 p
	[ -e p2 ] || mknod p2 p

clean:
	rm -f *.o *~ gather sim-cat map-cat mpx-blocks mpx-nonblock mpx-select p1 p2

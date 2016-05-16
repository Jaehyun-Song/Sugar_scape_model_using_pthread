sugar: sugar.c lcgrand.o simulation.o
	gcc -o sugar sugar.c lcgrand.o simulation.o -lpthread
lcgrand.o: lcgrand.c lcgrand.h
	gcc -c lcgrand.c lcgrand.h
simulation.o: simulation.c simulation.h
	gcc -c simulation.c simulation.h -lpthread

clean:
	rm *.o
	rm *.gch

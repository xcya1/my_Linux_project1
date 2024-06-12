objs = main.o server.o data_base.o

app: $(objs)
	gcc -o app $(objs) -lsqlite3;
		
main.o:main.c
	gcc -c $< -o $@

server.o:server.c
	gcc -c $< -o $@

database.o:data_base.c
	gcc -c data_base.c -o data_base.o 

clean:
	rm *.o
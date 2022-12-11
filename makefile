all:
	g++ -c -Wall ./Sources/server.cpp -o ./Exec/server.o
	g++ -c -Wall ./Sources/book.cpp -o ./Exec/book.o
	g++ -c -Wall ./Sources/user.cpp -o ./Exec/user.o
	g++ -o ./Exec/libraryServer ./Exec/server.o ./Exec/book.o ./Exec/user.o
clean:
	rm -f ./Exec/libraryServer ./Exec/server.o ./Exec/book.o ./Exec/user.o
run:
	./Exec/libraryServer
all:
	g++ -g compiler.cpp lexer.cpp main.cpp -o main

clean:
	rm -f main

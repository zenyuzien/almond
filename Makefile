all:
	g++ -g compiler.cpp lexer.cpp parser.cpp main.cpp -o main

clean:
	rm -f main

all:
	g++ -g src/compiler.cpp src/lexer.cpp src/node.cpp src/parser.cpp src/main.cpp -o main
lexer:
	g++ -g src/compiler.cpp src/lexer.cpp src/main.cpp -o lexer
clean:
	rm -f main




all:
	g++ *.h parser.cpp shell.cpp -o edward

clean:
	rm -f edward *o

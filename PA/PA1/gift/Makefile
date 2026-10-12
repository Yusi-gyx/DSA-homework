main: main.cpp
	g++ main.cpp -o main

debug: main.cpp
	g++ -g main.cpp -o main

check: 
	g++ -g -fsanitize=address main.cpp -o main

run: 
	./main < input.in > output.out

clean:
	rm main

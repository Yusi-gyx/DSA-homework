main: main.cpp
	g++ main.cpp -o main

debug: main.cpp
	g++ -g main.cpp -o main.cpp

clean: 
	rm main

run: 
	./main < input.in > output.out 
	diff output.out answer.out
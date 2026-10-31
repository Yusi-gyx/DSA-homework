main: main.cpp
	g++ main.cpp -o main

clean:
	rm main

run: 
	./main < input.in > output.out
	diff output.out answer.out

debug: main.cpp
	g++ -fsanitize=address -g -O0 main.cpp -o main
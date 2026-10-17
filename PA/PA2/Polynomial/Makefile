main: main.cpp
	g++ -O2 main.cpp -o main

run: main
	./main < input.in > output.out
	diff output.out answer.out

clean:
	rm main

debug: main.cpp
	g++ -g main.cpp -o main
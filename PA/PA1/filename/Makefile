main: filename_stripDP.cpp
	g++ filename_stripDP.cpp -o main

debug: filename_stripDP.cpp
	g++ filename_stripDP.cpp -g -o main

run: 
	./main < input.in > output.out

sanitize: filename_stripDP.cpp
	g++ filename_stripDP.cpp -o main -g -fsanitize=address

clean:
	rm main
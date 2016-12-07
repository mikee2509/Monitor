run: main.cpp monitor.h main.h iterable_queue.h
	g++ -std=c++11 -pthread main.cpp -o run

clean:
	rm run
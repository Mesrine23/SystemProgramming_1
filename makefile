all: compile

compile:
	g++ worker.cpp -o worker
	g++ listener.cpp -o listener
	g++ sniffer.cpp -o sniffer

worker:
	g++ worker.cpp -o worker

listener:
	g++ listener.cpp -o listener

sniffer:
	g++ sniffer.cpp -o sniffer

worker:
	g++ worker.cpp -o worker

clean:
	rm worker sniffer listener *.out
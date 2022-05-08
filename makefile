all: compile

compile:
	g++ worker.cpp -o worker
	g++ sniffer.cpp -o sniffer

clean:
	rm worker sniffer *.out ./named_pipes/* ./out_files/*
all: clean compile

compile:
	g++ worker.cpp -o worker
	g++ sniffer.cpp -o sniffer

clean:
	rm -f worker sniffer ./named_pipes/* ./out_files/*
	
move:
	mv *.txt ..

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

#define SIZE 50000

int main(int argc, char *argv[]) {

    char path[256];
    if(argc==1) 
        getcwd(path,256);
    else
        strcpy(path,argv[1]);

    int named_pipe;
    string temp1 = "./named_pipes/" + to_string(getpid());
    char work_pipe[25];
    strcpy(work_pipe,temp1.c_str());

    // Worker opens his named pipe to communicate with Manager
    if((named_pipe=open(work_pipe,O_RDONLY))==-1) {
        perror("{Worker} cannot open named pipe");
        exit(1);
    }

    int while_counter=0;

    // Worker enters an infinite loop that will stop only when he catches a SIGTERM signal from Manager
    while(1) {
        int ret1;
        char test[25] = {};
        ret1 = read(named_pipe, test, sizeof(test) - 1);        // Worker reads the .txt file that Manager gives him

        int fd, fd1, ret;
        char buffer[SIZE];
        string http = "http://";
        string temp = test;
        string writefile = "out_files/" + temp;
        string readfile = "./";
        if(argc==1)
            readfile = ".";
        else
            readfile = path;

        readfile.append("/");
        readfile.append(temp);
        writefile.append(".out");
        char file[readfile.length() + 1]={};
        strcpy(file, readfile.c_str());

        if ((fd = open(file, O_RDONLY, 0)) < 0) {               // Worker opens the .txt file
            perror("open() failed for 'text' file");
            exit(1);
        }

        ret = read(fd, buffer, sizeof(buffer) - 1);             // Worker reads the content of the .txt file and writes it in a buffer 
        buffer[ret] = 0x00;

        if (close(fd) != 0)
            perror("close() failed for 'text' file");

        vector <string> data;
        vector<string>::iterator it_str;
        vector<int> data_counter;

        istringstream my_str(buffer);
        string word;
        while (my_str >> word) {                                // Worker reads word-by-word from the buffer
            string first_seven = word.substr(0, 7);
            if (first_seven == http) {                          // If ' http:// ' is found at the very beginning of the word then Worker erases it and he continues parsing
                if (word[word.size() - 1] == '.' || word[word.size() - 1] == ',')
                    word.pop_back();
                word.erase(0, 7);
                string rest_four = word.substr(0, 4);
                if (rest_four == "www.") {                      // If ' www. ' is found then Worker erases it and continues parsing
                    word.erase(0, 4);
                }

                char temp[word.length() + 1]={};
                strcpy(temp, word.c_str());
                char *test = strchr(temp, '/');                 // If ' / ' is found after TLD then Worker erases '/' and everything after it
                if (test != NULL) {
                    int rem = test - temp + 1;
                    word.erase(rem - 1, word.length() - (rem - 1));
                }

                int index;
                it_str = find(data.begin(), data.end(), word);
                if (it_str != data.end()) {                     // If location found already exists in vector, Worker simply increases location's counter by 1
                    index = it_str - data.begin();
                    data_counter[index]++;
                } else {                                        // If location doesn't exist, Worker adds it at the end of the vector and sets its counter to 1
                    data.push_back(word);
                    data_counter.push_back(1);
                }
            }
        }
        string s = "";
        // In order to write in the .out file, Worker creates a string that contains all info : location num_of_appearance\n
        for (int i = 0; i < data.size(); ++i) {
            s += data[i] + " " + to_string(data_counter[i]) + "\n";
        }
        s += "\0";
        char file1[writefile.length() + 1]={};
        strcpy(file1, writefile.c_str());
        if ((fd1 = open(file1, O_WRONLY | O_CREAT, 0644)) < 0)  // Worker creates and open the .out file, where he will write the info collected
            perror("open() failed for 'out' file");

        char write_text[s.length() + 1]={};
        strcpy(write_text, s.c_str());
        write(fd1, write_text, strlen(write_text));             // Worker writes all the info

        if (close(fd1) != 0)                                    // Worker closes the .out file
            perror("close() failed for 'out' file");

        raise(SIGSTOP);                                         // Worker sends SIGSTOP signal to himself and waits till the Manager SIGCONT him
    }

}
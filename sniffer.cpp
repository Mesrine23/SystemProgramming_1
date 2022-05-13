#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <queue>
#include <bits/stdc++.h>
#include <sys/wait.h>
#include <deque>
#define SIZE 4096
using namespace std;

queue<int> available_workers;
deque<string> file_descriptors;
int listen;
int worker_counter=0;

// SIGCHLD handler : collects worker pid and adds him to the available workers
void sigCHLD_handler (int arg) {
    while (true) {
        pid_t child = waitpid(-1, NULL, WUNTRACED | WNOHANG);
        if (child <= 0) break;
        available_workers.push(child);
    }
}

// SIGINT handler: collects all pids (workers+listener) and terminates them
// Also prints some info.
void sigINT_handler (int arg) {
    cout << endl;
    cout << "Must terminate " << worker_counter << " workers." << endl;
    while(worker_counter!=0) {
        if(available_workers.empty())
            continue;
        int worker = available_workers.front();
        available_workers.pop();
        kill(SIGTERM,worker);
        cout << "Worker terminated!" << endl;
        --worker_counter;
    }
    cout << "Terminated all workers." << endl;
    kill(SIGTERM,listen);
    cout << "Listener terminated!" << endl;
    cout << "{Manager} finally exiting...\n" << endl;
    exit(0);
}

int main(int argc, char *argv[]) {

    // reading [-p path] optional
    char path[256];
    if(argc==1) 
        getcwd(path,256);
    else if(argc==3)
        strcpy(path,argv[2]);
    else {
        perror("Wrong arguments.");
        exit(1);
    }

    // creating unnamed pipe between Manager and Listener
    int listener[2];
    if (pipe(listener) == -1) {
        perror("Couldn't open listener pipe");
        exit(1);
    }

    int pid;
    if ((pid = fork()) < 0) {
        perror("fork~ {Manager -> Listener}");
        exit(1);
    } else if (pid == 0) {  // Listener
        cout << "{Listener}pid ~ " << getpid() << endl << endl;
        listen = getpid();
        if (close(listener[0]) != 0) {                          // Listener closes read-end of unnamed pipe
            perror("Couldn't close 'read'-end of listener pipe {Listener}");
            exit(1);
        }
        if (dup2(listener[1], 1) == -1) {                       // Listener "closes" stdout, so the main output stream is write-end of pipe.
            perror("dup2() failed");
            exit(1);
        }
        // inotifywait -q -m -e moved_to --format %f [path]
        char *args[] = {(char *) "inotifywait", (char *) "-q", (char *) "-m", (char *) "-e", (char *) "moved_to",
                        (char *) "--format", (char *) "%f", path, NULL};
        execvp(args[0], args);                                  // only if execvp fails, perror and exit is executed.
        perror("execvp {Listener} failed");
        exit(1);
    } else {    // Manager
        if (close(listener[1]) != 0) {                          // Manager closes write-end of unnamed pipe
            perror("Couldn't close 'write'-end of listener pipe {Manager}");
            exit(1);
        }

        signal(SIGCHLD,sigCHLD_handler);                        // Manager initializes handler of SIGCHLD signal
        signal(SIGINT,sigINT_handler);                          // Manager initializes handler of SIGINT signal
        string fifo = "./named_pipes/";

        while (1) {                                             // Infinite loop, terminated only when user gives ctrl+c (SIGINT)
            char buffer[SIZE] = {};
            string tokens[100];
            read(listener[0], buffer, sizeof(buffer) - 1);      // Manager reads one or more files at a time that Listener catches
            string buf = (string) buffer;
            size_t pos = 0;
            int counter = 0;
            while ((pos = buf.find('\n')) != string::npos) {    // Manager reads one-by-one file names and stores them in a string array
                if (counter == 100) {
                    perror("{Manager} too many files in pipe");
                    exit(1);
                }
                tokens[counter] = buf.substr(0, pos);
                counter++;                                      // counter++ -> Manager holds info of how many files were read
                buf.erase(0, pos + 1);
            }
            int available;

            // Manager holds the info of how many are the available workers at this exact time
            if(available_workers.empty())                
                available=0;
            else
                available = available_workers.size();   

            // There are 2 cases
            // 1) available Workers are more OR equal than the files
            // 2) available Workers are less than the files

            // If (1) then Manager "wakes up" the 'Stopped' Workers and passes them the files needed
            if(available>=counter) {
                for(int i=0 ; i < counter ; ++i){
                    int fd;
                    int worker = available_workers.front();
                    available_workers.pop();
                    kill(worker,SIGCONT);
                    for (auto &s: file_descriptors){
                        size_t pos = 0;
                        pos = s.find("|");
                        int temp_worker = atoi(s.substr(0,pos).c_str());
                        if (temp_worker!=worker) continue;
                        s.erase(0,pos+1);
                        fd = atoi(s.c_str());
                    }
                    string temp = "./named_pipes/" + to_string(worker);
                    char work_pipe[25] = {};
                    strcpy(work_pipe, temp.c_str());
                    write(fd, tokens[i].c_str(), tokens[i].length());
                }
            }
            // If (2) then Manager creates the Workers needed and "wakes up" the 'Stopped' ones
            else{
                int tok = 0;
                for (int i = 0; i < counter-available; ++i) {               
                    int worker;
                    if ((worker = fork()) < 0) {
                        perror("fork");
                        exit(1);
                    } else if (worker == 0) {                       // Worker execvp himself. If execvp fails, perror and exit is executed.
                        if(argc==1) {
                            char *args[] = {(char *) "./worker", NULL};
                            execvp(args[0], args);
                        } 
                        else {
                            char *args[] = {(char *) "./worker", path, NULL};
                            execvp(args[0], args);
                        }
                        perror("execvp for worker failed");
                        exit(1);
                    } else {                                        // Manager creates new Workers, creates their named pipes and passes them a file to handle
                        ++worker_counter;
                        string name = fifo + to_string(worker);
                        char work_pipe[25] = {};
                        strcpy(work_pipe, name.c_str());
                        if (mkfifo(work_pipe, 0666) == -1) {
                            perror("mkfifo");
                            exit(1);
                        }
                        int fd;
                        if ((fd = open(work_pipe, O_WRONLY)) == -1) {
                            perror("{Manager} can't open named pipe");
                            exit(1);
                        }
                        write(fd, tokens[tok].c_str(), tokens[tok].length());
                        ++tok;
                        file_descriptors.push_back(to_string(worker) + "|" + to_string(fd));    // Manager holds info : pid|file_descriptor in order to know the file descriptor of each Worker
                    }
                }
                for(int i=0 ; i < available ; ++i){                 // Manager, after creates some Workers, "wakes up" the rest ones that are needed and passes them a file to handle
                    int fd;
                    int worker = available_workers.front();
                    available_workers.pop();
                    kill(worker,SIGCONT);
                    for (auto &s: file_descriptors){                // Manager searches the correct file descriptor of each Worker
                        size_t pos = 0;
                        pos = s.find("|");
                        int temp_worker = atoi(s.substr(0,pos).c_str());
                        if (temp_worker!=worker) continue;
                        s.erase(0,pos+1);
                        fd = atoi(s.c_str());
                    }

                    string temp = "./named_pipes/" + to_string(worker);
                    char work_pipe[25] = {};
                    strcpy(work_pipe, temp.c_str());
                    write(fd, tokens[tok].c_str(), tokens[tok].length());
                    ++tok;
                }
            }
        }
    }
}

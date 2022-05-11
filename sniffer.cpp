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

void sigCHLD_handler (int arg) {
    while (true) {
        pid_t child = waitpid(-1, NULL, WUNTRACED | WNOHANG);
        if (child <= 0) break;
        available_workers.push(child);
    }
}

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
    cout << "{Manager} finally exiting..." << endl;
    exit(0);
}

int main() {
    int listener[2];
    if (pipe(listener) == -1) {
        perror("Couldn't open listener pipe");
        exit(1);
    }

    int pid;
    if ((pid = fork()) < 0) {
        perror("fork~ {Manager -> Listener}");
        exit(1);
    } else if (pid == 0) {
        cout << "{Listener}pid ~ " << getpid() << endl << endl;
        listen = getpid();
        if (close(listener[0]) != 0) {
            perror("Couldn't close 'read'-end of listener pipe {Listener}");
            exit(1);
        }
        if (dup2(listener[1], 1) == -1) {
            perror("dup2() failed");
            exit(1);
        }
        char *args[] = {(char *) "inotifywait", (char *) "-q", (char *) "-m", (char *) "-e", (char *) "moved_to",
                        (char *) "--format", (char *) "%f", (char *) "../sys_pro/", NULL};
        execvp(args[0], args);
        perror("execvp {Listener} failed");
        exit(1);
    } else {
        if (close(listener[1]) != 0) {
            perror("Couldn't close 'write'-end of listener pipe {Manager}");
            exit(1);
        }

        signal(SIGCHLD,sigCHLD_handler);
        signal(SIGINT,sigINT_handler);
        string fifo = "./named_pipes/";

        while (1) {
            cout << endl << "new loop" << endl << endl;
            char buffer[SIZE] = {};
            string tokens[100];
            read(listener[0], buffer, sizeof(buffer) - 1);
            cout << endl << "Manager received:" << endl << buffer << endl;
            string buf = (string) buffer;
            size_t pos = 0;
            int counter = 0;
            while ((pos = buf.find('\n')) != string::npos) {
                if (counter == 100) {
                    perror("{Manager} too many files in pipe");
                    exit(1);
                }
                tokens[counter] = buf.substr(0, pos);
                counter++;
                buf.erase(0, pos + 1);
            }
            int available;
            if(available_workers.empty())
                available=0;
            else
                available = available_workers.size();
            //cout << "AVAILABLE: " << available << endl;
            if(available>=counter) {
                //cout << "ENTERING: available>=counter" << endl;
                for(int i=0 ; i < counter ; ++i){
                    int fd;
                    int worker = available_workers.front();
                    available_workers.pop();
                    //cout << "I WILL CONTINUE!" << endl;
                    kill(worker,SIGCONT);
                    //cout << "HE GOT CONTINUED!" << endl;
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
                    cout << "To the raising worker " << getpid() << ": " << tokens[i] << endl;
                    write(fd, tokens[i].c_str(), tokens[i].length());
                }
            }
            else{
                //cout << "ENTERING: available<counter" << endl;
                int tok = 0;
                for (int i = 0; i < counter-available; ++i) {
                    //cout << "CREATING NEW ONE" << endl;
                    int worker;
                    if ((worker = fork()) < 0) {
                        perror("fork");
                        exit(1);
                    } else if (worker == 0) {
                        char *args[] = {(char *) "./worker", NULL};
                        execvp(args[0], args);
                        perror("execvp for worker failed");
                        exit(1);
                    } else {
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
                        //cout << "To new worker: " << tokens[tok] << endl;
                        write(fd, tokens[tok].c_str(), tokens[tok].length());
                        ++tok;
                        file_descriptors.push_back(to_string(worker) + "|" + to_string(fd));
                    }
                }
                for(int i=0 ; i < available ; ++i){
                    //cout << "USING AVAILABLE ONES" << endl;
                    int fd;
                    int worker = available_workers.front();
                    available_workers.pop();
                    cout << "I WILL CONTINUE: " << worker << endl;
                    kill(worker,SIGCONT);
                    //cout << "HE GOT CONTINUED!" << endl;
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
                    //cout << "To the available worker " << getpid() << ": " << tokens[tok] << endl;
                    write(fd, tokens[tok].c_str(), tokens[tok].length());
                    ++tok;
                }
            }
        }
    }
}

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

using namespace std;

#define SIZE 4096
#define LOCKFILE "./text.txt"

int main() {
    int listener[2];
    if (pipe(listener)==-1){
        perror("Couldn't open listener pipe");
        exit(1);
    }

    int pid;
    if((pid=fork())<0){
        perror("fork~ {Manager -> Listener}");
        exit(1);
    }
    else if (pid==0){
        cout << "{Listener}pid ~ " << getpid() << endl << endl;
        if (close(listener[0])!=0){
            perror("Couldn't close 'read'-end of listener pipe {Listener}");
            exit(1);
        }
        if (dup2(listener[1],1) == -1){
            perror("dup2() failed");
            exit(1);
        }
        char* args[] = {(char*)"inotifywait", (char*)"-q", (char*)"-m", (char*)"-e", (char*)"moved_to", (char*)"--format", (char*)"%f", (char*)"../sys_pro/", NULL};
        execvp(args[0], args);
        perror("execvp {Listener} failed");
        exit(1);
    }
    else
    {
        if (close(listener[1])!=0){
            perror("Couldn't close 'write'-end of listener pipe {Manager}");
            exit(1);
        }

        int pipe_counter=0;
        queue<int> available_workers;
        string fifo = "./named_pipes/fifo";

        while(1) {
            cout << "new loop" << endl << endl;
            char buffer[SIZE]={};
            string tokens[100];
            read(listener[0], buffer, sizeof(buffer) - 1);
            cout << "Manager received:" << endl << buffer << endl;
            string buf = (string) buffer;
            size_t pos = 0;
            cout << "~TOKENS~" << endl ;
            int counter = 0;
            while ( (pos=buf.find('\n')) != string::npos ){
                if (counter==100){
                    perror("{Manager} too many files in pipe");
                    exit(1);
                }
                tokens[counter] = buf.substr(0,pos);
                cout << ++counter << ") " << tokens[counter-1] << endl ;
                buf.erase(0, pos+1);
            }
            for(int i=0 ; i<counter ; ++i){
                pipe_counter++;
                string name = fifo + to_string(pipe_counter);
                char work_pipe[20];
                strcpy(work_pipe,name.c_str());
                cout << "{Manager} name of pipe: " << work_pipe << endl;
                if(mkfifo(work_pipe,0666)==-1){
                    perror("mkfifo");
                    exit(1);
                }
                cout << "{Manager} created pipe" << endl;
                int worker;
                if((worker=fork())<0){
                    perror("fork for worker failed");
                    exit(1);
                }
                else if(worker == 0){
                    char* args[] = {(char*)"./worker", (char*)work_pipe, NULL};
                    execvp(args[0],args);
                    perror("execvp for worker failed");
                    exit(1);
                }
                else{
                    int fd;
                    if((fd= open(work_pipe,O_WRONLY))==-1){
                        perror("{Manager} can't open named pipe");
                        exit(1);
                    }
                    write(fd,tokens[i].c_str(),tokens[i].length());
                    //sleep(2);
                    wait(NULL);
                    if (close(fd)!=0){
                        perror("{Manager} can't close named pipe");
                        exit(1);
                    }
                }
            }
            //cout << endl;
            sleep(1);
            //exit(1);
        }
    }
}
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
#include <algorithm>
#include <signal.h>

using namespace std;

int test=0;

static void sig_handler (int arg) {
    while (true) {
        pid_t child = waitpid(-1, NULL, WUNTRACED|WNOHANG);
        if(child<=0) break;
        cout << "{sig_handler}: pid collected = " << child << endl;
        ++test;
        kill(child,SIGCONT);
    }
}

int main () {
    //string fifo = "./named_pipes/";
    string name = "./named_pipes/" + to_string(getpid());
    char work_pipe[20];
    //char test1[] = "12123213213";
    strcpy(work_pipe,name.c_str());
    if((mkfifo(work_pipe,0666))==-1)
    {
        perror("mkfifo");
        exit(1);
    }
    cout << "created named pipe" << endl;
    //int test3 = atoi(test1);
    //cout << test3 << endl;
    exit(0);

    int pid;
    if ((pid=fork())<0){
        perror("fork");
        exit(1);
    }
    else if(pid==0){
        cout << "{Child:" << getpid() << "}" << endl;
        while(1){
            cout << "{Child}: I will wait for 2 seconds and then i will SIGSTOP..." << endl;
            sleep(2);
            raise(SIGSTOP);
            cout << "{Child}: Parent SIGCONT'ed me!" << endl;
        }
    }
    else{
        signal(SIGCHLD,sig_handler);
        while(test!=5){
            sleep(3);
            cout << "{Parent}: test = " << test << endl;
        }
        cout << "{Parent}: i will kill my child." << endl;
        kill(pid,SIGTERM);
        cout << "{Parent}: child was killed." << endl;
        exit (0);
    }
}
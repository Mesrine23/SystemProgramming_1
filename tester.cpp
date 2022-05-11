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
    string s = "50865|6";
    size_t pos = 0;
    pos = s.find("|");
    int worker = atoi(s.substr(0,pos).c_str());
    s.erase(0,pos+1);
    int fd = atoi(s.c_str());
    cout << "worker: " << worker << endl;
    cout << "file desc: " << fd << endl;
    string file_desc = to_string(worker) + "|" + to_string(fd);
    cout << "string concat: " << file_desc << endl;
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
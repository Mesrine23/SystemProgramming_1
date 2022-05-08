#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <signal.h>

using namespace std;


int main () {
    int pid;

    if ((pid=fork())<0){
        perror("fork");
        exit(1);
    }
    else if(pid==0){
        cout << "{Child} I will wait for 3 seconds and then i will SIGSTOP..." << endl;
        sleep(3);
    }
    else{

    }
}
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
#include <experimental/filesystem>

using namespace std;


int main (int argc, char *argv[]) {

    char path[256];
    if(argc==1) 
        getcwd(path,256);
    else
        strcpy(path,argv[2]);

    cout << "Desirable path: " << path << endl;
    return 0;
}
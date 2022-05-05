#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <bits/stdc++.h>
#include <algorithm>

using namespace std;

#define SIZE 4096
#define LOCKFILE "./text.txt"

int main() {
    vector<string> test;
    vector<string>::iterator it;
    test.push_back("test1");
    test.push_back("test2");
    it = find(test.begin(),test.end(),"test2");
    if (it != test.end()) {
        int index = it - test.begin();
        cout << index << endl;
    }
}
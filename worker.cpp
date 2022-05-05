#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

#define SIZE 4096


int main() {
    int fd,fd1,ret;
    char buffer[SIZE];
    string http = "http://";
    string writefile = "test.txt";
    string readfile = "./";
    readfile.append(writefile);
    writefile.append(".out");
    char file[readfile.length()+1];
    strcpy(file,readfile.c_str());

    if ( (fd = open(file, O_RDONLY, 0)) < 0 )
        perror("open() failed for 'text' file");

    ret = read(fd, buffer, sizeof(buffer)-1);
    buffer[ret] = 0x00;
    printf("\n%s\n\n",buffer);

    if(close(fd)!=0)
        perror("close() failed for 'text' file");

    vector<string> data;
    vector<string>::iterator it_str;
    vector<int> data_counter;

    istringstream my_str(buffer);
    string word;
    while (my_str >> word) {
        string first_seven = word.substr(0,7);
        if (first_seven == http)
        {
            //cout << "URL before: " << word << endl;
            if(word[word.size()-1] == '.' || word[word.size()-1] == ',')
                word.pop_back();
            word.erase(0,7);
            string rest_four = word.substr(0,4);
            if(rest_four == "www.") {
                word.erase(0,4);
            }

            char temp[word.length()+1];
            strcpy(temp,word.c_str());
            char* test = strchr(temp,'/');
            if (test!=NULL){
                int rem = test-temp+1;
                word.erase(rem-1,word.length()-(rem-1));
            }

            int index;
            it_str = find(data.begin(), data.end(), word);
            if (it_str != data.end()){
                index = it_str - data.begin();
                data_counter[index]++;
            }
            else {
                data.push_back(word);
                data_counter.push_back(1);
            }
            //cout << "URL after: " << word << endl << endl;
        }
    }
    cout << endl;
    string s = "";
    for (int i=0 ; i<data.size() ; ++i) {
        s += data[i] + " " + to_string(data_counter[i]) + "\n";
        //cout << "url: '" << data[i] << "' found " << data_counter[i] << " times." << endl;
    }
    s += "\0";
    cout << s << endl << endl;
    //return 0;
    char file1[writefile.length()+1];
    strcpy(file1,writefile.c_str());
    if ( (fd1 = open(file1, O_WRONLY | O_CREAT, 0644)) < 0 )
        perror("open() failed for 'out' file");

    char write_text[s.length()+1];
    strcpy(write_text,s.c_str());
    write(fd1,write_text, strlen(write_text));

    if(close(fd1)!=0)
        perror("close() failed for 'out' file");

    return 0;
}
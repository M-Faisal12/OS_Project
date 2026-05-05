#include <iostream>
#include <cstring>

using namespace std;
struct Line
{
    int bytes;
    char data[50];
};
int main()
{
    char line[50];
    Line l;
    strcpy(l.data, "Hello,2,3,4,5,, ,");
    char *k = l.data;
    char *token = strtok(k, ",");
    char *catagory = token;
    token = strtok(nullptr, ",");

    int value = 0;
    while (token != nullptr)
    {
        value += atoi(token);
        token = strtok(nullptr, ",");
    }
    cout<<catagory<<" "<<value;
}
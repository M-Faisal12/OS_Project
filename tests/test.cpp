#include <stdio.h>
#include <string.h>
#include <iostream>
char** BuildPath(char* folder,char* data) //Send paths of files in form of array
{
    int length=0;
    int i=0;
    while(data[i])
    {
        if(data[i]==',')length++;
        i++;
    }
    char** Paths=new char*[length-1];
    for(int j=0;j<length;j++)
       Paths[j]=new char[30];
    i=0;
    for(int file=0;file<length;file++)
    {
        Paths[file][0]='.';
        Paths[file][1]='.';
        Paths[file][2]='/';
        int k=3;
        int index=0;
        while(folder[index])
        {
          Paths[file][k++]=folder[index++];
        }
        Paths[file][k++]='/';
       while(data[i])
       {
        if(data[i]==',')
        {
            i++;
            break;
        }
         Paths[file][k++]=data[i++];
       }
    }
    return Paths;
}
int main() 
{
    char *a="a.csv,b.csv,";
    char *b="data";
    char** paths=BuildPath(b,a);
    std::cout<<paths[0];
    // for(int i=0;i<strlen(token);i++)
    // {
    //     std::cout<<token[i];
    // }
return 0;
}
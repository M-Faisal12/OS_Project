#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <fstream>
#include <semaphore.h>
#include <sys/mman.h>
using namespace std;
char *Path(char *dir,char* file)
{
    char *arr = new char[20]{'\0'};
    strcpy(arr, dir);
    int l1 = strlen(file);
    int l2 = strlen(dir);
    arr[l2] = '/';
    for (int i = l2 + 1, j = 0; i < 20 && j < l1; i++, j++)
    {
        arr[i] = file[j];
    }
    return arr;
}
void Report(char *dir)
{
    char* f=(char*)"Report.txt";
    cout<<"In txt "<<Path(dir,f)<<endl;
    int fd = open(Path(dir,f), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        perror("open failed");
        exit(40);
    }
    write(fd,(char*)"Hello file",sizeof("Hello file"));
    int a=10000;
    write(fd,(char*)a,sizeof((char*)a));
    close(fd);
}

void CSV(char *dir)
{
    ofstream file(Path(dir,"Report.csv"));
    cout<<"In CSV "<<Path(dir,"Report.csv")<<endl;
    file << "Category,Total\n";
    for (int i = 0; i <3; i++)
    {
        file << "test" << "," << i << "\n";
    }
    file.close();
}
int main()
{
    Report("report");
}
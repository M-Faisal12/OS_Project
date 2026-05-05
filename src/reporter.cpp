#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <fstream>
#include "common/Table.h"
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>
using namespace std;
char *Path(char *dir, char *file)
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
void CSV(Table *data, char *dir)
{
    char *f = (char *)"Report.csv";
    ofstream file(Path(dir, f));
    cout << "In CSV " << Path(dir, f) << endl;
    file << "Category,Total\n";
    for (int i = 0; i < data->size(); i++)
    {
        cout << data->getrow()[i].Catagory << "," << data->getrow()[i].total << "\n";
        file << data->getrow()[i].Catagory << "," << data->getrow()[i].total << "\n";
    }
    file.close();
}

void Report(Table *data, char *dir)
{
    char *f = (char *)"Report.txt";
    cout << "In txt " << Path(dir, f) << endl;
    int fd = open(Path(dir, f), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        perror("open failed");
        exit(40);
    }

    int totall = 0;
    int highest = 0;
    int revenu = 0;
    Row *rows = data->getrow();
    for (int i = 0; i < data->size(); i++)
    {
        if (rows[highest].total < rows[i].total)
            highest = i;
        write(fd, rows[i].Catagory, strlen(rows[i].Catagory));
        write(fd, " revenue : ", strlen(" revenue : "));
        revenu += rows[i].total;
        string a = to_string(rows[i].total);
        write(fd, a.c_str(), a.length());
        write(fd, "\n", 1);
        totall++;
    }
    string a = to_string(totall);
    write(fd, "Total catagories : ", strlen("Total catagories : "));
    write(fd, a.c_str(), a.length());
    write(fd, "\n", 1);

    a = to_string(revenu);
    write(fd, "Total revenue : ", strlen("Total revenue : "));
    write(fd, a.c_str(), a.length());
    write(fd, "\n", 1);

    write(fd, "Highest revenue catagory : ", strlen("Highest revenue catagory : "));
    write(fd, rows[highest].Catagory, strlen(rows[highest].Catagory));
    write(fd, "\n", 1);

    a = to_string(rows[highest].total);
    write(fd, "Highest catagory revenue: ", strlen("Highest catagory revenue: "));
    write(fd, a.c_str(), a.length());
    write(fd, "\n", 1);
    close(fd);
    return;
    // int dupp = dup(STDOUT_FILENO);
    // if (dupp < 0)
    // {
    //     perror("dup failed");
    //     exit(40);
    // }
    // if (dup2(fd, STDOUT_FILENO) < 0)
    // {
    //     perror("dup2 failed");
    //     exit(40);
    // }
    // close(fd);
    // fflush(stdout);
    // dup2(dupp, STDOUT_FILENO);
    // close(dupp);
}
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Args too few !!!" << endl;
        exit(10);
    }
    char *shm = argv[1];
    char *semtable = argv[2];
    char *dir = argv[3];

    Table *readed = new Table;
    // Semaphore for table
    sem_t *semt = sem_open(semtable, 0);
    if (semt == SEM_FAILED)
    {
        perror("sem_open");
        exit(20);
    }
    int c;
    sem_getvalue(semt, &c);
    cout<<"Reporter pid= ["<<getpid()<<"]"<< " Parent pid= ["<<getppid()<<"] waiting for shared memory lock"<<endl;
    
    sem_wait(semt);
    sem_getvalue(semt, &c);
    cout<<"Reporter pid= ["<<getpid()<<"]"<< " Parent pid= ["<<getppid()<<"] aquired shared memory lock"<<endl;
    // Shared memory
    int fd = shm_open(shm, O_RDWR, 0666);
    if (fd == -1)
    {
        exit(20);
    }
    Table *ptr = (Table *)mmap(NULL, sizeof(Table), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        cerr << "Shared memory faled !!!\n";
        exit(20);
    }
    *readed = *ptr;
    CSV(readed, dir);
    Report(readed, dir);
    kill(getppid(), SIGUSR1);
    cout << "=================REPORTER===============" << endl;
    readed->print();
    close(fd);
    sem_close(semt);
    munmap(ptr, sizeof(Table));
    exit(0);
}
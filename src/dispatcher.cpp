#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstring>
#include <chrono>
#include <sys/mman.h>
#include <semaphore.h>
#include "common/Table.h"
using namespace std;

bool shutdown = false;
bool childend = false;
int alive = 3;

void handle1(int a)
{
  shutdown = 1;
}

void handle2(int a)
{
  childend = 1;
}

void handle3(int a)
{
  shutdown = 1;
  cout<<"Child called SIGUSR1"<<endl;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cout << "Args too few !!!" << endl;
    exit(10);
  }
  // Signal handling
  signal(SIGINT, handle1);
  signal(SIGTERM, handle1);
  signal(SIGCHLD, handle2);
  signal(SIGUSR1, handle3);
  // FIFO pipe
  char *pipe = (char *)"pipe1";
  mkfifo(pipe, 0666);
  // Shared memory
  char *name = (char *)"file";
  int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
  if (fd == -1)
  {
    exit(20);
  }
  ftruncate(fd, sizeof(Table));
  // Semaphore for aggregate table
  char *p = (char *)"aggtable";
  sem_unlink(p);
  sem_t *semtable = sem_open(p, O_CREAT, 0644, 0);
  if (semtable == SEM_FAILED)
  {
    perror("sem_open");
    exit(20);
  }

  pid_t pid2;
  pid_t pid3;
  pid_t pid1;
  chrono::high_resolution_clock::time_point start1;
  chrono::high_resolution_clock::time_point start2;
  chrono::high_resolution_clock::time_point start3;

  start1 = chrono::high_resolution_clock::now();

  pid1 = fork();
  if (pid1 == 0)
  {
    int logfd = open("logs/ingester.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(logfd, STDOUT_FILENO);
    dup2(logfd, STDERR_FILENO);
    close(logfd);

    char *args[] = {(char *)"./ingester", pipe, argv[1], argv[2], argv[3], NULL};
    execvp("./ingester", args);

    cerr << "Ingester exec failed !!!!\n";
    exit(1);
  }
  else if (pid1 > 0)
  {
    start2 = chrono::high_resolution_clock::now();

    pid2 = fork();
    if (pid2 == 0)
    {
      int logfd = open("logs/processor.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
      dup2(logfd, STDOUT_FILENO);
      dup2(logfd, STDERR_FILENO);
      close(logfd);

      char *args[] = {(char *)"./processor", pipe, argv[3], argv[4], name, p, NULL};
      execvp("./processor", args);

      cerr << "Processor exec falied !!!\n";
      exit(1);
    }
    else if (pid2 < 0)
    {
      cout << "Fork failed for processor process" << endl;
      exit(1);
    }
    else
    {
      start3 = chrono::high_resolution_clock::now();

      pid3 = fork();
      if (pid3 == 0)
      {
        int logfd = open("logs/reporter.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(logfd, STDOUT_FILENO);
        dup2(logfd, STDERR_FILENO);
        close(logfd);

        char *args[] = {(char *)"./reporter", name, p, argv[5], NULL};
        execvp("./reporter", args);

        cerr << "Reporter exec falied !!!\n";
        exit(1);
      }
      else if (pid3 < 0)
      {
        cout << "Fork failed for reporter process" << endl;
        exit(1);
      }
    }

    sigset_t mask, oldmask;
    sigfillset(&mask);
    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGTERM);
    sigdelset(&mask, SIGCHLD);
    sigdelset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    sigemptyset(&mask);

    while (!shutdown && alive > 0)
    {
      if (childend)
      {
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
          cout << "Child finished: " << pid << endl;
          alive--;
        }
        childend = 0;
      }
      if (!shutdown && alive > 0)
        sigsuspend(&mask);
    }

    cout << "Shutdown detected, cleaning up...\n";

    // Sending signal to terinate
    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    kill(pid3, SIGTERM);

    int status1;
    waitpid(pid1, &status1, 0);
    auto end1 = chrono::high_resolution_clock::now();
    chrono::duration<double> duration1 = end1 - start1;

    cout << "Ingester process [" << pid1 << "]  Execution time : [" << duration1.count() << "]";
    if (WIFEXITED(status1))
    {
      cout << " Exited with CODE : " << WEXITSTATUS(status1) << endl;
    }
    if (WIFSIGNALED(status1))
    {
      cout << "Child killed by signal: " << WTERMSIG(status1) << endl;
    }

    waitpid(pid2, &status1, 0);
    auto end2 = chrono::high_resolution_clock::now();
    chrono::duration<double> duration2 = end2 - start2;

    cout << "Processor process [" << pid2 << "]" << "  Execution time : [" << duration2.count() << "]";
    if (WIFEXITED(status1))
    {
      cout << " Exited with CODE : " << WEXITSTATUS(status1) << endl;
    }
    if (WIFSIGNALED(status1))
    {
      cout << "Child killed by signal: " << WTERMSIG(status1) << endl;
    }

    waitpid(pid3, &status1, 0);
    auto end3 = chrono::high_resolution_clock::now();
    chrono::duration<double> duration3 = end3 - start3;

    cout << "Reporter process [" << pid3 << "]" << "  Execution time : [" << duration3.count() << "]";
    if (WIFEXITED(status1))
    {
      cout << " Exited with CODE : " << WEXITSTATUS(status1) << endl;
    }
    if (WIFSIGNALED(status1))
    {
      cout << "Exited by signal: " << WTERMSIG(status1) << endl;
    }

    unlink(pipe);
    close(fd);
    sem_close(semtable);
    shm_unlink(name);
    sem_unlink(p);
    cout << "Cleanup done\n";
  }
  else
  {
    cout << "Fork failed for Ignester process" << endl;
    exit(1);
  }
  exit(0);
}
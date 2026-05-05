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
void handle1(int a)
{
  shutdown = 1;
}

void handle2(int a)
{
  childend = 1;
}
int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cout << "Args too few !!!" << endl;
    exit(10);
  }
  signal(SIGINT, handle1);
  signal(SIGTERM, handle1);
  signal(SIGCHLD, handle2);
  // FIFO pipe
  char *pipe = (char *)"pipe1";
  mkfifo(pipe, 0666);
  // Shared Memory
  char *name = (char *)"file";
  int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
  if (fd == -1)
  {
    exit(20);
  }
  ftruncate(fd, sizeof(Table));
  // Named Semaphore for processor-reporter communication
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
  // First child process for ingester
  chrono::high_resolution_clock::time_point start1;
  chrono::high_resolution_clock::time_point start2;
  chrono::high_resolution_clock::time_point start3;
  start1 = chrono::high_resolution_clock::now();
  pid1 = fork();
  if (pid1 == 0)
  {
    char *args[] = {(char *)"./ingester", pipe, argv[1], argv[2], argv[3], NULL};
    execvp("./ingester", args);
    cout << "Exec failed" << endl;
    exit(1);
  }
  else if (pid1 > 0)
  {
    // Second child process for processor
    start2 = chrono::high_resolution_clock::now();
    pid2 = fork();
    if (pid2 == 0)
    {
      char *args[] = {(char *)"./processor", pipe, argv[3], argv[4], name, p, NULL};
      execvp("./processor", args);
      cout << "Exec failed" << endl;
      exit(1);
    }
    else if (pid2 < 0)
    {
      cout << "Fork failed for processor process" << endl;
      exit(1);
    }
    else
    {
      // Third child process for reporter
      start3 = chrono::high_resolution_clock::now();
      pid3 = fork();
      if (pid3 == 0)
      {
        char *args[] = {(char *)"./reporter", name, p, NULL};
        execvp("./reporter", args);
        cout << "Exec failed" << endl;
        exit(1);
      }
      else if (pid3 < 0)
      {
        cout << "Fork failed for reporter process" << endl;
        exit(1);
      }
    }

    while (!shutdown)
    {
      if (childend)
      {
        int status;
        pid_t pid;

        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
          cout << "Child finished: " << pid << endl;
        }

        childend = 0;
      }

      sleep(1); // prevents CPU burn (important)
    }
    cout << "Shutdown detected, cleaning up...\n";

    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    kill(pid3, SIGTERM);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    unlink(pipe);
    close(fd);
    sem_close(semtable);
    shm_unlink(name);
    sem_unlink(p);
    cout << "Cleanup done\n";
    // int status1;
    // waitpid(pid1, &status1, 0);
    // auto end1 = chrono::high_resolution_clock::now();
    // chrono::duration<double> duration1 = end1 - start1;
    // cout << "Ingester process [" << pid1 << "]  Execution time : " << duration1.count();
    // if (WIFEXITED(status1))
    // {
    //   cout << " Exited with CODE : " << WEXITSTATUS(status1) << endl;
    // }
    // if (WIFSIGNALED(status1))
    // {
    //   cout << "Child killed by signal: " << WTERMSIG(status1) << endl;
    // }
    // waitpid(pid2, &status1, 0);
    // auto end2 = chrono::high_resolution_clock::now();
    // chrono::duration<double> duration2 = end2 - start2;
    // cout << "Processor process [" << pid2 << "]" << "  Execution time : " << duration2.count();
    // if (WIFEXITED(status1))
    // {
    //   cout << " Exited with CODE : " << WEXITSTATUS(status1) << endl;
    // }
    // if (WIFSIGNALED(status1))
    // {
    //   cout << "Child killed by signal: " << WTERMSIG(status1) << endl;
    // }
    // waitpid(pid3, &status1, 0);
    // auto end3 = chrono::high_resolution_clock::now();
    // chrono::duration<double> duration3 = end3 - start3;
    // cout << "Reporter process [" << pid3 << "]" << "  Execution time : [" << duration3.count() << "]";
    // if (WIFEXITED(status1))
    // {
    //   cout << " Exited with CODE : " << WEXITSTATUS(status1) << endl;
    // }
    // if (WIFSIGNALED(status1))
    // {
    //   cout << "Child killed by signal: " << WTERMSIG(status1) << endl;
    // }
  }
  else
  {
    cout << "Fork failed for Ignester process" << endl;
    exit(1);
  }

  exit(0);
}
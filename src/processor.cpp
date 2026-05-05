#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include "common/queue.h"
#include "common/Chunk.h"
#include "common/Table.h"
#include <semaphore.h>
#include <sys/mman.h>
#include <utility>
using namespace std;
pthread_mutex_t lock;
pthread_mutex_t tablelock;
sem_t emp;
sem_t full;
bool ReaderFinished = false;

void Aggregate(Table *t, char *cat, int val)
{
    pthread_mutex_lock(&tablelock);
    t->insert(cat, val);
    pthread_mutex_unlock(&tablelock);
    return;
}

void *processor(void *arg)
{
    cout << "Processor pid= [" << getpid() << "]" << " Parent pid= [" << getppid() << "] Worker Thread ID= " << pthread_self() << " starting data reading" << endl;

    pair<Table *, Queue<Chunk *> *> *data = (pair<Table *, Queue<Chunk *> *> *)arg;

    while (true)
    {
        sem_wait(&full);

        pthread_mutex_lock(&lock);
        Chunk *buffer = data->second->front();
        data->second->pop();
        if (buffer == nullptr)
        {
            cout << "Worker thread id : " << pthread_self() << " receiveing signal to stop " << endl;
            pthread_mutex_unlock(&lock);
            pthread_exit(nullptr);
        }
        cout << "Worker THREAD : " << pthread_self() << " Chunk id : " << buffer->ChunkID << "  FileID : " << buffer->fileid << "  Lines no : " << buffer->LinesNo << "  Data : " << buffer->lines[0].data << endl;
        pthread_mutex_unlock(&lock);
        for (int line = 0; line < buffer->LinesNo; line++)
        {
            char *l = buffer->lines[line].data;
            cout << "Aggregated line : " << l << endl;
            char *token = strtok(l, ",");
            char *catagory = token;
            token = strtok(nullptr, ",");

            int value = 0;
            while (token != nullptr)
            {
                cout << "Token : " << token << endl;
                value += atoi(token);
                token = strtok(nullptr, ",");
            }

            Aggregate(data->first, catagory, value);
        }
        delete[] buffer;
        sem_post(&emp);
    }
}

void *ReadThread(void *b)
{
    cout << "Processor pid= [" << getpid() << "]" << " Parent pid= [" << getppid() << "] Reader Thread ID= " << pthread_self() << " starting data reading" << endl;

    pair<pair<int, int>, Queue<Chunk *> *> *data = (pair<pair<int, int>, Queue<Chunk *> *> *)b;

    int a = data->first.first;
    Chunk *buffer = new Chunk;
    int bytes;
    while ((bytes = read(a, buffer, sizeof(*buffer))) > 0)
    {
        sem_wait(&emp);
        if (bytes != sizeof(*buffer))
        {
            cout << "Partial read\n";
            break;
        }
        if (buffer->ChunkID == -1)
        {
            cout << "Closing FIFO Pipe" << endl;
            close(a);
            break;
        }

        pthread_mutex_lock(&lock);
        data->second->push(buffer);
        cout << "Consumer : " << pthread_self() << " Chunk ID :" << buffer->ChunkID << "  FileID :" << buffer->fileid << "  Lines: " << buffer->LinesNo << endl;
        pthread_mutex_unlock(&lock);
        sem_post(&full);
        buffer = new Chunk;
    }
    delete buffer;
    pthread_mutex_lock(&lock);
    ReaderFinished = true;
    pthread_mutex_unlock(&lock);
    for (int i = 0; i < data->first.second; i++) // Workers shutdown
    {
        sem_wait(&emp);
        pthread_mutex_lock(&lock);

        cout << "Processor pid= [" << getpid() << "]" << " Parent pid= [" << getppid() << "] Reader Thread ID= " << pthread_self() << " sending signal to workers" << endl;

        data->second->push(nullptr);
        pthread_mutex_unlock(&lock);
        sem_post(&full);
    }
    return nullptr;
}
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "Args too few !!!" << endl;
        exit(10);
    }
    char *pipe = argv[1];
    int threads = atoi(argv[2]);
    int qsize = atoi(argv[3]);
    char *name = argv[4];
    char *semtable = argv[5];
    // Mutex initilization
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&tablelock, NULL);
    // Semaphore initialization
    sem_init(&full, 0, 0);
    sem_init(&emp, 0, qsize);
    // Pipe opening
    int a = open(pipe, O_RDONLY);
    if (a == -1)
    {
        cout << "Pipe not found !!!" << endl;
        exit(20);
    }

    Queue<Chunk *> *q = new Queue<Chunk *>(qsize);
    Table *agg = new Table;

    pair<pair<int, int>, Queue<Chunk *> *> *readerarg = new pair<pair<int, int>, Queue<Chunk *> *>;
    readerarg->first.first = a;
    readerarg->first.second = threads;
    readerarg->second = q;

    pair<Table *, Queue<Chunk *> *> *processorarg = new pair<Table *, Queue<Chunk *> *>[threads];
    for (int i = 0; i < threads; i++)
    {
        processorarg[i].first = agg;
        processorarg[i].second = q;
    }

    pthread_t reader;
    pthread_t workers[threads];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&reader, &attr, ReadThread, readerarg);
    for (int i = 0; i < threads; i++)
    {
        pthread_create(&workers[i], &attr, processor, (void *)&processorarg[i]);
    }
    pthread_join(reader, NULL);
    for (int i = 0; i < threads; i++)
    {
        pthread_join(workers[i], NULL);
    }
    // Named semaphore for aggreage table
    sem_t *semt = sem_open(semtable, 0);
    if (semt == SEM_FAILED)
    {
        perror("sem_open");
        exit(20);
    }
    // Shared memory
    int fd = shm_open(name, O_RDWR, 0666);
    if (fd == -1)
    {
        exit(20);
    }
    Table *ptr = (Table *)mmap(NULL, sizeof(Table), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        cout << "Hello bye !!!\n";
        exit(20);
    }
    *ptr = *agg; // Shared mem writing
    cout << endl;
    cout << "===================================\n";
    agg->print();
    sem_post(semt);

    delete q;
    delete agg;
    delete readerarg;
    delete[] processorarg;
    close(a);
    close(fd);
    sem_close(semt);
    munmap(ptr, sizeof(Table));
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&tablelock);
    sem_destroy(&emp);
    sem_destroy(&full);
    exit(0);
}
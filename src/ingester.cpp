#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "common/Chunk.h"
using namespace std;
bool interupted = false;
int filesdone = 0;
int bytessent = 0;
int chunksent = 0;
void Interupter(int a)
{
    interupted = true;
}
void UserHandler(int a)
{
    cerr << "[Producer]   Files Processed : " << filesdone << endl;
    cerr << "[Producer]   Chunks sent     : " << chunksent << endl;
    cerr << "[Producer]   Bytes sent      : " << bytessent << endl;
}

char **BuildPath(char *folder, char *data) // Send paths of files in form of array
{
    int length = 0;
    int i = 0;
    while (data[i])
    {
        if (data[i] == ',')
            length++;
        i++;
    }

    char **Paths = new char *[length];
    for (int j = 0; j < length; j++)
        Paths[j] = new char[50];

    i = 0;
    int file;
    for (file = 0; file < length; file++)
    {
        int k = 0;
        int index = 0;
        while (folder[index]) // Folder
        {
            Paths[file][k++] = folder[index++];
        }
        Paths[file][k++] = '/';

        while (data[i]) // File
        {
            if (data[i] == ',')
            {
                i++;
                break;
            }
            Paths[file][k++] = data[i++];
        }
    }
    Paths[file] = nullptr;
    return Paths;
}

void ReadData(int pipe, char *folder, char *files, int &threads)
{
    signal(SIGTERM, Interupter);
    signal(SIGUSR1, UserHandler);

    char **Paths = BuildPath(folder, files);
    cout<<"Ingester pid= ["<<getpid()<<"]"<< " Parent pid= ["<<getppid()<<"] Build Paths for files"<<endl;
    int chunkid = 1;

    for (int i = 0; Paths[i] != nullptr; i++)
    {
        int fd = open(Paths[i], O_RDONLY);
        if (fd == -1)
        {
            cout << "File not found: " << Paths[i] << endl;
            close(fd);
            continue;
        }

        char buffer;
        int count = 0;
        Line lines[1000];
        int l = 0;
        int byte;

        while ((byte = read(fd, &buffer, 1)) > 0)
        {
            if (interupted)
            {
                break;
            }
            if (buffer == '\n')
            {
                lines[count].data[l++] = buffer;
                lines[count].data[l] = '\0';

                lines[count].bytes = byte;
                bytessent += byte;

                count++;
                l = 0;
            }
            else
            {
                lines[count].data[l++] = buffer;
            }
        }
        close(fd);
        filesdone++;
        if (interupted)
        {
            break;
        }
        int ChunkSize = (count) / threads;
        l = 0;
        while (l != count)
        {
            if (interupted)
            {
                break;
            }
            Chunk c;
            c.ChunkID = chunkid++;
            c.LinesNo = 0;
            strcpy(c.fileid, Paths[i]);
            c.fileid[strlen(Paths[i]) + 1] = '\0';

            for (int k = 0; k < ChunkSize; k++)
            {
                if (l >= count)
                    break;
                c.lines[k] = lines[l++];
                c.LinesNo++;
            }
            cout<<"Ingester pid= ["<<getpid()<<"]"<< " Parent pid= ["<<getppid()<<"] sended data from PIPE"<<endl;
            cout << "Sended data chunk ID : " << c.ChunkID << " FileID for Chunk" << c.fileid << " Lines :" << c.LinesNo << endl;
            write(pipe, &c, sizeof(c));
            chunksent++;
        }
        if (interupted)
        {
            break;
        }
    }
    Chunk c;
    cout<<"Ingester pid= ["<<getpid()<<"]"<< " Parent pid= ["<<getppid()<<"] write EOF chunk"<<endl;
    c.ChunkID = -1;
    write(pipe, &c, sizeof(c));
    close(pipe);

    for (int i = 0; Paths[i] != nullptr; i++)
    {
        delete[] Paths[i];
    }
    delete[] Paths;
    if (interupted)
    {
        exit(130);
    }
    else
    {
        exit(0);
    }
}
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Args too few !!!" << endl;
        exit(10);
    }

    char *pipe = argv[1];
    int a = open(pipe, O_WRONLY);
    if (a == -1)
    {
        cout << "Pipe not found !!!" << endl;
        exit(20);
    }

    int threads = stoi(argv[4]);
    cout<<"Ingester pid= ["<<getpid()<<"]"<< " Parent pid= ["<<getppid()<<"] starting data reading"<<endl;
    ReadData(a, argv[2], argv[3], threads);
    close(a);
    exit(0);
}
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "../src/common/Chunk.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: ./processor <pipe_name>\n";
        return 1;
    }

    const char *pipe = argv[1];
    int fd = open(pipe, O_RDONLY);

    if (fd == -1)
    {
        perror("open pipe failed");
        return 1;
    }

    cout << "Processor started, waiting for chunks...\n";

    Chunk* buffer=new Chunk;
    int bytes;
    while((bytes=read(fd,buffer,sizeof(*buffer)))>0)
    {
        if(bytes != sizeof(*buffer))
    {
        cout << "Partial read\n";
        break;
    }
        if(buffer->ChunkID==-1){cout<<"Closing FIFO Pipe"<<endl;close(fd);break;}
        
        cout<<"Consumer : "<<buffer->ChunkID<<"  "<<buffer->fileid<<"  "<<buffer->LinesNo<<endl;
        buffer=new Chunk;
    }
     delete buffer;
    close(fd);
    return 0;
}
#include <iostream>
#include <thread>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
char** BuildPath(char* folder,char* data) //Send paths of files in form of array
{
    int length=0;
    int i=0;
    while(data[i])
    {
        if(data[i]==',')length++;
        i++;
    }
    
    char** Paths=new char*[length];
    for(int j=0;j<length;j++)
       Paths[j]=new char[50];

    i=0;
    int file;
    for(file=0;file<length;file++)
    {
        int k=0;
        int index=0;
        while(folder[index])   //Folder
        {
          Paths[file][k++]=folder[index++];
        }
        Paths[file][k++]='/';

       while(data[i])          //File
       {
        if(data[i]==',')
        {
            i++;
            break;
        }
         Paths[file][k++]=data[i++];
       }
    }
    Paths[file]=nullptr;
    return Paths;
}
using namespace std;

struct Line{
   int bytes;
   char data[1000];
};
struct Chunk {
   int ChunkID;
   char* fileid;
   int LinesNo;
   Line lines[100];
};

void ReadData(int pipe,char* folder,char* files,int &threads)
{
    char** Paths=BuildPath(folder,files);
    int chunkid=0;

        cout<<"Helloo";
    for(int i=0;Paths[i]!=nullptr;i++)
    {
        int fd=open(Paths[i], O_RDONLY);
        if(fd==-1)
        {
            cout<<"File not found: "<<Paths[i]<<endl;
            close(fd);
            continue;
        }

        char buffer;
        int count=0;
        Line lines[1000];
        int l=0;
        int byte;
        cout<<"Helloo";

        while((byte=read(fd, &buffer, 1))>0)
        {
            if (buffer == '\n') 
            {
                lines[count].data[l++]=buffer;

                lines[count].bytes=byte;

                count++;
                l=0;
            } 
            else
            {
                lines[count].data[l++]=buffer;
            }
        }
        close(fd);
        cout<<"Lines : "<<count<<endl;
        int ChunkSize=(count)/threads;
        cout<<"Chunks : "<<ChunkSize<<endl;
        l=0;
        while(l!=count)
        {
            Chunk c;
            c.ChunkID=chunkid++;
            c.LinesNo=0;
            c.fileid=new char[strlen(Paths[i])+1];
            strcpy(c.fileid,Paths[i]);

            for(int k=0;k<ChunkSize;k++)
            {
                if(l>=count)break;
                c.lines[k]=lines[l++];
                c.LinesNo++;
            }
            cout<<c.lines[0].data<<endl;
        }
    }

    for(int i=0;Paths[i]!=nullptr;i++)
    {
        delete[] Paths[i];
    }
    delete[] Paths;
}
int main()
{
    int a=2;
    ReadData(0,"data","retail_transactions.csv,",a);
    // sleep(6);
}

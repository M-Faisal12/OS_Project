#pragma once
struct Line{
   int bytes;
   char data[50]={'\0'};
};
struct Chunk {
   int ChunkID;
   char fileid[50]={'\0'};
   int LinesNo;
   Line lines[50];
};
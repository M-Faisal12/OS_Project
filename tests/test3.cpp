#include <iostream>
using namespace std;
#include <string.h>
char* Path(char* dir)
{
   char *arr=new char[20]{'\0'};
   strcpy(arr,dir);
   char* arr2="Report.txt";
   int l1=strlen(arr2);
   int l2=strlen(dir);
   arr[l2]='/';
   for(int i=l2+1,j=0;i<20 && j<l1;i++,j++)
   {
    arr[i]=arr2[j];
   }
   return arr;
}
int main()
{
   cout<<Path("report");
}
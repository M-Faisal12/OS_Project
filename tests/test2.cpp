#include <iostream>
#include <cstdlib>

using namespace std;

class A
{
public:
A(){cout<<"Constructor\n";}
    ~A()
    {
        cout << "Destructor\n";
    }
};

int main()
{
    A x;

    cout << "Before abort\n";

    exit(0);

    cout << "After abort\n";
}
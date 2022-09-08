#include <iostream>
#include "head.h"

using namespace std;

int main()
{
    int a = 125;
    int b = 17;

    cout << "a = " << a << "  " << "b = " << b << endl;
    cout << "a + b = " << add(a,b) << endl;
    cout << "a - b = " << sub(a,b) << endl;
    cout << "a * b = " << multi(a,b) << endl;
    cout << "a / b = " << divi(a,b) << endl;
}
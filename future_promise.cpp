/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <future>
using namespace std;

int factorial(int n)
{
    int res= 1;
    for(int i=n;i>1;i--)
        res *= i;
    return res;
}

int main()
{
    int x;
    std::promise<int> p;
    std::future<int> f = p.get_future();
    //std::future<int> fu=std::async(factorial,4);
    std::future<int> fu = std::async(std::launch::async | std::launch::deferred,factorial,std::ref(f));
    //std::future<int> fu=std::async(std::launch::async,factorial,4);
    //do somthing else
    std::this_thread::sleep_for(chrono::milliseconds(20));
    //p.set_exception(std::make_exception_ptr(std::runtime_error("to err is human")));
    p.set_value(4);
    x = fu.get();
    cout<<"get from child:"<<x<<endl;
    return 0;
}
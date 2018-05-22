#include <iostream>
#include <bitset>
using namespace::std;
int main()
{
    bitset <32> bt = 0x11;
    bitset <16> bt1(0x1);      /********* 从右到左 ********************/
    string stB("01100111");    /********* 从右到左  ***************/
    bitset <32> btS(stB);
    bitset <32> bt3(stB,3);    /******** 下标3起, 就是 00111 *******/
    bitset <32> bt4(stB,3,4);  /******** 下标3起，长度4，就是 0011 ******/        
    cout << "stS count -> " << btS.count() << endl;
    cout << "st3 cout  -> " << bt3.count() << endl;
    cout << "st4 cout  -> " << bt4.count() << endl;
    cout << "st3 bit0 -> " << bt3[0] << endl;   
    unsigned long btSize = bt3.to_ulong();
    cout << "bt3 ulong -> " << btSize << endl;
    cout << "btS-> " << btS << endl;
}

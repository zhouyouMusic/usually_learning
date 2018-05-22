#include <iostream>
#include <vector>
#include <iterator>
using namespace ::std;
int main()
{

    vector<int> arry1;
    vector<int> arry2(arry1);
    vector<int> arry3(5,2);     
    vector<int> arry4(3);
    vector<string> str(10,"null");
  
  #if 0
    /********       vector  func    ********/
    vector<vector<int> > ivec;
    
    vector <int>::size_type arryS;
    arryS = arry4.size();
    cout << "arry1 empty: ?  " << arry1.empty() << endl;
    cout << "arry3 size ? " << arry3.size() << endl;
    arry1.push_back(-1);
    cout << "arry1 push value :  "<< arry1[0] << endl;
    cout << "arry3[3] : " <<  arry3[3] << endl;
    arry1 = arry3;
    if(arry2 > arry1)
        cout << "arry2 more than arry1 " << endl;
    for(vector<int>::size_type it =0;it!=arry3.size();it++)
    {
        cout << "it arry3 " << arry3[it] << endl;
   } 
  #endif

    for(vector<int>::const_iterator it =arry3.begin();it!=arry3.end();++it)
    {
        cout << "itrear value->  "<< *it << endl;
    }

}

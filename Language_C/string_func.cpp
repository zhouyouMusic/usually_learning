#include <iostream>
using namespace ::std;
using std::string;
int main()
{	
	string str("hello,world");
    string::size_type unSize = str.size(); 
	cout << unSize <<"     empty:"<<str.empty() << endl;
	cout << str[2] << endl;
	string big = "big";
	string small = "smaller";
	if(big == small)
	{
		cout <<  "equal"<< endl;
	}else if(big > small){
		cout << "bigger" << endl;
	}else{
		cout << "smaller" << endl;
	}
	string hello = "hello";
	string strH = hello + ",";
	string heWd = strH + "world";
//	string heErr = "hello" + ",";
	cout <<  strH << endl;
	cout << heWd << endl;
	
}

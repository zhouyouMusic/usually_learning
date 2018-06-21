#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <list>
using namespace ::std;
class People{
public:
	char * m_name;
	int age;
	People(char * m_name,int age);
};

People::People(char * m_name,int age)
{
	
}

class Student:public People{
public:
	float m_score;
	Student(char *name,int age,float score);
	void display();
};

Student::Student(char*name,int age,float score):People(name,age),m_score(score){}
void Student::display()
{
	cout << "m_score" << m_score << endl;
}

int main()
{
//	Student Stu("mj",22,9.9);
//	Stu.display();
	list<string> lst;
	list<string>:: iterator iter =   lst.begin();
	string word;
	int i = 0;
	for(i = 0; i < 4; i++)
	{
		cin >> word;
		iter = lst.insert(iter,word);
		iter++;
	}
	for(iter=lst.begin();iter != lst.end();iter++)
	{
		cout << *iter << endl;
	}
}


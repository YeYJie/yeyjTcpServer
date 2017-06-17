#include "include.h"

int getTimeInSecond()
{
	timeval temp;
	gettimeofday(&temp, NULL);
	return temp.tv_sec;
}

int getTimeInMicroSecond()
{
	timeval temp;
	gettimeofday(&temp, NULL);
	return temp.tv_usec;
}

// string trim

string & ltrim(string &s)
{
    if (s.empty()) return s;
    string::const_iterator iter = s.begin();
    while (iter != s.end() && isspace(*iter++));
    s.erase(s.begin(), --iter);
    return s;
}

string & rtrim(string &s)
{
    if (s.empty()) return s;
    string::const_iterator iter = s.end();
    while (iter != s.begin() && isspace(*--iter));
    s.erase(++iter, s.end());
    return s;
}

string & trim(string &s)
{
    ltrim(s);
    rtrim(s);
    return s;
}


// string split

vector<string> split(const string & s, const string & d)
{
    vector<string> v;
    char * str = new char[s.size()+1];
    strcpy(str, s.c_str());
    while (char *t = strsep(&str, d.c_str()))
        v.push_back(t);
    delete[] str;
    return v;
}
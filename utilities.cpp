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

int getTimeInMilliSecond()
{
	return getTimeInMicroSecond() / 1000;
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


// string format

std::string format(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int size = vsnprintf(nullptr, 0, fmt, ap) + 1;
	va_end(ap);
	char *buf = new char[size];
	va_start(ap, fmt);
	vsnprintf(buf, size, fmt, ap);
	va_end(ap);
	std::string fs(buf);
	delete[] buf;
	return fs;
}


void process_mem_usage(double & vm_usage, double & resident_set)
{
	using std::ios_base;
	using std::ifstream;
	using std::string;

	vm_usage	= 0.0;
	resident_set = 0.0;

	// 'file' stat seems to give the most reliable results
	ifstream stat_stream("/proc/self/stat",ios_base::in);

	// dummy vars for leading entries in stat that we don't care about
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	string utime, stime, cutime, cstime, priority, nice;
	string O, itrealvalue, starttime;

	// the two fields we want
	unsigned long vsize;
	long rss;

	stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
				>> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
				>> utime >> stime >> cutime >> cstime >> priority >> nice
				>> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

	stat_stream.close();

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
	vm_usage	= vsize / 1024.0;
	resident_set = rss * page_size_kb;
}
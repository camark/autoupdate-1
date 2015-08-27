#ifndef INCLUDED_PROCESS_H
#define INCLUDED_PROCESS_H

namespace cacti
{
	class XProcess
	{
	public:
#ifdef WIN32
		typedef DWORD exit_type;
#else
		typedef int exit_type;
#endif
#ifdef WIN32 
		typedef DWORD pid_t;
#endif
	public:
		// create a process and run it until it finished 
		// return 0 when success, otherwise failed
		static bool create(const std::string& cmdline, const std::string& workpath, exit_type& exitcode);
		// create a process and return
		// return 0 when success, otherwise failed
		static bool create(const std::string& cmdline, const std::string& workpath);

		// argv ���б�ʽ����һ��Ԫ���ǽ������֣�֮���ǽ�����Ҫ�Ĳ����б�����һ��NULL��ʾ����
		// ʹ�ô˷�ʽ�ȽϷ���linux��ϰ��
		static bool create(char* argv[], const std::string& workpath, exit_type& exitcode);
		static bool create(char* argv[], const std::string& workpath);

		static bool isrunning(pid_t procid);
		static bool isrunning(const std::string& procname);
	};
}
#endif // INCLUDED_PROCESS_H

// �Զ����ǳ���

#include <windows.h>
#include <stdio.h>
#include <string>
using namespace std;

#include <io.h>
#include <direct.h>

// �汾��
#define VERSION_CURRENT "autocover.version=1.0.0"

#define SYSTEM_CFG_FILE  "./autoupdate/autoupdate.ini"

#define UPDATE_FLAG_FILE "./autoupdate/updateflag.dat"

#define COVER_SOURCE_DIR "./autoupdate/downloaded"

// ���ǵ�Ŀ��Ŀ¼
#define COVER_DST_DIR "./"

// ���ǳ���������Ŀ¼
#define COVER_SELF_DIR "./autoupdate"

#define MAIN_EXE_PATH ".\\" // ������·��
#define DEFAULT_MAIN_EXE_NAME "sale.exe"		// Ĭ����exe������

int CopyDir(const char * pSrc,const char *pDes);
int DelDir(const char * pSrc);

int StartProgram(char* path, char* name);

BOOL FindAndKillProcessByName(LPCTSTR strProcessName);

// ȫ�ֱ��� 
char g_mainExeName[256];     // ��exe������

// д�汾��
void WriteVersion()
{
	char szVersionFile[256];
	sprintf_s(szVersionFile, 256, "%s/version_cover.txt", COVER_SELF_DIR);
	printf("%s\n", szVersionFile);

	FILE *pf;
	fopen_s(&pf, szVersionFile, "w");
	if (pf == NULL)
	{
		printf("open version file fail\n");
		return;
	}

	fwrite(VERSION_CURRENT, 1, strlen(VERSION_CURRENT), pf);
	fclose(pf);
}

int main(int argc, char* argv[])
{
	::CreateMutex(NULL, FALSE, "autocovermutex"); /*����һ��������*/
	if(ERROR_ALREADY_EXISTS == GetLastError()) 
	{ 
		printf("���б�����ʵ����������...\n");
		return FALSE; 
	}

	WriteVersion();

	// ��ѯ�Ƿ���Ҫ����
	int needUpdate = GetPrivateProfileInt("info", "needupdate", 0, UPDATE_FLAG_FILE);
	if (1 != needUpdate) // ����Ҫ����
	{
		printf("����Ҫ���£���������������...\n");
		getchar();
		return 0;
	}

	// �����ô���ȡ��������
	GetPrivateProfileString("autocover", "mainexename", DEFAULT_MAIN_EXE_NAME, g_mainExeName, sizeof(g_mainExeName), SYSTEM_CFG_FILE);
	printf("��������:%s\n", g_mainExeName);

	// �ر���exe ����������
	BOOL bRet = FindAndKillProcessByName(g_mainExeName);
	if (FALSE == bRet)
	{
		printf("end main exe failed, exit.\n");
		getchar();
		return 1;
	}
	printf("����������\n");

	// ������������
	for (int i=1; ; i++)
	{
		char key[100];
		sprintf_s(key, 100, "processname%d", i);
		char exeName[100];
		GetPrivateProfileString("kill", key, "", exeName, 100, SYSTEM_CFG_FILE);

		printf("process%d:%s\n", i, exeName);

		if (0 == strcmp("", exeName))
		{
			break;
		}

		bRet = FindAndKillProcessByName(exeName);
		if (FALSE == bRet)
		{
			printf("�������� %s ʧ��\n", exeName);
		}
		else
		{
			printf("�������� %s\n", exeName);
		}
	}

	printf("���ڸ��£����Ե�\n������...\n");

	// �����ص����ļ�����ԭ�ļ�
	int retCopy = CopyDir(COVER_SOURCE_DIR, COVER_DST_DIR);
	if (retCopy < 0)  // ����ʧ��
	{
		printf("���������ļ�ʧ��...\n");
		StartProgram(MAIN_EXE_PATH, g_mainExeName);
		getchar();
		return 2;
	}

	int retDel = DelDir(COVER_SOURCE_DIR);
	if (retDel < 0)  // ����ʧ��
	{
		printf("ɾ�������ļ�ʧ��...\n");
		StartProgram(MAIN_EXE_PATH, g_mainExeName);
		getchar();
		return 3;
	}

	// ɾ�����±�־�ļ�
	FILE *pf = fopen(UPDATE_FLAG_FILE, "w");
	if (!pf) // �����ʧ�ܣ�����autoupdate����������ڲ������ļ����ȴ���������в���
	{
		Sleep(3000);
		pf = fopen(UPDATE_FLAG_FILE, "w");		
	}

	if (!pf)
	{
		printf("�򿪱�־�ļ�ʧ��...\n");
		StartProgram(MAIN_EXE_PATH, g_mainExeName);
		getchar();
		return 4;
	}

	char *content = "[info]\nneedupdate=0\n";
	fwrite(content, 1, strlen(content), pf);
	fclose(pf);		

	printf("������ɣ���������������...\n");
	StartProgram(MAIN_EXE_PATH, g_mainExeName);

	//printf("��������رձ�����\n");
	//getchar();
	
	return 0;
}

// �������߳�
// path:·��  name:������
int StartProgram(char* path, char* name)
{
	// �л�����Ŀ¼
	chdir(path);

	char tmp[256];
	getcwd(tmp, 256);
	printf("current work dir: %s\n", tmp);

	// ����Ҫ��װȫ·������Ϊ�Ѿ��л��˹���Ŀ¼������ֱ��ִ��exe����
// 	char cmdline[256];
// 	sprintf_s(cmdline, 256, "%s\\%s", path, name);
// 
// 	printf("%s\n", cmdline);

	STARTUPINFO si = { sizeof(si) }; 
	PROCESS_INFORMATION pi; 

	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = TRUE; //TRUE��ʾ��ʾ�����Ľ��̵Ĵ���
	//char cmdline[] =TEXT(path);
	BOOL bRet = ::CreateProcess ( 
		NULL,
		name, //��Unicode�汾�д˲�������Ϊ�����ַ�������Ϊ�˲����ᱻ�޸�	 
		NULL, 
		NULL, 
		FALSE, 
		CREATE_NEW_CONSOLE, 
		NULL, 
		NULL, 
		&si, 
		&pi); 

	int error = GetLastError();
	if(bRet) 
	{ 
		::CloseHandle (pi.hThread); 
		::CloseHandle (pi.hProcess); 

		printf("�½��̵Ľ���ID�ţ�%d \n", pi.dwProcessId); 
		printf("�½��̵����߳�ID�ţ�%d \n", pi.dwThreadId); 
	} 
	else
	{
		printf("error code:%d\n",error );
		printf("����������ʧ�ܣ����ֶ���������������رձ�����\n");
		getchar();
		return 5;
	}
	return 0;
}

#define BUF_SIZE 2048
int copyFile(const char * pSrc,const char *pDes)
{
	FILE *in_file, *out_file;
	char data[BUF_SIZE];
	size_t bytes_in, bytes_out;
	long len = 0;
	if ( (in_file = fopen(pSrc, "rb")) == NULL )
	{
		perror(pSrc);
		return -2;
	}
	if ( (out_file = fopen(pDes, "wb")) == NULL )
	{
		perror(pDes);
		return -3;
	}
	while ( (bytes_in = fread(data, 1, BUF_SIZE, in_file)) > 0 )
	{
		bytes_out = fwrite(data, 1, bytes_in, out_file);
		if ( bytes_in != bytes_out )
		{
			perror("Fatal write error.\n");
			return -4;
		}
		len += bytes_out;
		//printf("copying file .... %d bytes copy\n", len);
	}
	fclose(in_file);
	fclose(out_file);
	return 1;
}

/*********************************************************************
���ܣ�����(�ǿ�)Ŀ¼
������pSrc��ԭĿ¼��
     pDes��Ŀ��Ŀ¼��
���أ�<0��ʧ��
     >0���ɹ�
*********************************************************************/
int CopyDir(const char * pSrc,const char *pDes)
{
    if (NULL == pSrc || NULL == pDes)	return -1;
    mkdir(pDes);
    char dir[MAX_PATH] = {0};
    char srcFileName[MAX_PATH] = {0};
    char desFileName[MAX_PATH] = {0};
    char *str = "\\*.*";
    strcpy(dir,pSrc);
    strcat(dir,str);
    //���Ȳ���dir�з���Ҫ����ļ�
    long hFile;
    _finddata_t fileinfo;
    if ((hFile = _findfirst(dir,&fileinfo)) != -1)
    {
        do
        {
            strcpy(srcFileName,pSrc);
            strcat(srcFileName,"\\");
            strcat(srcFileName,fileinfo.name);
            strcpy(desFileName,pDes);
            strcat(desFileName,"\\");
            strcat(desFileName,fileinfo.name);
            //����ǲ���Ŀ¼
            //�������Ŀ¼,����д����ļ���������ļ�
            if (!(fileinfo.attrib & _A_SUBDIR))
            {
				printf("src:%s, dst:%s\n", srcFileName, desFileName);
                copyFile(srcFileName,desFileName);
            }
            else//����Ŀ¼���ݹ����
            {
                if ( strcmp(fileinfo.name, "." ) != 0 && strcmp(fileinfo.name, ".." ) != 0 )
                {
                    CopyDir(srcFileName,desFileName);
                }
            }
        } while (_findnext(hFile,&fileinfo) == 0);
        _findclose(hFile);
        return 1;
    }
    return -3;
}

/*********************************************************************
���ܣ��ݹ�ɾ��Ŀ¼�������ļ�
������pSrc��Ŀ¼��
���أ�<0��ʧ��
     >0���ɹ�
*********************************************************************/
int DelDir(const char * pSrc)
{
	if (NULL == pSrc)	return -1;

	char dir[MAX_PATH] = {0};
	char srcFileName[MAX_PATH] = {0};
	char *str = "\\*.*";
	strcpy(dir,pSrc);
	strcat(dir,str);
	//���Ȳ���dir�з���Ҫ����ļ�
	long hFile;
	_finddata_t fileinfo;
	if ((hFile = _findfirst(dir,&fileinfo)) != -1)
	{
		do
		{
			strcpy(srcFileName,pSrc);
			strcat(srcFileName,"\\");
			strcat(srcFileName,fileinfo.name);
			
			//����ǲ���Ŀ¼
			//�������Ŀ¼,����д����ļ���������ļ�
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				DeleteFile(srcFileName);
			}
			else//����Ŀ¼���ݹ����
			{
				if ( strcmp(fileinfo.name, "." ) != 0 && strcmp(fileinfo.name, ".." ) != 0 )
				{
					DelDir(srcFileName);
					// ɾ��Ŀ¼����
					DeleteFile(srcFileName);
				}
			}
		} while (_findnext(hFile,&fileinfo) == 0);
		_findclose(hFile);
		return 1;
	}
	return -3;
}

#include <tlhelp32.h>
BOOL FindAndKillProcessByName(LPCTSTR strProcessName)
{
	if(NULL == strProcessName)
	{
		return FALSE;
	}
	HANDLE handle32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == handle32Snapshot)
	{
		return FALSE;
	}

	PROCESSENTRY32 pEntry;       
	pEntry.dwSize = sizeof( PROCESSENTRY32 );

	BOOL bFound = FALSE;

	//Search for all the process and terminate it
	if(Process32First(handle32Snapshot, &pEntry))
	{
		if (!stricmp(pEntry.szExeFile, strProcessName))
		{			
			bFound = TRUE;
		}
		while((!bFound)&&Process32Next(handle32Snapshot, &pEntry))
		{
			//
			//printf("%s\n", pEntry.szExeFile);
			if (!stricmp(pEntry.szExeFile, strProcessName))
			{
				bFound = TRUE;
			}
		}
		if(bFound)
		{
			CloseHandle(handle32Snapshot);
			HANDLE handLe =  OpenProcess(PROCESS_TERMINATE , FALSE, pEntry.th32ProcessID);
			BOOL bResult = TerminateProcess(handLe,0);
			return bResult;
		}
	}

	CloseHandle(handle32Snapshot);

	if (!bFound)		// δ�ҵ�����������Ϊ�رճɹ�
	{
		printf("δ�ҵ�ָ������\n");
		return TRUE;
	}

	return FALSE;
}


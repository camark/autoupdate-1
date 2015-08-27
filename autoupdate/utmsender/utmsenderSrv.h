#ifndef __INC__ALARMSRV_H
#define __INC__ALARMSRV_H

#include "Service.h"
#include "cacti/mtl/MessageDispatcher.h"

using namespace cacti;

class USService;

class USServer : public Service
{
public:
	USServer():Service("�Զ�����ģ��", "AutoUpdate_V1.0.1","�Զ�����ģ��")
	{
		m_pAlarmSrvService = NULL;
		m_pMessageDispatcher = NULL;
		m_appport = 0;
	}

	virtual bool start();
	virtual void stop();
	virtual void snapshot();

private:
	void handleUICommand(std::vector<std::string> & vec);		// �������̨����

private:
	MessageDispatcher	*m_pMessageDispatcher;
	USService		*m_pAlarmSrvService;
	int				m_appport;					// ��������ģ��ĳ����Ӧ��appport
};
#endif //__INC__ALARMSRV_H

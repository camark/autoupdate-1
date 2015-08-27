#include "DataSource.h"

TDataSource::TDataSource()
{
}

TDataSource::~TDataSource()
{
	list<ServiceIdentifier> *sidList = NULL;
	for (map<u32, list<ServiceIdentifier>*>::iterator it = m_serviceIDMap.begin(); it != m_serviceIDMap.end(); ++it)
	{
		sidList = it->second;

		if (sidList)
		{
			sidList->clear();
			delete sidList;
			sidList = NULL;
		}
 	}
}

// ���SID
// ����: 0�ɹ�  ����ʧ��
u32 TDataSource::addServiceIDByApport(u32 apport, const ServiceIdentifier& sid)
{
	list<ServiceIdentifier> *sidList = NULL;
	map<u32, list<ServiceIdentifier>*>::iterator it = m_serviceIDMap.find(apport);
	if (it == m_serviceIDMap.end())			// key�����ڣ��½�
	{
		sidList = new list<ServiceIdentifier>;
		sidList->push_back(sid);
		m_serviceIDMap.insert(pair<u32, list<ServiceIdentifier>*>(apport, sidList));
	}
	else									// key���ڣ�׷��
	{
		sidList = it->second;
		sidList->push_back(sid);
	}

	return 0;
}

// ɾ��SID
// ����: 0�ɹ�  ����ʧ��
u32 TDataSource::removeServiceIDByApport(u32 apport, const ServiceIdentifier& sid)
{
	list<ServiceIdentifier> *sidList = NULL;
	map<u32, list<ServiceIdentifier>*>::iterator it = m_serviceIDMap.find(apport);
	if (it == m_serviceIDMap.end())			// key������
	{
		return 1;
	}
	
	sidList = it->second;
	
	// ��������ɾ����Ӧ�ڵ�
	for (list<ServiceIdentifier>::iterator sidIt = sidList->begin(); sidIt != sidList->end(); ++sidIt)
	{
		if ((*sidIt) == sid)				// �ҵ���Ӧ�ڵ�
		{
			sidList->erase(sidIt);
			return 0;
		}
	}

	return 1;
}

// ��ȡSID
// ����: 0�ɹ�  ����ʧ��
u32	TDataSource::getServiceIDByApport(u32 apport, ServiceIdentifier& sid)
{
	list<ServiceIdentifier> *sidList = NULL;
	map<u32, list<ServiceIdentifier>*>::iterator it = m_serviceIDMap.find(apport);
	if (it == m_serviceIDMap.end())			// key������
	{
		return 1;
	}

	sidList = it->second;

	// ��ѯ����
	if (sidList->size() > 0)
	{
		sid = sidList->front();
		sidList->pop_front();
		sidList->push_back(sid);
		return 0;
	}

	return 1;
}

// ����
void TDataSource::snapShot()
{
	list<ServiceIdentifier> *sidList = NULL;
	for (map<u32, list<ServiceIdentifier>*>::iterator it = m_serviceIDMap.begin(); it != m_serviceIDMap.end(); ++it)
	{
		sidList = it->second;

		printf("appport: %d\n", it->first);

		// ��������
		for (list<ServiceIdentifier>::iterator sidIt = sidList->begin(); sidIt != sidList->end(); ++sidIt)
		{
			printf("ServiceID: %s\n", ((ServiceIdentifier)(*sidIt)).toString().c_str());
		}
	}
}
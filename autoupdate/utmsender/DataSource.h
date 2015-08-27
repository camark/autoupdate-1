#ifndef __DATA_SOURCE_H
#define __DATA_SOURCE_H

#include <map>
#include <list>
#include "cacti/message/ServiceIdentifier.h"

using namespace cacti;
using namespace std;

class TDataSource
{
public:
	TDataSource();
	~TDataSource();

public:
	u32		addServiceIDByApport(u32 apport, const ServiceIdentifier& sid);			// ���SID
	u32		removeServiceIDByApport(u32 apport, const ServiceIdentifier& sid);		// ɾ��SID
	u32		getServiceIDByApport(u32 apport, ServiceIdentifier& sid);				// ��ȡSID
	// TODO: ��������������

	void	snapShot();

private:
	TDataSource(const TDataSource& ret){}	// ��������
	
private:
	map<u32, list<ServiceIdentifier>*> m_serviceIDMap;			// ServiceID map, keyΪapport��valueΪ��̬����������ʱ���ͷ�

};
#endif
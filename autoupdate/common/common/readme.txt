-20080312
1. DbConnect Sybase�汾����֧��effectCount()ʵ�֣���ʾinsert��update��delete�����Ӱ��ļ�¼��

-20070527
1. PostMessageToPeer��ʱ���ж����peerû��active�Ļ����򷵻��ϲ㷢��ʧ��
	��־��ӡpeer(%d.%d) not active
2. �Ѹ���passive connection ID�ĵ��÷��ڸ�����active��־֮��
   ���ڿ�����ȷ��ӡ����־��ʾ��Щ����û�����õ�[peers]����
   û�����õ�[peers]�ĶԶ����ӻ��ӡ��anonymous service active
3. ��������֧�ֲ���˳�򣬲���֧��arg=value����ʽ
4. windows����������̨���޸��˱���������ʾ��Ϣ

-20080414
1. TransferClient��postMessage���ӿڵ�ʱ���Լ���m_dstappid����dst.m_appref
	�Ա��ϲ�ӿڿ���֪�����Ǹ�client��������Ϣ
2. �ӿ�Logger�ڳ����˳�ʱ����ܵ��µķǷ�����

-20080421
1. void TransferClient::stop() ��Զ˳��Է���һ���Ͽ����ӵ�Э����Ϣ���öԶ˹رմ�TCP����
2. ActiveClient::threadProcess() �޸������ȴ����ԣ����ȴ�ʱ��ֳɼ���С�εȴ�
3. TLV& TLV::operator = (const ServiceIdentifier& sid) ����һ���ڴ�й¶������
4. Startup/Main.cpp ���WINDOWS���ڴ�й¶���Դ���
5. ActiveServer && ActiveClient ��ӹ��캯����ȱʡ������֧��ָ��logger������

-20080422
1. �޸�MessageDispatcher::onSystemMessage��_EvtServiceInited��Ϣ�Ĵ���
	�ȸ��Է��㲥active��Ϣ��Ȼ�������Լ�ģ�鱨��active��
	��֤ģ������֮���͵�һ����Ϣ֮ǰ���Զ�ģ���Ѿ��յ���ģ���reactive�¼���

-20080424
1. �޸�Startup��Main.cpp stopWin32Proc���������̷���WM_QUIT��Ϣ3����֮����̲����Լ������Ļ���ǿ��TerminateProcess
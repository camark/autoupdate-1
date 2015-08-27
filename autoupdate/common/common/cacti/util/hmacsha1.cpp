//-------------------------------------------------------
//HMAC_SHA1.CPP   code 

//******************************************************************************
//* HMAC_SHA1.cpp : Implementation of HMAC SHA1 algorithm
//*                 Comfort to RFC 2104
//*
//******************************************************************************
#include "stdafx.h"
#include <iostream>
#include <memory>
#include "HMACSHA1.h"
#include "base64.h"
#include "codeset.h"


void CHMAC_SHA1::HMAC_SHA1(BYTE *text, int text_len, BYTE *key, int key_len, BYTE *digest)
{
    memset(SHA1_Key, 0, SHA1_BLOCK_SIZE);

    /* repeated 64 times for values in ipad and opad */
    memset(m_ipad, 0x36, sizeof(m_ipad));
    memset(m_opad, 0x5c, sizeof(m_opad));

    /* STEP 1 */
    if (key_len > SHA1_BLOCK_SIZE)  //����64λ
    {
        CSHA1::Reset();
        CSHA1::Update((UINT_8 *)key, key_len);
        CSHA1::Final();

        CSHA1::GetHash((UINT_8 *)SHA1_Key);//20
    }
    else
        memcpy(SHA1_Key, key, key_len);

    /* STEP 2 */
    for (int i=0; i<sizeof(m_ipad); i++)
    {
        m_ipad[i] ^= SHA1_Key[i];        
    }

    /* STEP 3 */
    memcpy(AppendBuf1, m_ipad, sizeof(m_ipad));
    memcpy(AppendBuf1 + sizeof(m_ipad), text, text_len);

    /* STEP 4 */
    CSHA1::Reset();
    CSHA1::Update((UINT_8 *)AppendBuf1, sizeof(m_ipad) + text_len);
    CSHA1::Final();

    CSHA1::GetHash((UINT_8 *)szReport);

    /* STEP 5 */
    for (int j=0; j<sizeof(m_opad); j++)
    {
        m_opad[j] ^= SHA1_Key[j];
    }

    /* STEP 6 */
    memcpy(AppendBuf2, m_opad, sizeof(m_opad));
    memcpy(AppendBuf2 + sizeof(m_opad), szReport, SHA1_DIGEST_LENGTH);

    /*STEP 7 */
    CSHA1::Reset();
    CSHA1::Update((UINT_8 *)AppendBuf2, sizeof(m_opad) + SHA1_DIGEST_LENGTH);
    CSHA1::Final();

    CSHA1::GetHash((UINT_8 *)digest);


//    char * mu;
//    CSHA1::ReportHash(mu,REPORT_HEX);

}

/************************************************************************/
/* 
����: ��gbkԴ������hmacsha1���ܣ��ٽ���base64����
˵��: 
	1.�ú����ṩ���������ŽӿڶԽ�ʹ�ã��Է�����java�����У���Դ����key��ת��Ϊutf8�����������ڴ˴�Ҳ��Ҫ��תutf8���ٽ��м���ת��
	2.�����ط�����Ҫʹ�øú�������ע���ַ������⣬�ж��Ƿ���������
����:
	data: Դ����
	key: ��Կ
����:
	�����
*/
/************************************************************************/
string HmacSHA1Base64(string data, string key)
{		
	string u8Src = GBKToUTF8((const char*)data.c_str());			// ����ת��Ϊutf8��ʽ��������Է�(������������)һ��
	string u8Key = GBKToUTF8((const char*)key.c_str());

	BYTE ret[100];
	CHMAC_SHA1 hs1;
	hs1.HMAC_SHA1((unsigned char*)u8Src.c_str(), strlen((const char*)u8Src.c_str()), (unsigned char*)u8Key.c_str(), strlen((const char*)u8Key.c_str()), ret);

#ifdef _DEBUG
	printf("hmacsha1 result hex:\n", ret);
	for (int i=0; i<20; i++)
	{
		printf("%02x ", ret[i]);
	}
	printf("\n");
#endif
	
	ZBase64 b64;
	return b64.Encode((const unsigned char*)ret, 20);		// ���ȹ̶�Ϊ20
}
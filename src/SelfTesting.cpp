/*
 * SelfTesting.cpp
 *
 *  Created on: Apr 19, 2017
 *      Author: user
 */

#include <SelfTesting.h>

//=============================================================================
TSelfTestingManager::TSelfTestingManager() {
	listener = NULL;

}
//=============================================================================
TSelfTestingManager::~TSelfTestingManager()
{
	Stop();
}
//=============================================================================
void TSelfTestingManager::Init( void *config )
{
	SelfTestingSettings *cfg = (SelfTestingSettings*)config;
	if(cfg != NULL){
		SocketSettings src_sett;
		src_sett.IpAddress 				= "0.0.0.0";  //INADDR_ANY
		src_sett.BindPort 				= cfg->udpPort;
		src_sett.SocketRecvTimeout 		= cfg->RecvTimeout;
		src_sett.SocketSendTimeout 		= cfg->SendTimeout;
		listener = new UdpListener(&src_sett);

		managers = cfg->managers;
		monmanagers = cfg->modmanagers;

	}
}
//=============================================================================
void TSelfTestingManager::DoWork( void *runFlg )
{
	if(listener != NULL)
	{
		listener->Start(runFlg);
		IPort *client = listener->AcceptSocket();
		if(client != NULL)
		{
			int len = client->Recv(Buffer, sizeof(Buffer));
			if(len > 0)
			{
				string ss = string((char*)Buffer, len);
				//Log::DEBUG("[TSelfTestingManager::DoWork] ss={" + ss+"}");
				vector<string>  lst = TBuffer::Split(ss, ":");
				if(lst.size() >=3 && lst[0] == "GetSelfTestData")
				{
					int et = atoi(lst[1].c_str());
					int en = atoi(lst[2].c_str());
					string ret = "[";
					for(auto curr: managers){
						ret+=curr->GetSelfTestData(et, en);
					}
					if(ret.c_str()[ret.size()-1]==','){
						ret=ret.substr(0,ret.size()-1);
					}
					ret+="]";
					//Log::DEBUG("[TSelfTestingManager::DoWork] GetSelfTestData=" + ret);
					client->Send((Byte*)ret.c_str(), ret.size());
				}
				else if(lst.size() >=4 && lst[0] == "GetModulsTestData")
				{
					int addr = atoi(lst[1].c_str());
					int et = atoi(lst[2].c_str());
					int en = atoi(lst[3].c_str());
					string ret = "[";

					for(auto curr: monmanagers){
						ret+=curr->GetSelfTestMonData(addr, et, en);
					}

					if(ret.c_str()[ret.size()-1]==','){
						ret=ret.substr(0,ret.size()-1);
					}
					ret+="]";
					client->Send((Byte*)ret.c_str(), ret.size());
				}
			}
			client->Close();
		}
	}
}
//=============================================================================
void TSelfTestingManager::Stop( void )
{
	try
	{
		Log::DEBUG("[TSelfTestingManager] Stop");
		managers.clear();
		if(listener != NULL)
		{
			listener->Stop();
			listener = NULL;
		}
	}
	catch(exception &e)
	{
		Log::ERROR( e.what() );
	}
}
	//=============================================================================

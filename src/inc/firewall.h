/*
 * firewall.h
 *
 *  Created on: Oct 14, 2015
 *      Author: user
 */

#ifndef FIREWALL_H_
#define FIREWALL_H_

#include <iostream>
#include <stdexcept>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <Log.h>
#include <Classes.h>
#include <SocketPort.h>
#include <uci.h>
#include <Thread.h>

using namespace std;
using namespace log4net;

namespace firewall {

class ForwardingRule;
class PortForwarding;
class Tfirewall;

class Connection: public Thread, public TFastTimer
{
public:
	Connection(IPort *src, SocketSettings dest):TFastTimer(1,&SecCount)
	{
		src_port = src;
		dest_port = NULL;

		if(dest.Proto == "tcp")
		{
			dest_port = new SocketPortTcp();
			dest_port->Init(&dest);
		}
		else if(dest.Proto == "udp")
		{
			dest_port = new SocketPortUdp();
			dest_port->Init(&dest);
		}

		SetIsRuning( false );
		life_time = 60;
		error_cnt = 0;
	}

	~Connection(){}

	void run()
	{
		try
		{
		    int errCnt = 0;
			TBuffer	srcBuffer(src_port->mtu);
			TBuffer	destBuffer(dest_port->mtu);
			SetTimer(0, life_time);
			Log::DEBUG( "[Connection::run] conn start src_port="+toString(src_port->GetDescriptor()) +" dest_port="+toString(dest_port->GetDescriptor()) );
			while(IsRuning())
			{
				if(src_port == NULL)
					break;
				if(dest_port == NULL)
					break;
				if(GetTimValue(0) <= 0 )
					break;

				if(src_port->IsConnected())
				{
					srcBuffer.DataLen = src_port->Recv( srcBuffer.BUF, srcBuffer.MaxSize);
					if( srcBuffer.DataLen > 0 )
					{
						SetTimer(0, life_time);
						Log::DEBUG( "[Connection::run] Recv from src_port len="+toString(srcBuffer.DataLen ) );
						if(dest_messagess.size() < MAX_MESS_IN_PORTFORWARD)
						{
							TBuffer *nbuf = new TBuffer(srcBuffer.DataLen);
							for(int i = 0; i < srcBuffer.DataLen; i++)
								nbuf->BUF[i] = srcBuffer.BUF[i];
							nbuf->DataLen = srcBuffer.DataLen;
							dest_messagess.push_back(nbuf);
							//Log::INFO("[TcpServer] messagess.size="+toString( dest_messagess.size() ));
						}
					}
					if(!src_messagess.empty())
					{
						TBuffer *curr_mess = src_messagess.front();
						if(curr_mess != NULL)
						{
							int len = src_port->Send(curr_mess->BUF, curr_mess->DataLen);
							if(len >= 0)
							{
								src_messagess.pop_front();
								delete curr_mess;
							}
						}
					}
				}
				else
				{
					Log::DEBUG( "[Connection::run] !src_port->IsConnected() break!" );
					break;
				}

				if(dest_port->ConnectOK())
				{
					errCnt = 0;
					destBuffer.DataLen = dest_port->Recv( destBuffer.BUF, destBuffer.MaxSize);
					if(destBuffer.DataLen > 0 )
					{
						SetTimer(0, life_time);
						Log::DEBUG( "[Connection::run] Recv from dest_port len="+toString(destBuffer.DataLen ) );
						if(src_messagess.size() < MAX_MESS_IN_PORTFORWARD)
						{
							TBuffer *nbuf = new TBuffer(destBuffer.DataLen);
							for(int i = 0; i < destBuffer.DataLen; i++) nbuf->BUF[i] = destBuffer.BUF[i];
							nbuf->DataLen = destBuffer.DataLen;
							src_messagess.push_back(nbuf);
							//Log::INFO("[TcpServer] messagess.size="+toString( src_messagess.size() ));
						}
					}
					if(!dest_messagess.empty())
					{
						TBuffer *curr_mess = dest_messagess.front();
						if(curr_mess != NULL)
						{
							int len = dest_port->Send(curr_mess->BUF, curr_mess->DataLen);
							if(len >= 0)
							{
								dest_messagess.pop_front();
								delete curr_mess;
							}
						}
					}
				}
				else
				{
					if(errCnt++ >= error_cnt)
					{
						errCnt = 0;
						//Log::DEBUG( "[Connection::run] !dest_port->ConnectOK() > "+toString(MAX_MESS_IN_PORTFORWARD)+" cicles break!" );
						break;
					}
				}
			}
		}
		catch(exception &e)
		{
			Log::ERROR( "[Connection::run] "+string(e.what()) );
		}
		if(src_port != NULL)
		{
			if(src_port->IsConnected())
				src_port->Close();
			delete src_port;
			src_port = NULL;
		}
		if(dest_port != NULL)
		{
			if(dest_port->IsConnected())
				dest_port->Close();
			delete dest_port;
			dest_port = NULL;
		}
		SetIsRuning( false );
	}

	void start()
	{
		try
		{
			if( thread_start() != 0)
				throw runtime_error("start error!");
			SetIsRuning( true );
			Log::DEBUG( "[Connection::start]");
		}
		catch(exception &e)
		{
			Log::ERROR( "[FirewallRule::start] "+string(e.what()) );
		}
	}

	void stop ()
	{
		try
		{
			SetIsRuning( false );
			this->wait();

			if(src_port != NULL)
			{
				if(src_port->IsConnected())
					src_port->Close();
				delete src_port;
				src_port = NULL;
			}
			if(dest_port != NULL)
			{
				if(dest_port->IsConnected())
					dest_port->Close();
				delete dest_port;
				dest_port = NULL;
			}

			int s = src_messagess.size();
			for(int i = 0; i < s; i++)
			{
				TBuffer *curr = src_messagess[i];
				if(curr != NULL)
					delete curr;
			}
			src_messagess.clear();

			s = dest_messagess.size();
			for(int i = 0; i < s; i++)
			{
				TBuffer *curr = dest_messagess[i];
				if(curr != NULL)
					delete curr;
			}
			dest_messagess.clear();
			//pthread_mutex_unlock(&sych);
		}
		catch(exception &e)
		{
			Log::ERROR( "[Connection::stop] "+string(e.what()) );
		}
	}

	deque<TBuffer *>	src_messagess;
	deque<TBuffer *>	dest_messagess;


	bool IsRuning()
	{
		bool ret;
		//pthread_mutex_lock(&sych);
		ret = isRuning;
		//pthread_mutex_unlock(&sych);
		return ret;
	}

	void SetIsRuning(bool new_val)
	{
		//pthread_mutex_lock(&sych);
		isRuning = new_val;
		//pthread_mutex_unlock(&sych);
	}

private:

	bool isRuning;

	IPort *src_port;
	IPort *dest_port;

	Word error_cnt;
	Word life_time;
	//pthread_mutex_t 	sych;

};


class PortForwarding: public Thread
{
public:

	PortForwarding(IServer *s, IPort *d)
	{
		try
		{
			mtu = 2048;
			src = s;
			dest = d;
			isRuning = false;

			//src->OnDataRecive = transportData;
			src->ptr = NULL;
			if(dest != NULL)
				src->ptr = &dest->messagess;


			errCNT 		= 0;
			closeCNT 	= 0;
		}
		catch(exception &e)
		{
			Log::ERROR( "[PortForwarding::PortForwarding] "+string(e.what()) );
		}
	}

	~PortForwarding()
	{
		try
		{
			stop();
			if(src != NULL){
				delete src;
				src = NULL;
			}
			if(dest != NULL){
				delete dest;
				dest = NULL;
			}

		}
		catch(exception &e)
		{
			Log::ERROR( "[PortForwarding::~PortForwarding] "+string(e.what()) );
		}

	}

	void run()
	{
		try
		{
			while(isRuning)
			{
				src->Start();

				//Log::DEBUG( "[destPort] src->IsConnected()="+toString(src->IsConnected()));
				if(src->IsConnected())
				{
					closeCNT = 0;
					if(dest->ConnectOK())
					{
						if(!dest->messagess.empty())
						{
							Log::INFO("PortForwarding dest->messagess.size="+toString(dest->messagess.size()));
							TBuffer *currMessage = dest->messagess.front();
							if(currMessage != NULL)
							{
								Log::INFO("PortForwarding::Sending currMessage.Len="+ toString( currMessage->DataLen ));
								int len = dest->Send(currMessage->BUF,currMessage->DataLen);
								if(len >= 0)
								{
									Log::INFO("PortForwarding::Send currMessage.Len="+ toString( len ));
									errCNT = 0;
									dest->messagess.pop_front();
									if(currMessage != NULL)
									{
										delete currMessage;
										currMessage = NULL;
									}
									Log::DEBUG( "[destPort] send ok len="+toString(len));
								}
								else
								{
									Log::INFO("PortForwarding::Send error");
									if(errCNT++ > 10)
									{
										errCNT = 0;
										dest->messagess.pop_front();
										if(currMessage != NULL)
										{
											Log::DEBUG( "[destPort] errCNT > 10");
											delete currMessage;
											currMessage = NULL;
										}
									}
									Log::ERROR( "[destPort] send error len ="+toString(len));
									continue;
								}
							}
						}
						TBuffer	Buffer(dest->mtu);
						Buffer.DataLen = dest->Recv(Buffer.BUF, dest->mtu);
						if(Buffer.DataLen > 0)
						{//ok
							Log::INFO("PortForwarding::Recv OK DataLen="+toString(Buffer.DataLen));
							if(src->messagess.size() < MAX_MESS_IN_PORTFORWARD)
							{
								TBuffer *nbuf = new TBuffer(Buffer.DataLen);
								for(int i = 0; i < Buffer.DataLen; i++)
									nbuf->BUF[i] = Buffer.BUF[i];
								nbuf->DataLen = Buffer.DataLen;
								src->messagess.push_back(nbuf);
								Log::INFO("PortForwarding::transportData messagess.size="+toString(src->messagess.size()));
							}

						}
					}
				}
				else
				{
					if(dest->IsConnected())
					{
						//Log::DEBUG( "[destPort] closeCNT="+toString(closeCNT));
						if(closeCNT++ > 60)
						{
							Log::DEBUG( "[destPort] closeCNT="+toString(closeCNT));
							closeCNT = 0;
							dest->Close();
						}
						sleep(1);
					}
				}
			}
		}
		catch(exception &e)
		{
			Log::ERROR( "[PortForwarding::run] "+string(e.what()) );
		}
		isRuning = false;
		if(src != NULL)  src->Stop();
		if(dest != NULL) dest->Close();
	}

	void start()
	{
		try
		{
			if(src == NULL || dest == NULL)
				throw runtime_error("start error (src == NULL || dest == NULL)!");
			if(isRuning)
				stop();
			if( thread_start() != 0)
				throw runtime_error("start error!");
			isRuning = true;
			Log::DEBUG( "[PortForwarding::start]");

		}
		catch(exception &e)
		{
			Log::ERROR( "[PortForwarding::start] "+string(e.what()) );
		}
	}

	void stop()
	{
		try
		{
			isRuning = false;
			if(src != NULL){
				src->Stop();
			}

			if(dest != NULL){
				dest->Close();
			}
			this->wait();
		}
		catch(exception &e)
		{
			Log::ERROR( "[PortForwarding::start] "+string(e.what()) );
		}
	}


	void *DoTrasport(void *data)
	{
		TBuffer *retBuf = NULL;
		try
		{
				TBuffer *buf =  (TBuffer *)data;
				if(buf != NULL) {
					Log::DEBUG("[PortForwarding] DoTrasport start");


					Log::DEBUG("[PortForwarding] messagess.size="+toString(dest->messagess.size()));

					//Log::DEBUG("dest->messagess.size() < MAX_MESS_IN_PORT_FORVARD="+toString(dest->messagess.size() < MAX_MESS_IN_PORT_FORVARD));
					if(dest->messagess.size() < MAX_MESS_IN_PORTFORWARD){

						TBuffer *nbuf = new TBuffer(buf->DataLen);
						for(int i = 0; i < buf->MaxSize; i++)
							nbuf->BUF[i] = buf->BUF[i];

						nbuf->DataLen = buf->DataLen;

						Log::INFO("[PortForwarding] DoTrasport DataLen="+toString(nbuf->DataLen));
					}
				}


		}
		catch(exception &e)
		{
			Log::ERROR( "[PortForwarding::run] "+string(e.what()) );
		}
		Log::DEBUG("[PortForwarding] DoTrasport end");
		return retBuf;
	}


private:

	static void *transportData(void *data, void *ptr)
	{
		void *ret = NULL;
		try
		{
			Log::INFO("PortForwarding::transportData start");
			if(ptr != NULL){
				PortForwarding *pf = (PortForwarding*)ptr;
				if(pf != NULL){
					ret = pf->DoTrasport(data);
				}
			}
		}
		catch(exception &e)
		{
			Log::ERROR( "[PortForwarding::transportData] "+string(e.what()) );
		}
		Log::DEBUG("[PortForwarding] transportData end");
		return ret;
	}

	int  mtu;
	bool isRuning;
	IServer *src	= NULL;
	IPort   *dest	= NULL;

	int  errCNT;
	long closeCNT;
};


class ForwardingRule: public Thread, public IRule
{
public:

	ForwardingRule(IListener *src, SocketSettings dest)
	{
		listener 		= src;
		dest_settings 	= dest;
		isRuning 		= false;
		isPaused 		= false;
	}

	void run()
	{
		try
		{
			while( isRuning )
			{
				if(isPaused) continue;
				if(listener != NULL)
				{
					listener->Start();
					IPort *client = listener->AcceptSocket();
					//Log::DEBUG("[FirewallRule::run] client=" + toString((int)client));
					if(client != NULL)
					{
						Log::DEBUG("[FirewallRule::run] client=" + toString((int)client));
						Connection *new_conn = new Connection(client, dest_settings);
						new_conn->start();
						connections.push_back(new_conn);
						detectClosedConnections();
					}
				}
			}
		}
		catch(exception &e)
		{
			Log::ERROR( "[FirewallRule::run global error] "+string(e.what()) );
		}
		if(listener != NULL) listener->Stop();
		isRuning = false;
	}

	void start()
	{
		try
		{
			if( thread_start() != 0)
				throw runtime_error("start error!");
			isRuning = true;
			isPaused = false;
			Log::DEBUG( "[FirewallRule::start]");
		}
		catch(exception &e)
		{
			Log::ERROR( "[FirewallRule::start] "+string(e.what()) );
		}
	}

	void stop ()
	{
		try
		{
			isPaused = true;
			isRuning = false;
			if(listener != NULL)
			{
				listener->Stop();
				listener = NULL;
			}
			this->wait();
			clear();
		}
		catch(exception &e)
		{
			Log::ERROR( "[FirewallRule::stop] "+string(e.what()) );
		}
	}

	bool IsRuning()
	{
		return isRuning;
	}

	void detectClosedConnections()
	{
		Log::DEBUG( "[FirewallRule::detectClosedConnections] start");
		int delete_index = -1;
		do
		{
			delete_index = -1;
			for(int i = 0; i < connections.size(); i++)
			{
				Connection *curr = connections[i];
				if(curr != NULL)
				{
					if(!curr->IsRuning())
					{
						delete_index = i;
						break;
					}
				}
			}
			if(delete_index >= 0)
			{
				Log::DEBUG( "[FirewallRule::detectClosedConnections] delete_index="+toString(delete_index)+" connections.size=" + toString(connections.size()));
				Connection *curr = connections[delete_index];
				if(curr != NULL)
				{
					delete curr;
					connections.erase(connections.begin()+(Word)delete_index);
				}
			}
		}
		while(delete_index >= 0);
		Log::DEBUG( "[FirewallRule::detectClosedConnections] delete_index OK rules.size=" + toString(connections.size()) );
	}

private:

	void clear()
	{
		Log::DEBUG( "[FirewallRule::stop] clear connections start size="+toString(connections.size()));
		for(auto curr: connections)
		{
			if(curr != NULL)
			{
				if(curr->IsRuning())
					curr->stop();
				delete curr;
			}
		}
		connections.clear();
		Log::DEBUG( "[FirewallRule::stop] clear connections OK");
	}

	IListener 				*listener;
	SocketSettings 			dest_settings;
	vector<Connection *> 	connections;

	bool 		isRuning;
	bool        isPaused;

};



class Tfirewall
{
public:
	Tfirewall(){
		isRuning = false;
		isEnable = false;
	}
	virtual ~Tfirewall(){
		Stop();
	}

	void Init(string network, string firewall)
	{

		string rulesList = uci::Get(firewall, "rules", "rulename");
		if(rulesList.length() > 0)
		{
			vector<string> rules_list = TBuffer::Split(rulesList, " ");
			for(auto curr: rules_list)
			{
				if(curr.length() > 0)
				{
					string src = uci::Get(firewall, curr, "src");
					if(src.length() > 0)
					{
						string if_name = uci::Get(network, src, "ifname");
						if(if_name == "3g")
							if_name +="-wan";
						string src_ip = Socket::getIpAddress(if_name);
						DWord src_timeout  = 100;
						string src_t = uci::Get(firewall, curr, "src_timeout");
						if(src_t.length() > 0)
							src_timeout = atol(src_t.c_str());
						string sp = uci::Get(firewall, curr, "src_port");
						if(sp.length() == 0)
							break;
						DWord sport = atol(sp.c_str());
						SocketSettings src_sett;
						src_sett.IpAddress 				= src_ip;
						src_sett.IpPort 				= sport;
						src_sett.SocketRecvTimeout 		= src_timeout;
						src_sett.SocketSendTimeout 		= src_timeout;

						Log::DEBUG( "[rule_name="+curr+"]" );
						Log::DEBUG(" src="+src+" if_name="+if_name+" src_ip="+src_ip+":"+sp+" timeout="+toString(src_timeout)+"" );

						struct in_addr m_addr;
						string dest_ip 	= uci::Get(firewall, curr, "dest_addr");
						string dp 		= uci::Get(firewall, curr, "dest_port");
						if( !(dest_ip.size() > 0 && inet_aton(dest_ip.c_str(), &m_addr) != 0 && dp.length() > 0) )
							break;
						DWord dest_timeout = 100;
						string dest_t = uci::Get(firewall, curr, "dest_timeout");
						if(dest_t.size() > 0)
							dest_timeout = atol(dest_t.c_str());
						DWord dport = atol(dp.c_str());
						SocketSettings dest_sett;
						dest_sett.IpAddress 				= dest_ip;
						dest_sett.IpPort 					= dport;
						dest_sett.SocketRecvTimeout 		= dest_timeout;
						dest_sett.SocketSendTimeout 		= dest_timeout;
						dest_sett.Proto						= "tcp";

						Log::DEBUG(" dest_ip="+dest_ip+":"+dp+" timeout="+toString(dest_timeout)+"" );
						string src_proto = "tcp";
						string proto = uci::Get(firewall, curr, "src_proto");
						if(proto.length() > 0)
							src_proto = proto;
						proto = uci::Get(firewall, curr, "dest_proto");
						if(proto.length() > 0)
							dest_sett.Proto = proto;
						IListener *listener = NULL;
						if(src_proto == "tcp")
							listener = new TcpListener(&src_sett);
						else if(src_proto == "udp")
						{
							//listener = new UdpListener(src_sett);
						}
						if(listener !=NULL)
						{
							IRule *rule = new ForwardingRule( listener, dest_sett );
							rules.push_back(rule);
							Log::DEBUG("rule="+curr+" push_back");
						}
					}
				}
			}
		}


/*
		string src = uci::Get(firewall, "ibp", "src");
		if(src.length() > 0)
		{
			string if_name = uci::Get(network, src, "ifname");
			if(if_name == "3g")
				if_name +="-wan";
			string src_ip = Socket::getIpAddress(if_name);
			Log::DEBUG( "[firewall::Init] src_ip=["+src_ip+"]" );

			DWord src_timeout  = 100;
			string src_t = uci::Get(firewall, "ibp", "src_timeout");
			if(src_t.size() > 0)
			{
				src_timeout = atol(src_t.c_str());
				Log::DEBUG( "[firewall::Init] src_timeout=["+toString(src_timeout)+"]" );
			}
			DWord sport = 81;
			string sp = uci::Get(firewall, "ibp", "src_port");
			if(sp.size() > 0)
				sport = atol(sp.c_str());
			Log::DEBUG( "[firewall::Init] src_port=["+toString(sport)+"]" );

			SocketSettings src_sett;
			src_sett.IpAddress 				= src_ip;
			src_sett.IpPort 				= sport;
			src_sett.SocketRecvTimeout 		= src_timeout;
			src_sett.SocketSendTimeout 		= src_timeout;

			DWord dport = 80;
			struct in_addr m_addr;
			string lan_ip = uci::Get(firewall, "ibp", "dest_addr");
			if(lan_ip.size() > 0 && inet_aton(lan_ip.c_str(), &m_addr) != 0)
			{
				Log::DEBUG( "[firewall::Init] dest_ip=["+lan_ip+"]" );
				string p = uci::Get(firewall, "ibp", "dest_port");
				if(p.size() > 0)
					dport = atol(p.c_str());
				Log::DEBUG( "[firewall::Init] dest_port=["+toString(dport)+"]" );
			}
			else
				lan_ip = "192.168.1.10";

			DWord dest_timeout = 100;
			string dest_t = uci::Get(firewall, "ibp", "dest_timeout");
			if(dest_t.size() > 0)
			{
				dest_timeout = atol(dest_t.c_str());
				Log::DEBUG( "[firewall::Init] dest_timeout=["+toString(dest_timeout)+"]" );
			}

			SocketSettings dest_sett;
			dest_sett.IpAddress 				= lan_ip;
			dest_sett.IpPort 					= dport;
			dest_sett.SocketRecvTimeout 		= dest_timeout;
			dest_sett.SocketSendTimeout 		= dest_timeout;
			dest_sett.Proto						= "tcp";


			string src_proto = "tcp";
			string proto = uci::Get(firewall, "ibp", "src_proto");
			if(proto.length() > 0)
				src_proto = proto;
			proto = uci::Get(firewall, "ibp", "dest_proto");
			if(proto.length() > 0)
				dest_sett.Proto = proto;

			IListener *listener = NULL;
			if(src_proto == "tcp")
				listener = new TcpListener(&src_sett);
			else if(src_proto == "udp")
			{
				//listener = new UdpListener(src_sett);
			}
			if(listener !=NULL)
			{
				IRule *rule = new ForwardingRule( listener ,dest_sett );
				rules.push_back(rule);
			}

		}

		string src_ssl = uci::Get(firewall, "ssl", "src");
		if(src_ssl.length() > 0)
		{
			string if_name = uci::Get(network, src_ssl, "ifname");
			if(if_name == "3g")
				if_name +="-wan";
			string src_ip = Socket::getIpAddress(if_name);
			Log::DEBUG( "[firewall::Init] "+src_ssl+" src_ip=["+src_ip+"]" );

			DWord src_timeout  = 100;
			string src_t = uci::Get(firewall, "ssl", "src_timeout");
			if(src_t.size() > 0)
			{
				src_timeout = atol(src_t.c_str());
				Log::DEBUG( "[firewall::Init] "+src_ssl+" src_timeout=["+toString(src_timeout)+"]" );
			}
			DWord sport = 443;
			string sp = uci::Get(firewall, "ssl", "src_port");
			if(sp.size() > 0)
				sport = atol(sp.c_str());
			Log::DEBUG( "[firewall::Init] "+src_ssl+" src_port=["+toString(sport)+"]" );

			SocketSettings src_sett;
			src_sett.IpAddress 				= src_ip;
			src_sett.IpPort 				= sport;
			src_sett.SocketRecvTimeout 		= src_timeout;
			src_sett.SocketSendTimeout 		= src_timeout;

			DWord dport = 443;
			struct in_addr m_addr;
			string lan_ip = uci::Get(firewall, "ssl", "dest_addr");
			if(lan_ip.size() > 0 && inet_aton(lan_ip.c_str(), &m_addr) != 0)
			{
				Log::DEBUG( "[firewall::Init] "+src_ssl+" dest_ip=["+lan_ip+"]" );
				string p = uci::Get(firewall, "ssl", "dest_port");
				if(p.size() > 0)
					dport = atol(p.c_str());
				Log::DEBUG( "[firewall::Init] "+src_ssl+" dest_port=["+toString(dport)+"]" );
			}
			else
				lan_ip = "192.168.1.10";

			DWord dest_timeout = 100;
			string dest_t = uci::Get(firewall, "ssl", "dest_timeout");
			if(dest_t.size() > 0)
			{
				dest_timeout = atol(dest_t.c_str());
				Log::DEBUG( "[firewall::Init] "+src_ssl+" dest_timeout=["+toString(dest_timeout)+"]" );
			}

			SocketSettings dest_sett;
			dest_sett.IpAddress 				= lan_ip;
			dest_sett.IpPort 					= dport;
			dest_sett.SocketRecvTimeout 		= dest_timeout;
			dest_sett.SocketSendTimeout 		= dest_timeout;
			dest_sett.Proto						= "tcp";


			string src_proto = "tcp";
			string proto = uci::Get(firewall, "ssl", "src_proto");
			if(proto.length() > 0)
				src_proto = proto;
			proto = uci::Get(firewall, "ssl", "dest_proto");
			if(proto.length() > 0)
				dest_sett.Proto = proto;

			IListener *listener = NULL;
			if(src_proto == "tcp")
				listener = new TcpListener(&src_sett);
			else if(src_proto == "udp")
			{
				//listener = new UdpListener(src_sett);
			}
			if(listener !=NULL)
			{
				IRule *rule = new ForwardingRule( listener ,dest_sett );
				rules.push_back(rule);
			}

		}

*/

	}

	void ReloadConfig(void)
	{

	}

	void Start(void)
	{
		try
		{
			if( isRuning )
			{
				int delete_index = -1;
				for(int i = 0; i < rules.size(); i++)
				{
					IRule *curr = rules[i];
					if(curr != NULL)
					{
						if(!curr->IsRuning())
						{
							delete_index = i;
							break;
						}
					}
				}
				if(delete_index >= 0)
				{
					Log::DEBUG( "[firewall::Start] delete_index="+toString(delete_index)+" rules.size=" + toString(rules.size()));
					IRule *curr = rules[delete_index];
					if(curr != NULL)
					{
						delete curr;
						rules.erase(rules.begin()+(Word)delete_index);
					}
					Log::DEBUG( "[firewall::Start] delete_index OK rules.size=" + toString(rules.size()) );
				}
			}
			else
			{
				for(auto curr: rules)
				{
					if(curr != NULL)
					{
						curr->start();
					}
				}
				isRuning = true;
			}
		}
		catch(exception &e)
		{
			Log::ERROR( "[firewall::Start] "+string(e.what()) );
		}
	}

	void Stop(void)
	{
		try
		{
			for(auto curr: rules)
			{
				if(curr != NULL)
				{
					if(curr->IsRuning())
						curr->stop();
					delete curr;
				}
			}
			rules.clear();
		}
		catch(exception &e)
		{
			Log::ERROR( "[firewall::Stop] "+string(e.what()) );

		}
		isRuning = false;
	}

private:
	bool          isRuning;
	bool          isEnable;

	vector<IRule*> rules;
};



} /* namespace firewall */

#endif /* FIREWALL_H_ */


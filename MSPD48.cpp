//============================================================================
// Name        : MSPD48
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, ANSI-style
//============================================================================

#include <iostream>
#include <stdexcept>
#include <stdlib.h>
//#include <stdio.h>ClimatManagerSettings
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <deque>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
//#include <netdb.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <vector>
#include <list>
#include <deque>
#include <sys/wait.h>
#include <signal.h>
#include <resolv.h>
#include <ucontext.h>
#include <uci.h>
//#include <mysql.h>
#include <execinfo.h>
//////////////////////////////////
#include <Classes.h>
#include <SocketPort.h>
#include <CommPort.h>
#include <Ecounter.h>
#include <Ibp.h>
#include <CmdMessages.h>
#include <FreeCooling.h>
#include <DiselPanel.h>
#include <VirtualPort.h>
#include <ModbusRTU.h>
#include <Climat.h>
#include <firewall.h>
#include <DAL.h>
#include <SelfTesting.h>
#include <InputOutputManager.h>
//////////////////////////////////
using namespace std;
using namespace log4net;
using namespace MyClassTamlates;
using namespace firewall;
using namespace datalayer;
//////////////////////////////////
static void signal_error(int sig, siginfo_t *si, void *ptr);
void 		SetPidFile(string filename);
int 		LoadConfig(string filename);
int 		ReloadConfig(void);
int 		InitWorkThreads(void);
void 		DestroyWorkThreads(void);
int 		SetFdLimit(int max);
///============================
void InitRouterTask(void);
void InitRS485Task(void);
void InitRS485Task2(void);
void InitUdpServerTask(void);
void InitInnerRs485Task(void);
void InitTimerTask(void);
void InitCounterTask(void);
void InitCurrDataTask(void);
void InitIbpTask(void);
void InitFreeTask(void);
void InitDguTask(void);
void InitVirtualPortTask(void);
void InitModbus1Task(void);
void InitModbus2Task(void);
void InitFirewallTask(void);
void InitFirewallTask(void);
void InitSQL(void);
bool InitClimatTask(void);
void InitSelfTestingTask(void);
void InitFifoBuffer(void);
void InitInputsDetectTask(void);
///============================
int 		 WorkProc(void);
void 		*RouterTask(void *ptr);
void 		*RS485Task(void *ptr);
void 		*RS485Task2(void *arg);
void 		*UdpServerTask(void *arg);
void 		*InnerRs485Task(void *arg);
void 		*TimerTask(void *arg);
void 		*MilSecTask(void *arg);
void 		*DetectStatesTask(void *arg);
void 		*FifoTask(void *arg);
void 		*CounterTask(void *arg);
void 		*DoCounterJobsTask(void *arg);
void 		*CurrDataTask(void *arg);
void 		*IbpTask(void *arg);
void 		*FreeTask(void *arg);
void 		*DguTask(void *arg);
void 		*VirtualPortTask(void *arg);
void 		*ModbusTask1(void *arg);
void 		*ModbusTask2(void *arg);
void 		*ReadOtherTask(void *arg);
void 		*FirewallTask(void *arg);
void 		*ManageClimatTask(void *arg);
void 		*SelfTestingTask(void *arg);
void 		*InputsDetectTask(void *arg);
//=======================================================
void RouterOnAnswerErrorExecute(void *Par);
void GenerateDUMMY_NS_command(void *Par);
void OnDataFromMKK(void *Par);
void OnDataFromMKK_1(void *Par);
void OnDataFromMKK_2(void *Par);
void OnDataFromMKK_3(void *Par);
void OnDataFromMKAB(void *Par);
void OnDataFromModBusRTU1(void *Par, void* p);
void OnDataFromModBusRTU2(void *Par, void* p);
///////// Defined /////////////////////////
//=======================================================
#define STD_BUFFER_SIZE			5000
#define RS485_BUFFER_SIZE		2000
#define FD_LIMIT 				1024*10*2
#define CHILD_NEED_WORK 		1
#define CHILD_NEED_TERMINATE 	2
//#define PID_FILE 				"/var/run/mspd_daemon.pid"
//#define CONFIG_FILE			"mspd48"
//#define FIFO_FILE 			"/mnt/data/fifo.log"
//=======================================================
///////// Global Variables /////////////////////////
//=======================================================
bool 				 GlobalStartedFlag = false;
Data_Params          Configuration;
Word64				 SecCount;
Word64				 MilSecCount;
TDateTime			 SystemTime;
TFastTimer           ObjectFunctionsTimer(6, &SecCount);//2

TCmd_msg			 	 BaseRouterCmd;
ICmd					*RouterExeCmd = NULL;
TBuffer				 	 SocketBuffer(STD_BUFFER_SIZE);
sync_deque<TCmdMessage*> SocketCmdMessages;
SocketPort 		   		 SockPort;

TCmd_msg			 	 BaseRS485Cmd;
ICmd					*RS485ExeCmd  = NULL;
TBuffer				 	 RS485Buffer(RS485_BUFFER_SIZE);
sync_deque<TCmdMessage*> RS485CmdMessages;
IPort             		*RS485Port = NULL;
TGetCurrentDataTask 	 GetCurrDataObject;

TCmd_msg			 	 BaseRS485Cmd2;
ICmd				    *RS485ExeCmd2  = NULL;
TBuffer				 	 RS485Buffer2(RS485_BUFFER_SIZE);
sync_deque<TCmdMessage*> RS485CmdMessages2;
IPort             		*RS485Port2 = NULL;
TGetCurrentDataTask 	 GetCurrDataObject2;

TCmd_msg			 	 BaseUdpServerCmd;
ICmd				    *UdpServerExeCmd  = NULL;
TBuffer				 	 UdpServerBuffer(RS485_BUFFER_SIZE);
sync_deque<TCmdMessage*> UdpServerCmdMessages;
IPort             		*UdpServerPort = NULL;


TFifo<string>       FifoBuffer(3000);
TFifo<string> 		EcounterJobBuffer(10);


ComPort            EcounterPort;
ComPort            FreePort;
VirtualPort        VSP;

Tfirewall				FireWall;

TGetVersion         	GetVersionObject;
TEnergyCounterManager 	EnergyCounterManager;
TIbpManager 			IbpManager;
TFreeCoolingManager     FreeCoolingManager;
TDiselPanelManager      DiselPanelManager;
TExternClimatManager  	ExternClimatManager;
TInnerRs485Manager      InnerRs485Manager;

TMkkClimatManager    	MkkClimatManager;
TModbusManager          ModbusManager1;
TModbusManager          ModbusManager2;
TSelfTestingManager		SelfTestingManager;
TIOManager				IOManager;
//=======================================================
//MYSQL 					mysql;
//=======================================================
pthread_mutex_t syn_object;
//=======================================================
int main(int argc, char **argv)
{
	try
	{
		Log::Configure("MSPD48");
		Log::INFO("Starting MSPD48");

		int status;
		int pid = 0, sid = 0;
		//status = MonitorProc();

		status = LoadConfig("config.cfg");//argv[1]);
		Log::INFO("[main] LoadConfig OK");
		if(!status)
		{
			Log::ERROR("[main] Load config failed!");
			return -1;
		}
		pid = fork();//create child
		Log::INFO("[main] pid1 = " + toString(pid));
		if(pid == -1)// if child not started
		{
			string err = strerror(errno);
			Log::ERROR("[main] Start demon failed!" + err);
			return -1;
		}
		if(!pid)
		{// if it is child
			umask(0);// root for all created files
			sid = setsid();//create new session
		    Log::INFO("[main] sid1 = " + toString(sid));
			if((chdir("/")) < 0)
			{
				Log::ERROR("[main] Failed set root dir!" );
				exit(1);
			}
			status = WorkProc();//MonitorProc();
		}
		else// if it is parent
		{// end of process becouse start demon is complited
			Log::INFO("End1 Starting MSPD48");
			return 0;
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "GlobalError: "+string(e.what()) );
	}
	Log::INFO("End MSPD48");
	return 0;
}
//=======================================================
int WorkProc(void)
{
	struct sigaction sigact;
	sigset_t sigset;
	int signo;
	int status;

	sigact.sa_flags = SA_SIGINFO;
	sigact.sa_sigaction = signal_error;
	sigemptyset(&sigact.sa_mask);

	sigaction(SIGFPE, &sigact, 0);//error FPU
	sigaction(SIGILL, &sigact, 0);//wrong instruction
	sigaction(SIGSEGV, &sigact, 0);//memory access error
	sigaction(SIGBUS, &sigact, 0);//wire error (phisical memory)

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGQUIT);// signal stopped process by user
	sigaddset(&sigset, SIGINT);// signal stopped process by user from terminal
	sigaddset(&sigset, SIGTERM);// signal call of stop process
	sigaddset(&sigset, SIGUSR1);// user signal using for reload config
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	SetFdLimit(FD_LIMIT);

	Log::INFO("[DAEMON] Started!");
	status = InitWorkThreads();
	if(!status)
	{
		Log::INFO("[DAEMON] Create work threads OK!");
		SetPidFile(PID_FILE);
		for(;;)
		{
			sigwait(&sigset, &signo);
			if(signo == SIGUSR1)
			//if(siginfo.si_signo == SIGUSR1)//
			{
				status = ReloadConfig();
				if(status == 0)
				{
					Log::INFO("[DAEMON] Reload config failed!");
				}
				else
				{
					Log::INFO("[DAEMON] Reload config OK!");
				}
			}
			else
			{
				string signl = strsignal(signo);
				Log::INFO("[MONITOR] Come signal: "+signl);
				break;
			}
		}
		DestroyWorkThreads();
	}
	else
	{
		Log::ERROR("[DAEMON] Create work threads error!");
	}
	Log::INFO("[DAEMON] Stopped!");
	unlink(PID_FILE);//remove file whith pid
	return CHILD_NEED_TERMINATE;
}
//=======================================================
//*******************************************************
//=======================================================
void *RouterTask(void *arg)
{
	static int routerErrorCnt = 0;
	sleep(1);
	Log::INFO("[RouterTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[RouterTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
		InitRouterTask();
		GenerateDUMMY_NS_command(BaseRouterCmd.MainMessages);
		Log::INFO("[RouterTask] GenerateDUMMY_NS_command OK");

		while(GlobalStartedFlag)
		{
			try
			{
				if(RouterExeCmd != NULL)
				{
					if(RouterExeCmd->ConnectOK(&SockPort) == true)
					{
						routerErrorCnt = 0;
						//Log::DEBUG( "[RouterTask] Fase = "+toString((int)RouterExeCmd->GetFase()) );
						switch( RouterExeCmd->GetFase() )
					    {
					      case WAIT_MES:
					    	  RouterExeCmd = RouterExeCmd->GET_Message(&SockPort,&SocketBuffer);
					        break;
					      case RX_CMD:
					    	  RouterExeCmd = RouterExeCmd->CMD_Recv(&SockPort,&SocketBuffer);
					        break;
					      case EXE_CMD:
					    	  RouterExeCmd = RouterExeCmd->CMD_Execute(&SockPort,&SocketBuffer, &FifoBuffer);
					        break;
					      case TX_CMD:
					    	  RouterExeCmd = RouterExeCmd->CMD_Send(&SockPort,&SocketBuffer);
					        break;
					      default:
					    	  RouterExeCmd->SetFase(RX_CMD);
					    }
					}
					else
					{
						if(routerErrorCnt++ > 100)
						{
							routerErrorCnt = 0;
							Log::INFO("Router routerErrorCnt > 100 network restart");
							SockPort.Close();
							string rebootStr = "/etc/init.d/network restart";
							sendToConsole(rebootStr);
							Log::INFO("Router routerErrorCnt > 100 network restart OK");
						}
						sleep(5);
					}
				}
			}
			catch(exception &e)
			{
				Log::ERROR( "[RouterTask] exception: " + string( e.what() ) );
				RouterExeCmd->SetFase(RX_CMD);
				sleep(1);
			}
			//usleep(1000);
			//usleep(200000);//0.2s
			sleep(1);
		}
		SockPort.Close();
	}
	catch(exception &e)
	{
		Log::ERROR( "[RouterTask]" + string(e.what()) );
	}
	Log::INFO("[RouterTask] Ending");
	return 0;
}
//=======================================================
void *RS485Task(void *arg)
{
	sleep(1);
	Log::INFO("[RS485Task] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[RS485Task] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	if(Configuration.RS485Device.size() > 0 && Configuration.DeviceList.size() > 0)
	{
		try
		{
			InitRS485Task();
			while(GlobalStartedFlag)
			{
				try
				{
					if(RS485ExeCmd != NULL && RS485Port != NULL)
					{
						if(RS485ExeCmd->ConnectOK(RS485Port) == true)
						{
							switch( RS485ExeCmd->GetFase() )
							{
							  case WAIT_MES:
								  RS485ExeCmd = RS485ExeCmd->GET_Message(RS485Port,&RS485Buffer);
								break;
							  case RX_CMD:
								  RS485ExeCmd = RS485ExeCmd->CMD_Recv(RS485Port,&RS485Buffer);
								break;
							  case EXE_CMD:
								  RS485ExeCmd = RS485ExeCmd->CMD_Execute(RS485Port,&RS485Buffer);
								break;
							  case TX_CMD:
								  RS485ExeCmd = RS485ExeCmd->CMD_Send(RS485Port,&RS485Buffer);
								break;
							  default:
								  RS485ExeCmd->SetFase(RX_CMD);
							}
						}
						else
							sleep(1);
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[RS485Task]" + string( e.what() ) );
					sleep(1);
				}
				usleep(1000);
			}
			if(RS485Port != NULL )
				RS485Port->Close();
		}
		catch(exception &e)
		{
			Log::ERROR( "[RS485Task]" + string( e.what() ) );
		}

	}
	Log::INFO("[RS485Task] Ending");
	return 0;
}
//=======================================================
void *RS485Task2(void *arg)
{
	sleep(1);
	Log::INFO("[RS485Task2] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[RS485Task2] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	if(Configuration.RS485Device2.size() > 0 && Configuration.DeviceList2.size() > 0)
	{
		try
		{
			InitRS485Task2();
			while(GlobalStartedFlag)
			{
				try
				{
					if(RS485ExeCmd2 != NULL && RS485Port2 != NULL)
					{
						if(RS485ExeCmd2->ConnectOK(RS485Port2) == true)
						{
							switch( RS485ExeCmd2->GetFase() )
							{
							  case WAIT_MES:
								  RS485ExeCmd2 = RS485ExeCmd2->GET_Message(RS485Port2, &RS485Buffer2);
								break;
							  case RX_CMD:
								  RS485ExeCmd2 = RS485ExeCmd2->CMD_Recv(RS485Port2, &RS485Buffer2);
								break;
							  case EXE_CMD:
								  RS485ExeCmd2 = RS485ExeCmd2->CMD_Execute(RS485Port2, &RS485Buffer2);
								break;
							  case TX_CMD:
								  RS485ExeCmd2 = RS485ExeCmd2->CMD_Send(RS485Port2, &RS485Buffer2);

								break;
							  default:
								  RS485ExeCmd2->SetFase(RX_CMD);
							}
						}
						else
							sleep(1);
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[RS485Task2]" + string( e.what() ) );
					sleep(1);
				}
				usleep(1000);
			}
			if(RS485Port2 != NULL )
				RS485Port2->Close();
		}
		catch(exception &e)
		{
			Log::ERROR( "[RS485Task2]" + string( e.what() ) );
		}
	}

	Log::INFO("[RS485Task2] Ending");
	return 0;
}
//=======================================================
void *UdpServerTask(void *arg)
{
	sleep(15);
	Log::INFO("[UdpServerTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[UdpServerTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
			InitUdpServerTask();
			while(GlobalStartedFlag)
			{
				try
				{
					if(UdpServerExeCmd != NULL && UdpServerPort != NULL)
					{
						if(UdpServerExeCmd->ConnectOK(UdpServerPort) == true)
						{

							//Log::ERROR( "[UdpServerTask] ConnectOK");
							switch( UdpServerExeCmd->GetFase() )
							{
							  case WAIT_MES:
								  UdpServerExeCmd = UdpServerExeCmd->GET_Message(UdpServerPort, &UdpServerBuffer);
								break;
							  case RX_CMD:
								  UdpServerExeCmd = UdpServerExeCmd->CMD_Recv(UdpServerPort, &UdpServerBuffer);
								break;
							  case EXE_CMD:
								  UdpServerExeCmd = UdpServerExeCmd->CMD_Execute(UdpServerPort,&UdpServerBuffer);
								break;
							  case TX_CMD:
								  UdpServerExeCmd = UdpServerExeCmd->CMD_Send(UdpServerPort, &UdpServerBuffer);
								break;
							  default:
								  UdpServerExeCmd->SetFase(RX_CMD);
							}
						}
						else
						{
							//Log::ERROR( "[UdpServerTask] Not Connected");
							sleep(1);
						}
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[UdpServerTask]" + string( e.what() ) );
					sleep(1);
				}
				usleep(1000);
				//sleep(1);
			}

			if(UdpServerPort != NULL )
				UdpServerPort->Close();
	}
	catch(exception &e)
	{
		Log::ERROR( "[UdpServerTask]" + string( e.what() ) );
	}

	Log::INFO("[UdpServerTask] Ending");
	return 0;
}
//=======================================================
void *InnerRs485Task(void *arg)
{
	static Byte 	_connectErrorCNT = 0;
	sleep(1);
	Log::INFO("[InnerRs485Task] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[InnerRs485Task] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
		InitInnerRs485Task();
		if( InnerRs485Manager.Enable() )
		{
			while(GlobalStartedFlag)
			{
					try
					{
						   if(InnerRs485Manager.GetTimValue(COMMON_TIMER) <= 0)
						   {
							  if(InnerRs485Manager.GetPort() != NULL &&
									  InnerRs485Manager.GetPort()->ConnectOK() == true)
							  {
									 switch( InnerRs485Manager.Fase )
									 {
									   case CREATE_CMD:
										   InnerRs485Manager.CreateCMD();
										 break;
									   case SEND_CMD:
										   InnerRs485Manager.SendCMD();
										 break;
									   case RECV_CMD:
										   InnerRs485Manager.RecvCMD();
										 break;
									   case EXEC_CMD:
										   InnerRs485Manager.ExecCMD();
										 break;
									   default:
										   InnerRs485Manager.Fase = CREATE_CMD;
									 }
							  }
							  else
							  {
								  if(_connectErrorCNT++ >= 20)
								  {
										int portpid = (int)InnerRs485Manager.GetPort();
										Log::ERROR( "[InnerRs485Task] _connectErrorCNT=" + toString(_connectErrorCNT) + " Port pid="+toString(portpid));

									  _connectErrorCNT       = 0;
									  if(InnerRs485Manager.GetPort() != NULL)
										  InnerRs485Manager.GetPort()->Close();
									  InnerRs485Manager.ChangeController();
									  Log::ERROR( "[InnerRs485Task] _connectErrorCNT=" + toString((int)_connectErrorCNT) );
								  }
								  sleep(1);
							  }
						   }
					}
					catch(exception &e)
					{
						Log::ERROR( "[InnerRs485Task]" + string( e.what() ) );
						sleep(1);
					}
				usleep(1000);
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[InnerRs485Task]" + string( e.what() ) );
	}
	Log::INFO("[InnerRs485Task] Ending");
	return 0;
}
//=======================================================
void *TimerTask(void *arg)
{
	static int cnt = 0;
	pid_t pid = getpid();
	Log::INFO("[TimerTask] Starting, pid="+toString(pid));

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[TimerTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}

	InitTimerTask();
	while(GlobalStartedFlag)
	{
		try
		{
			bool flag = true;
			if(++cnt >= 60)
			{
				cnt = 0;
				long div = SystemTime.GetTimeDiv();
				Log::DEBUG("TimerTask Sync with linux time GetTimeDiv="+toString(div));
				if(div > 1)//dogoniaem
				{
					SystemTime.IncSecond();
					div = SystemTime.GetTimeDiv();
					Log::DEBUG("Otstaem GetTimeDiv="+toString(div));
				}
				if(div < -1)
				{
					flag = false;
					Log::DEBUG("Speshim GetTimeDiv="+toString(div));
				}
				int st = ReloadConfig();
				Log::DEBUG("TimerTask ReloadConfig st="+toString(st));
			}
			if(flag) SystemTime.IncSecond();
			SecCount.Inc();

			//if(cnt % 10 == 0 ) Log::DEBUG("TimerTask MilSecCount.Value=["+toString(MilSecCount.Value)+"] cnt="+toString(cnt));
			sleep(1);
		}
		catch(exception &e)
		{
			Log::ERROR(  "[TimerTask]" + string(e.what()) );
			sleep(1);
		}
	}
	Log::INFO("[TimerTask] Ending");
	return 0;
}
//=======================================================
void *ReadOtherTask(void *arg)
{
	sleep(5);
	Log::INFO("[ReadOtherTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[ReadOtherTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	//SQL_Mkk2Repository Repository("127.0.0.1", 3306);
	//Repository::Init("127.0.0.1", 3306);
	while(GlobalStartedFlag)
	{
		try
		{
		  /*string data;
			//cout << "Enter value:";
			getline(std::cin, data);
			cout << "{" << data << "}\n";
			if(data == "p"){
			}*/
			if(GetVersionObject.NeedFirstFlg == false)
			{

			}
		}
		catch(exception &e)
		{
			Log::ERROR(  "[ReadOtherTask]" + string(e.what()) );
			sleep(1);
		}
		sleep(1);
	}
	Log::INFO("[ReadOtherTask] Ending");
	return 0;
}
//=======================================================
void *MilSecTask(void *arg)
{
	pid_t pid = getpid();
	Log::INFO("[MilSecTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 2;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[MilSecTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}

	int ms_count = 100;
	while(GlobalStartedFlag)
	{
		try
		{
			//MilSecCount.Inc();

			MilSecCount.Inc(ms_count);
		}
		catch(exception &e)
		{
			Log::ERROR(  "[MilSecTask]" + string(e.what()) );
			sleep(1);
		}
		usleep(ms_count*990);
	}
	Log::INFO("[MilSecTask] Ending");
	return 0;
}
//=======================================================
void *DetectStatesTask(void *arg)
{
	static int profil_days = 1;
	static long cnt = 0;
	static bool needClearFifo = false;
	pid_t pid = getpid();
	InitFifoBuffer();
	sleep(15);
	Log::INFO("[DetectStatesTask] Starting, pid="+toString(pid));

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 2;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[DetectStatesTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	while(GlobalStartedFlag)
	{
		try
		{
			if(SystemTime.GetSynchAct())
			{
				//Log::INFO("[DetectStatesTask] before ENERGY_DetectStates");
	        	EnergyCounterManager.ENERGY_DetectStates();
	        	EnergyCounterManager.ENERGY_CreateMessageCMD( &FifoBuffer );

	        	IbpManager.DetectStates();
	        	IbpManager.CreateMessageCMD( &FifoBuffer );

	        	FreeCoolingManager.FREE_DetectStates();
	        	FreeCoolingManager.FREE_CreateMessageCMD( &FifoBuffer );

	        	DiselPanelManager.DetectStates();
	        	DiselPanelManager.CreateMessageCMD( &FifoBuffer );

	        	if(MkkClimatManager.Settings().IsAuto){
	        		ExternClimatManager.CLIMAT_DetectStates();
	        		ExternClimatManager.CLIMAT_CreateMessageCMD( &FifoBuffer );
	        	}
	        	InnerRs485Manager.DetectStates();
	        	InnerRs485Manager.CreateMessageCMD( &FifoBuffer );

	        	IOManager.DetectStates();
	        	IOManager.CreateMessageCMD( &FifoBuffer );

	        	GetVersionObject.VER_DetectStates();
	        	GetVersionObject.VER_CreateMessageCMD( &FifoBuffer );

	        	long long profil1cnt  = ObjectFunctionsTimer.GetTimValue(TIME_EVENT_TIMER1);
	        	long long profil2cnt  = ObjectFunctionsTimer.GetTimValue(TIME_EVENT_TIMER2);
	        	long long profil3cnt  = ObjectFunctionsTimer.GetTimValue(TIME_EVENT_TIMER3);
	        	long long selfprofcnt = ObjectFunctionsTimer.GetTimValue(TIME_EVENT_TIMER4);

	        	if(cnt++ >= 60)
	        	{
	        		cnt = 0;
	        		Log::DEBUG("DetectStatesTask selfprofcnt="+toString(selfprofcnt)+
	        				" profil1cnt="+toString(profil1cnt)+
	        				" profil2cnt="+toString(profil2cnt)+
	        				" profil3cnt=" + toString(profil3cnt) +
	        				" GlobalStartedFlag="+toString(GlobalStartedFlag));
	        	}

	        	if(selfprofcnt <= 0 && GlobalStartedFlag)
	        	{
	        		long period = 1800;//3600;//0.5h
	        		ObjectFunctionsTimer.SetTimer(TIME_EVENT_TIMER4, period);
	        		//Log::DEBUG( "TIME_EVENT_TIMER4 getEprofil period="+toString(period));
	        		string job = "CheckProfil_"+toString(profil_days);
	        		EcounterJobBuffer.push(job);

	        		if(profil_days++ >= 45)
	        			profil_days = 1;
	        	}

	        	if( profil1cnt <= 0 && GlobalStartedFlag)
	        	{//
	        		long period = MAX_SECOND_IN_DAY - 1;
	        		ObjectFunctionsTimer.SetTimer(TIME_EVENT_TIMER1, period);
	        		GetVersionObject.NeedFirstFlg = true;
	        		sleep(5);
	        	}

	        	if( profil2cnt <= 0 && GlobalStartedFlag)
	        	{
	        		long period = MAX_SECOND_IN_DAY - 1;//Configuration.EprofilPeriod - 1;
	        		ObjectFunctionsTimer.SetTimer(TIME_EVENT_TIMER2, period);
	        	}
	        	if( profil3cnt <= 0 && GlobalStartedFlag)
	        	{
	        		long period = MAX_SECOND_IN_DAY - 1;//Configuration.EprofilPeriod - 1;
	        		ObjectFunctionsTimer.SetTimer(TIME_EVENT_TIMER3, period);
	        	}
	        	//Log::INFO("[DetectStatesTask] after ENERGY_CreateMessageCMD FifoBuffer.size()="+toString(FifoBuffer.size()));
	        	if(FifoBuffer.size() > 0)
	        	{
	        		if( ObjectFunctionsTimer.GetTimValue(FIFO_SAVE_FILE_TIMER) <= 0)
	        		{
	        			ObjectFunctionsTimer.SetTimer(FIFO_SAVE_FILE_TIMER, Configuration.FifoPeriod);
	        			if(!GlobalStartedFlag)break;
	        			Log::DEBUG( "FIFO_SAVE_FILE_TIMER save fifo_file size="+toString(FifoBuffer.size()));
	        			FifoBuffer.save(Configuration.FifoFile);
	        		}
	        	}
        		else
        		{
        			if(FifoBuffer.isexist(Configuration.FifoFile)){
        				Log::DEBUG( "FIFO file="+Configuration.FifoFile + " is exist, remove_file");
        				FifoBuffer.remove_file(Configuration.FifoFile);
        			}
        			else{
        				//Log::DEBUG( "FIFO file="+Configuration.FifoFile + " is not exist");
        			}
        		}
			}
		}
		catch(exception &e)
		{
			Log::ERROR(  "[DetectStatesTask]" + string(e.what()) );
			sleep(1);
		}
		sleep(1);
	}
	Log::INFO("[DetectStatesTask] Ending");
	return 0;
}
//=======================================================
void *FifoTask(void *arg)
{
	sleep(1);
	pid_t pid = getpid();
	Log::INFO("[FifoTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[FifoTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
		GetVersionObject.Init(Configuration.DataTaskPeriod);
		while(GlobalStartedFlag)
		{
			try
			{
				if( SocketCmdMessages.size() == 0 && FifoBuffer.size() > 0)
				{
					string tmpmess = FifoBuffer.pop();
					if(tmpmess.length() > 0)
					{
			            vector<string> tmpList;
			            TBuffer::Split(tmpmess,">>",tmpList);
			            tmpmess.clear();
			            if(tmpList.size() >= 2)
			            {
			            	string cmd = "Data";
			            	bool answer = true;
			            	for(int i = 2; i < tmpList.size(); i++)
			            	{
			            		string curr = tmpList[i];
			            		vector<string> tl;
			            		TBuffer::Split(curr,"=",tl);
			            		if(tl.size() == 2)
			            		{
			            			if(tl[0] == "CMD"){
			            				cmd = tl[1];
			            			}
			            			else if(tl[0] == "NeedCmdAnswer"){
			            				answer = atoi(tl[1].c_str());
			            				Log::DEBUG("FifoTask NeedCmdAnswer="+toString(answer));
			            			}
			            		}
			            	}

			                TCmdMessage *message = new TCmdMessage();
			                message->DateTime = TBuffer::StringToDateTime(tmpList[0]);
			                message->CMD      = cmd;
			                message->Addr     = "";
			                message->State    = " ";
			                message->NeedCmdAnswer = answer;  // debug
			                message->Data.push_back(tmpList[1]);
			                tmpList.clear();
			                SocketCmdMessages.push_back(message);
			            }
					}
	 			}
			}
			catch(exception &e)
			{
				Log::ERROR( "[FifoTask]" + string( e.what() ) );
				sleep(1);
			}
			sleep(1);
			//usleep(1000);
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[FifoTask]" + string( e.what() ) );
	}
	Log::INFO("[FifoTask] Ending");
	return 0;
}
//=======================================================
void *CounterTask(void *arg)
{
	static int 	_connectErrorCNT = 0;
	pid_t pid = getpid();
	sleep(5);
	Log::INFO("[CounterTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[CounterTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
		InitCounterTask();
		if(EnergyCounterManager.Enable())
		{
			while(GlobalStartedFlag)
			{
				try
				{
					long long wt = EnergyCounterManager.GetTimValue(ADDITIONAL_TIMER);
					if(wt <= 0)//pause timer for direct connect
					{
						if(EnergyCounterManager.GetTimValue(COMMON_TIMER) <= 0)
						{
						   if(EnergyCounterManager.GetPort() != NULL &&
								   EnergyCounterManager.GetPort()->ConnectOK())
						   {
							 //Log::DEBUG( "[CounterTask] Fase = "+toString((int)EnergyCounterManager->GetFase()) );
							 switch( EnergyCounterManager.GetFase() )
							 {
							   case CREATE_CMD:
								 EnergyCounterManager.ENERGY_CreateCMD();
								 break;
							   case SEND_CMD:
								 EnergyCounterManager.ENERGY_SendCMD();
								 break;
							   case RECV_CMD:
								 EnergyCounterManager.ENERGY_RecvCMD();
								 break;
							   case EXEC_CMD:
								 EnergyCounterManager.ENERGY_ExecCMD();
								 break;
							   case DO_ECMD:
								 EnergyCounterManager.ENERGY_DoECMD( &FifoBuffer );
								 break;
							   default:
								EnergyCounterManager.SetFase(CREATE_CMD);
							 }
							 _connectErrorCNT       = 0;
						   }
						   else
						   {
								  if(_connectErrorCNT++ >= 20)
								  {
									  _connectErrorCNT       = 0;
									  if(EnergyCounterManager.GetPort()!=NULL)
										  EnergyCounterManager.GetPort()->Close();
									  EnergyCounterManager.ENERGY_ChangeCounter();
									  EnergyCounterManager.SetTimer(COMMON_TIMER, 30000);
								  }
								  Log::ERROR( "[CounterTask] _connectErrorCNT=" + toString(_connectErrorCNT) );
								  sleep(1);
						   }
						}
					}
					else
					{
						Log::INFO("[CounterTask] WAIT ADDITIONAL_TIMER wt="+toString(wt));
						sleep(60);
					}

				}
				catch(exception &e)
				{
					Log::ERROR( "[CounterTask]" + string( e.what() ) );
					sleep(1);
				}
				usleep(1000);
			}
			EnergyCounterManager.Stop();
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[CounterTask]" + string( e.what() ) );
	}
	Log::INFO("[CounterTask] Ending");
	return 0;
}
//=======================================================
void *DoCounterJobsTask(void *arg)
{
	pid_t pid = getpid();
	sleep(15);
	Log::INFO("[DoCounterJobsTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;

    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[DoCounterJobsTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
		if(EnergyCounterManager.Enable())
		{
			while(GlobalStartedFlag)
			{
				try
				{
					if( EcounterJobBuffer.size() > 0)
					{
						string job = EcounterJobBuffer.pop();
						if(job.length() > 0)
						{
							vector<string> lst = Split(job, "_");
							if(lst.size() == 2)
							{
								if(lst[0] == "CheckProfil")
								{
									int profil_days = atoi(lst[1].c_str());
					        		if(EnergyCounterManager.CheckProfil(profil_days))
					        		{
					        			EnergyCounterManager.SetGetProfilCmd( -1*profil_days, &FifoBuffer );
					        			//sleep(30);
					        		}
								}
							}
						}
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[DoCounterJobsTask]" + string( e.what() ) );
					sleep(10);
				}
				sleep(1);
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[DoCounterJobsTask]" + string( e.what() ) );
	}
	Log::INFO("[DoCounterJobsTask] Ending");
	return 0;
}
//=======================================================
void *IbpTask(void *arg)
{
	static int 	_connectErrorCNT = 0;
	sleep(20);//!!!
	Log::INFO("[IbpTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[IbpTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
		InitIbpTask();
		if( IbpManager.Enable() )
		{
			while(GlobalStartedFlag)
			{
				usleep(1000);
				if( VSP.IsRuning() ) continue;
				try
				{
					long long wt = IbpManager.GetTimValue(ADDITIONAL_TIMER);
					if(wt <= 0)//pause timer for direct connect
					{
						if(IbpManager.GetTimValue(COMMON_TIMER) <= 0)
						{
							if(IbpManager.GetPort() != NULL && IbpManager.GetPort()->ConnectOK() == true)
							{
								 switch( IbpManager.GetFase() )
								 {
								   case CREATE_CMD:
									   IbpManager.CreateCMD();
									 break;
								   case SEND_CMD:
									   IbpManager.SendCMD();
									 break;
								   case RECV_CMD:
									   IbpManager.RecvCMD();
									 break;
								   case EXEC_CMD:
									   IbpManager.ExecCMD();
									 break;
								   case DO_ECMD:
									   IbpManager.DoECMD( &FifoBuffer );
									 break;
								   default:
									   IbpManager.SetFase(CREATE_CMD);
								 }
								 _connectErrorCNT       = 0;
							}
							else
							{
								int sockpid = (int)IbpManager.GetPort();
								Log::ERROR( "[IbpTask] _connectErrorCNT=" + toString(_connectErrorCNT) + " Sock pid="+toString(sockpid));
								  if(_connectErrorCNT++ >= 100)
								  {
									  _connectErrorCNT       = 0;
									  if(IbpManager.GetPort() != NULL)
										  IbpManager.GetPort()->Close();
									  IbpManager.ChangeController();
									  IbpManager.SetTimer(COMMON_TIMER, 30000);
									  Log::ERROR( "[IbpTask] _connectErrorCNT=" + toString(_connectErrorCNT) + " wait 30s" );

									  if(IbpManager.GetPort()->PortIsLAN()){
											string rebootStr = "/etc/init.d/network restart";
											sendToConsole(rebootStr);
											Log::INFO("IbpManager PortIsLAN _connectErrorCNT > 100 network restart OK");
									  }
								  }
								  else
								  {
									  IbpManager.SetTimer(COMMON_TIMER, 5000);
								  }
								  sleep(1);
							}
						}
					}
					else
					{
						Log::INFO("[IbpTask] WAIT ADDITIONAL_TIMER wt="+toString(wt));
						sleep(60);
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[IbpTask]" + string( e.what() ) );
					sleep(1);
				}
			}
			IbpManager.Stop();
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[IbpTask]" + string( e.what() ) );
	}
	Log::INFO("[IbpTask] Ending");
	return 0;
}
//=======================================================
void *DguTask(void *arg)
{
	static Byte 	_connectErrorCNT = 0;
	pid_t pid = getpid();
	sleep(21);//!!!
	Log::INFO("[DguTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[DguTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
		InitDguTask();
		if( DiselPanelManager.Enable() )
		{
			while(GlobalStartedFlag)
			{
				try
				{
					long long wt = DiselPanelManager.GetTimValue(ADDITIONAL_TIMER);
					if(wt <= 0)//pause timer for direct connect
					{
						if(DiselPanelManager.GetTimValue(COMMON_TIMER) <= 0)
						{
						  if(DiselPanelManager.GetPort() != NULL && DiselPanelManager.GetPort()->ConnectOK() == true)
						  {
							  	 //Log::DEBUG( "[DguTask] Fase="+toString((int)DiselPanelManager.Fase) );
								 switch( DiselPanelManager.Fase )
								 {
								   case CREATE_CMD:
									   DiselPanelManager.CreateCMD();
									 break;
								   case SEND_CMD:
									   DiselPanelManager.SendCMD();
									 break;
								   case RECV_CMD:
									   DiselPanelManager.RecvCMD();
									 break;
								   case EXEC_CMD:
									   DiselPanelManager.ExecCMD();
									 break;
								   case DO_ECMD:
									   DiselPanelManager.DoECMD( &FifoBuffer );
									 break;
								   default:
									   DiselPanelManager.Fase = CREATE_CMD;
								 }
								 _connectErrorCNT       = 0;
						  }
						  else
						  {
							  if(_connectErrorCNT++ >= 20)
							  {
								  _connectErrorCNT       = 0;
								  if(DiselPanelManager.GetPort() != NULL)
									  DiselPanelManager.GetPort()->Close();
								  DiselPanelManager.ChangeController();
								  DiselPanelManager.SetTimer(COMMON_TIMER, 30000);
							  }
							  Log::ERROR( "[DguTask] _connectErrorCNT=" + toString((int)_connectErrorCNT) );
							  sleep(1);
						  }
						}
					}
					else
					{
						Log::INFO("[DguTask] WAIT ADDITIONAL_TIMER wt="+toString(wt));
						sleep(60);
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[DguTask]" + string( e.what() ) );
					sleep(1);
				}
				usleep(1000);
			}
			DiselPanelManager.Stop();
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[DguTask]" + string( e.what() ) );
	}
	Log::INFO("[DguTask] Ending");
	return 0;
}
//=======================================================
void *FreeTask(void *arg)
{
	static Word commPortErrorIndex = 0;
	pid_t pid = getpid();
	sleep(22);//!!!
	Log::INFO("[FreeTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[FreeTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	try
	{
		InitFreeTask();
		if(FreeCoolingManager.Enable())
		{
			Log::INFO("[FreeTask] FreeCoolingManager.Enable StartFase="+toString((int)FreeCoolingManager.Fase));
			while(GlobalStartedFlag)
			{
				try
				{
					if(FreeCoolingManager.GetTimValue(COMMON_TIMER) <= 0)
					{
						//Log::INFO("[FreeTask] FreeCoolingManager.Fase="+toString((int)FreeCoolingManager.Fase));
						if(FreePort.ConnectOK() == true)
						{
						   switch( FreeCoolingManager.Fase )
						   {
							   case CREATE_CMD:
								   FreeCoolingManager.FREE_CreateCMD();
								 break;
							   case SEND_CMD:
								   FreeCoolingManager.FREE_SendCMD();
								 break;
							   case RECV_CMD:
								   FreeCoolingManager.FREE_RecvCMD();
								 break;
							   case EXEC_CMD:
								   FreeCoolingManager.FREE_ExecCMD();
								 break;
							   case DO_ECMD:
								   FreeCoolingManager.FREE_DoECMD( );
								 break;
							   default:
								   FreeCoolingManager.Fase = RECV_CMD;
						   }
					   	   FreeCoolingManager.FREE_Manage();
					   	   if(FreeCoolingManager.NeedRestart)
					   	   {
						   	   FreeCoolingManager.NeedRestart = false;
						   	   FreeCoolingManager.FREE_Restart();
						   	   FreePort.Close();
						   	   FreeCoolingManager.SetTimer(COMMON_TIMER, 15000);
					   	   }
					   	   commPortErrorIndex = 0;
						}
						else
						{
							sleep(1);
							Log::INFO("[FreeTask] commPortErrorIndex error="+toString(commPortErrorIndex));
							if(commPortErrorIndex++ >= 100)
							{
								commPortErrorIndex = 0;
						  	  	Log::INFO("[FreeTask] commPortErrorIndex >= 100 !!! close port and wait 60 sec!");
						  	  	FreePort.Close();
						  	  	FreeCoolingManager.SetTimer(COMMON_TIMER, 60000);
							}
						}
					}
				}
				catch(exception &e)
				{
					Log::ERROR( "[FreeTask]" + string( e.what() ) );
					sleep(2);
				}
				usleep(1000);
				//sleep(1);
			}
			FreeCoolingManager.Stop();
			//Log::INFO("[FreeTask] Close FreePort");
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[FreeTask]" + string( e.what() ) );
	}
	Log::INFO("[FreeTask] Ending");
	return 0;
}
//=======================================================
void *CurrDataTask(void *arg)
{
	pid_t pid = getpid();
	Log::INFO("[CurrDataTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[CurrDataTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}

	try
	{
		InitCurrDataTask();
		sleep(10);
		while(GlobalStartedFlag)
		{
			try
			{
				if(GetCurrDataObject.GetTimValue(PERIUD_TIMER) <= 0)
				{
					GetCurrDataObject.FASE_PullingRS_485();
				}
				if(GetCurrDataObject2.GetTimValue(PERIUD_TIMER) <= 0)
				{
					GetCurrDataObject2.FASE_PullingRS_485();
				}
			}
			catch(exception &e)
			{
				Log::ERROR( "[CurrDataTask]" + string( e.what() ) );
				sleep(1);
			}
			sleep(1);
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[CurrDataTask]" + string( e.what() ) );
	}
	Log::INFO("[CurrDataTask] Ending");
	return 0;
}
//=======================================================
void *VirtualPortTask(void *arg)
{
	pid_t pid = getpid();
	sleep(30);//!!! important! InitVirtualPortTask after InitIbpTask
	Log::INFO("[VirtualPortTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[VirtualPortTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}

	InitVirtualPortTask();
	while(GlobalStartedFlag)
	{
		if(!Configuration.VPSEnable) break;
		try
		{
			VSP.ReloadConfig();
			VSP.DoWork();
			//VSP.SocketPort.Start();
			sleep(1);
		}
		catch(exception &e)
		{
			Log::ERROR(  "[VirtualPortTask]" + string(e.what()) );
			sleep(1);
		}
		//usleep(100000);//1000
		sleep(1);
	}
	VSP.SocketPort.Stop();
	Log::INFO("[VirtualPortTask] Ending");
	return 0;
}
//=======================================================
void *ModbusTask1(void *arg)
{
	pid_t pid = getpid();
	Log::INFO("[ModbusTask1] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[ModbusTask1] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	modbus_t *ctx;
	uint16_t tab_reg[10];
	try
	{
		InitModbus1Task();
		sleep(5);

		//if(ModbusRTUManager1.DeviceList.size() > 0)
		{
			while(GlobalStartedFlag)
			{
				try
				{
					//if(ModbusManager1.NeedReloadConfig.GetData()){ ModbusManager1.ReloadConfig();}

					if(ModbusManager1.IsStopped.GetData())
					{
						Log::INFO( "ModbusRTUManager1.IsStopped");
						sleep(5);
						continue;
					}
					//Log::INFO( "ModbusManager1.DeviceList.size()="+toString(ModbusManager2.DeviceList.size()));
					for(int i=0; i < ModbusManager1.DeviceList.size(); i++)
					{
						if(GlobalStartedFlag == false || ModbusManager1.IsStopped.GetData()) break;
						IModbusRegisterList *curr = ModbusManager1.DeviceList[i];
						if(curr != NULL)
						{
							//Log::INFO( "ModbusTask curr->DoCmd");
							bool st = curr->DoCmd();
							if(st){
								if(ModbusManager1.OnCmdComplited != NULL){
									ModbusManager1.OnCmdComplited(curr, curr->outPtr);
								}
								usleep(20000);
							}
							if(curr->NeedDelete())
							{
								ModbusManager1.SetNeedPopBack(true);
							}
						}
					}
					if(ModbusManager1.DeviceList.size() == 0){
						//sleep(1);
					}
					else
					{
						ModbusManager1.ManageDeviceList();
					}
					//Log::INFO( "#########################################################");
				}
				catch(exception &e)
				{
					Log::ERROR( "[ModbusTask1]" + string( e.what() ) );
					sleep(2);
				}
				usleep(1000);
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[ModbusTask1]" + string( e.what() ) );
	}
	Log::INFO("[ModbusTask1] Ending");
	return 0;
}
//=======================================================
void *ModbusTask2(void *arg)
{
	pid_t pid = getpid();
	Log::INFO("[ModbusTask2] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[ModbusTask2] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	modbus_t *ctx;
	uint16_t tab_reg[10];
	try
	{
		InitModbus2Task();
		sleep(5);

		//if(ModbusRTUManager2.DeviceList.size() > 0)
		{
			while(GlobalStartedFlag)
			{
				try
				{
					//if(ModbusManager2.NeedReloadConfig.GetData()){// ModbusManager2.ReloadConfig();}
					if(ModbusManager2.IsStopped.GetData())
					{
						Log::INFO( "ModbusRTUManager2.IsStopped");
						sleep(5);
						continue;
					}

					//Log::INFO( "ModbusManager2.DeviceList.size()="+toString(ModbusManager2.DeviceList.size()));

					for(int i=0; i < ModbusManager2.DeviceList.size(); i++)
					{
						if(GlobalStartedFlag == false || ModbusManager2.IsStopped.GetData()) break;
						IModbusRegisterList *curr = ModbusManager2.DeviceList[i];
						if(curr != NULL)
						{
							Log::INFO( "ModbusManager2 curr->DoCmd");
							bool st = curr->DoCmd();
							if(st){
								if(ModbusManager2.OnCmdComplited != NULL){
									//Log::DEBUG( "ModbusManager2 OnCmdComplited="+toString((long)ModbusManager2.OnCmdComplited));
									ModbusManager2.OnCmdComplited(curr, curr->outPtr);
								}
								usleep(20000);
							}
							if(curr->NeedDelete())
							{
								ModbusManager2.SetNeedPopBack(true);
							}
						}
					}
					if(ModbusManager2.DeviceList.size() == 0){
						//sleep(1);
					}
					else
					{
						ModbusManager2.ManageDeviceList();
					}
					//Log::INFO( "#########################################################");
				}
				catch(exception &e)
				{
					Log::ERROR( "[ModbusTask2]" + string( e.what() ) );
					sleep(2);
				}
				usleep(1000);
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[ModbusTask2]" + string( e.what() ) );
	}
	Log::INFO("[ModbusTask2] Ending");
	return 0;
}
//=======================================================
void *FirewallTask(void *arg)
{
	pid_t pid = getpid();
	sleep(20);
	Log::INFO("[FirewallTask] Starting");


    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[FirewallTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}
	//InitFirewallTask();

	//while(GlobalStartedFlag)
	{
		try
		{
			sleep(1);
			//FireWall.ReloadConfig();
			//FireWall.Start();
			//Log::INFO("eth0 ["+ServerPort::getIpAddress("eth0", IP)+"]");
			//Log::INFO("eth1 ["+ServerPort::getIpAddress("eth1", IP)+"]");
			//Log::INFO("ppp0 ["+ServerPort::getIpAddress("ppp0", IP)+"]");
			//Log::INFO("3g-wan ["+ServerPort::getIpAddress("3g-wan", IP)+"]");
			//sleep(10);

			usleep(1000);
		}
		catch(exception &e)
		{
			Log::ERROR(  "[FirewallTask]" + string(e.what()) );
			sleep(1);
		}
	}
	FireWall.Stop();
	Log::INFO("[FirewallTask] Ending");
	return 0;
}
//=======================================================
void *ManageClimatTask(void *arg)
{
	pid_t pid = getpid();
	sleep(10);
	Log::INFO("[ManageClimatTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[ManageClimatTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}

	bool initFlg = false;
	int  cnt = 30;
	while(GlobalStartedFlag && MkkClimatManager.Settings().Enable)
	{
		try
		{
			if(!initFlg)
			{
				if(SystemTime.GetSynchAct())
				{
					if( InitClimatTask() )
					{
						//
					}
					else
					{
						//
					}
					initFlg = true;
				}
			}
			if(cnt++ > 30)
			{
				cnt = 0;
				MkkClimatManager.ReloadConfig();
			}

			//sleep(1);
		}
		catch(exception &e)
		{
			Log::ERROR(  "[ManageClimatTask]" + string(e.what()) );
			sleep(1);
		}
		//usleep(100000);//
		sleep(1);
	}
	MkkClimatManager.Stop();
	Log::INFO("[ManageClimatTask] Ending");
	return 0;
}
//=======================================================
void *SelfTestingTask(void *arg)
{
	pid_t pid = getpid();
	sleep(21);
	Log::INFO("[SelfTestingTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;
    //pthread_setschedparam(pthread_self(), SCHED_RR, &nparam);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[SelfTestingTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}

	try
	{
		InitSelfTestingTask();
		while(GlobalStartedFlag)
		{
			try
			{
				SelfTestingManager.DoWork(&GlobalStartedFlag);

				//Log::DEBUG("[SelfTestingTask] working");
			}
			catch(exception &e)
			{
				Log::ERROR( "[SelfTestingTask]" + string( e.what() ) );
				sleep(5);
			}
			sleep(1);//ok
			//usleep(1000);
		}
		SelfTestingManager.Stop();
	}
	catch(exception &e)
	{
		Log::ERROR( "[SelfTestingTask]" + string( e.what() ) );
	}
	Log::INFO("[SelfTestingTask] Ending");
	return 0;
}
//=======================================================
void *InputsDetectTask(void *arg)
{
	pid_t pid = getpid();
	sleep(5);
	Log::INFO("[InputsDetectTask] Starting");

    int policy, s;
    struct sched_param param;
    struct sched_param nparam;
    nparam.sched_priority = 1;

    s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s == 0)
	{
		string pol = (policy == SCHED_FIFO) ? "SCHED_FIFO" : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???";
		Log::INFO("[InputsDetectTask] Starting, priority="+toString(param.sched_priority) + " policy="+pol);
	}

	try
	{
		InitInputsDetectTask();
		while(GlobalStartedFlag)
		{
			try
			{
				IOManager.DoWork(&GlobalStartedFlag);
			}
			catch(exception &e)
			{
				Log::ERROR( "[InputsDetectTask]" + string( e.what() ) );
				sleep(10);
			}
			sleep(5);//ok
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[InputsDetectTask]" + string( e.what() ) );
	}
	Log::INFO("[InputsDetectTask] Ending");
	return 0;
}
//*******************************************************
//=======================================================
int InitWorkThreads(void)
{
	try
	{
			int 	id1 = 1, id2 = 2, id3 = 3, id4 = 4, id5 = 5, id6 = 6, id7 = 7, id8 = 8,
					id9 = 9, id10 = 10, id11 = 11, id12 = 12, id13 = 13, id14 = 14, id15 = 15, id16 = 16,
					id17 = 17, id18 = 18, id19 = 19, id20 = 20, id21 = 21, id22 = 22, id23 = 23;

			GlobalStartedFlag = true;

			pthread_t 	ThreadRouter,
						ThreadRS485,
						ThreadRS485_2,
						ThreadTimer,
						ThreadFifo,
						ThreadCounter,
						ThreadCurrData,
						ThreadMilSec,
						ThreadDetect,
						ThredIbp,
						ThredFree,
						ThreadVirtual,
						ThreadReadKey,
						ThreadManageClimat,
						ThreadModbus1,
						ThreadModbus2,
						ThreadFirewall,
						ThreadDisel,
						ThreadInnerRs485,
						ThreadSelfTesting,
						ThreadUdpServer,
						ThreadDoCounterJobs,
						ThreadInputsDetect,
						ThreadReadOtherTask;

			pthread_create(&ThreadTimer, 			NULL, TimerTask, 	&id1);
			pthread_create(&ThreadMilSec, 			NULL, MilSecTask, 	&id2);
			pthread_create(&ThreadFifo, 			NULL, FifoTask, 	&id3);
			pthread_create(&ThreadRouter, 			NULL, RouterTask, 	&id4);
			pthread_create(&ThreadRS485, 			NULL, RS485Task, 	&id5);
			pthread_create(&ThreadRS485_2, 			NULL, RS485Task2, 	&id6);
			pthread_create(&ThreadCurrData, 		NULL, CurrDataTask, &id7);
			pthread_create(&ThreadCounter, 			NULL, CounterTask, 	&id8);
			pthread_create(&ThredIbp, 				NULL, IbpTask, 		&id9);
			pthread_create(&ThredFree, 				NULL, FreeTask, 	&id10);
			pthread_create(&ThreadDetect,   		NULL, DetectStatesTask, &id11);
			pthread_create(&ThreadVirtual,  		NULL, VirtualPortTask, 	&id12);
			pthread_create(&ThreadModbus1,  		NULL, ModbusTask1, 		&id13);
			pthread_create(&ThreadModbus2,  		NULL, ModbusTask2, 		&id14);
			pthread_create(&ThreadManageClimat,  	NULL, ManageClimatTask, &id15);
			pthread_create(&ThreadDisel,  			NULL, DguTask, 			&id16);
			pthread_create(&ThreadInnerRs485,  		NULL, InnerRs485Task, 	&id17);
			pthread_create(&ThreadSelfTesting,  	NULL, SelfTestingTask, 	&id18);
			pthread_create(&ThreadUdpServer,  		NULL, UdpServerTask, 	&id19);
			pthread_create(&ThreadDoCounterJobs,    NULL, DoCounterJobsTask, &id20);
			pthread_create(&ThreadInputsDetect,     NULL, InputsDetectTask, &id21);
			//pthread_create(&ThreadReadOtherTask,    NULL, ReadOtherTask,    &id22);

//
			ObjectFunctionsTimer.SetTimer(USB_RS485_ACTIV_WAIT_TIMER, 30);
			ObjectFunctionsTimer.SetTimer(TIME_EVENT_TIMER1, 3600);//1h
			ObjectFunctionsTimer.SetTimer(TIME_EVENT_TIMER2, 14400);//4h
			ObjectFunctionsTimer.SetTimer(TIME_EVENT_TIMER3, 28800);//8h
			ObjectFunctionsTimer.SetTimer(TIME_EVENT_TIMER4, 1800);//1800);//0.5h

			float ver = VERSION;
			float currVer = 1.0;
			string tmp_ver = uci::Get(CONFIG_FILE, "main", "version");
			if(tmp_ver.size() > 0){
				currVer = atof( tmp_ver.c_str() );

			}
			Log::DEBUG( "InitWorkThreads main.version=[" + tmp_ver + "] ver="+toString(ver) );
			if(ver != currVer){
				bool status = uci::Set(CONFIG_FILE, "main", "version", toString(ver) );
				Log::DEBUG( "InitWorkThreads set main.version status=" + toString(status) );
			}
			Log::DEBUG( "InitWorkThreads complited" );
	}
	catch(exception &e)
	{
		Log::ERROR( e.what() );
		return 1;
	}
	return 0;
}
//=======================================================
void DestroyWorkThreads(void)
{
	GlobalStartedFlag = false;
	Log::INFO("[DAEMON] DestroingWorkThreads wait 5s!");
	sleep(5);
	Log::INFO("[DAEMON] DestroingWorkThreads finish!");
}
//=======================================================
int LoadConfig(string filename)
{//n

	string debug_flag = uci::Get(CONFIG_FILE, "main", "debug");
	if(debug_flag.size() > 0)
	{
		Log::debug_flag = atoi(debug_flag.c_str());
	}

	Configuration.DevAddress 		= uci::Get(CONFIG_FILE, "main", "address");
	Configuration.PassWord 			= uci::Get(CONFIG_FILE, "main", "password");
	Configuration.PassWord485 		= uci::Get(CONFIG_FILE, "main", "password485");
	Configuration.Unit_Type 		= uci::Get(CONFIG_FILE, "main", "utype");
	Configuration.SerialNumber 		= atol(uci::Get(CONFIG_FILE, "main", "serial").c_str());
	Configuration.Version 			= atoi(uci::Get(CONFIG_FILE, "main", "version").c_str());
	Configuration.SubVersion  		= atoi(uci::Get(CONFIG_FILE, "main", "subversion").c_str());
	Configuration.SmsPort			= SMS_PORT;
	Configuration.AsduNumber = "";
	Configuration.MkddNumber = "";
	Configuration.RepeatCount = REPEAT_RECV_COUNT;
	Configuration.ResetCount  = MAX_ERRORS_TO_RESET_ROUTER;

	string gsmmodemport = uci::Get(NETWORK_FILE, "wan2", "device");
	if(gsmmodemport.size() > 0)
	{
		string port = trimAllNull(gsmmodemport);
		Log::INFO("LoadConfig GsmModemPort=["+port+"]");
		if(port == "/dev/ttyUSB0")//huawei
			Configuration.SmsPort = "/dev/ttyUSB2";
		else if(port == "/dev/ttyUSB3")//quctel
			Configuration.SmsPort = "/dev/ttyUSB2";
		else if(port == "/dev/ttyUSB2")//longsung
			Configuration.SmsPort = "/dev/ttyUSB1";
		else if(port == "/dev/ttyAPP4")//quctel 2G
			Configuration.SmsPort = "/dev/ttyAPP0";
	}
	else
	{
		gsmmodemport = uci::Get(NETWORK_FILE, "wan", "device");
		if(gsmmodemport.size() > 0)
		{
			string port = trimAllNull(gsmmodemport);
			Log::INFO("LoadConfig GsmModemPort=["+port+"]");
			if(port == "/dev/ttyAPP4")//quctel 2G
				Configuration.SmsPort = "/dev/ttyAPP0";
		}
	}


	string mkddnumber = uci::Get(CONFIG_FILE, "main", "mkddnumber");
	if(mkddnumber.size() > 0)
		Configuration.MkddNumber = mkddnumber;

	string asdunumber = uci::Get(CONFIG_FILE, "main", "asdunumber");
	if(asdunumber.size() > 0)
		Configuration.AsduNumber = asdunumber;

	string smsport = uci::Get(CONFIG_FILE, "main", "smsport");
	if(smsport.size() > 0)
		Configuration.SmsPort = smsport;

	string repeatcnt = uci::Get(CONFIG_FILE, "main", "repeat");
	if(repeatcnt.size() > 0)
		Configuration.RepeatCount = atoi(repeatcnt.c_str());

	string resetcnt = uci::Get(CONFIG_FILE, "main", "reset");
	if(resetcnt.size() > 0)
		Configuration.ResetCount = atoi(resetcnt.c_str());

	Configuration.FifoFile          = FIFO_FILE;
	string fifo = uci::Get(CONFIG_FILE, "main", "fifofile");
	if(fifo.size() > 0)
		Configuration.FifoFile = fifo;

	string installer = uci::Get(CONFIG_FILE, "main", "installer");
	if(installer.size() > 0)
		Configuration.Installer = installer;

	string commissioning = uci::Get(CONFIG_FILE, "main", "commissioning");
	if(commissioning.size() > 0)
		Configuration.Commissioning = commissioning;

	Configuration.EcounterPeriod 	= atoi(uci::Get(CONFIG_FILE, "period", "ecounter").c_str());
	Configuration.DataTaskPeriod 	= atoi(uci::Get(CONFIG_FILE, "period", "data").c_str());
	Configuration.IbpPeriod			= atoi(uci::Get(CONFIG_FILE, "period", "ibp").c_str());
	Configuration.FreePeriod		= atoi(uci::Get(CONFIG_FILE, "period", "free").c_str());
	Configuration.DguPeriod		    = atoi(uci::Get(CONFIG_FILE, "period", "dgu").c_str())  ? atoi(uci::Get(CONFIG_FILE, "period", "dgu").c_str()): 1800;
	Configuration.FifoPeriod	    = atoi(uci::Get(CONFIG_FILE, "period", "fifo").c_str()) ? atoi(uci::Get(CONFIG_FILE, "period", "fifo").c_str()): 1050;

	string tmp_str = uci::Get(CONFIG_FILE, "period", "profil");
	if(tmp_str.size() > 0)
		Configuration.EprofilPeriod = atoi(tmp_str.c_str());
	else
		Configuration.EprofilPeriod  = 43200;


	Log::INFO("Configuration.EcounterPeriod="+toString(Configuration.EcounterPeriod));
	Log::INFO("Configuration.DataTaskPeriod="+toString(Configuration.DataTaskPeriod));
	Log::INFO("Configuration.IbpPeriod="+toString(Configuration.IbpPeriod));
	Log::INFO("Configuration.FreePeriod="+toString(Configuration.FreePeriod));
	Log::INFO("Configuration.DguPeriod="+toString(Configuration.DguPeriod));
	Log::INFO("Configuration.FifoPeriod="+toString(Configuration.FifoPeriod));

	Configuration.IpAddress 		= uci::Get(CONFIG_FILE, "socket", "address");
	Configuration.IpPort 			= atol(uci::Get(CONFIG_FILE, "socket", "port").c_str());
	Configuration.SocketSendTimeout = atol(uci::Get(CONFIG_FILE, "socket", "sendtimeout").c_str());
	Configuration.SocketRecvTimeout = atol(uci::Get(CONFIG_FILE, "socket", "recvtimeout").c_str());

	Configuration.UdpLocalPort = 9010;
	Configuration.UdpRemotePort = 9010;

	string udplocalport = uci::Get(CONFIG_FILE, "socket", "udplocalport");
	if(udplocalport.size() > 0)
		Configuration.UdpLocalPort = atol(udplocalport.c_str());

	string udpremoteport = uci::Get(CONFIG_FILE, "socket", "udpremoteport");
	if(udpremoteport.size() > 0)
		Configuration.UdpRemotePort = atol(udpremoteport.c_str());

	Configuration.WebApiPort = 9002;
	string webapiport = uci::Get(CONFIG_FILE, "socket", "webapiport");
	if(webapiport.size() > 0)
		Configuration.WebApiPort = atol(webapiport.c_str());


	Configuration.RS485Device 		= uci::Get(CONFIG_FILE, "comport1", "device");
	if(Configuration.RS485Device.size() > 0)
	{
		Configuration.RS485SendTimeout 	= atol(uci::Get(CONFIG_FILE, "comport1", "sendtimeout").c_str());
		Configuration.RS485RecvTimeout 	= atol(uci::Get(CONFIG_FILE, "comport1", "recvtimeout").c_str());
		Configuration.RS485BaudRate 	= atol(uci::Get(CONFIG_FILE, "comport1", "baudrate").c_str());

		string strdt = uci::Get(CONFIG_FILE, "comport1", "devices");
		vector<string> results;
		TBuffer::Split(strdt, " " , results);
		if(results.size() == 0){
			strdt = uci::Get(CONFIG_FILE, "datatask", "devices");
			TBuffer::Split(strdt, " " , results);
		}
		for(auto curr: results)
		{
			DeviceOnLineSettings ds;
			ds.address = atoi(curr.c_str());
			Configuration.DeviceList.push_back(ds);
		}
	}
	Configuration.RS485Device2 		= uci::Get(CONFIG_FILE, "comport2", "device");
	if(Configuration.RS485Device2.size() > 0)
	{
		Configuration.RS485SendTimeout2 = atol(uci::Get(CONFIG_FILE, "comport2", "sendtimeout").c_str());
		Configuration.RS485RecvTimeour2 = atol(uci::Get(CONFIG_FILE, "comport2", "recvtimeout").c_str());
		Configuration.RS485BaudRate2	= atol(uci::Get(CONFIG_FILE, "comport2", "baudrate").c_str());

		string strdt = uci::Get(CONFIG_FILE, "comport2", "devices");
		vector<string> results;
		TBuffer::Split(strdt, " " , results);
		for(auto curr: results)
		{
			DeviceOnLineSettings ds;
			ds.address = atoi(curr.c_str());
			Configuration.DeviceList2.push_back(ds);
		}
	}

////////////////////////////////// ECOUNTER NEW
	int ecounter_index = 0;
	string ecfg_str = "";
	do
	{
		string uci_base_str = "@ecounter["+toString(ecounter_index)+"]";
		ecfg_str = uci::Get(CONFIG_FILE, uci_base_str);
		Log::DEBUG("ecounter=["+ecfg_str+"] index="+toString(ecounter_index));
		if(ecfg_str.size() > 0){
			EcounterSettings ecfg;
			string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
			if(tmp_str.size() > 0){
				ecfg.enable = atoi(tmp_str.c_str());
			}
			Log::DEBUG("ecounter.enable="+toString(ecfg.enable));
			if(ecfg.enable)
			{
				//default init sett
				ecfg.address	 			= "0";
				ecfg.type 					= 0;
				ecfg.ename 					= 0;
				ecfg.betweentimeout 		= 100;
				ecfg.portsettings.BaudRate 		= 9600;
				ecfg.portsettings.DataBits 		= 8;
				ecfg.portsettings.StopBit 		= 1;
				ecfg.portsettings.DeviceName 	= "/dev/ttyAPP2";
				ecfg.portsettings.RecvTimeout 	= 1000;
				ecfg.portsettings.SendTimeout 	= 100;
				ecfg.portsettings.Parity 		= PARITY_NONE;
				ecfg.poverka 					= "";
				ecfg.cmd 						= "";
				ecfg.plc 						= 0;
				ecfg.timezone					= 0;
				ecfg.index 						= ecounter_index;
				ecfg.hex 						= false;

				// set new settings
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".address");
				if(tmp_str.size() > 0){ ecfg.address=tmp_str.c_str();}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
				if(tmp_str.size() > 0){ ecfg.type	= atol(tmp_str.c_str());}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
				if(tmp_str.size() > 0){ ecfg.ename	= atol(tmp_str.c_str());}
				else{ ecfg.ename = ecfg.type;}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".betweentimeout");
				if(tmp_str.size() > 0){ ecfg.betweentimeout	= atol(tmp_str.c_str());}

				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".device");
				if(tmp_str.size() > 0){ ecfg.portsettings.DeviceName 	= tmp_str;}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".baudrate");
				if(tmp_str.size() > 0){ ecfg.portsettings.BaudRate 		= atol(tmp_str.c_str());}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".sendtimeout");
				if(tmp_str.size() > 0){ ecfg.portsettings.SendTimeout 	= atol(tmp_str.c_str());}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".recvtimeout");
				if(tmp_str.size() > 0){ ecfg.portsettings.RecvTimeout	= atol(tmp_str.c_str());}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".databits");
				if(tmp_str.size() > 0){ ecfg.portsettings.DataBits	= atol(tmp_str.c_str());}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".stopbit");
				if(tmp_str.size() > 0){ ecfg.portsettings.StopBit	= atol(tmp_str.c_str());}
				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".parity");
				if(tmp_str.size() > 0){ ecfg.portsettings.Parity	= atol(tmp_str.c_str());}

				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".poverka");
				if(tmp_str.size() > 0){ ecfg.poverka=tmp_str.c_str();}

				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".cmd");
				if(tmp_str.size() > 0){ ecfg.cmd=tmp_str.c_str();}

				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".plc");
				if(tmp_str.size() > 0){ ecfg.plc	= atoi(tmp_str.c_str());}

				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".timezone");
				if(tmp_str.size() > 0){ ecfg.timezone	= atoi(tmp_str.c_str());}

				tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".hex");
				if(tmp_str.size() > 0){ ecfg.hex	= atoi(tmp_str.c_str());}


				ecfg.period = Configuration.EcounterPeriod;
				Configuration.CounterList.push_back(ecfg);
			}
		}
		ecounter_index++;
	}
	while(ecfg_str.size() > 0);
	///////////////////////////////////

	////////////////////////////////// IBP NEW
		int ibp_index = 0;
		string ibp_str = "";
		do{
			string uci_base_str = "@ibp["+toString(ibp_index)+"]";
			ibp_str = uci::Get(CONFIG_FILE, uci_base_str);
			Log::DEBUG("ibp=["+ibp_str+"] index="+toString(ibp_index));
			if(ibp_str.size() > 0){
				IbpSettings icfg;
				string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
				if(tmp_str.size() > 0){
					icfg.enable = atoi(tmp_str.c_str());
				}
				Log::DEBUG("ibp.enable="+toString(icfg.enable));
				if(icfg.enable){
					//default init sett
					icfg.address	 			= 1;
					icfg.type 					= 1;
					icfg.etype 					= 26;
					icfg.ename 					= 0;
					icfg.betweentimeout 		= 100;
					icfg.ktran					= 1.0;

					icfg.portsettings.BaudRate 		= 9600;
					icfg.portsettings.BaudRate2     = 63118;
					icfg.portsettings.DataBits 		= 8;
					icfg.portsettings.StopBit 		= 1;
					icfg.portsettings.DeviceName 	= "/dev/ttyAPP3";
					icfg.portsettings.RecvTimeout 	= 1000;
					icfg.portsettings.SendTimeout 	= 100;
					icfg.portsettings.Parity 		= PARITY_NONE;
					icfg.snmpsettings.read			= "public";
					icfg.snmpsettings.write			= "private";
					icfg.snmpsettings.trap			= "trap";

					// set new settings
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".address");
					if(tmp_str.size() > 0){ icfg.address	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
					if(tmp_str.size() > 0){ icfg.type	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".etype");
					if(tmp_str.size() > 0){ icfg.etype	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
					if(tmp_str.size() > 0){ icfg.ename	= atol(tmp_str.c_str());}
					else{ icfg.ename = icfg.type;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".betweentimeout");
					if(tmp_str.size() > 0){ icfg.betweentimeout	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ktran");
					if(tmp_str.size() > 0){ icfg.ktran	= atof(tmp_str.c_str());}


					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".device");
					if(tmp_str.size() > 0){ icfg.portsettings.DeviceName 	= tmp_str;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".baudrate");
					if(tmp_str.size() > 0){ icfg.portsettings.BaudRate 		= atol(tmp_str.c_str());}
					////////////////////////
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".bindport");
					if(tmp_str.size() > 0)
					{
						icfg.portsettings.BaudRate2 = atol(tmp_str.c_str());
					}
					else
					{//important for FP2 smartpack ethernet
						icfg.portsettings.BaudRate2 = icfg.portsettings.BaudRate;
					}
					////////////////////////
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".sendtimeout");
					if(tmp_str.size() > 0){ icfg.portsettings.SendTimeout 	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".recvtimeout");
					if(tmp_str.size() > 0){ icfg.portsettings.RecvTimeout	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".databits");
					if(tmp_str.size() > 0){ icfg.portsettings.DataBits	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".stopbit");
					if(tmp_str.size() > 0){ icfg.portsettings.StopBit	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".parity");
					if(tmp_str.size() > 0){ icfg.portsettings.Parity	= atol(tmp_str.c_str());}

					//Log::DEBUG("uci::Get "+uci_base_str+".read");
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".read");
					if(tmp_str.size() > 0){ icfg.snmpsettings.read	= tmp_str.c_str();}

					//Log::DEBUG("uci::Get "+uci_base_str+".write");
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".write");
					if(tmp_str.size() > 0){ icfg.snmpsettings.write	= tmp_str.c_str();}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".trap");
					if(tmp_str.size() > 0){ icfg.snmpsettings.trap	= tmp_str.c_str();}

					Log::DEBUG("icfg.snmpsettings.read="+icfg.snmpsettings.read);
					Log::DEBUG("icfg.snmpsettings.write="+icfg.snmpsettings.write);
					Log::DEBUG("icfg.snmpsettings.trap="+icfg.snmpsettings.trap);

					icfg.period = Configuration.IbpPeriod;
					Configuration.IbpList.push_back(icfg);
				}
			}
			ibp_index++;
		}
		while(ibp_str.size()>0);
		///////////////////////////////////

		////////////////////////////////// cond
		int cond_index = 0;
		string cond_str = "";
		do{
			string uci_base_str = "@cond["+toString(cond_index)+"]";
			cond_str = uci::Get(CONFIG_FILE, uci_base_str);
			Log::DEBUG("cond=["+cond_str+"] index="+toString(cond_index));
			if(cond_str.size() > 0){
				IbpSettings icfg;
				string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
				if(tmp_str.size() > 0){
					icfg.enable = atoi(tmp_str.c_str());
				}
				if(icfg.enable){
					//default init sett
					icfg.address	 			= 1;
					icfg.type 					= 101;
					icfg.etype 					= 128;
					icfg.ename 					= 1;
					icfg.betweentimeout 		= 100;
					icfg.portsettings.BaudRate 		= 9600;
					icfg.portsettings.BaudRate2     = 63128;
					icfg.portsettings.DataBits 		= 8;
					icfg.portsettings.StopBit 		= 1;
					icfg.portsettings.DeviceName 	= "/dev/ttyAPP3";
					icfg.portsettings.RecvTimeout 	= 1000;
					icfg.portsettings.SendTimeout 	= 100;
					icfg.portsettings.Parity 		= PARITY_NONE;
					icfg.snmpsettings.read			= "public";
					icfg.snmpsettings.write			= "carel";
					icfg.snmpsettings.trap			= "trap";


					// set new settings
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".address");
					if(tmp_str.size() > 0){ icfg.address	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
					if(tmp_str.size() > 0){ icfg.type	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".etype");
					if(tmp_str.size() > 0){ icfg.etype	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
					if(tmp_str.size() > 0){ icfg.ename	= atol(tmp_str.c_str());}

					else{ icfg.ename = icfg.type;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".betweentimeout");
					if(tmp_str.size() > 0){ icfg.betweentimeout	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".device");
					if(tmp_str.size() > 0){ icfg.portsettings.DeviceName 	= tmp_str;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".baudrate");
					if(tmp_str.size() > 0){ icfg.portsettings.BaudRate 		= atol(tmp_str.c_str());}
					////////////////////////
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".bindport");
					if(tmp_str.size() > 0){ icfg.portsettings.BaudRate2 = atol(tmp_str.c_str());}
					////////////////////////
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".sendtimeout");
					if(tmp_str.size() > 0){ icfg.portsettings.SendTimeout 	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".recvtimeout");
					if(tmp_str.size() > 0){ icfg.portsettings.RecvTimeout	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".databits");
					if(tmp_str.size() > 0){ icfg.portsettings.DataBits	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".stopbit");
					if(tmp_str.size() > 0){ icfg.portsettings.StopBit	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".parity");
					if(tmp_str.size() > 0){ icfg.portsettings.Parity	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".read");
					if(tmp_str.size() > 0){ icfg.snmpsettings.read	= tmp_str.c_str();}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".write");
					if(tmp_str.size() > 0){ icfg.snmpsettings.write	= tmp_str.c_str();}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".trap");
					if(tmp_str.size() > 0){ icfg.snmpsettings.trap	= tmp_str.c_str();}

					Log::DEBUG("cond.snmpsettings.read="+icfg.snmpsettings.read);
					Log::DEBUG("cond.snmpsettings.write="+icfg.snmpsettings.write);
					Log::DEBUG("cond.snmpsettings.trap="+icfg.snmpsettings.trap);

					icfg.period = Configuration.IbpPeriod;
					Configuration.IbpList.push_back(icfg);
				}
			}
			cond_index++;
		}
		while(cond_str.size()>0);
		///////////////////////////////////


		////////////////////////////////// pdu
		int pdu_index = 0;
		string pdu_str = "";
		do{
			string uci_base_str = "@pdu["+toString(pdu_index)+"]";
			pdu_str = uci::Get(CONFIG_FILE, uci_base_str);
			Log::DEBUG("pdu=["+pdu_str+"] index="+toString(pdu_index));
			if(pdu_str.size() > 0){
				IbpSettings icfg;
				string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
				if(tmp_str.size() > 0){
					icfg.enable = atoi(tmp_str.c_str());
				}
				if(icfg.enable){
					//default init sett
					icfg.address	 			= 1;
					icfg.type 					= 201;
					icfg.etype 					= 130;
					icfg.ename 					= 1;
					icfg.betweentimeout 		= 100;
					icfg.portsettings.BaudRate 		= 9600;
					icfg.portsettings.BaudRate2     = 63138;
					icfg.portsettings.DataBits 		= 8;
					icfg.portsettings.StopBit 		= 1;
					icfg.portsettings.DeviceName 	= "/dev/ttyAPP3";
					icfg.portsettings.RecvTimeout 	= 1000;
					icfg.portsettings.SendTimeout 	= 100;
					icfg.portsettings.Parity 		= PARITY_NONE;
					icfg.snmpsettings.read			= "public";
					icfg.snmpsettings.write			= "private";
					icfg.snmpsettings.trap			= "trap";


					// set new settings
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".address");
					if(tmp_str.size() > 0){ icfg.address	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
					if(tmp_str.size() > 0){ icfg.type	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".etype");
					if(tmp_str.size() > 0){ icfg.etype	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
					if(tmp_str.size() > 0){ icfg.ename	= atol(tmp_str.c_str());}

					else{ icfg.ename = icfg.type;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".betweentimeout");
					if(tmp_str.size() > 0){ icfg.betweentimeout	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".device");
					if(tmp_str.size() > 0){ icfg.portsettings.DeviceName 	= tmp_str;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".baudrate");
					if(tmp_str.size() > 0){ icfg.portsettings.BaudRate 		= atol(tmp_str.c_str());}
					////////////////////////
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".bindport");
					if(tmp_str.size() > 0){ icfg.portsettings.BaudRate2 = atol(tmp_str.c_str());}
					////////////////////////
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".sendtimeout");
					if(tmp_str.size() > 0){ icfg.portsettings.SendTimeout 	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".recvtimeout");
					if(tmp_str.size() > 0){ icfg.portsettings.RecvTimeout	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".databits");
					if(tmp_str.size() > 0){ icfg.portsettings.DataBits	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".stopbit");
					if(tmp_str.size() > 0){ icfg.portsettings.StopBit	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".parity");
					if(tmp_str.size() > 0){ icfg.portsettings.Parity	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".read");
					if(tmp_str.size() > 0){ icfg.snmpsettings.read	= tmp_str.c_str();}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".write");
					if(tmp_str.size() > 0){ icfg.snmpsettings.write	= tmp_str.c_str();}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".trap");
					if(tmp_str.size() > 0){ icfg.snmpsettings.trap	= tmp_str.c_str();}

					Log::DEBUG("pdu.snmpsettings.read="+icfg.snmpsettings.read);
					Log::DEBUG("pdu.snmpsettings.write="+icfg.snmpsettings.write);
					Log::DEBUG("pdu.snmpsettings.trap="+icfg.snmpsettings.trap);

					icfg.period = Configuration.IbpPeriod;
					Configuration.IbpList.push_back(icfg);
				}
			}
			pdu_index++;
		}
		while(pdu_str.size()>0);
		///////////////////////////////////

		////////////////////////////////// freecool
		int free_index = 0;
		string free_str = "";
		do{
			string uci_base_str = "@free["+toString(free_index)+"]";
			free_str = uci::Get(CONFIG_FILE, uci_base_str);
			Log::DEBUG("freecooling=["+free_str+"] index="+toString(free_index));
			if(free_str.size() > 0){
				FreeSettings fcfg;
				string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
				if(tmp_str.size() > 0){
					fcfg.enable = atoi(tmp_str.c_str());
				}
				//Log::DEBUG("free.enable="+toString(fcfg.enable));
				if(fcfg.enable){
					//default init sett
					fcfg.address	 				= 0;
					fcfg.type 						= 0;
					fcfg.ename 						= 0;
					fcfg.betweentimeout 			= 100;
					fcfg.portsettings.BaudRate 		= 9600;
					fcfg.portsettings.DataBits 		= 8;
					fcfg.portsettings.StopBit 		= 1;
					fcfg.portsettings.DeviceName 	= "/dev/ttyAPP0";
					fcfg.portsettings.RecvTimeout 	= 1000;
					fcfg.portsettings.SendTimeout 	= 100;
					fcfg.portsettings.Parity 		= PARITY_NONE;

					// set new settings
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".address");
					if(tmp_str.size() > 0){ fcfg.address	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
					if(tmp_str.size() > 0){ fcfg.type	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
					if(tmp_str.size() > 0){ fcfg.ename	= atol(tmp_str.c_str());}
					else{ fcfg.ename = fcfg.type;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".betweentimeout");
					if(tmp_str.size() > 0){ fcfg.betweentimeout	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".device");
					if(tmp_str.size() > 0){ fcfg.portsettings.DeviceName 	= tmp_str;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".baudrate");
					if(tmp_str.size() > 0){ fcfg.portsettings.BaudRate 		= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".sendtimeout");
					if(tmp_str.size() > 0){ fcfg.portsettings.SendTimeout 	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".recvtimeout");
					if(tmp_str.size() > 0){ fcfg.portsettings.RecvTimeout	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".databits");
					if(tmp_str.size() > 0){ fcfg.portsettings.DataBits	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".stopbit");
					if(tmp_str.size() > 0){ fcfg.portsettings.StopBit	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".parity");
					if(tmp_str.size() > 0){ fcfg.portsettings.Parity	= atol(tmp_str.c_str());}
					fcfg.period = Configuration.FreePeriod;
					Configuration.FreeList.push_back(fcfg);
				}
			}
			free_index++;
		}
		while(free_str.size()>0);
		///////////////////////////////////
		////////////////////////////////// freecool
		int vru_index = 0;
		string vru_str = "";
		do{
			string uci_base_str = "@vru["+toString(vru_index)+"]";
			vru_str = uci::Get(CONFIG_FILE, uci_base_str);
			Log::DEBUG("vru=["+vru_str+"] index="+toString(vru_index));
			if(vru_str.size() > 0){
				//FreeSettings fcfg;
				DguSettings fcfg;
				string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
				if(tmp_str.size() > 0){
					fcfg.enable = atoi(tmp_str.c_str());
				}
				//Log::DEBUG("free.enable="+toString(fcfg.enable));
				if(fcfg.enable){
					//default init sett
					fcfg.address	 				= 0;
					fcfg.type 						= 0;
					fcfg.etype						= 3;
					fcfg.ename 						= 1;
					fcfg.portsettings.BaudRate 		= 9600;
					fcfg.portsettings.DataBits 		= 8;
					fcfg.portsettings.StopBit 		= 1;
					fcfg.portsettings.DeviceName 	= "/dev/ttyAPP0";
					fcfg.portsettings.RecvTimeout 	= 1000;
					fcfg.portsettings.SendTimeout 	= 100;
					fcfg.portsettings.Parity 		= PARITY_NONE;

					fcfg.betweentimeout 			= 100;
					fcfg.needinittimeout 			= 100;
					fcfg.aperiod					= 60;
					fcfg.ktran						= 1.0;


					// set new settings
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".address");
					if(tmp_str.size() > 0){ fcfg.address	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
					if(tmp_str.size() > 0){ fcfg.type	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".etype");
					if(tmp_str.size() > 0){ fcfg.etype	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
					if(tmp_str.size() > 0){ fcfg.ename	= atol(tmp_str.c_str());}
					else{ fcfg.ename = fcfg.type;}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".device");
					if(tmp_str.size() > 0){ fcfg.portsettings.DeviceName 	= tmp_str;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".baudrate");
					if(tmp_str.size() > 0){ fcfg.portsettings.BaudRate 		= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".sendtimeout");
					if(tmp_str.size() > 0){ fcfg.portsettings.SendTimeout 	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".recvtimeout");
					if(tmp_str.size() > 0){ fcfg.portsettings.RecvTimeout	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".databits");
					if(tmp_str.size() > 0){ fcfg.portsettings.DataBits	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".stopbit");
					if(tmp_str.size() > 0){ fcfg.portsettings.StopBit	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".parity");
					if(tmp_str.size() > 0){ fcfg.portsettings.Parity	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".betweentimeout");
					if(tmp_str.size() > 0){ fcfg.betweentimeout	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ktran");
					if(tmp_str.size() > 0){ fcfg.ktran	= atof(tmp_str.c_str());}


					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".needinittimeout");
					if(tmp_str.size() > 0){
						fcfg.needinittimeout	= atol(tmp_str.c_str());
						Log::DEBUG("vru.needinittimeout="+toString(fcfg.needinittimeout));
					}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".aperiod");
					if(tmp_str.size() > 0){ fcfg.aperiod 		= atoi(tmp_str.c_str());}

					fcfg.period = Configuration.DguPeriod;//.FreePeriod;
					Configuration.DguList.push_back(fcfg);
				}
			}
			vru_index++;
		}
		while(vru_str.size()>0);
		///////////////////////////////////


	////////////////////////////////// DGU NEW
		int dgu_index = 0;
		string dcfg_str = "";
		do{
			string uci_base_str = "@dgu["+toString(dgu_index)+"]";
			dcfg_str = uci::Get(CONFIG_FILE, uci_base_str);
			Log::DEBUG("dgu=["+dcfg_str+"] index="+toString(dgu_index));
			if(dcfg_str.size() > 0){
				DguSettings dcfg;
				string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
				if(tmp_str.size() > 0){
					dcfg.enable = atoi(tmp_str.c_str());
				}
				Log::DEBUG("dgu.enable="+toString(dcfg.enable));
				if(dcfg.enable){
					//default init sett
					dcfg.address	 			= 0;
					dcfg.type 					= 0;
					dcfg.etype 					= 0;
					dcfg.ename 					= 0;
					dcfg.portsettings.BaudRate 		= 9600;
					dcfg.portsettings.DataBits 		= 8;
					dcfg.portsettings.StopBit 		= 1;
					dcfg.portsettings.DeviceName 	= "/dev/ttyAPP4";
					dcfg.portsettings.RecvTimeout 	= 1000;
					dcfg.portsettings.SendTimeout 	= 1000;
					dcfg.portsettings.Parity 		= PARITY_NONE;

					dcfg.betweentimeout 			= 100;
					dcfg.needinittimeout 			= 100;
					dcfg.aperiod					= 60;
					dcfg.ktran						= 1.0;

					// set new settings
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".address");
					if(tmp_str.size() > 0){ dcfg.address	= atoi(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
					if(tmp_str.size() > 0){ dcfg.type	= atoi(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".etype");
					if(tmp_str.size() > 0){ dcfg.etype	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
					if(tmp_str.size() > 0){ dcfg.ename	= atoi(tmp_str.c_str());}
					else{ dcfg.ename = dcfg.type;}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".device");
					if(tmp_str.size() > 0){ dcfg.portsettings.DeviceName 	= tmp_str;}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".baudrate");
					if(tmp_str.size() > 0){ dcfg.portsettings.BaudRate 		= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".sendtimeout");
					if(tmp_str.size() > 0){ dcfg.portsettings.SendTimeout 	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".recvtimeout");
					if(tmp_str.size() > 0){ dcfg.portsettings.RecvTimeout	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".databits");
					if(tmp_str.size() > 0){ dcfg.portsettings.DataBits	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".stopbit");
					if(tmp_str.size() > 0){ dcfg.portsettings.StopBit	= atol(tmp_str.c_str());}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".parity");
					if(tmp_str.size() > 0){ dcfg.portsettings.Parity	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".betweentimeout");
					if(tmp_str.size() > 0){ dcfg.betweentimeout	= atol(tmp_str.c_str());}

					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ktran");
					if(tmp_str.size() > 0){ dcfg.ktran	= atof(tmp_str.c_str());}


					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".needinittimeout");
					if(tmp_str.size() > 0){
						dcfg.needinittimeout	= atol(tmp_str.c_str());
						Log::DEBUG("dgu.needinittimeout="+toString(dcfg.needinittimeout));
					}
					tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".aperiod");
					if(tmp_str.size() > 0){ dcfg.aperiod 		= atoi(tmp_str.c_str());}

					dcfg.period = Configuration.DguPeriod;//.FreePeriod;
					Configuration.DguList.push_back(dcfg);
				}
			}


			dgu_index++;
		}
		while(dcfg_str.size()>0);
		//////////////////////////////////

		////////////////////////////////// Modules NEW
			int mod_index = 0;
			string mcfg_str = "";
			do
			{
				string uci_base_str = "@module["+toString(mod_index)+"]";
				mcfg_str = uci::Get(CONFIG_FILE, uci_base_str);
				Log::DEBUG("module=["+mcfg_str+"] index="+toString(mod_index));
				if(mcfg_str.size() > 0)
				{
					ModuleSettings mcfg;
					string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
					if(tmp_str.size() > 0){
						mcfg.enable = atoi(tmp_str.c_str());
					}
					Log::DEBUG("module.enable="+toString(mcfg.enable));
					if(mcfg.enable)
					{
						//default init sett
						mcfg.type 					= 0;
						//mcfg.etype	 				= 50;
						//mcfg.ename 					= 0;
						mcfg.portsettings.BaudRate 		= 9600;
						mcfg.portsettings.DataBits 		= 8;
						mcfg.portsettings.StopBit 		= 1;
						mcfg.portsettings.DeviceName 	= "/dev/ttyAPP4";
						mcfg.portsettings.RecvTimeout 	= 1000;
						mcfg.portsettings.SendTimeout 	= 1000;
						mcfg.portsettings.Parity 		= PARITY_NONE;
						mcfg.betweentimeout   			= 100;
						mcfg.period 					= 600;
						mcfg.aperiod 					= 60;
						mcfg.maxerrors					= 5;
						mcfg.resetport					= 0;
						mcfg.selvalue					= 0;
						mcfg.seltimeout					= 0;

						// set new settings
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
						if(tmp_str.size() > 0){ mcfg.type	= atoi(tmp_str.c_str());}

					    tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".etype");
						if(tmp_str.size() > 0){ mcfg.etype	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
						if(tmp_str.size() > 0){ mcfg.ename	= atoi(tmp_str.c_str());}
						else{ mcfg.ename = mcfg.type;}

						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".device");
						if(tmp_str.size() > 0){ mcfg.portsettings.DeviceName 	= tmp_str;}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".baudrate");
						if(tmp_str.size() > 0){ mcfg.portsettings.BaudRate 		= atol(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".sendtimeout");
						if(tmp_str.size() > 0){ mcfg.portsettings.SendTimeout 	= atol(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".recvtimeout");
						if(tmp_str.size() > 0){ mcfg.portsettings.RecvTimeout	= atol(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".databits");
						if(tmp_str.size() > 0){ mcfg.portsettings.DataBits	= atol(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".stopbit");
						if(tmp_str.size() > 0){ mcfg.portsettings.StopBit	= atol(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".parity");
						if(tmp_str.size() > 0){ mcfg.portsettings.Parity	= atol(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".betweentimeout");
						if(tmp_str.size() > 0){ mcfg.betweentimeout	= atol(tmp_str.c_str());}

						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".maxerrors");
						if(tmp_str.size() > 0){ mcfg.maxerrors	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".resetport");
						if(tmp_str.size() > 0){ mcfg.resetport	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".selvalue");
						if(tmp_str.size() > 0){ mcfg.selvalue	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".seltimeout");
						if(tmp_str.size() > 0){ mcfg.seltimeout	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".selports");
						if(tmp_str.size() > 0){ mcfg.selports 	= tmp_str;}


						string commands = uci::Get(CONFIG_FILE, uci_base_str+".command");
						mcfg.commands = TBuffer::Split(commands, " " );

						string configs = uci::Get(CONFIG_FILE, uci_base_str+".config");
						mcfg.configs = TBuffer::Split(configs, " " );


						mcfg.period = Configuration.DataTaskPeriod;
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".period");
						if(tmp_str.size() > 0){ mcfg.period	= atoi(tmp_str.c_str());}

						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".aperiod");
						if(tmp_str.size() > 0){ mcfg.aperiod	= atoi(tmp_str.c_str());}


						Configuration.ModuleList.push_back(mcfg);
					}
				}
				mod_index++;
			}
			while(mcfg_str.size() > 0);
			//////////////////////////////////

			////////////////////////////////// input
			int inp_index = 0;
			string inp_str = "";
			do
			{
				string uci_base_str = "@input["+toString(inp_index)+"]";
				inp_str = uci::Get(CONFIG_FILE, uci_base_str);
				Log::DEBUG("input=["+inp_str+"] index="+toString(inp_index));
				if(inp_str.size() > 0)
				{
					InputSettings icfg;
					string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".enable");
					if(tmp_str.size() > 0){
						icfg.enable = atoi(tmp_str.c_str());
					}
					Log::DEBUG("input.enable="+toString(icfg.enable));
					if(icfg.enable)
					{
						icfg.type		= 0;
						icfg.etype  	= 51;
						icfg.ename  	= 1;
						icfg.ptype  	= 200;
						icfg.pname 		= inp_index + 1;
						icfg.timeout 	= 3000;
						icfg.period 	= Configuration.DataTaskPeriod;
						icfg.port 		= 0;
						icfg.inversia	= false;

						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".type");
						if(tmp_str.size() > 0){ icfg.type	= atoi(tmp_str.c_str());}
					    tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".etype");
						if(tmp_str.size() > 0){ icfg.etype	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ename");
						if(tmp_str.size() > 0){ icfg.ename	= atoi(tmp_str.c_str());}
					    tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".ptype");
						if(tmp_str.size() > 0){ icfg.ptype	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".pname");
						if(tmp_str.size() > 0){ icfg.pname	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".timeout");
						if(tmp_str.size() > 0){ icfg.timeout	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".period");
						if(tmp_str.size() > 0){ icfg.period	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".port");
						if(tmp_str.size() > 0){ icfg.port	= atoi(tmp_str.c_str());}
						tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".inv");
						if(tmp_str.size() > 0){ icfg.inversia	= atoi(tmp_str.c_str());}
						Configuration.InputList.push_back(icfg);
					}
				}
				inp_index++;
			}
			while(inp_str.size() > 0);
			//////////////////////////////////

	string climat = uci::Get(CONFIG_FILE, "climat", "enable");
	if(climat.size() > 0)
	{
		bool isEnable = atoi(climat.c_str());
		Configuration.ClimatManagerSettings.Enable = isEnable;
		if(isEnable)
		{
			string isauto = uci::Get(CONFIG_FILE, "climat", "isauto");
			if(isauto.size() > 0)
			{
				bool isAuto = atoi(isauto.c_str());
				Configuration.ClimatManagerSettings.IsAuto = isAuto;
			}

			Configuration.ClimatManagerSettings.Cond1OnTemp = atof(uci::Get(CONFIG_FILE, "climat", "cond1on").c_str());
			Configuration.ClimatManagerSettings.Cond1OffTemp = atof(uci::Get(CONFIG_FILE, "climat", "cond1off").c_str());
			Configuration.ClimatManagerSettings.Cond2OnTemp = atof(uci::Get(CONFIG_FILE, "climat", "cond2on").c_str());
			Configuration.ClimatManagerSettings.Cond2OffTemp = atof(uci::Get(CONFIG_FILE, "climat", "cond2off").c_str());

			Configuration.ClimatManagerSettings.Heater1OnTemp = atof(uci::Get(CONFIG_FILE, "climat", "heater1on").c_str());
			Configuration.ClimatManagerSettings.Heater1OffTemp = atof(uci::Get(CONFIG_FILE, "climat", "heater1off").c_str());
			Configuration.ClimatManagerSettings.Heater2OnTemp = atof(uci::Get(CONFIG_FILE, "climat", "heater2on").c_str());
			Configuration.ClimatManagerSettings.Heater2OffTemp = atof(uci::Get(CONFIG_FILE, "climat", "heater2off").c_str());

			string period = uci::Get(CONFIG_FILE, "climat", "period");
			if(period.size() > 0)
			{
				int Per = atoi(period.c_str());
				Configuration.ClimatManagerSettings.Period = Per;
			}
			else
			{ Configuration.ClimatManagerSettings.Period = 600;}

			string rotate = uci::Get(CONFIG_FILE, "climat", "rotate");
			if(rotate.size() > 0)
			{
				long Rot = atol(rotate.c_str());
				Configuration.ClimatManagerSettings.RotateTime = Rot;
			}
			else{
				Configuration.ClimatManagerSettings.RotateTime = 720;//720min=12h
			}
			Configuration.ClimatManagerSettings.ImpulseCount = 3200;
			string impulses = uci::Get(CONFIG_FILE, "climat", "impulses");
			if(impulses.size() > 0){
				Configuration.ClimatManagerSettings.ImpulseCount = atoi(impulses.c_str());
			}
			MkkClimatManager.SetSettings(Configuration.ClimatManagerSettings);
/////////////////////////////////////////////////////
			ExternClimatManager.Cond1OnTemp.SetData(	Configuration.ClimatManagerSettings.Cond1OnTemp);
			ExternClimatManager.Cond1OffTemp.SetData(	Configuration.ClimatManagerSettings.Cond1OffTemp);
			ExternClimatManager.Cond2OnTemp.SetData(	Configuration.ClimatManagerSettings.Cond2OnTemp);
			ExternClimatManager.Cond2OffTemp.SetData(	Configuration.ClimatManagerSettings.Cond2OffTemp);
			ExternClimatManager.Heater1OnTemp.SetData(	Configuration.ClimatManagerSettings.Heater1OnTemp);
			ExternClimatManager.Heater1OffTemp.SetData(	Configuration.ClimatManagerSettings.Heater1OffTemp);
			ExternClimatManager.Heater2OnTemp.SetData(	Configuration.ClimatManagerSettings.Heater2OnTemp);
			ExternClimatManager.Heater2OffTemp.SetData(	Configuration.ClimatManagerSettings.Heater2OffTemp);
			ExternClimatManager.Period.SetData( 		Configuration.ClimatManagerSettings.Period );
			Log::INFO("ExternClimatManager.Period="+toString(ExternClimatManager.Period.GetData()));


			string etype = uci::Get(CONFIG_FILE, "climat", "etype");
			if(etype.size() > 0)
			{
				int et = atoi(etype.c_str());
				Configuration.ClimatManagerSettings.Etype = et;
			}
			else
				Configuration.ClimatManagerSettings.Etype = 60;

			string ename = uci::Get(CONFIG_FILE, "climat", "ename");
			if(ename.size() > 0)
			{
				int en = atoi(ename.c_str());
				Configuration.ClimatManagerSettings.Ename = en;
			}
			else
				Configuration.ClimatManagerSettings.Ename = 1;

		}
	}
	Configuration.VPSEnable = false;
	string vsp = uci::Get(CONFIG_FILE, "vsp", "enable");
	if(vsp.size() > 0)
	{
		bool isEnable = atoi(vsp.c_str());
		Configuration.VPSEnable = isEnable;
		Configuration.VPServerPort 		  = atol(uci::Get(CONFIG_FILE, "vsp", "port").c_str());
		Configuration.VPSocketSendTimeout = atol(uci::Get(CONFIG_FILE, "vsp", "sendtimeout").c_str());
		Configuration.VPSocketRecvTimeout = atol(uci::Get(CONFIG_FILE, "vsp", "recvtimeout").c_str());
		Configuration.VPSCloseTimeout     = atol(uci::Get(CONFIG_FILE, "vsp", "closetimeout").c_str());
	}
	return 1;
}
//=======================================================
int ReloadConfig(void)
{
	pthread_mutex_lock(&syn_object);
	try
	{
		string debug_flag = uci::Get(CONFIG_FILE, "main", "debug");
		if(debug_flag.size() > 0)
		{
			Log::debug_flag = atoi(debug_flag.c_str());
		}

	}
	catch(exception &e)
	{
		Log::ERROR( "ReloadConfig " + string( e.what() ) );
	}
	pthread_mutex_unlock(&syn_object);
	return 1;
}
//=======================================================
//*******************************************************
//=======================================================
void SetPidFile(string filename)
{
	FILE* f;

	f = fopen(filename.c_str(), "w+");
	if(f)
	{
		int pid = getpid();
		Log::INFO("[DAEMON] pid is " + toString(pid));
		fprintf(f,"%u", pid);
		fclose(f);
	}

}
//=======================================================
static void signal_error(int sig, siginfo_t *si, void *ptr)
{
	void *ErrorAddr;
	void *Trace[16];
	int x;
	int TraceSize;
	char** Messages;
	Log::INFO("[DAEMON] signal_error");
	string signl = strsignal(sig);
	string addr = toString(si->si_addr);
	Log::ERROR("[DAEMON] Signal: " + signl + ", Addr: "+addr);

	ErrorAddr = (void*)((ucontext_t*)ptr)->uc_mcontext.fault_address;
	TraceSize = backtrace(Trace, 16);
	Trace[1] = ErrorAddr;
	Messages = backtrace_symbols(Trace, TraceSize);
	if(Messages)
	{
		Log::ERROR("== Backtrace ==");
		for(x = 1; x < TraceSize; x++)
		{
			Log::ERROR(Messages[x]);
		}
		Log::ERROR("== End Backtrace ==");
		free(Messages);
	}
	Log::INFO("DEMON Stopped!");
	DestroyWorkThreads();
	exit(CHILD_NEED_WORK);
}
//=======================================================
int SetFdLimit(int max)
{
	struct rlimit lim;
	int status;

	lim.rlim_max = max;
	lim.rlim_cur = max;

	status = setrlimit(RLIMIT_NOFILE, &lim);
	return status;
}
//=======================================================
//=======================================================
//=======================================================
//=======================================================
void InitRouterTask(void)
{
	Log::INFO("[RouterTask] InitRouterTask");
	SocketSettings sett;
	sett.IpAddress 				= Configuration.IpAddress;
	sett.IpPort 				= Configuration.IpPort;
	sett.SocketRecvTimeout 		= Configuration.SocketRecvTimeout;// 10000;//!!
	sett.SocketSendTimeout 		= Configuration.SocketSendTimeout;
	SockPort.Init(&sett);

	Log::INFO("[RouterTask] SockPort IpAddress="+sett.IpAddress);
	Log::INFO("[RouterTask] SockPort IpPort="+toString(sett.IpPort));

	Data_Params const   *ptr = &Configuration;
	BaseRouterCmd.AddToCMD_Messages(new TCmd_ERROR(&BaseRouterCmd,"ERROR"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_UnknownCMD(&BaseRouterCmd, "UnknownCMD"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_SetCfg(&BaseRouterCmd,"SetCfg"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_GetCfg(&BaseRouterCmd,"GetCfg"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_Data(&BaseRouterCmd,"Data"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_GetData(&BaseRouterCmd,"GetData"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_SetData(&BaseRouterCmd,"SetData"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_UB(&BaseRouterCmd,"UB"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_UD(&BaseRouterCmd,"UD"));
	BaseRouterCmd.AddToCMD_Messages(new TCmd_UE(&BaseRouterCmd,"UE"));


	BaseRouterCmd.PasswordPtr        = ptr->PassWord;
	BaseRouterCmd.AddrPtr            = ptr->DevAddress;
	BaseRouterCmd.MainMessages       = &SocketCmdMessages;
	BaseRouterCmd.AltMessList.push_back(&RS485CmdMessages);
	BaseRouterCmd.AltMessList.push_back(&RS485CmdMessages2);
	BaseRouterCmd.OnAnswerErrorExecute = RouterOnAnswerErrorExecute;
	BaseRouterCmd.ReloadConfig 		   = ReloadConfig;

	BaseRouterCmd.REPEAT_COUNT 			= Configuration.RepeatCount;
	BaseRouterCmd.MAX_ERRORS_TO_RESET   = Configuration.ResetCount;
	BaseRouterCmd.Info = "Router";

	RouterExeCmd = &BaseRouterCmd;

	Log::INFO("[RouterTask] InitRouterTask OK");


}
//=======================================================
void InitRS485Task(void)
{
	Log::INFO("[RS485Task] InitRS485Task");

	CommSettings sett;
	sett.DeviceName 	= Configuration.RS485Device;
	sett.BaudRate 		= Configuration.RS485BaudRate;
	sett.RecvTimeout 	= Configuration.RS485RecvTimeout;
	sett.SendTimeout 	= Configuration.RS485SendTimeout;

	unsigned long adr;
	adr=inet_addr(sett.DeviceName.c_str());
	if (adr != INADDR_NONE)
	{//ip
		Log::INFO("[RS485Task] Init SocketPortUdp");
		RS485Port = new SocketPortUdp();
		SocketSettings sett_ip;
		sett_ip.IpAddress 				= sett.DeviceName;
		sett_ip.IpPort 					= sett.BaudRate;
		sett_ip.SocketRecvTimeout 		= sett.RecvTimeout;
		sett_ip.SocketSendTimeout 		= sett.SendTimeout;
		RS485Port->Init(&sett_ip);
	}
	else
	{//com
		Log::INFO("[RS485Task] Init ComPort");
		RS485Port = new ComPort();
		RS485Port->Init(&sett);
	}

	Data_Params const   *ptr = &Configuration;
	BaseRS485Cmd.AddToCMD_Messages(new TCmd485_UnknownCMD(&BaseRS485Cmd,"UnknownCMD"));
	BaseRS485Cmd.AddToCMD_Messages(new TCmd485_Data(&BaseRS485Cmd,"Data"));

	BaseRS485Cmd.PasswordPtr        =ptr->PassWord485;
	BaseRS485Cmd.AddrPtr            = "";
	BaseRS485Cmd.MainMessages       = &RS485CmdMessages;
	BaseRS485Cmd.AltMessList.push_back(&SocketCmdMessages);
	BaseRS485Cmd.AltMessList.push_back(&UdpServerCmdMessages);

	BaseRS485Cmd.OnGetDataFrom485 = OnDataFromMKK;
	BaseRS485Cmd.OnGetDataFrom485_1 = OnDataFromMKK_1;
	BaseRS485Cmd.OnGetDataFrom485_2 = OnDataFromMKK_2;
	BaseRS485Cmd.OnGetDataFrom485_3 = OnDataFromMKK_3;
	BaseRS485Cmd.OnGetDataFrom485_4 = OnDataFromMKAB;

	BaseRS485Cmd.Info = "RS4851";
	RS485ExeCmd = &BaseRS485Cmd;

	GetCurrDataObject.Controllers = &(BaseRS485Cmd.Controllers);

	Log::DEBUG("InitRS485Task Controllers addr="+toString((long)&(BaseRS485Cmd.Controllers)));
	//Log::DEBUG( "InitRS485Task RS485ExeCmd="+toString((int)RS485ExeCmd));

}
//=======================================================
void InitRS485Task2(void)
{
	Log::INFO("[RS485Task2] InitRS485Task2");

	CommSettings sett;
	sett.DeviceName 	= Configuration.RS485Device2;
	sett.BaudRate 		= Configuration.RS485BaudRate2;
	sett.RecvTimeout 	= Configuration.RS485RecvTimeour2;
	sett.SendTimeout 	= Configuration.RS485SendTimeout2;

	unsigned long adr;
	adr=inet_addr(sett.DeviceName.c_str());
	if (adr != INADDR_NONE)
	{//ip
		Log::INFO("[RS485Task2] Init SocketPortUdp");
		RS485Port2 = new SocketPortUdp();
		SocketSettings sett_ip;
		sett_ip.IpAddress 				= sett.DeviceName;
		sett_ip.IpPort 					= sett.BaudRate;
		sett_ip.SocketRecvTimeout 		= sett.RecvTimeout;
		sett_ip.SocketSendTimeout 		= sett.SendTimeout;
		RS485Port2->Init(&sett_ip);
	}
	else
	{//com
		Log::INFO("[RS485Task2] Init ComPort");
		RS485Port2 = new ComPort();
		RS485Port2->Init(&sett);
	}

	Data_Params const   *ptr = &Configuration;
	BaseRS485Cmd2.AddToCMD_Messages(new TCmd485_UnknownCMD(&BaseRS485Cmd2,"UnknownCMD"));
	BaseRS485Cmd2.AddToCMD_Messages(new TCmd485_Data(&BaseRS485Cmd2,"Data"));

	BaseRS485Cmd2.PasswordPtr        =ptr->PassWord485;
	BaseRS485Cmd2.AddrPtr            = "";
	BaseRS485Cmd2.MainMessages       = &RS485CmdMessages2;
	BaseRS485Cmd2.AltMessList.push_back(&SocketCmdMessages);
	BaseRS485Cmd2.AltMessList.push_back(&UdpServerCmdMessages);

	BaseRS485Cmd2.Info = "RS4852";
	RS485ExeCmd2 = &BaseRS485Cmd2;

	GetCurrDataObject2.Controllers = &(BaseRS485Cmd2.Controllers);
	Log::DEBUG("InitRS485Task2 Controllers addr="+toString((long)&(BaseRS485Cmd2.Controllers)));
}
//=======================================================
void InitUdpServerTask(void)
{
	Log::INFO("[UdpServerTask] InitUdpServerTask");

	SocketSettings sett;
	sett.IpAddress 				= "0.0.0.0";  //INADDR_ANY
	sett.IpPort 				= Configuration.UdpRemotePort;
	sett.BindPort 				= Configuration.UdpLocalPort;
	sett.SocketRecvTimeout 		= Configuration.SocketRecvTimeout;
	sett.SocketSendTimeout 		= Configuration.SocketSendTimeout;

	Log::INFO("[UdpServerTask] Init SocketPortUdp");//na samom dele ispolzuetsia TcpListener
	UdpServerPort = new ClientServer();
	UdpServerPort->Init(&sett);

	Data_Params const   *ptr = &Configuration;
	BaseUdpServerCmd.AddToCMD_Messages(new TCmd_ERROR(&BaseUdpServerCmd,"ERROR"));
	BaseUdpServerCmd.AddToCMD_Messages(new TCmd_UnknownCMD(&BaseUdpServerCmd, "UnknownCMD"));

	BaseUdpServerCmd.PasswordPtr        = ptr->PassWord;
	BaseUdpServerCmd.AddrPtr            = ptr->DevAddress;
	BaseUdpServerCmd.MainMessages       = &UdpServerCmdMessages;
	BaseUdpServerCmd.AltMessList.push_back(&RS485CmdMessages);
	BaseUdpServerCmd.AltMessList.push_back(&RS485CmdMessages2);

	BaseUdpServerCmd.Info = "UdpServer";
	UdpServerExeCmd = &BaseUdpServerCmd;

	Log::INFO("[UdpServerTask] InitUdpServerTask OK");
}
//=======================================================
void InitInnerRs485Task(void)
{
	Log::INFO("[InnerRs485Task] InnerRs485Task start Init ");
	InnerRs485Manager.Init(&Configuration.ModuleList);
	Log::INFO("[InnerRs485Task] InnerRs485Task end Init");
}
//=======================================================
void InitTimerTask(void)
{
	Log::INFO("[TimerTask] InitTimerTask");

	//time_t seconds = time(NULL);
	//tm *timeinfo =  localtime(&seconds);
	tm timeinfo = SystemTime.Now();


	SystemTime.SetTime(&timeinfo);
	Log::INFO("[TimerTask] systemtime = "+ string(asctime(&timeinfo)));

	Log::INFO("[TimerTask] InitTimerTask complited");
}
//=======================================================
void InitIbpTask(void)
{
	Log::INFO("[InitIbpTask] InitIbpTask");


	//IbpManager.Init(IbpPort, &Configuration.IbpList, &FifoBuffer);
	IbpManager.Init( &Configuration.IbpList, &FifoBuffer);
	Log::INFO("[InitIbpTask] InitIbpTask complited");
}
//=======================================================
void InitDguTask(void)
{
	Log::INFO("[InitDguTask] InitDguTask");
	DiselPanelManager.Init(&Configuration.DguList, &ModbusManager1);
	Log::INFO("[InitDguTask] InitDguTask complited");
}
//=======================================================
void InitFreeTask(void)
{
	Log::INFO("[InitFreeTask] InitFreeTask");
	FreeCoolingManager.Init(&FreePort, &Configuration.FreeList, &ModbusManager2);
	Log::INFO("[InitFreeTask] InitFreeTask complited");
}
//=======================================================
void InitCounterTask(void)
{
	Log::INFO("[CounterTask] InitCounterTask");

	EnergyCounterManager.Init(&Configuration.CounterList, &FifoBuffer);
	EnergyCounterManager.ApiAddress = Configuration.IpAddress;
	EnergyCounterManager.ApiPort	= Configuration.WebApiPort;
	EnergyCounterManager.ObjectId 	=  Configuration.DevAddress;
}
//=======================================================
void InitCurrDataTask(void)
{

	Log::INFO("[CurrDataTask] InitCurrDataTask");
	TaskListSettings sett;
	sett.DeviceList = Configuration.DeviceList;
	sett.PassWord   = Configuration.PassWord485;
	sett.mainMessages = &RS485CmdMessages;
	GetCurrDataObject.Init(sett);

	Log::INFO("[CurrDataTask] InitCurrDataTask2");
	sett.DeviceList = Configuration.DeviceList2;
	sett.PassWord   = Configuration.PassWord485;
	sett.mainMessages = &RS485CmdMessages2;
	GetCurrDataObject2.Init(sett);


}
//=======================================================
void InitVirtualPortTask(void)
{
	Log::INFO("[VirtualPortTask] InitVirtualPortTask");
	if(Configuration.VPSEnable)
	{
		Log::INFO("[VirtualPortTask] Configuration.VPSEnable=true");
		SocketSettings sett;
		sett.IpPort 				= Configuration.VPServerPort;
		sett.SocketRecvTimeout 		= Configuration.VPSocketSendTimeout;
		sett.SocketSendTimeout 		= Configuration.VPSocketRecvTimeout;
		VSP.Init(&Configuration.VPSCloseTimeout, &Configuration.VPSocketSendTimeout);
		VSP.SocketPort.Init(&sett);
		VSP.SocketPort.OnDataRecive = VSP.TransportData;
		VSP.SocketPort.ptr 			= &VSP;

		for(auto curr: IbpManager.Controllers)
		{
			if(curr->Port != NULL)
			{
				VirtualPortSettings *vs = new VirtualPortSettings();
				vs->Port 			 = curr->Port;
				VSP.PortList.push_back(vs);
				Log::DEBUG( "[VirtualPortTask] vs="+toString((int)vs)+" "+curr->Port->GetSettingsString() );
			}
		}

		Log::INFO("[VirtualPortTask] InitVirtualPortTask OK");
	}
}
//=======================================================
void InitModbus1Task(void)
{
	Log::INFO("[InitModbusTask1] InitModbusTask");
	ModbusManager1.Init("modbus1");
	ModbusManager1.OnCmdComplited = OnDataFromModBusRTU1;
	Log::INFO("[InitModbusTask1] InitModbusTask complited");
}
//=======================================================
void InitModbus2Task(void)
{
	Log::INFO("[InitModbusTask2] InitModbusTask");
	ModbusManager2.Init("modbus2");
	ModbusManager2.OnCmdComplited = OnDataFromModBusRTU2;
	Log::INFO("[InitModbusTask2] InitModbusTask complited");
}
//=======================================================
void InitFirewallTask(void)
{
	//Log::INFO("[InitFirewallTask] InitFirewallTask");
	//FireWall.Init(NETWORK_FILE, FIREWALL_FILE);
	//Log::INFO("[InitFirewallTask] InitFirewallTask complited");
}
//=======================================================
void InitSQL(void)
{
	Log::INFO("[InitSQL] InitSQL");
	//system("sql_install_db");
	Repository::Init("/mnt/data/", 0);
}
//=======================================================
bool InitClimatTask(void)
{
	bool ret = false;
	InitSQL();
	ret = MkkClimatManager.InitDB();//Repository::Mkk2Table.InitTable();
	Log::INFO("InitClimatTask complited ret="+toString(ret));
	return ret;
}
//=======================================================
void InitSelfTestingTask(void)
{
	SelfTestingSettings cfg;
	cfg.udpPort = 5000;
	cfg.RecvTimeout = 100;
	cfg.SendTimeout = 100;
	cfg.managers.push_back(&EnergyCounterManager);
	cfg.managers.push_back(&IbpManager);
	cfg.managers.push_back(&FreeCoolingManager);
	cfg.managers.push_back(&DiselPanelManager);
	cfg.managers.push_back(&InnerRs485Manager);
	cfg.managers.push_back(&IOManager);


	cfg.modmanagers.push_back(&GetCurrDataObject);
	cfg.modmanagers.push_back(&GetCurrDataObject2);

	SelfTestingManager.Init(&cfg);

	Log::INFO("InitSelfTestingTask complited.");
}
//=======================================================
void InitFifoBuffer(void)
{
	FifoBuffer.load(Configuration.FifoFile);
	ObjectFunctionsTimer.SetTimer(FIFO_SAVE_FILE_TIMER, Configuration.FifoPeriod);
}
//=======================================================
void InitInputsDetectTask(void)
{
	Log::INFO("[InitInputsDetectTask] InnerRs485Task start Init ");
	IOManager.Init(&Configuration.InputList);
	Log::INFO("[InitInputsDetectTask] InnerRs485Task end Init");
}
//******************************************************************************
//******************************************************************************
//******************************************************************************
void RouterOnAnswerErrorExecute(void *Par)
{
	/*static int cntErr = 0;
	if(++cntErr >= 6)
	{
		cntErr = 0;

		Log::INFO("RouterOnAnswerErrorExecute router reboot");
		string rebootStr = "/sbin/reboot";
		sendToConsole(rebootStr);
		Log::INFO("RouterOnAnswerErrorExecute router reboot OK");
	}
	else*/
	{
		Log::INFO("RouterOnAnswerErrorExecute network restart");

		SockPort.Close();
		string rebootStr = "/etc/init.d/network restart";
		sendToConsole(rebootStr);

		Log::INFO("RouterOnAnswerErrorExecute network restart OK");
	}



}
//******************************************************************************
// perexvatbsoobsheniybot MKK formata [,releState,Tin,Tout1,Tout2,Tcpu]
//******************************************************************************
void OnDataFromMKK(void *Par)
{//climat control all data fdrom mkk has [,state,temp1,temp2,temp3,tempCpu,]
	try
	{
		if(!MkkClimatManager.Settings().Enable)
			return;
		vector<string> retVect = *((vector<string> *)Par);
		string bf = "";
		for(auto curr: retVect){
			bf+=curr+",";
		}
		//Log::DEBUG("From MKK ["+bf+"]");
		string log = "echo '"+bf+"' > /tmp/mkk.log";
		system(log.c_str());
		if(retVect.size() > 2)
		{
			//Log::DEBUG("MkkClimatManager.Settings().Enable");
			Word lastState = atoi(retVect[0].c_str());
			if(MkkClimatManager.Settings().IsAuto == false)
			{//
				Word newState = 0;
				Word errorState = 0;

				float tempVal = atof(retVect[1].c_str());
				if(lastState == 0 && tempVal == 0.0)
				{
					float tempVal1 = atof(retVect[2].c_str());
					float tempVal2 = atof(retVect[3].c_str());
					if(tempVal1 == 0.0 && tempVal2 == 0.0)
					{
						Log::DEBUG("FirstMess from MKK detected");
						return;
					}
				}
				if( isnan(tempVal) ){//proverka na dopustimost znachenia temperaturi
					errorState = 1;
				}
				else if(tempVal >= 199) {//obriv
					errorState = 2;
				}
				else if(tempVal <= -99){//KZ
					errorState = 3;
				}

				MkkClimatManager.LastErrorState = errorState;
				if(errorState > 0){
					newState = 0x0F;
				}
				else
				{//all ok - prinimaem reshenie o perecluchenii rele
					Word lState = MkkClimatManager.GetLastState(lastState);
					//Log::DEBUG("lState="+toString(lState)+" tempVal="+toString(tempVal));

					bool cond1enable = (1 << 3) & lState;
					if(cond1enable){
						if( tempVal > MkkClimatManager.Settings().Cond1OffTemp){// stavin 1 eshe oxlajdaem
							newState |= (1 << 3);
						}
					}
					else{
						if( tempVal >= MkkClimatManager.Settings().Cond1OnTemp){// vklu4aem
							newState |= (1 << 3);
						}
					}
					bool cond2enable = (1 << 2) & lState;
					if(cond2enable){
						if( tempVal > MkkClimatManager.Settings().Cond2OffTemp){// stavin 1 eshe oxlajdaem
							newState |= (1 << 2);
						}
					}
					else{
						if( tempVal >= MkkClimatManager.Settings().Cond2OnTemp){// vklu4aem
							newState |= (1 << 2);
						}
					}
					bool heater1enable = (1 << 1) & lState;
					if(heater1enable){
						if( tempVal < MkkClimatManager.Settings().Heater1OffTemp){//
							newState |= (1 << 1);
						}
					}
					else{
						if( tempVal <= MkkClimatManager.Settings().Heater1OnTemp){//
							newState |= (1 << 1);
						}
					}
					bool heater2enable = (1 << 0) & lState;
					if(heater2enable){
						if( tempVal < MkkClimatManager.Settings().Heater2OffTemp){//
							newState |= (1 << 0);
						}
					}
					else{
						if( tempVal <= MkkClimatManager.Settings().Heater2OnTemp){//
							newState |= (1 << 0);
						}
					}
				}
				MkkClimatManager.RotateStates(newState);
				newState = MkkClimatManager.SetStates(newState);
				MkkClimatManager.LastState = newState;//

				if(newState != lastState)
				{
					Log::DEBUG("newState="+toString(newState) + " lastState="+toString(lastState) + " tempVal="+toString(tempVal));

					if(  ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0 ){
						string data ="50,1,>,50,1,"+toString(newState);
							  TCmdMessage *NewMessage = new TCmdMessage();
							  NewMessage->CMD = "SetData";
							  NewMessage->Password    = BaseRS485Cmd.PasswordPtr;
							  NewMessage->Addr        = "02";
							  NewMessage->Data.push_back(data);
							  RS485CmdMessages.push_front(NewMessage);
							  Log::DEBUG("Send To MKK data="+data);
					}
				}
			}
			else{//

			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "OnDataFromMKK " + string( e.what() ) );
	}
}
//******************************************************************************
void OnDataFromMKK_1(void *Par)
{//if(et.EType == 41 && et.EName == 50 && addr == 2)
	try
	{
		if(!MkkClimatManager.Settings().Enable)
			return;
		TEt et = *((TEt*)Par);
		int size = et.Ptypes.size();
		if( size > 3)
		{
			string sendMessage = "";
			sendMessage += toString(MkkClimatManager.Settings().Etype)+","+
					toString(MkkClimatManager.Settings().Ename)+",";
			for(auto pt: et.Ptypes)
			{
				if(pt.PType == 40)
				{
					sendMessage += ">,"+toString(pt.PType)+','+toString(pt.PName)+',';
					for(auto val: pt.Value)
					{
						sendMessage += val+',';
					}
				}
				else if(pt.PType == 42 || pt.PType == 43)
				{
					if(pt.Value.size() == 6)
					{
						sendMessage += ">,"+toString(pt.PType)+','+toString(pt.PName)+',';
						sendMessage += pt.Value[0] +',';
						sendMessage += pt.Value[1] +',';
						sendMessage += pt.Value[2] +',';

						sendMessage += ">,"+toString(pt.PType)+','+toString(pt.PName+1)+',';
						sendMessage += pt.Value[3] +',';
						sendMessage += pt.Value[4] +',';
						sendMessage += pt.Value[5] +',';
					}
					else if(pt.Value.size() == 9){
						sendMessage += ">,"+toString(pt.PType)+','+toString(pt.PName)+',';
						sendMessage += pt.Value[3] +',';
						sendMessage += pt.Value[4] +',';
						sendMessage += pt.Value[5] +',';

						sendMessage += ">,"+toString(pt.PType)+','+toString(pt.PName+1)+',';
						sendMessage += pt.Value[6] +',';
						sendMessage += pt.Value[7] +',';
						sendMessage += pt.Value[8] +',';

					}
				}
			}
			TRtcTime dt = SystemTime.GetTime();
			string dtStr = toString(dt.Day)+"."+toString(dt.Month)+"."+toString(dt.Year)+","+
					toString(dt.Hour)+":"+toString(dt.Minute)+":"+toString(dt.Second)+">>";
			string framStr = dtStr + sendMessage;
			 if(framStr.size() > 0)
			 {
				 FifoBuffer.push(framStr);
			 }
			 framStr.clear();
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "OnDataFromMKK_1 " + string( e.what() ) );
	}
}
//******************************************************************************
void OnDataFromMKK_2(void *Par)
{//(et.EType == 50 && et.EName == 1 && addr == 2) (relestate)//recv by mkk2 reglament!
	try
	{
		if(!MkkClimatManager.Settings().Enable) return;
		MkkClimatManager.ReloadConfig();

		TEt et = *((TEt*)Par);
		int size = et.Ptypes.size();
		string parmessage = "";
		if(MkkClimatManager.Settings().IsAuto){
			/*parmessage += ">,99,1,"+toString(MkkClimatManager.Settings().Enable)+","+
					FloatToString(ExternClimatManager.Cond1OnTemp.GetData())+","+
					FloatToString(ExternClimatManager.Cond1OffTemp.GetData())+","+
					FloatToString(ExternClimatManager.Cond2OnTemp.GetData())+","+
					FloatToString(ExternClimatManager.Cond2OffTemp.GetData())+","+
					FloatToString(ExternClimatManager.Heater1OnTemp.GetData())+","+
					FloatToString(ExternClimatManager.Heater1OffTemp.GetData())+","+
					FloatToString(ExternClimatManager.Heater2OnTemp.GetData())+","+
					FloatToString(ExternClimatManager.Heater2OffTemp.GetData())+","+
					">,100,1,"+ toString(MkkClimatManager.Settings().IsAuto)+",";*/
		}
		else
		{
			parmessage +=
				">,230,1,"+toString(MkkClimatManager.Settings().Enable)+","+
				FloatToString(MkkClimatManager.Settings().Cond1OnTemp)+","+
				FloatToString(MkkClimatManager.Settings().Cond1OffTemp)+","+
				FloatToString(MkkClimatManager.Settings().Cond2OnTemp)+","+
				FloatToString(MkkClimatManager.Settings().Cond2OffTemp)+","+
				FloatToString(MkkClimatManager.Settings().Heater1OnTemp)+","+
				FloatToString(MkkClimatManager.Settings().Heater1OffTemp)+","+
				FloatToString(MkkClimatManager.Settings().Heater2OnTemp)+","+
				FloatToString(MkkClimatManager.Settings().Heater2OffTemp)+","+
				">,230,2,"+ toString(MkkClimatManager.Settings().RotateTime)+","+

				">,100,1,"+ toString(MkkClimatManager.Settings().IsAuto)+","+
				">,101,1,"+ toString(MkkClimatManager.LastErrorState)+","+
				toString(MkkClimatManager.GetDBstate())+",";

				string climatStr = MkkClimatManager.GetStringValue();
				if(climatStr.size() > 0){
					parmessage = climatStr + "," + parmessage;
				}

		}
		string sendMessage = "";
		/*if(MkkClimatManager.Settings().IsAuto){
			sendMessage += toString(MkkClimatManager.Settings().Etype+1)+","+
					toString(MkkClimatManager.Settings().Ename)+",";
		}
		else*/
		{
			sendMessage += toString(MkkClimatManager.Settings().Etype)+","+
					toString(MkkClimatManager.Settings().Ename)+",";
		}

		if( size > 0 )
		{
			for(auto pt: et.Ptypes)
			{
				if(pt.PType == 50 && pt.Value.size() > 0)
				{
					sendMessage += ">,"+toString(200)+','+toString(pt.PName)+',';
					Word value = atoi(pt.Value[0].c_str());
					for(int ind=3; ind >= 0; ind--)
					{
						if(value & (1 << ind))
							sendMessage += "1,";
						else
							sendMessage += "0,";
					}
				}
			}
		}
		TRtcTime dt = SystemTime.GetTime();
		string dtStr = toString(dt.Day)+"."+toString(dt.Month)+"."+toString(dt.Year)+","+
				toString(dt.Hour)+":"+toString(dt.Minute)+":"+toString(dt.Second)+">>";
		string framStr = dtStr + sendMessage+parmessage;
		if(framStr.size() > 0)
		{
			FifoBuffer.push(framStr);
		}
		framStr.clear();
	}
	catch(exception &e)
	{
		Log::ERROR( "OnDataFromMKK_2 " + string( e.what() ) );
	}
}
//******************************************************************************
void OnDataFromMKK_3(void *Par)
{//if(et.EType == 61 && et.EName == 1 && addr == 2)
	try
	{
		if(!MkkClimatManager.Settings().Enable)
			return;
		TEt et = *((TEt*)Par);
		int size = et.Ptypes.size();
		if( size > 0 )
		{
			string sendMessage = toString(MkkClimatManager.Settings().Etype)+","+
					toString(MkkClimatManager.Settings().Ename)+",";

			string parString = "";
			int index = 0;
			for(auto pt: et.Ptypes)
			{
				if(pt.PType == 9)
				{
					parString += ">,"+toString(pt.PType)+','+toString(pt.PName)+',';
					for(auto val: pt.Value)
					{
						parString += val+',';
					}
				}
				else if(pt.PType == 202)
				{
					if(pt.Value.size() > 0){
						int val = atoi(pt.Value[0].c_str());
						float value = val*1000.0*60.0/MkkClimatManager.Settings().ImpulseCount;
						int sta = MkkClimatManager.LastState;
						//if(sta != 0xF)
						{
							bool f1= (1 << (3-index)) & sta;
							bool f2= (1 << (1-index)) & sta;
							int i = index;

							if(f1 && f2){
								value = value/2.0;
							}
							if(f1){
								Log::DEBUG("f1 calc value="+toString(value)+" i="+toString(i));
								MkkClimatManager.ClimatPower.Params[i]->PowerValue.CalcMinAverMax(value);
							}
							else{
								MkkClimatManager.ClimatPower.Params[i]->PowerValue.CalcMinAverMax(0);
							}
							if(f2){
								Log::DEBUG("f2 calc value="+toString(value)+" i="+toString(i));
								MkkClimatManager.ClimatPower.Params[i+2]->PowerValue.CalcMinAverMax(value);
							}
							else{
								MkkClimatManager.ClimatPower.Params[i+2]->PowerValue.CalcMinAverMax(0);
							}
							MkkClimatManager.ClimatPower.Enable = true;
							Log::DEBUG("sta="+toString(sta)+" index="+toString(index)+" value="+toString(value)+" f1="+toString(f1)+" f2="+toString(f2));
						}
					}
				}
				index++;
			}
			if(parString.size() > 0)
			{
				TRtcTime dt = SystemTime.GetTime();
				string dtStr = toString(dt.Day)+"."+toString(dt.Month)+"."+toString(dt.Year)+","+
						toString(dt.Hour)+":"+toString(dt.Minute)+":"+toString(dt.Second)+">>";
				string framStr = dtStr + sendMessage + parString;
				if(framStr.size() > 0)
				{
					FifoBuffer.push(framStr);
				}
				framStr.clear();
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "OnDataFromMKK_3 " + string( e.what() ) );
	}
}
//******************************************************************************
//******************************************************************************
void OnDataFromMKAB(void *Par)
{//et.EType == 26 && pt = 83 && addr == (11 || 12 || 13 || 14))
	try
	{
		TRtcTime dt = SystemTime.GetTime();
		TEt et = *((TEt*)Par);
		int size = et.Ptypes.size();
		Log::DEBUG("OnDataFromMKAB size="+toString(size));
		string sendMessage = "";

		for(auto pt: et.Ptypes)
		{
			if(et.EType == 26 && pt.PType == 83)
			{
				bool disFlg 	= false;
				int  nulCnt  	= 0;
				string tmpStr 	= "";
				for(auto val: pt.Value)
				{
					float fvalue 	= atof(val.c_str());
					if(fvalue < 1.0)
						nulCnt++;
					else if(fvalue >= 1.0 && fvalue < 12.65)
					{
						disFlg = true;
					}
					tmpStr 	+= val+',';
				}
				bool nulFlg 	= nulCnt == pt.Value.size();
				Log::DEBUG("OnDataFromMKAB nulFlg=" +toString(nulFlg));
				if(nulFlg)
					continue;

				if(dt.Minute % 30 == 0 || disFlg ){
					sendMessage += ">,"+toString(pt.PType)+','+toString(pt.PName)+',' + tmpStr;
				}
			}
			else if(et.EType == 0 && pt.PType == 100)
			{
				string tmpStr 	= "";
				for(auto val: pt.Value){
					tmpStr 	+= val+',';
				}
				if( dt.Minute % 30 == 0 ){
					sendMessage += ">,"+toString(pt.PType)+','+toString(pt.PName)+',' + tmpStr;
				}
			}
		}

		if(!isNullOrWhiteSpace(sendMessage))
		{
			sendMessage = toString(et.EType)+","+ toString(et.EName)+"," + sendMessage;
			string dtStr = toString(dt.Day)+"."+toString(dt.Month)+"."+toString(dt.Year)+","+ toString(dt.Hour)+":"+toString(dt.Minute)+":"+toString(dt.Second)+">>";
			string framStr = dtStr + sendMessage;
			if(framStr.size() > 0)
			{
				FifoBuffer.push(framStr);
			}
			Log::DEBUG("OnDataFromMKAB framStr=["+framStr+"]");
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "OnDataFromMKAB " + string( e.what() ) );
	}
}
//******************************************************************************
void OnDataFromModBusRTU1(void *Par, void *p)
{
	try
	{
	}
	catch(exception &e)
	{
		Log::ERROR( "OnDataFromModBusRTU " + string( e.what() ) );
	}
}

void OnDataFromModBusRTU2(void *Par, void *p)
{
	try
	{
/*
		if(Par != NULL)
		{
			IModbusRegisterList *curr = (IModbusRegisterList *)Par;
			//curr->GetCfgString();
			if(curr->fun == 3)
			{
				ModbusRegPar *con1on  = curr->FindParameter(18);
				ModbusRegPar *con1off = curr->FindParameter(19);
				ModbusRegPar *con2on  = curr->FindParameter(22);
				ModbusRegPar *con2off = curr->FindParameter(23);

				ModbusRegPar *h1on  = curr->FindParameter(26);
				ModbusRegPar *h1off = curr->FindParameter(27);
				ModbusRegPar *h2on  = curr->FindParameter(28);
				ModbusRegPar *h2off = curr->FindParameter(29);

				ModbusRegPar *maintemp = curr->FindParameter(3);

				ModbusRegPar *roomTemp = curr->FindParameter(1);
				ModbusRegPar *extTemp  = curr->FindParameter(2);

				ModbusRegPar *state  = curr->FindParameter(148);

				ModbusRegPar *fstpstart 	= curr->FindParameter(11);
				ModbusRegPar *fstpend 		= curr->FindParameter(12);
				ModbusRegPar *secstpstart	= curr->FindParameter(13);
				ModbusRegPar *secstpend 	= curr->FindParameter(14);

				ModbusRegPar *sf1 = curr->FindParameter(130);
				ModbusRegPar *sp1 = curr->FindParameter(129);


				if(sf1 != NULL) {
					TMamStruct sf = ExternClimatManager.Sf1.GetData();
					sf.CalcMinAverMax(sf1->data);
					ExternClimatManager.Sf1.SetData(sf);
					delete sf1;
				}
				if(sp1 != NULL) {
					TMamStruct sp = ExternClimatManager.Sp1.GetData();
					sp.CalcMinAverMax(sp1->data);
					ExternClimatManager.Sp1.SetData(sp);
					delete sp1;
				}


				if(fstpstart != NULL) {
					ExternClimatManager.FirstPointsStepStart.SetData(fstpstart->data/10.0);
					delete fstpstart;
				}
				if(fstpend != NULL) {
					ExternClimatManager.FirstPointsStepEnd.SetData(fstpend->data/10.0);
					delete fstpend;
				}
				if(secstpstart != NULL) {
					ExternClimatManager.SecondPointsStepStart.SetData(secstpstart->data/10.0);
					delete secstpstart;
				}
				if(secstpend != NULL) {
					ExternClimatManager.SecondPointsStepEnd.SetData(secstpend->data/10.0);
					delete secstpend;
				}

				if(con1on != NULL) {
					ExternClimatManager.Cond1OnTemp.SetData(con1on->data/10.0);
					delete con1on;
				}
				if(con1off != NULL) {
					ExternClimatManager.Cond1OffTemp.SetData(con1off->data/10.0);
					delete con1off;
				}
				if(con2on != NULL) {
					ExternClimatManager.Cond2OnTemp.SetData(con2on->data/10.0);
					delete con2on;
				}
				if(con2off != NULL) {
					ExternClimatManager.Cond2OffTemp.SetData(con2off->data/10.0);
					delete con2off;
				}

				if(h1on != NULL) {
					ExternClimatManager.Heater1OnTemp.SetData(h1on->data/10.0);
					delete h1on;
				}
				if(h1off != NULL) {
					ExternClimatManager.Heater1OffTemp.SetData(h1off->data/10.0);
					delete h1off;
				}
				if(h2on != NULL) {
					ExternClimatManager.Heater2OnTemp.SetData(h2on->data/10.0);
					delete h2on;
				}
				if(h2off != NULL) {
					ExternClimatManager.Heater2OffTemp.SetData(h2off->data/10.0);
					delete h2off;
				}

				if(maintemp != NULL) {
					ExternClimatManager.MainTempParameter.SetData(maintemp->data/10.0);
					delete maintemp;
				}

				if(roomTemp != NULL) {
					TMamStruct rt = ExternClimatManager.RoomTemp.GetData();
					rt.CalcMinAverMax(roomTemp->data/10.0);
					ExternClimatManager.RoomTemp.SetData(rt);
					delete roomTemp;
				}
				if(extTemp != NULL) {
					TMamStruct et = ExternClimatManager.ExtTemp.GetData();
					et.CalcMinAverMax(extTemp->data/10.0);
					ExternClimatManager.ExtTemp.SetData(et);
					delete extTemp;
				}

				if(state != NULL) {
					Byte hst = ExternClimatManager.State.GetData();
					Byte nst = state->data;
					if(hst != nst)
					{
						ExternClimatManager.State.SetData(nst);
						ExternClimatManager.FirstInitFlg.SetData(true);
					}
					delete state;
				}
			}
			else if(curr->fun == 2)
			{
				ModbusRegPar *par1 = curr->FindParameter(1);
				ModbusRegPar *par2 = curr->FindParameter(2);
				ModbusRegPar *par3 = curr->FindParameter(3);
				ModbusRegPar *par4 = curr->FindParameter(4);

				Word lastState = ExternClimatManager.ClimatStateData.GetData();
				Word newState = 0;

				if(par1 != NULL) {
					//Log::DEBUG("Cond 1 On value = "+ toString((int)par1->data));
					newState |= (par1->data << 3);
					delete par1;
				}
				if(par2 != NULL) {
					//Log::DEBUG("Cond 2 On value = "+ toString((int)par2->data));
					newState |= (par2->data << 2);
					delete par2;
				}
				if(par3 != NULL){
					//Log::DEBUG("Heater 1 On value = "+ toString((int)par3->data));
					newState |= (par3->data << 1);
					delete par3;
				}
				if(par4 != NULL){
					//Log::DEBUG("Heater 2 On value = "+ toString((int)par4->data));
					newState |= (par4->data << 0);
					delete par4;
				}
				if(newState != lastState)
				{
					if(  ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0 )
					{
						string data ="50,1,>,50,1,"+toString(newState);
							  TCmdMessage *NewMessage = new TCmdMessage();
							  NewMessage->CMD = "SetData";
							  NewMessage->Password    = BaseRS485Cmd.PasswordPtr;
							  NewMessage->Addr        = "02";
							  NewMessage->Data.push_back(data);
							  RS485CmdMessages.push_front(NewMessage);
							  Log::DEBUG("RS485CmdMessages.push_front data="+data);
							  ExternClimatManager.ClimatStateData.SetData(newState);
					}
				}
			}

		}
		//Log::ERROR( "OnDataFromModBusRTU2 " + string( e.what() ) );*/
	}
	catch(exception &e)
	{
		Log::ERROR( "OnDataFromModBusRTU2 " + string( e.what() ) );
	}
}
//******************************************************************************
void GenerateDUMMY_NS_command(void *Par)
{
 if( Par != NULL)
  {
	sync_deque<TCmdMessage*> *Messages = (sync_deque<TCmdMessage*>*)Par;
	Log::DEBUG("GenerateDUMMY_NS_command Messages->size() ="+toString(Messages->size()));
    if( Messages->size() >= MAX_MESS_IN_MAIN)	return;
    TCmdMessage *NewMessage = new TCmdMessage();
    NewMessage->CMD         = "Data";
    NewMessage->Password    = Configuration.PassWord;
    NewMessage->Addr        = "";
    NewMessage->State       = "NS,>>";
    NewMessage->NeedCmdAnswer  = false;
    Messages->push_front(NewMessage);

  }
}


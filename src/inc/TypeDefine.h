/*
 * TypeDefine.h
 *
 *  Created on: Sep 23, 2014
 *      Author: user
 */

#ifndef TYPEDEFINE_H_
#define TYPEDEFINE_H_

#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <string.h>
#include <vector>
#include <list>
#include <deque>

using namespace std;
//******************************************************************************
typedef unsigned char  Byte;
typedef unsigned int   Word;
typedef unsigned long  DWord;
typedef unsigned short Short;
typedef signed   short sShort;
//******************************************************************************
typedef signed char sByte;
typedef signed int  sWord;
typedef signed long sDWord;
//******************************************************************************
typedef union
{
    DWord Data_l;
    Byte Data_b[4];
} Blong;
//******************************************************************************
typedef union
{
	Short Data_s;
    Byte Data_b[2];
} Bshort;
//******************************************************************************
typedef union
{
	sShort Data_s;
    Byte Data_b[2];
} sBshort;
//******************************************************************************
typedef union
{
    float Data_f;
    Byte Data_b[4];
} Bfloat;
//******************************************************************************
typedef union
{
    float Data_f;
    Byte Data_b[4];
} Ufloat;
//******************************************************************************
typedef union
{
    double Data_d;
    Byte Data_b[8];
} Bdouble;
//******************************************************************************
struct inetData
{
	int Port;
	string Address;
    int RecvTimeout;
    int SendTimeout;
};
//******************************************************************************


typedef struct Time
{
  int Year, Month, DayOfWeek, Day, Hour, Minute, Second;
}TRtcTime;


/*
class TRtcTime
{
public:
	TRtcTime(){
		Year = 0;
		Month = 0;
		DayOfWeek = 0;
		Day = 0;
		Hour = 0;
		Minute = 0;
		Second = 0;
	}

	int Year;
	int Month;
	int DayOfWeek;
	int Day;
	int Hour;
	int Minute;
	int Second;

	string ToString(){
		return "";

	}
private:

};*/

/*toString(Year)+"-"+toString(Month)+"-"+toString(Day)+
"T"+toString(Hour)+":"+toString(Minute)+":"+toString(Second);*/
//******************************************************************************
class CommSettings
{
public:
	CommSettings()
	{
		DeviceName = "/";
		BaudRate = 9600;
		BaudRate2 = 0;
		SendTimeout = 1001;
		RecvTimeout = 1001;
		DataBits = 8;
		StopBit = 1;
		Parity = 0;
	}

	bool operator==(const CommSettings& right) const
	{
		return  DeviceName 	== right.DeviceName 	&&
				BaudRate 	== right.BaudRate 		&&
				SendTimeout == right.SendTimeout 	&&
				RecvTimeout == right.RecvTimeout 	&&
				DataBits 	== right.DataBits 		&&
				StopBit 	== right.StopBit 		&&
				Parity 		== right.Parity;
	}
    string 	DeviceName;
    //string 	DeviceName2;
    DWord 	BaudRate;
    DWord 	BaudRate2;
    DWord 	SendTimeout;
    DWord 	RecvTimeout;
    Byte    DataBits;
    Byte    StopBit;
    Byte    Parity;/*PARITY_NONE=0 PARITY_ODD=1 PARITY_EVEN=2*/
};
//******************************************************************************
class SnmpSettings
{
public:

	bool operator==(const SnmpSettings& right) const
	{
		return  read 	== right.read 	&&
				write 	== right.write 		&&
				trap    == right.trap;
	}
    string 	read;
    string 	write;
    string 	trap;
};

//******************************************************************************
struct IbpSettings
{
	CommSettings 	portsettings;
	bool 			enable;
	Byte			type;
	int    		    address;
	int 			etype;
	int 			ename;
	Word            period;
	Word 			betweentimeout;
	SnmpSettings    snmpsettings;
	float			ktran;
	vector<string>  commands;
	//vector<string>  configs;
};
//******************************************************************************
struct FreeSettings
{
	CommSettings 	portsettings;
	bool 			enable;
	Byte			type;
	int    		    address;
	int 			etype;
	int 			ename;
	Word            period;
	Word 			betweentimeout;
	Byte            screen;
};
//******************************************************************************
struct DguSettings
{
	CommSettings 	portsettings;
	bool 			enable;
	Byte			type;
	int             address;
	int 			etype;
	int 			ename;
	vector<string>  commands;
	vector<string>  configs;
	Word            period;
	Word            aperiod;
	Word 			betweentimeout;
	Word 			needinittimeout;
	float			ktran;
};
//******************************************************************************
struct ModuleSettings
{
	CommSettings 	portsettings;
	bool 			enable;
	Byte			type;
	Word            period;
	Word            aperiod;
	int             etype;
	int 			ename;
	vector<string>  commands;
	vector<string>  configs;
	Word 			betweentimeout;
	Word 			maxerrors;
	Word			resetport;
	Word			selvalue;
	Word			seltimeout;
	string			selports;
};
//******************************************************************************
struct InputSettings
{
	bool 			enable;
	Byte			type;
	Word            period;

	int             etype;
	int 			ename;
	int             ptype;
	int 			pname;

	bool			inversia;
	Word			port;
	Word			timeout;//ms
};
//******************************************************************************
struct ClimatSettings
{
	bool  Enable;
	bool  IsAuto;

	long RotateTime;

	float Heater1OnTemp;
	float Heater1OffTemp;
	float Heater2OnTemp;
	float Heater2OffTemp;

	float Cond1OnTemp;
	float Cond1OffTemp;
	float Cond2OnTemp;
	float Cond2OffTemp;

	int   Etype;
	int   Ename;

	int   Period;

	int   ImpulseCount;


};
//******************************************************************************
struct EcounterSettings
{
	CommSettings 	portsettings;
	bool 			enable;
	Byte            type;
	string          address;
	int 			ename;
	Word 			betweentimeout;
	Word 			period;
	string			poverka;
	string 			cmd;
	bool			hex;
	int			    plc;
	int 			timezone;
	int 			index;
};
//******************************************************************************
struct SocketSettings
{
        string IpAddress;
        //string IpAddress2;
        string Proto;
        DWord IpPort;//remote
        DWord BindPort;//local
        DWord SocketSendTimeout;   //
        DWord SocketRecvTimeout; //
};
//******************************************************************************
struct EcounterCmdParameter
{
	Byte Et;
	Byte En;
	Byte addr;
	Byte cmd;
	Byte sub_cmd;
	int index;
	vector<Byte> parameters;

};



//******************************************************************************
struct IbpCmdParameter
{
 Byte index;
 Byte pn;
 Byte sub_sys_index;
 Byte data_object;
 Byte data_element;
 vector<Byte> 	Data;

};

//******************************************************************************
/*struct IbpSettings
{
	CommSettings 		 portsettings;
	Word 				 period;
	//vector<CommSettings> controllers;
	vector<Word> controllers;
};*/
//******************************************************************************
struct DeviceOnLineSettings
{
	Word	  		address;
	string 			cmd;
	CommSettings 	comsettings;
	DWord			AckCnt;
};
//******************************************************************************
struct TaskListSettings
{
	string  						PassWord;
	vector<DeviceOnLineSettings> 	DeviceList;
	void 							*mainMessages;
};
//******************************************************************************
struct Data_Params
{
        string  DevAddress;//
        string  PassWord;
        string  PassWord485;
        string  Unit_Type;
        string  FifoFile;
        DWord 	SerialNumber;
        string 	AsduNumber;
        string 	MkddNumber;
        string  SmsPort;


        Word 	RepeatCount;
        Word    ResetCount;

        Word EcounterPeriod;
        Word EprofilPeriod;

        Word DataTaskPeriod;
        Word IbpPeriod;
        Word FreePeriod;
        Word DguPeriod;
        Word FifoPeriod;

        string IpAddress;

        DWord IpPort;
        DWord SocketSendTimeout;   //
        DWord SocketRecvTimeout; //

        DWord UdpLocalPort;
        DWord UdpRemotePort;

        DWord WebApiPort;

        bool VPSEnable;
        DWord VPServerPort;
        DWord VPSocketSendTimeout;   //
        DWord VPSocketRecvTimeout; //
        Word  VPSCloseTimeout;

        string RS485Device;
        DWord RS485BaudRate;
        DWord RS485SendTimeout;   //
        DWord RS485RecvTimeout; //
        vector<DeviceOnLineSettings> 	DeviceList;

        string RS485Device2;
        DWord RS485BaudRate2;
        DWord RS485SendTimeout2;   //
        DWord RS485RecvTimeour2; //
        vector<DeviceOnLineSettings> 	DeviceList2;

        //string EcounterDevice;
        //DWord EcounterBaudRate;
        //DWord EcounterSendTimeout;   //
        //DWord EcounterRecvTimeour; //
        //Word EcounterBetweenTimeout; //

        vector<EcounterSettings> CounterList;

        vector<IbpSettings>             IbpList;
        vector<FreeSettings>            FreeList;
        vector<DguSettings>             DguList;
        vector<ModuleSettings>          ModuleList;
        vector<InputSettings>			InputList;

        ClimatSettings					ClimatManagerSettings;

        Word 					Version;
        Word 					SubVersion;
        string 					Installer;
        string 					Commissioning;
};

//******************************************************************************
typedef unsigned int  HANDLE;
//******************************************************************************
typedef enum
{
    COM0  = 1,
    COM1,
    COM2,
    COM3
}Port;
//******************************************************************************
typedef enum
{
    STX_IND = 0,
    PASSWORD_IND,
    ADDR_IND,
    DATA_IND,
    TIME_IND,
    CMD_IND,
    ERSTAT_IND,
    CMD_DATA_IND,
    CRC_IND,
    ETX_IND
}PROTO_IND;
//******************************************************************************
typedef enum
{
    RX_CMD = 0,
    EXE_CMD,	//1
    TX_CMD,		//2
    PARSING_CMD,//3
    PREPARE_CMD,//4
    WAIT_MES,	//5
    RESET		//6
}FASE_CMD;
//******************************************************************************
typedef enum
{
    OK_IND = 0,
    PASW_ER_IND,
    ID_ER_IND,
    CMD_ER_IND,
    CRC_ER_IND,
    UNKNOWN_ER_IND,
    NO_DATA_ER_IND
}ERROR_IND;
//******************************************************************************
typedef enum
{
    BOOTLOADER  = 0,
    APPLICATION
}BOOT_STAT;
//******************************************************************************
enum DATA_TASK_TIMER {TASK_PERIUD_TIMER = 0, TASK_MAIN_TIMER};
//******************************************************************************
enum TIMERS  		{ PERIUD_TIMER = 0, COMMON_TIMER, ADDITIONAL_TIMER, LONG_TIMER, ALARM_TIMER};
enum EFASE          { CREATE_CMD   = 0, SEND_CMD, RECV_CMD, EXEC_CMD, DO_ECMD };
enum ESUBFASE       {
	SET_PLC = 0,
	OPEN_CHANNAL,
	GET_ETYPE,
	GET_EISP,
	GET_E,//4
	GET_ER,//5
	GET_E1,
	GET_E2,
	GET_E3,
	GET_E4,
	GET_E5,
	GET_Ef,//11
	GET_E1f,
	GET_E2f,
	GET_E3f,
	GET_E4f,
	GET_U,//16
	GET_Ua,
	GET_Ub,
	GET_Uc,
	GET_I,//20
	GET_Ia,
	GET_Ib,
	GET_Ic,
	GET_P,//24
	GET_Pa,
	GET_Pb,
	GET_Pc,
	GET_Q,//28
	GET_Qa,
	GET_Qb,
	GET_Qc,
	GET_S,//32
	GET_Sa,
	GET_Sb,
	GET_Sc,
	GET_KM,//36
	GET_F,
	GET_SERIAL,
	CLOSE_CHANNAL,//39
	AUTORIZATE,
	AUTORIZATE1,
	GET_KTRAN,	//42
	WAIT_ECMD,
	SET_ECMD,//44
	END_ECMD,
	GET_NETADDR
};

//WAIT_ECMD = 0, OPEN_ECHANNAL, SET_ECMD, END_ECMD


enum SH_SCREENS  	{ SH_UNKNOWN = 0, SH_MAIN, SH_PASSWORD, SH_MAIN_MENU, SH_ALARM};
//******************************************************************************
typedef enum
{
    DETECT_STATES = 0,
    FORMING_MESSAGE
}DATCHIK_MNG;
//******************************************************************************
enum DETECTING_RESULT
{
	NO_RESULT=0,MAX_DETECTED,MIN_DETECTED
};
//******************************************************************************
typedef enum
{
	SNMP_GET_req      	= 0xA0,
	SNMP_GET_NEXT_req 	= 0xA1,
	SNMP_GET_resp     	= 0xA2,
	SNMP_SET_req		= 0xA3,
	SNMP_TRAP    		= 0xA4
} PDU_TYPE;

typedef enum
{
	SNMP_V1     = 0,
	SNMP_V2 	= 1,
} SNMP_VER;
//******************************************************************************
typedef enum
{
    MODBUS_RTU = 0,
    MODBUS_TCP,		//1
    MODBUS_OVER_TCP,//2
}MODBUS_TYPE;
//******************************************************************************

typedef void (*PFV)(void* p );
//******************************************************************************
typedef void (*PFV2)(void* p1, void *p2 );
//******************************************************************************
typedef void* (*PFVptr)(void* p1, void *p2 );
//******************************************************************************
typedef int (*PFVint)(void);
//*****************************************************************************
//=============================================================================
// TComPort
//====================================
#define INVALID_HANDLE_VALUE  0

#define RX_BYTE_TIMEOUT       0x00      //
#define RX_TOTAL_TIMEOUT      0x01
#define TX_BYTE_TIMEOUT       0x02
#define TX_TOTAL_TIMEOUT      0x03

#define MAX_BUF_LEN           2000
#define PASSWORD_CNT          1
#define ADDR_CNT              2
#define CMD_CNT               3
//#define ERROR               (-1)
//====================================
#define SOH           0x01
#define STX           0x02
#define ETX           0x03
#define SOI           0x7E
#define EOI           0x0D

#define STC           0x21
#define STD           0x3A
//=============================================================================
// TCmd_msg
//====================================
const unsigned char ERROR_MASS[][16]=
{
    {"OK"},//
    {"PASW_ER"},
    {"ID_ER"},
    {"CMD_ER"},//
    {"CRC_ER "},//
    {"UNKN_ERROR"},//
    {"NO_DATA"},
};

//=============================================================================
// TObjectFunctions
//====================================
//#define MAX_MESS_IN_SOCKET      10
//#define MAX_MESS_IN_RS485       10
#define MAX_MESS_IN_ALTERNATE   200
#define MAX_MESS_IN_MAIN        200

#define MAX_DEV_ADDR        		20
#define MAX_ECOUNTER_ADDR   		5
#define MAX_DATCHIKS        		32
#define MAX_TERMO_DATCHIKS  		8
#define MAX_CODES           		40

#define USB_RS485_ACTIV_WAIT_TIMER 	0
#define FIFO_SAVE_FILE_TIMER 		1
#define TIME_EVENT_TIMER1 			2
#define TIME_EVENT_TIMER2 			3
#define TIME_EVENT_TIMER3 			4
#define TIME_EVENT_TIMER4 			5
//=============================================================================
// TAdcPwmFeedback
//====================================
#define MAX_ADC_INPUTS          12    0
#define ADC_TIMER_PERIUD        10 //
//=============================================================================

#define MAX_ERRORS_TO_RESET_ROUTER 60
#define REPEAT_RECV_COUNT          10     //!!!!!USB_Timeout
//=============================================================================
#define PARITY_NONE 0
#define PARITY_ODD  1//
#define PARITY_EVEN 2//

#define PID_FILE 				"/var/run/mspd_daemon.pid"
#define FIFO_FILE 				"/mnt/data/fifo.log"
#define SMS_PORT 				"/dev/ttyUSB1"
#define MAX_SIM_NUMBER_LEN		12
#define CONFIG_FILE				"mspd48"
#define NETWORK_FILE			"network"
#define FIREWALL_FILE			"my_firewall"
#define ETH_PORT 				"eth0"
#define GPRS_PORT				"3g-wan"


#define VERSION                 4.74

#define MAX_MESS_IN_PORTFORWARD  30


#define PSCH_CONSTANT 250

#define MAX_SECOND_IN_DAY 87400 //86399



#endif /* TYPEDEFINE_H_ */

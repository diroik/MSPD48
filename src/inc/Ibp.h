/*
 * Ibp.h
 *
 *  Created on: Oct 20, 2014
 *      Author: user
 */

#ifndef IBP_H_
#define IBP_H_

#include <CommPort.h>
#include <Classes.h>
#include <netsnmp.h>
#include <math.h>
#include <stdio.h>
#include <SocketPort.h>
#include <clocale>
#include <map>


using namespace std;
using namespace log4net;
using namespace MyClassTamlates;
//=============================================================================
//enum EFASE          { CREATE_CMD   = 0, SEND_CMD, RECV_CMD, EXEC_CMD, DO_ECMD };
enum ICMDSUBFASE    { WAIT_ICMD = 0, SET_ICMD, END_ICMD };
enum IBPSUBFASE       {
IBP_INIT = 0,
GET_STAT,
GET_SYS,
GET_SYS_MODE,
GET_AC1,
GET_AC2,//5
GET_AC3,
GET_RECT,//7
GET_RECT_ALARM,
GET_RECT_BOOL,
GET_LOAD,//10
GET_AKB,
GET_JOURNAL,//12
GET_UE,
GET_ALARMS,
GET_ALARMS1,//15
GET_TEMP,
GET_COMP,//17
GET_SITE,
GET_SERIALN,
GET_AKB_TYPE,//20
GET_POINT_STEPS,
IBP_SET_CMD,
IBP_EXIT,//23

GET_I_IN1,
GET_I_IN2,
GET_I_IN3,
GET_FREQ_IN,

GET_AC1_OUT,
GET_AC2_OUT,
GET_AC3_OUT,
GET_I_OUT1,
GET_I_OUT2,
GET_I_OUT3,
GET_FREQ_OUT,
GET_LOAD_OUT1,
GET_LOAD_OUT2,
GET_LOAD_OUT3,

GET_AC1_BYP,
GET_AC2_BYP,
GET_AC3_BYP,
GET_FREQ_BYP,
GET_I_BYP1,
GET_I_BYP2,
GET_I_BYP3,

GET_NEG_UE,
GET_NEG_AKB,

GET_UNIT_MODULES,
GET_UNIT_CAPACITY,

IBP_ERROR
};
//=============================================================================
class TIbpCMD//:public TFastTimer
	  {
	  public:
	TIbpCMD():SubSystemIndex(0),DataObject(0),DataElement(0),ControllerInd(0)
		  {
				Fase = CREATE_CMD;
				SubFase = WAIT_ICMD;
				Pn=0;
		  }

		TRtcTime        DateTime;
		Byte			Pn;
		Byte            ControllerInd;
		Byte        	SubSystemIndex;
		Byte			DataObject;
		Byte			DataElement;

		vector<Byte> 	Data;

		Byte Fase;
		Byte SubFase;
};
 //=============================================================================
class TJournalData//:public TFastTimer
{
public:
		 TJournalData()
	  		  {

	  				TestType = 0;
	  				AkbType = "";
	  				Duration = 0;
	  				AverageCurrent = 0;
	  				Discharged = 0;
	  				Temp = 0;
	  				EndVoltage = 0;
	  				BattQ = 0;
	  				Termination = 0;
	  				Result = 0;

	  		  }

		  void clear()
  		  {

  				TestType = 0;
  				AkbType = "";
  				Duration = 0;
  				AverageCurrent = 0;
  				Discharged = 0;
  				Temp = 0;
  				EndVoltage = 0;
  				BattQ = 0;
  				Termination = 0;
  				Result = 0;

  		  }
	  		TRtcTime		StartDateTime;
	  		TRtcTime		EndDateTime;
	  		Byte            TestType;
	  		string			AkbType;
	  		Word			Duration;
	  		int				AverageCurrent;
	  		int             Discharged;
	  		int             Temp;
	  		int				EndVoltage;
	  		Byte			BattQ;
	  		Byte			Termination;
	  		Byte			Result;

	  		string GetStringValue(void)
	  		{
	  			string ReturnString = "";
	  			ReturnString += WordToString(StartDateTime.Day,2)+'.'+WordToString(StartDateTime.Month,2)+'.'+WordToString(StartDateTime.Year,4)+',';
	  			//ReturnString += WordToString(StartDateTime.Hour)+':'+WordToString(StartDateTime.Minute)+':'+WordToString(StartDateTime.Second)+',';
	  			ReturnString += WordToString(EndDateTime.Hour,2)+':'+WordToString(EndDateTime.Minute,2)+':'+WordToString(EndDateTime.Second,2)+',';
	  			ReturnString += WordToString(TestType)+',' + AkbType +',';
	  			ReturnString += IntToString(Duration)+','+IntToString(AverageCurrent)+','+IntToString(Discharged)+',';
	  			ReturnString += IntToString(Temp)+','+FloatToString(EndVoltage/100.0)+',';
	  			ReturnString += WordToString(BattQ)+','+WordToString(Termination)+','+WordToString(Result)+',';
	  			return ReturnString;
	  		}
};
 //=============================================================================
  class TinputVoltage:public IParams
{
 public:
	TinputVoltage(Byte pn):IParams(42,pn)
	{
		Params.push_back(&Ua);
		Params.push_back(&Ub);
		Params.push_back(&Uc);
		Size = 3*3;
	}
	TinputVoltage():IParams(42,1)
	{
		Params.push_back(&Ua);
		Params.push_back(&Ub);
		Params.push_back(&Uc);
		Size = 3*3;
	}
	~TinputVoltage(){}
	   string GetValueString(void)
	   {
	     string ReturnStr = "";
	       for( int i=0; i < Params.size();i++)
	       {
	         TMamStruct *CurrMam = Params[i];
	         ReturnStr += "\r\n>,"+
	                                  WordToString(Pt.Ptype)+','+
	                                  WordToString(Pt.Pname + i)+',';
	         ReturnStr += FloatToString(CurrMam->Aver, 0, 2)+','+
	                      FloatToString(CurrMam->Min, 0, 2)+','+
	                      FloatToString(CurrMam->Max, 0, 2)+',';
	         CurrMam->ClrDivider();
	       }
	       return ReturnStr;
	   }
		TMamStruct Ua;
		TMamStruct Ub;
		TMamStruct Uc;
};
  //=============================================================================
class TinputCurrent:public IParams
{
 public:
	  TinputCurrent(Byte pn):IParams(26,pn)
	{
		Params.push_back(&Ia);
		Params.push_back(&Ib);
		Params.push_back(&Ic);
		Size = 3*3;
	}
	  TinputCurrent(Byte pt, Byte pn):IParams(pt,pn)
	{
		Params.push_back(&Ia);
		Params.push_back(&Ib);
		Params.push_back(&Ic);
		Size = 3*3;
	}

	~TinputCurrent(){}
	   string GetValueString(void)
	   {
	     string ReturnStr = "";
	       for( int i=0; i < Params.size();i++)
	       {
	         TMamStruct *CurrMam = Params[i];
	         ReturnStr += "\r\n>,"+
	                                  WordToString(Pt.Ptype)+','+
	                                  WordToString(Pt.Pname + i)+',';
	         ReturnStr += FloatToString(CurrMam->Aver, 0, 2)+','+
	                      FloatToString(CurrMam->Min, 0, 2)+','+
	                      FloatToString(CurrMam->Max, 0, 2)+',';
	         CurrMam->ClrDivider();
	       }
	       return ReturnStr;
	   }
		TMamStruct Ia;
		TMamStruct Ib;
		TMamStruct Ic;
};
 //=============================================================================
  class TVoltsAndCurrents:public IParams
  {
  public:
	TVoltsAndCurrents():IParams(25, 1),State(0),HState(0)
    {
    	Params.push_back(&U);//1
    	Params.push_back(&Irect);//2
    	Params.push_back(&Iakb);//3
    	Params.push_back(&Iout);//4
    	Params.push_back(&Temp);//5

    	Size = Params.size()*3+1;
    }

	TVoltsAndCurrents(Byte pn):IParams(25, pn),State(0),HState(0)
    {
    	Params.push_back(&U);//1
    	Params.push_back(&Irect);//2
    	Params.push_back(&Iakb);//3
    	Params.push_back(&Iout);//4
    	Params.push_back(&Temp);//5

    	Size = Params.size()*3+1;
    }

   ~TVoltsAndCurrents()
   {}

   string GetValueString(void)
   {
     string ReturnStr = "";/*"\r\n>,"+
                         WordToString(Pt.Ptype)+','+
                         WordToString(Pt.Pname)+',';*/
       for( int i=0; i < Params.size();i++)
       {
         TMamStruct *CurrMam = Params[i];
         ReturnStr += "\r\n>,"+
                                  WordToString(Pt.Ptype)+','+
                                  WordToString(i+1)+',';
         ReturnStr += FloatToString(CurrMam->Aver, 0, 2)+','+
                      FloatToString(CurrMam->Min, 0, 2)+','+
                      FloatToString(CurrMam->Max, 0, 2)+',';
         CurrMam->ClrDivider();
       }
       ReturnStr += "\r\n>,201,1,"+ WordToString(State)+',';
       return ReturnStr;
   }
    TMamStruct U;
    TMamStruct Iakb;
    TMamStruct Irect;
    TMamStruct Iout;
    TMamStruct Temp;

    Byte       State;
    Byte       HState;
  };

//=============================================================================
 class TDigitatAkbInfo:public IParams
 {
 public:
	TDigitatAkbInfo():IParams(29, 1)
   {
   	Params.push_back(&Utot);//1
   	Params.push_back(&Ucmin);//2
   	Params.push_back(&Ucmax);//3
   	Params.push_back(&Ich);//4
   	Params.push_back(&Idch);//5
   	Params.push_back(&Tmin);//6
   	Params.push_back(&Tmax);//7
   	Params.push_back(&Soc);//8
   	Params.push_back(&Soh);//9


   	Size = Params.size()*3+1;

   }
  ~TDigitatAkbInfo()
  {}

  string GetValueString(void)
  {
    string ReturnStr = "";
      for( int i=0; i < Params.size();i++)
      {
        TMamStruct *CurrMam = Params[i];
        ReturnStr += "\r\n>,"+
                                 WordToString(Pt.Ptype)+','+
                                 WordToString(i+1)+',';
        ReturnStr += FloatToString(CurrMam->Aver, 0, 2)+','+
                     FloatToString(CurrMam->Min, 0, 2)+','+
                     FloatToString(CurrMam->Max, 0, 2)+',';
        CurrMam->ClrDivider();
      }
      return ReturnStr;
  }
   TMamStruct Utot;
   TMamStruct Ucmin;
   TMamStruct Ucmax;

   TMamStruct Ich;
   TMamStruct Idch;

   TMamStruct Tmin;
   TMamStruct Tmax;

   TMamStruct Soc;
   TMamStruct Soh;



 };
//=============================================================================
class Iibp: public IDevice
{
	  //friend class TIbpManager;
	public:
		Iibp(Byte Ename, Byte adr);
		Iibp(Byte Etype, Byte Ename, Byte adr);

		~Iibp();

        virtual string  GetStringValue(void);
        virtual string  GetAdvStringValue(void);
        virtual string  GetStringCfgValue(void);

        virtual Word    CreateCMD(Byte CMD, Byte *Buffer) = 0;
        virtual sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
        virtual bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);
        virtual Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len) = 0;

        virtual void    InitGPIO();
        virtual bool    ChangeState(Byte newState){
      	  return true;
        }
        virtual bool    ChangeParameters(vector<string> parameters){
      	  return true;
        }

        virtual bool    DetectStates()
        {
        	bool ret = false;
        	for(auto curr: Alarms.Alarms){
        		if(curr->HValue != curr->Value){
        			curr->HValue = curr->Value;
        			ret = true;
        		}
        	}

        	for(auto curr: PointSteps.Params){
        		if(curr->HValue != curr->Value){
        			curr->HValue = curr->Value;
        			ret = true;
        		}
        	}

        	if(UE.HState != UE.State){
        		UE.HState = UE.State;
        		ret = true;
        	}
        	return ret;
        }

        virtual void 	 ClearValues(void){
        	Uin.ClearValues();
        	//CurrController->UE.ClearValues();
        }


        map<int, TRectifier*> Rectifiers;
        TRectifiersInfo       RectifiersInfo;

        TRtcTime           	CurrTime;
        TinputVoltage      	Uin;
	    TVoltsAndCurrents  	UE;
        //TSiteInfo          	SiteInfo;

        TFrequency			Fa_in;
        TFrequency			Fb_in;
        TFrequency			Fc_in;

        TFrequency			Fa_out;
        TFrequency			Fb_out;
        TFrequency			Fc_out;

        //0-inp volt alarm A,
        //1-inp volt alarm B,
        //2-inp volt alarm C,
        //3-dc voltage alarm,
        //4-dc fuse_alarm,
        //5-global_dc_alarm,
        //6-global rect alarm
	    TIbpAlarms         	Alarms;
	    TUfloatPtype 		PointSteps;

        //Byte          	Et;
        //Byte          	En;

        //bool          	Modify;
        //bool          	IsEnable;
        //Byte          	Type;
        //IPort          *Port;
        //Word 			BetweenTimeout;


        float			Ktran;
        //DWord			AckCnt;
        //DWord			ErrCnt;

        //bool PortIsLAN;



      Byte          		Addr;
      CommSettings  		settings;
      Byte         			alarmIndex;
      //sync_deque<string> 	CmdMessages;

    protected:
      netsnmp  snmp_driver;
};
//=============================================================================
class Fp1: public Iibp
{
	public:
	    Fp1(Byte Ename, Byte adr);
	    ~Fp1();

        Word    CreateCMD(Byte CMD, Byte *Buffer);
        Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
        // sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
        // bool    ParsingAnswer( Byte *BUF, Word Len);
        string  GetStringValue(void);

    protected:

};
//=============================================================================
class Fp2: public Iibp
{
	public:
	  Fp2(Byte Ename, Byte adr);
	    ~Fp2();

        Word    CreateCMD(Byte CMD, Byte *Buffer);
        Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
        // sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
        // bool    ParsingAnswer( Byte *BUF, Word Len);
    protected:

};
//=============================================================================
class Emerson1: public Iibp
{
  	public:
	  Emerson1(Byte Ename, Byte adr);
  	 ~Emerson1();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);


      protected:

};
//=============================================================================
class Huawei: public Iibp
{
    	public:
    	Huawei(Byte Ename, Byte adr);
    	 ~Huawei();

            Word    CreateCMD(Byte CMD, Byte *Buffer);
            Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
            sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
            bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

        protected:

            Bshort ProtoVer;
};
//=============================================================================
class Huawei2: public Iibp
{
		public:
    	  Huawei2(Byte Ename, Byte adr);
    	  Huawei2(Byte Ename, Byte adr, string read, string write, string trap);
      	 ~Huawei2();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);

        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};
//=============================================================================
class HuaweiUPS: public Iibp
{
		public:
			HuaweiUPS(Byte Etype, Byte Ename, Byte adr);
			HuaweiUPS(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap);
			~HuaweiUPS();

		  string  GetStringValue(void);
          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);


          TinputVoltage      	Uout;
          TinputCurrent         Iin;
          TinputCurrent			Iout;

          float LoadPersent1;
          float LoadPersent2;
          float LoadPersent3;

        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};
//=============================================================================
class TsiBravoUPS: public Iibp
{
		public:
			TsiBravoUPS(Byte Etype, Byte Ename, Byte adr);
			TsiBravoUPS(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, SNMP_VER ver);
			~TsiBravoUPS();

		  string  GetStringValue(void);
          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);


          TinputVoltage      	Uout;
          TinputCurrent         Iin;
          TinputCurrent			Iout;

        protected:

          SNMP_VER snmp_ver;
          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};
//=============================================================================
class TsiBravoINV: public Iibp
{
		public:
			TsiBravoINV(Byte Etype, Byte Ename, Byte adr);
			TsiBravoINV(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, SNMP_VER ver);
			~TsiBravoINV();

		  string  GetStringValue(void);
          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);


          TinputVoltage      	Uout;
          TinputCurrent         Iin;
          TinputCurrent			Iout;

        protected:


          SNMP_VER snmp_ver;
          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};

//=============================================================================
class UniversalUPS: public Iibp
{
		public:
			UniversalUPS(Byte Etype, Byte Ename, Byte adr);
			UniversalUPS(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap);
			~UniversalUPS();

		  string  GetStringValue(void);
          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);


          TinputVoltage      	Uout;
          TinputVoltage      	Ubyp;
          //TinputCurrent			Ibyp;
          TinputCurrent         Iin;
          TinputCurrent			Iout;

          float LoadPersent1;
          float LoadPersent2;
          float LoadPersent3;

        protected:

          virtual string GetBatteryTemp(){
        	  return "1.3.6.1.2.1.33.1.2.7.0";
          }
          virtual string GetBatteryVoltage(){
			  return "1.3.6.1.2.1.33.1.2.5.0";
		  }
          virtual string GetBatteryCurrent(){
        	  return "1.3.6.1.2.1.33.1.2.6.0";
          }

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};
//=============================================================================
class LiebertUPS: public UniversalUPS
{
	public:
		LiebertUPS(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap);
		~LiebertUPS();

	protected:

		string GetBatteryTemp(){
		  return "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.4291";
		}
		string GetBatteryVoltage(){
		  return "1.3.6.1.4.1.476.1.42.3.5.2.4.1.4.1";
		}

		string GetBatteryCurrent(){
		  return "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.4149";
		}
};
//=============================================================================
class LiebertAPM: public Iibp
{
		public:
			LiebertAPM(Byte Etype, Byte Ename, Byte adr);
			LiebertAPM(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap);
			~LiebertAPM();

		  string  GetStringValue(void);
          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);


          TinputVoltage      	Uout;
          TinputVoltage      	Ubyp;
          //TinputCurrent			Ibyp;
          TinputCurrent         Iin;
          TinputCurrent			Iout;

          TinputCurrent 		LoadPersent;


        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};
//=============================================================================
class Emerson2: public Iibp
{
		public:
			Emerson2(Byte Ename, Byte adr);
			Emerson2(Byte Ename, Byte adr, string read, string write, string trap);
		   ~Emerson2();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);

        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};
//=============================================================================
class EnatelSM32: public Iibp
{
		public:
			EnatelSM32(Byte Ename, Byte adr);
			EnatelSM32(Byte Ename, Byte adr, string read, string write, string trap);
		   ~EnatelSM32();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);

        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};
//=============================================================================
class DeltaPsc3: public Iibp
{
		public:
			DeltaPsc3(Byte Ename, Byte adr);
			DeltaPsc3(Byte Ename, Byte adr, string read, string write, string trap);
		   ~DeltaPsc3();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);

          string  GetStringValue(void);


        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;
};
//=============================================================================
class DeltaPsc1000: public Iibp
{
    public:
		DeltaPsc1000(Byte Ename, Byte adr);
    	 ~DeltaPsc1000();

            Word    CreateCMD(Byte CMD, Byte *Buffer);
            Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
            sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
            bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);
            bool 	ChangeParameters( vector<string> parameters );
            string  GetStringValue(void);
    private:
            string  		GetParamValue(string param, Byte *BUF, Word Len );
            vector<string>  GetAlarmList(string start, string param, Byte *BUF, Word Len );
};
//=============================================================================
class LiFeAkb: public Iibp
{
    	public:
			LiFeAkb(Byte Ename, Byte adr);
			~LiFeAkb();

            Word    CreateCMD(Byte CMD, Byte *Buffer);
            Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
            sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
            bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);
            string  GetStringValue(void);

        protected:
            TDigitatAkbInfo Data;
            float Time;

            Bshort ProtoVer;
};
//=============================================================================
class CondLennox: public Iibp
{
		public:
			CondLennox(Byte Etype, Byte Ename, Byte adr);
			CondLennox(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, Byte typ);
			~CondLennox();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);

          string  GetStringValue(void);

        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;

          TMamStruct 		RoomTemp;//input
          TMamStruct 		RoomTempOut;
          TMamStruct 		ExtTemp;
          TMamStruct        Humidity;
          TMamStruct    	AirPressure;
};
//=============================================================================
class CondLiebert: public Iibp
{
		public:
			CondLiebert(Byte Etype, Byte Ename, Byte adr);
			CondLiebert(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, Byte typ);
			~CondLiebert();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);

          string  GetStringValue(void);

        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;

          TMamStruct 		AirTempSupply;//input
          TMamStruct 		AirTempReturn;
          //TMamStruct 		ExtTemp;

          TMamStruct        HumiditySupply;
          TMamStruct        HumidityReturn;
          //TMamStruct    	AirPressure;

          TMamStructWord FanSpeed;

};
//=============================================================================
class XmFast: public Iibp
{
		public:

			XmFast(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, Byte typ);
			~XmFast();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);

          string  GetStringValue(void);

        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;

          TMamStruct 		RoomTemp;//input
          TMamStruct 		RoomTempOut;
          TMamStruct 		FreeTemp;
          TMamStruct 		DischTempComp1;
          //TMamStruct 		DischTempComp2;
          TMamStruct 		ExtTemp;

          TMamStruct 		AnalogValue7;
          TMamStruct 		AnalogValue8;
          TMamStruct 		AnalogValue9;
          TMamStruct 		AnalogValue10;


          TMamStruct    	DischPressureComp1;
          //TMamStruct    	DischPressureComp2;

  	    //TVoltsAndCurrents  	UE2;
};
//=============================================================================
class XmFastComp2: public Iibp
{
		public:

			XmFastComp2(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, Byte typ);
			~XmFastComp2();

          Word    CreateCMD(Byte CMD, Byte *Buffer);
          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

          bool    ChangeState(Byte newState);
          bool    ChangeParameters(vector<string> parameters);

          string  GetStringValue(void);

        protected:

          string last_oid;

          string c_read;
          string c_write;
          string c_trap;

          //TMamStruct 		RoomTemp;//input
          //TMamStruct 		RoomTempOut;
          //TMamStruct 		FreeTemp;
          //TMamStruct 		DischTempComp1;
          TMamStruct 		DischTempComp2;
          //TMamStruct 		ExtTemp;

          TMamStruct 		AnalogValue7;
          TMamStruct 		AnalogValue8;
          TMamStruct 		AnalogValue9;
          TMamStruct 		AnalogValue10;


          //TMamStruct    	DischPressureComp1;
          TMamStruct    	DischPressureComp2;

  	    //TVoltsAndCurrents  	UE2;
};
//=============================================================================
class VertivPDU: public Iibp
{
		public:
			VertivPDU(Byte Etype, Byte Ename, Byte adr);
			VertivPDU(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap);
			~VertivPDU();

			  string  GetStringValue(void);
	          Word    CreateCMD(Byte CMD, Byte *Buffer);
	          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
	          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
	          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

	          bool    ChangeState(Byte newState);
	          bool    ChangeParameters(vector<string> parameters);

        protected:
	          TFaseEnergy   E;
	          TPower        P;
	          TSpower       S;
	          TCoefficient  KM;


	          TinputCurrent         Iin;
	          TinputCurrent			Iout;



			  string last_oid;
			  string c_read;
			  string c_write;
			  string c_trap;
};
//=============================================================================
class ApcPDU: public Iibp
{
		public:
			ApcPDU(Byte Etype, Byte Ename, Byte adr);
			ApcPDU(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap);
			~ApcPDU();

			  string  GetStringValue(void);
	          Word    CreateCMD(Byte CMD, Byte *Buffer);
	          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
	          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
	          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

	          bool    ChangeState(Byte newState);
	          bool    ChangeParameters(vector<string> parameters);

        protected:
	          TFaseEnergy   E;
	          TPower        P;
	          TSpower       S;
	          TCoefficient  KM;


	          //TinputCurrent         Iin;
	          TinputCurrent			Iout;
	          int Fases;
	          int Outlets;
	          int MaxCurrent;


			  string last_oid;
			  string c_read;
			  string c_write;
			  string c_trap;
};
//=============================================================================
class UniversalSnmpDevice: public IDevice
{
		public:
			UniversalSnmpDevice(Byte Etype, Byte Ename, Byte type);
			UniversalSnmpDevice(Byte Etype, Byte Ename, Byte type, SNMP_VER ver, string read, string write, string trap);
			~UniversalSnmpDevice();

	          Word    CreateCMD(Byte CMD, Byte *Buffer);
	          Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len);
	          sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
	          bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase);

	          bool    ChangeState(Byte newState);
	          bool    ChangeParameters(vector<string> parameters);

	          string  GetStringValue(void);
	          string  GetAdvStringValue(void);
	          string  GetStringCfgValue(void);
	          bool    DetectStates(void);
	          void 	  ClearValues(void);
	          void    InitGPIO(void);

        protected:

	          SNMP_VER snmp_ver;
			  string last_oid;
			  string c_read;
			  string c_write;
			  string c_trap;

			  vector<TSimpleDevice*> analizeCommands(vector<string> cmds);
			  netsnmp  snmp_driver;
};
//=============================================================================
class TIbpManager:public TFastTimer, public IManager
{
    public:
	  TIbpManager();
      ~TIbpManager();

      void Init( /*IPort *CurrCOM, */ void *config, void *f );

      void CreateCMD();
      void SendCMD();
      void RecvCMD();
      void ExecCMD();
      void DoECMD(void *Par = NULL);

      void DetectStates(void);
      void CreateMessageCMD(void *Par = NULL);

      void ChangeController(void)
      {
    	   int size = Controllers.size();
    	   if( size > 0)
    	    {
  	          if( ++ControllerIndex >=  Controllers.size())//progon po ostalnim t.k. dalee
  	        	  ControllerIndex = 0;
  	          IDevice *CurrController = GetSimple(ControllerIndex);//new
    	      if(CurrController != NULL)
    	      {
    	    	  int controllerPort = (int)CurrController->Port;
    	    	  int managerPort = (int)Port;
    	    	  Log::DEBUG( "[IbpManager] ChangeController controllerPort="+toString(controllerPort) + " managerPort="+toString(managerPort));
    	    	  if( Port != CurrController->Port )
    	    	  {
    	    		  if(Port != NULL)
    	    			  Port->Close();
    	    		  Port = CurrController->Port;//change
    	    		  Log::DEBUG("[TIbpManager] ControllerPort:"+ CurrController->Port->GetSettingsString());
    	    	  }
    	      }
    	    }
    	  	//SetTimer( COMMON_TIMER, 1000);
      		SubFase = IBP_INIT;
      		Fase    = CREATE_CMD;
      }

      void ChangeState(Word controllerIndex, Word newState);
      void ChangeParameters(Word controllerIndex, vector<string> parameters);


      bool      SetNewIcmd(vector<IbpCmdParameter> cmdList);

      bool 		Enable(void);

      Byte      GetFase();
      void      SetFase(Byte nfase);

      vector<IDevice*>   	  Controllers;
      bool                FirstInitFlg;
      IPort	*GetPort()
      {
    	  IPort*ret = NULL;
    	  pthread_mutex_lock(&sych);
    	  ret = Port;
    	  pthread_mutex_unlock(&sych);
    	  return ret;
      }

      string GetSelfTestData(int et, int en){
    	  string ret = "";

    	  for(auto curr: Controllers){
    		  if(curr->Et == et && curr->En==en)
    		  {
    			  ret += curr->GetStringCfgValue();
    			  /*
    			  ret += "{";
    			  ret += "\"Et\":"+toString((int)curr->Et)+",";
    			  ret += "\"En\":"+toString((int)curr->En)+",";
    			  ret += "\"Address\":"+toString((int)curr->Addr)+",";
    			  ret += "\"Number\":\""+curr->SiteInfo.SerialNo+"\""+",";
    			  ret += "\"Value\":"+toString((int)curr->UE.U.Value)+",";
    			  ret += "\"IsEnable\":"+toString(curr->IsEnable)+",";
    			  ret += "\"AckCnt\":"+toString((int)curr->AckCnt)+",";
    			  ret += "\"ErrCnt\":"+toString((int)curr->ErrCnt);
    			  ret+="},";*/
    		  }
    	  }
    	  return ret;
      }

      void Stop(void)
      {
    	  for(auto curr: Controllers)
    	  {
    		 if(curr!=NULL)
    			 curr->Port->Close();
    	  }
      }

    private:
      IPort 			 *Port;

      Byte                NewFase;
      Byte                Fase;

      IDevice* GetSimple( Word index );
      IDevice* GetControllerByEname( Byte ename );

      string ParseCmdString(string cmd){
    	  string CRLF = "[CRLF]";
    	  cmd = replaceAll(cmd, CRLF, "\r\n");
		return cmd;
      }

      TRtcTime            DateTime;
      Word                ControllerIndex;


      DWord           ModifyTime;
      TFifo<string> 	*fd;

      //sync_deque<TIbpCMD*> 	CMDMessages;
      //TIbpCMD          		*CurrentCMD;
      //TJournalData          tempJournalData;

      pthread_mutex_t 		sych;

      Byte Buffer[1000];
      Word DataLen;
      Byte SubFase;
      Word Period;
};
//=============================================================================


#endif /* IBP_H_ */

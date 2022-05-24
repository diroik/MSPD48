/*
 * Classes.h
 *
 *  Created on: Sep 23, 2014
 *      Author: user
 */

#ifndef CLASSES_H_
#define CLASSES_H_

#include <string>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <time.h>
#include <sys/time.h>
#include <functional>

#include <TypeDefine.h>
#include <convert.h>
#include <Log.h>
#include <TFifo.h>
#include <pthread.h>
#include <uci.h>

using namespace std;
using namespace log4net;
using namespace MyClassTamlates;

class TEt;
class IModbusRegisterList;
//class TSiteInfo;
//class ICmdController;
//=============================================================================
// TBuffer declaration
//====================================
	class TBuffer
	{
	public:
			TBuffer(Word Size)
			{
				DataLen = 0;
				MaxSize = Size;
				BUF = new Byte[Size];
			}
		   ~TBuffer()
			{
			  delete[] BUF;
			}

			Byte    *BUF;
			Word     MaxSize;
			sWord    DataLen;

			static Short  Crc(Byte const* pcBlock, sWord len);
			static Byte   Crc8(Byte const* pcBlock, sWord len);
			static void   XOR_Crypt(Byte* in, Word inlen, string pass);
			static void   ClrBUF(Byte *BUF, sWord Len);

			static string 	DateTimeToString(TRtcTime *datetime);
			static TRtcTime *StringToDateTime(string strdt);

			static Word   CopyString(Byte* DestStr, Byte* SrcStr);
			static Word   CopyBuffer(Byte* DestStr, Byte* SrcStr, Word Len);

			static bool   StrCmp(Byte const* Str1, Byte const* Str2);
			static void   			Split(string& InStr, string separator, vector<string>& results, int Len = -1);
			static vector<string> 	Split(string str, string separator);
			static vector<string> 	Split(string str, char sep);
			static void   			Split(Byte *BUFFER, int Size, string separator, vector<string>& results, int Len = -1);

			static int     find(TBuffer *BUFFER, string str);
			static int     find(Byte *BUFFER, Word Len, string str);
			static int     find_first_of(TBuffer *BUFFER, Byte simb);
			static int     find_first_of(Byte *BUFFER, Word Len, Byte simb);
			static int     find_last_of(TBuffer *BUFFER, Byte simb);
			static Word    clear_all_nulls(char *_ptr, Word _size);

			static string clear_start_end(string &str, char start, char end)
			{
				int si = str.find(start);
				int ei = str.find(end);
				if(si >= 0 && ei >=0)
				{
					si++;
					int len = ei-si;
					if(len >= 0)
						return str.substr(si, len);
					else
						return "";
				}
				else
					return "";
			}


	private:
	};
    //====================================
	class TCmdMessage
{
public:
  TCmdMessage():DateTime(NULL),NeedCmdAnswer(false),CMD("Data"), Addr(""),Password(""),State(" "),WithQuotes(true),ErrorCnt(0)
  {}

  string         CMD;
  string         Password;
  string         Addr;
  string         State;
  TRtcTime      *DateTime;
  Word 			 ErrorCnt;

  bool           NeedCmdAnswer;
  bool           WithQuotes;

  vector<string>  Data;
};
	//====================================
   class TMamStruct
   {
  public:
   TMamStruct()
    {
       Max	   = 0;
       Min     = 0;
       Aver    = 0;
       Value   = 0;
       Divider = 0;
    }
  ~TMamStruct(){}

   void CalcMinAverMax(float newVal)
   {
     if(Divider == 0)
     {
       Max = newVal;
       Min = newVal;
     }
     Divider++;
     if(newVal > Max){
    	 Max = newVal;
     }
     if(newVal < Min){
    	 Min = newVal;
     }
     Aver   = ( (float)((Aver*(Divider-1))  + newVal )) / ((float)Divider);
     Value  =  newVal;
   }
   void ClrDivider()////////////////////////
   {
     Divider  = 0;
   }

   void ClrValue()////////////////////////
   {
     Divider  = 0;
     Aver     = 0;
     Max      = 0;
     Min      = 0;
     Value    = 0;
   }

   float Max;
   float Min;
   float Aver;
   float Value;

   long  Divider;
  private:

   };
    //====================================
   class THistBoolParam
   {
   public:
	   THistBoolParam()
   		{
		   Value 	= false;
		   HValue 	= false;
		   filter 	= 3;
		   cnt 		= 0;
		   Enable   = true;
   		}

	   THistBoolParam(bool en):THistBoolParam()
   		{
		   Enable   = en;
   		}

       void SetValue(bool val)
       {
    	   if(val != Value)
    	   {
    		   if( cnt++ >= filter)
    		   {
        		   Value = val;
        		   cnt = 0;
    		   }
    	   }
    	   else{
    		   cnt = 0;
    	   }
       }

       bool Value;
       bool HValue;
       bool Enable;

       int filter;
       int cnt;
   private:


   };
   //====================================
   class THistFloatParam
  {
  public:
	   THistFloatParam()
  		{
		   Value  = 0.0;
		   HValue = 0.0;
  		}

  	float Value;
  	float HValue;
  };
   //====================================
   class THistShortParam
  {
  public:
	   THistShortParam()
  		{
		   Value  = 0;
		   HValue = 0;
  		}

	   short Value;
	   short HValue;
  };
  //====================================
	class THistStringParam
 {
 public:
	  THistStringParam()
 		{
		   Value  = "0.00";
		   HValue = "0.00";
 		}

 	string Value;
 	string HValue;
 };
   //====================================
	class TReturnValue
{
public:
	TReturnValue()
	{
		_flag = false;
		 //int st = pthread_mutex_init(&_sych, NULL);
		 //if(st != 0 ) Log::DEBUG("TReturnValue Error in pthread_mutex_init st="+toString(st));

	}
	 ~TReturnValue()
	 {
		 //int st = pthread_mutex_destroy(&_sych);
		 //if(st != 0 ) Log::DEBUG("TReturnValue Error in pthread_mutex_init st="+toString(st));
	 }

	 bool IsComplite(void)
	 {
		 bool ret;

		 //int st = pthread_mutex_lock(&_sych);
		 //if(st != 0 ) Log::DEBUG("TReturnValue Error in pthread_mutex_lock st="+toString(st));
		 ret = _flag;
		 //st = pthread_mutex_unlock(&_sych);
		 //if(st != 0 ) Log::DEBUG("TReturnValue Error in pthread_mutex_unlock st="+toString(st));
		 return _flag;
	 }

	 void IsComplite(bool val)
	 {
		 //int st = pthread_mutex_lock(&_sych);
		 //if(st != 0 ) Log::DEBUG("TReturnValue Error in pthread_mutex_lock st="+toString(st));
		 _flag = val;

   	  	 //st = pthread_mutex_unlock(&_sych);
   	  	 //if(st != 0 ) Log::DEBUG("TReturnValue Error in pthread_mutex_unlock st="+toString(st));
	 }


	 vector<string> Data;

private:

	 bool _flag;
	 //pthread_mutex_t _sych;

};
  /*class sync_MamStruct
  {
  public:
	sync_MamStruct()
    {
       Max=Min=Aver    = 0;
       Min     = 0;
       Aver    = 0;
       Value   = 0;
       Divider = 0;
    }
  ~sync_MamStruct(){}

   void CalcMinAverMax(float newVal)
   {
     if(Divider == 0)
     {
       this->Max = newVal;
       this->Min = newVal;
     }
     Divider++;
     if(newVal > this->Max)
     {  this->Max = newVal;}
     if(newVal < this->Min)
     {  this->Min = newVal;}

     this->Aver   = ( (float)((this->Aver*(Divider-1))  + newVal )) / ((float)Divider);
     this->Value  =  newVal;
   }
   void ClrDivider()////////////////////////
   {
     this->Divider  = 0;
   }

   void ClrValue()////////////////////////
   {
     this->Divider  = 0;
     this->Aver     = 0;
     this->Max      = 0;
     this->Min      = 0;
     this->Value    = 0;
   }

   sync_type<float> Max;
   sync_type<float> Min;
   sync_type<float> Aver;
   sync_type<float> Value;

   sync_type<long  Divider>;
  private:

  };*/
    //====================================
    class TMamStructWord
    {
	  public:
		TMamStructWord()
	    {
	       Max=Min=Aver    = 0;
	       Min     = 0;
	       Aver    = 0;
	       Value   = 0;
	       Divider = 0;
	    }
	  ~TMamStructWord(){}

	   void CalcMinAverMax(Word newVal)
	   {
	     if(Divider == 0)
	     {
	       this->Max = newVal;
	       this->Min = newVal;
	     }
	     Divider++;
	     if(newVal > this->Max)
	     {  this->Max = newVal;}
	     if(newVal < this->Min)
	     {  this->Min = newVal;}

	     this->Aver   = ( (Word)((this->Aver*(Divider-1))  + newVal )) / ((Word)Divider);
	     this->Value  =  newVal;
	   }
	   void ClrDivider()////////////////////////
	   {
	     this->Divider  = 0;
	   }

	   void ClrValue()////////////////////////
	   {
	     this->Divider  = 0;
	     this->Aver     = 0;
	     this->Max      = 0;
	     this->Min      = 0;
	     this->Value    = 0;
	   }

	   Word Max;
	   Word Min;
	   Word Aver;
	   Word Value;

	   long  Divider;
	  private:

   };
    //====================================
	class Word64
  {
  public:
    Word64()
    {
    	Value = 0;
    }
    ~Word64(){}
    inline void Inc()
    {
    	Value++;
    }

    inline void Inc(long add)
    {
    	Value+= add;
    }

    inline long long Compare(Word64 *VAL)// true VAL>=   false <
    {
      //return VAL->Value > this->Value;
    	return VAL->Value - this->Value;
    }
    inline Word64& operator=( const Word64 &newval )
    {
      Value = newval.Value;
      return *this;
    }


  //private:
    unsigned long long Value;

  };
	//====================================
	class TFastTimer
  {
  public:
    TFastTimer(Byte CNT, Word64* GLT)
    {
      TimerCNT=CNT;
      Timer = new Word64[TimerCNT];
      for(Word i = 0; i < TimerCNT; i++)
      {
        GlobalTimer = GLT;
        SetTimer(i,0);
      }
      int st = pthread_mutex_init(&sych, NULL);
	  if(st != 0 )
		  Log::DEBUG("TFastTimer Error in pthread_mutex_init st="+toString(st));
    }
    ~TFastTimer()
    {
    	int st = pthread_mutex_destroy(&sych);
		if(st != 0 ) Log::DEBUG("TFastTimer Error in pthread_mutex_destroy st="+toString(st));

    }
    bool inline  SetTimer(Byte TimNam, DWord Value)
    {
        bool state;
        pthread_mutex_lock(&sych);
        if (TimNam<TimerCNT)
        {
           //Timer[TimNam] = *GlobalTimer;
           Timer[TimNam].Value = GlobalTimer->Value + Value;
           state = true;
           //Log::DEBUG("add "+toString(Value));
        }
        else
        {  state = false;}
        pthread_mutex_unlock(&sych);
        return state;
    }
    long long inline GetTimValue(Byte TimNam) //0 - Timer
    {
    	long long Val;
        pthread_mutex_lock(&sych);
        if (TimNam < TimerCNT){
        	//Log::DEBUG("GlobalTimer "+toString(GlobalTimer->Value));
        	Val = GlobalTimer->Compare(&Timer[TimNam]);
        }
        else{
        	Val = 0;
        }
        pthread_mutex_unlock(&sych);
        return Val;
    }

  private:
    Word64* GlobalTimer;
    Word64* Timer;
    Byte   TimerCNT;
    pthread_mutex_t sych;
  };
	//====================================
	class TDateTime
  {
  public:
    TDateTime();
    TDateTime(TRtcTime time);
   ~TDateTime();

    void 		SyncDateTime(string &date, string &time);
    void 		SetDateTime(string &date, string &time);
    void 		SetSystemTimeOfDay(TRtcTime &dt);
    long 		GetTimeDiv(void);
    TRtcTime 	GetTime(void);

    void      	SetTime(TRtcTime dateTime);
    void      	SetTime(struct tm *dateTime);
    void      	IncSecond(void);
    DWord 		GetGlobalSeconds(void);
    //long long	GetFullGlobalSecond(void);
    void 		SetSynchAct(bool flg);
    bool 		GetSynchAct(void);
    DWord 		GetAbsTime(void);

    static struct tm	Now();
    static struct tm	NowUtc();
    //static Time RtcNow();

    TRtcTime    DateTime;

    string ToString();

    TDateTime AddMinutes(int cnt)
    {
    	TDateTime newDt;
    	newDt.SetTime(this->DateTime);

    	if(cnt > 0)
    		for(int i = 0; i < cnt; i++) newDt.IncMinute();
    	else
    		for(int i = 0; i < -1*cnt; i++) newDt.DecMinute();

    	return newDt;
    }

    TDateTime AddHours(int cnt)
    {
    	TDateTime newDt;
    	newDt.SetTime(this->DateTime);

    	if(cnt > 0)
    		for(int i = 0; i < cnt; i++)newDt.IncHour();
    	else
    		for(int i = 0; i < -1*cnt; i++)newDt.DecHour();

    	return newDt;
    }

  private:
    void 		IncMinute(void);
    void 		IncHour(void);
    void 		IncDay(void);
    void 		IncMonth(void);

    void      	DecSecond(void);
    void 		DecMinute(void);
    void 		DecHour(void);
    void 		DecDay(void);
    void 		DecMonth(void);

    bool 		IsLeapYear(Word Year);

    DWord 		GlobalSeconds;
    bool 		TimeSynchIsActual;

    pthread_mutex_t 	sych;
  };
	//====================================
	class TPt
  {
  public:
    TPt(int pType, int eName)
    {
       this->PType = pType;
       this->PName = eName;
    }
    int          PType;
    int          PName;
    vector<string>  Value;
  };
	//====================================
	class TEt
  {
  public:
    TEt(int eType, int eName)
    {
       EType = eType;
       EName = eName;
       AckCnt = 0;
       ErrCnt = 0;

    }
    int          EType;
    int          EName;
    vector<TPt>  Ptypes;

    DWord			AckCnt;
    DWord			ErrCnt;
  };
	//====================================
	class IPort
	{
	public:
		IPort(){
			mtu = 1492;
			//timeout = 100;
			isvirtual=false;
		}
		virtual ~IPort(){
			//Clear();
		}
		virtual void  Init(void *cfg) 									= 0;
		virtual bool  ConnectOK(void)									= 0;
		virtual bool  ConnectOK(int timeout)							= 0;
		virtual void  Reconnect(int timeout = 0)						= 0;
		virtual bool  IsConnected(void)									= 0;
		virtual sWord RecvTo(Byte *Buf, Word MaxLen, Byte StopSymbol) 	= 0;
		virtual sWord Recv(Byte *Buf, Word Len, Word MaxLen)			= 0;
		virtual sWord Recv(Byte *Buf, Word Len)							= 0;
		virtual sWord Send(Byte *Buf, Word Len) 						= 0;
		virtual bool Close(void) 										= 0;
		virtual string GetSettingsString(void) 							= 0;
		virtual int GetDescriptor(void) 								= 0;

		virtual int GetBytesInBuffer(void)								= 0;
		virtual string ToString(){
			return "";
		}
		virtual void SetVirtual(bool virt){
			isvirtual = virt;
		}
		virtual bool IsVirtual()
		{
			return isvirtual;
		}
		virtual void Clear()
		{
			int s = messagess.size();
			for(int i = 0; i < s; i++)
			{
				TBuffer *curr = messagess[i];
				if(curr != NULL)
					delete curr;
			}
			messagess.clear();
		}
		virtual bool PortIsLAN() 										= 0;

		sync_deque<TBuffer *>	messagess;
		int      mtu;

	protected:
		//long     timeout;
		bool isvirtual;
	};
	//====================================
	class IServer
	{
	public:
		 	 	 IServer ()
					{
		 	 		 ptr=NULL;
		 	 		 OnDataRecive=NULL;
		 	 		 mtu = 2048;
					}
		virtual ~IServer () {}
		virtual void  Init(void *cfg) 									= 0;
		virtual void  Start(void)										= 0;
		virtual void  Stop(void)										= 0;
		virtual void  Close(void)										= 0;
		virtual bool  IsRuning(void)									= 0;
		virtual bool  IsConnected(void)									= 0;

		virtual void  Clear()
		{
			int s = messagess.size();
			for(int i = 0; i < s; i++)
			{
				TBuffer *curr = messagess[i];
				if(curr != NULL)
					delete curr;
			}
			messagess.clear();
		}

		PFVptr  OnDataRecive;

		void*   ptr;
		int     mtu;

		sync_deque<TBuffer *>	messagess;

		//std::function< void*(void* p )> OnDataRecive;
	};
	//====================================
	class IRule
	{
	public:

		virtual ~IRule () {}

		virtual void  start(void)										= 0;
		virtual void  stop(void)										= 0;
		virtual bool  IsRuning(void)										= 0;
	};
	//====================================
	class IListener
	{
	public:

		virtual ~IListener () {}

		virtual void   Start(void *cfg=NULL)							= 0;
		virtual bool   Stop(void)										= 0;
		virtual IPort* AcceptSocket(void)								= 0;
		virtual bool   IsRuning(void)									= 0;
		virtual	string GetSettingsString(void)
		{
			return "";
		}
	};
	//====================================
	class ICmd
  {
  public:
     virtual          ~ICmd()  { };
     virtual ICmd*      GET_Message(void* p1 = 0,void* p2 = 0) = 0;
     virtual ICmd*      CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0) = 0;
     virtual ICmd*      CMD_Recv( void* p1,void* p2 )          = 0;
     virtual ICmd*      CMD_Send( void* p1,void* p2 )          = 0;

     virtual bool 		ConnectOK(void* p1 = 0) 			   = 0;
     virtual Byte       GetFase(void)                          = 0;
     virtual void       SetFase(Byte NewFase)                  = 0;
     virtual bool       NeedReset(void)                        = 0;
     virtual bool       NeedAnswer(void)                       = 0;

  protected:
     //ICmdController

  };
	//====================================
	class TGetVersion:public TFastTimer
	  {
	  public:

	     TGetVersion();
	    ~TGetVersion();

	    void Init(Word period);

	    void VER_DetectStates( void  );
	    void VER_CreateMessageCMD( void *Par );

	    bool          Modify;
	    bool          FirstInitFlg;
	    TRtcTime      DateTime;
	    bool          NeedFirstFlg;


	  protected:
	  private:
	    string GetSimNumber();
	    string GetInterfaceAddr(string name);

	    string simnumber;
	    string interfaceaddr1;
	    string interfaceaddr2;

        DWord           	ModifyTime;

	    Word           Period;


	  };
	//=======================================================
	class ICmdController
		{
			public:
				ICmdController(Word addr)
				{
					Addr = addr;
					AckCnt = 0;
					RcvCnt = 0;
				}
				Word  Addr;
				DWord AckCnt;
				DWord RcvCnt;

				map<Word, TEt*> Equipments;

				void GetEquipmentsData(vector<TEt> eqs)
				{
					for(auto curr : eqs)
					{
						int et = curr.EType;
						int en = curr.EName;

						Word key = et*100+en*10;
					    map<Word, TEt*>::iterator it = Equipments.find(key);
					    if( it == Equipments.end())
					    	Equipments[key] = new TEt(et, en);
					    Equipments[key]->Ptypes = curr.Ptypes;
					    /*
						Log::DEBUG("GetEquipmentsData from addr="+toString(Addr)+" et.size="+toString(Equipments.size())+
								" key="+toString(key)+ " pt.size="+toString(Equipments[key]->Ptypes.size())+ " RcvCnt="+toString(RcvCnt));*/
					}
				}
			private:
		};
	//=============================================================================
	class IManager
	{
		public:

			virtual ~IManager(){}

			virtual bool Enable(void) = 0;
			virtual string GetSelfTestData(int et, int en){
				return "";
			}
			virtual string GetSelfTestMonData(int addr, int et, int en){
				return "";
			}

		private:
	};
	//====================================
	class TGetCurrentDataTask:public TFastTimer, public IManager
	{
	public:
	  TGetCurrentDataTask();
	 ~TGetCurrentDataTask();

	  void Init(TaskListSettings ListConfig);
	  void FASE_PullingRS_485( void );

	  int  AddrCount( void );
	  DeviceOnLineSettings*  GetAddress( int ix );

      string GetSelfTestMonData(int addr, int et, int en){
    	  string ret = "";
    	  if(Controllers!=NULL)
    	  {
    		  map<Word, ICmdController*> modules = *Controllers;//Log::DEBUG("Controllers addr="+toString((long)&Controllers));
    		  Log::DEBUG("TGetCurrentDataTask GetSelfTestMonData modules.size="+toString(modules.size())+
    				  " addr="+toString(addr)+" program.addr="+toString((long)Controllers));

    		  map<Word, ICmdController*>::iterator it = modules.find(addr);
    		  if( it != modules.end())
    		  {
    			  //Log::DEBUG("modules.count(addr)");
    			  ICmdController *con = modules[addr];
    			  Word key = et*100+en*10;
    			  map<Word, TEt*>::iterator it2 = con->Equipments.find(key);
    			  if( it2 != con->Equipments.end())
    			  {
    				  Log::DEBUG("con->Equipments.count(key) key="+toString(key));
    				  TEt *eq =  con->Equipments[key];
        			  ret += "{";
        			  ret += "\"Et\":"+toString((int)et)+",";
        			  ret += "\"En\":"+toString((int)en)+",";
        			  ret += "\"Address\":"+toString((int)addr)+",";
        			  ret += "\"IsEnable\":"+toString(con->RcvCnt > 0)+",";
        			  ret += "\"AckCnt\":"+toString((int)con->RcvCnt)+",";
        			  ret += "\"DatchikList\":[";

        			  int i = 0;
    				  for(auto curr: eq->Ptypes){
            			  ret += "{";
    					  ret += "\"Pt\":"+toString((int)curr.PType)+",";
            			  ret += "\"Pn\":"+toString((int)curr.PName)+",";
            			  double summ = 0;
            			  for(auto cv :curr.Value){
            				  if(i++%3 == 0) summ +=atof(cv.c_str());
            			  }
            			  ret += "\"Value\":"+toString((int)(summ*10));
            			  ret+="},";
    				  }
        			  ret+="],";
        			  ret+="},";
    			  }
    		  }
    	  }
    	  return ret;
      }

      bool Enable( void )
      {
      	return AddrCount() > 0;
      }

      map<Word, ICmdController*> *Controllers;

	private:
	  //Byte                    	Fase;
	  sync_deque<TCmdMessage*> *MainMessages;
	  TaskListSettings  		Config;
	};
	//=============================================================================
	class TSimplEtype
	{
	  public:
	    TSimplEtype(Byte Et, Byte En):Etype(Et),Ename(En)
	    {}
	   ~TSimplEtype(){}

	    Byte Etype;
	    Byte Ename;
	};
	//=============================================================================
	class TSimplPtype
	{
	  public:
	    TSimplPtype(Byte Pt, Byte Pn):Ptype(Pt),Pname(Pn)
	    {}
	   ~TSimplPtype(){}

	    Byte Ptype;
	    Byte Pname;
	};
	//=============================================================================
	class IParams
	{
	  public:
	    IParams(Byte Pt, Byte Pn):Pt(Pt, Pn),Size(0)//,Period(0)
	    {}

	    virtual ~IParams()
	    {

	    }

	    string GetCfgString(void)
	    {
	      return string("\r\n>,"+
	                  WordToString(Pt.Ptype)+','+
	                  WordToString(Pt.Pname)+','+
	                  WordToString(Size));//+','+
	                  //WordToString(Period));
	    }
	    string GetValueString(void)
	    {
	      string ReturnStr = "\r\n>,"+
	                          WordToString(Pt.Ptype)+','+
	                          WordToString(Pt.Pname)+',';
	        for( int i=0; i < Params.size();i++)
	        {
	          TMamStruct *CurrMam = Params[i];
	          ReturnStr += FloatToString(CurrMam->Aver, 0, 2)+','+
	                       FloatToString(CurrMam->Min, 0, 2)+','+
	                       FloatToString(CurrMam->Max, 0, 2)+',';
	          CurrMam->ClrDivider();
	        }
	        return ReturnStr;
	    }

	    virtual void ClearValues()
	    {
	        for( int i=0; i < Params.size();i++)
	        {
	          TMamStruct *CurrMam = Params[i];
	          CurrMam->ClrValue();
	        }
	    }

	    TSimplPtype Pt;
	    Byte 		Size;

	    vector<TMamStruct*> Params;
	};
	//====================================
	class IDatchik: public IParams
	{
		public:
			IDatchik(Byte Pt, Byte Pn):IParams(Pt, Pn)
			{
				Register = 0;
				Index = 0;
				Type = 0;//2-discrete 3-analog
				Modify = false;
				StateValue = NO_RESULT;
				HStateValue = NO_RESULT;
				Enable = false;
				Visible = false;
			}

			virtual ~IDatchik()
			{}


			virtual string 				GetValueString(void) 				= 0;
			virtual string 				GetSimpleValueString(void) 		    = 0;
			virtual string 				GetTinyValueString(void) 			= 0;
			virtual string 				GetConfigString(void) 		        = 0;


			virtual DETECTING_RESULT  	DetectPorogs(void* ptrToValue) 		= 0;
			virtual void              	CalculateValue(void* newValptr) 	= 0;
			virtual void* 				GetValuePtr(void) = 0;

			virtual bool                IsWrong(void*  val)
			{
				return false;
			}

			virtual bool IsEnable()
			{
				return Enable;
			}


			virtual void  				InitGPIO()
			{

			}


			unsigned short				GetRegister(void){
				return Register + Index;
			}


			bool 						Visible;
			bool 						Enable;

			unsigned short Register;
			unsigned short Index;

			unsigned char  Type;
			bool		   Modify;

			DETECTING_RESULT 			StateValue;
			DETECTING_RESULT 			HStateValue;

		protected:

			  //int type;
	};
	//====================================
	class TAnalogDatchik: public IDatchik
	{
		public:
		TAnalogDatchik(Byte Pt, Byte Pn, bool enable=true, bool IsVisible=true);
		TAnalogDatchik(Byte Pt, Byte Pn, float max, float min, float multy, float def, float maxerr = 199.0, float minerr = -99.0, bool enable=true, bool IsVisible=true);

		  ~TAnalogDatchik();


		  string 			GetValueString(void);
		  string			GetSimpleValueString(void);
		  string 			GetTinyValueString(void);
		  string 			GetConfigString(void){
				string ret = "";
				if(Visible && Enable){
					ret += "{";
					  ret += "\"Pt\":"+toString((int)Pt.Ptype)+",";
					  ret += "\"Pn\":"+toString((int)Pt.Pname)+",";
					  ret += "\"Type\":"+string("analog")+",";
					  ret += "\"Register:\":"+toString((int)Register)+",";
					  ret += "\"Index:\":"+toString((int)Index);
					ret+="},";
				}
			    return ret;
		  }

		  DETECTING_RESULT  DetectPorogs(void* ptrToValue)
		  {
			  if(ptrToValue!= NULL)
			  {
				  sShort val = *((Word*)ptrToValue);
				  //Log::DEBUG("IDatchik::DetectPorogs(void* ptrToValue) val="+toString(val));
				  return DetectPorogs(val*Multyplier);
			  }
			  else
				  return NO_RESULT;
		  }

		  void              CalculateValue(void* newValptr)
		  {
			  if(newValptr!= NULL)
			  {
				  //Log::DEBUG("!!!!!!!!!!!newValptr="+toString(newValptr));
				  sShort val = *((Word*)newValptr);
				  //Log::DEBUG("CalculateValue val="+toString(val));
				  CalculateValue(val*Multyplier);
			  }
		  }

		  DETECTING_RESULT  DetectPorogs(float Value);
		  void              CalculateValue(float newVal);
		  void* 			GetValuePtr(void)
		  {
			  return (void*)&Value;
		  }
		  bool              IsWrong(void*  ptrToval);

		  //bool              IsEnable(void);


		private:

		  bool              IsWrong(float val);

		  float 					MinPorog;
		  float 					MaxPorog;
		  float						Multyplier;

		  float 					MinError;
		  float						MaxError;

		  float 					gisteresis;

		  TMamStruct                Value;

		  int 					    errCnt;
	};
	//====================================
	class TDiscreteDatchik: public IDatchik
	{
		public:
		TDiscreteDatchik(Byte Pt, Byte Pn, bool inversia, bool enable=true, bool IsVisible=true);

		  ~TDiscreteDatchik();

		  string 			GetValueString(void);
		  string			GetSimpleValueString(void);
		  string 			GetTinyValueString(void);
		  string 			GetConfigString(void){
				string ret = "";
				if(Visible && Enable){
					ret += "{";
					  ret += "\"Pt\":"+toString((int)Pt.Ptype)+",";
					  ret += "\"Pn\":"+toString((int)Pt.Pname)+",";
					  ret += "\"Type\":"+string("discrete")+",";
					  ret += "\"Register:\":"+toString((int)Register)+",";
					  ret += "\"Index:\":"+toString((int)Index);
					ret+="},";
				}
			    return ret;
		  }


		  DETECTING_RESULT  DetectPorogs(void* ptrToValue)
		  {
			  if(ptrToValue!= NULL){
				  Word val = *((Word*)ptrToValue);
				  return DetectPorogs( GetValue(val) );
			  }
			  else
				  return NO_RESULT;
		  }

		  void              CalculateValue(void* newValptr)
		  {
			  if(newValptr!= NULL){
				  Word val = *((Word*)newValptr);
				  CalculateValue( GetValue(val) );
			  }
		  }

		  DETECTING_RESULT  DetectPorogs(bool Value);
		  void              CalculateValue(bool newVal);
		  void* 			GetValuePtr(void)
		  {
			  return (void*)&Value;
		  }

		  //bool              IsEnable(void);

		  void  			InitGPIO()
		  {
				string sendStr = "(echo "+toString(Register)+" > /sys/class/gpio/export) >& /dev/null";
				sendToConsole(sendStr);
				sendStr = "echo in > /sys/class/gpio/gpio"+toString(Register)+"/direction";
				sendToConsole(sendStr);
		  }


		private:

		  bool 						GetValue(Word val)
		  {
			  return ((bool)((val >> Index)&1))^Inversia;
		  }

		  bool 						MinPorog;
		  bool 						MaxPorog;

		  bool						Inversia;
		  bool                		Value;

		  //int 						Index;
	};
	//====================================
	class TConstantDatchik: public IDatchik
	{
		public:

		TConstantDatchik(Byte Pt, Byte Pn, bool enable=true, bool IsVisible=true);
		TConstantDatchik(Byte Pt, Byte Pn, float multy, float maxerr = 32760.0, float minerr = -32760.0, bool enable=true, bool IsVisible=true);
		  ~TConstantDatchik();

		  string 			GetValueString(void);
		  string			GetSimpleValueString(void);
		  string 			GetTinyValueString(void);
		  string 			GetConfigString(void){
				string ret = "";
				if(Visible && Enable){
					ret += "{";
					  ret += "\"Pt\":"+toString((int)Pt.Ptype)+",";
					  ret += "\"Pn\":"+toString((int)Pt.Pname)+",";
					  ret += "\"Type\":"+string("constant")+",";
					  ret += "\"Register:\":"+toString((int)Register)+",";
					  ret += "\"Index:\":"+toString((int)Index);
					ret+="},";
				}
			    return ret;
		  }


		  DETECTING_RESULT  DetectPorogs(void* ptrToValue)
		  {
			  return NO_RESULT;
		  }
		  void              CalculateValue(void* newValptr)
		  {
			  if(newValptr!= NULL)
			  {
				  sShort val = *((Word*)newValptr);
				  CalculateValue(val*Multyplier);
			  }
		  }
		  void              CalculateValue(float newVal);

		  void* 			GetValuePtr(void)
		  {
			  return (void*)&Value;
		  }
		  bool              IsWrong(void*  ptrToval);

		  //bool              IsEnable(void);


		private:

		  bool              IsWrong(float val);

		  float 					MinError;
		  float						MaxError;
		  float						Multyplier;
		  float                		Value;

		  int 					    errCnt;
	};
	//=============================================================================
	class TSimpleDevice
	{
	public:
		TSimpleDevice(Byte et, Byte en):Et(et, en)
		{
			Address 		= 0;
			SerialNumber 	= "sd";
			IsEnable 		= false;
			cmd 			= NULL;
			Oid				= "";
		}

		string 	GetValueString(void)
		{
			string ReturnStr = "";
			if(DatchikList.size() > 0)
			{
				//string header = WordToString(Et.Etype)+','+ WordToString(Et.Ename)+',';
				string header = toString((int)Et.Etype)+','+ toString((int)Et.Ename)+',';
				string value = "";
				for(auto curr: DatchikList)
				{
					if(curr->Modify)
					{
						value += curr->GetValueString();
					}
				}
				if(!isNullOrWhiteSpace(value))
				{
					ReturnStr = header + value + WordToString(IsEnable)+",";
				}

			}
			return ReturnStr;
		}

		string 	GetConfigString(void)
		{
			string ReturnStr = "\r\n[";
			if(DatchikList.size() > 0)
			{
				ReturnStr = "\r\n{ \"Etype\":"+WordToString(Et.Etype)+','+ "\"Ename=\""+WordToString(Et.Ename)+','+"\"Address=\""+WordToString(Address)+',';
				ReturnStr += "\"DatchikList\":[";
				for(auto curr: DatchikList)
				{
					ReturnStr += curr->GetConfigString();
				}
				ReturnStr+="]}";
			}
			ReturnStr+="]";
			return ReturnStr;
		}

		TMamStruct GetMinMax()
		{
			TMamStruct ret;

			TMamStruct aver;
			TMamStruct min;
			TMamStruct max;
			for(auto curr: DatchikList)
			{
				if(curr->Type == 3 && curr->Enable)
				{
					TMamStruct curm =  *((TMamStruct*)curr->GetValuePtr());

/*
					Log::DEBUG("TSimpleDevice::GetMinMax Type:"+toString((int)curr->Type) +
							" Register:" +toString((int)curr->Register)+" Index:"+toString((int)curr->Index)+
							" Enable:"+toString((int)curr->Enable));

*/
					if(!curr->IsWrong( &curm.Aver ))
						aver.CalcMinAverMax(curm.Aver);

					if(!curr->IsWrong( &curm.Min ))
						min.CalcMinAverMax(curm.Min);

					if(!curr->IsWrong( &curm.Max ))
						max.CalcMinAverMax(curm.Max);
				}
			}
			ret.Aver = aver.Aver;
			ret.Max = max.Max;
			ret.Min = min.Min;
			ret.Divider = aver.Divider+max.Divider+min.Divider;

			return ret;
		}

		Word GetCnt()
		{
			Word cnt = 0;

			for(auto curr: DatchikList)
			{
				if(curr->Visible)
				{
					cnt++;
				}
			}
			return cnt;
		}

		Word GetErr()
		{
			Word err = 0;

			//for(auto curr: DatchikList)
			{
				if(!IsEnable)
				{
					err++;
				}
			}
			return err;
		}


	    void    SetModify(bool val)
	    {
		    for(auto curr: DatchikList){
		    	//Log::DEBUG("TSimpleDevice::SetModify val="+toString(val)+" Index="+toString(curr->Index) );
		    	curr->Modify = val;
		    }
	    }

	    bool 	GetModify(){
		    for(auto curr: DatchikList)
		    {
		    	if(curr->Modify){
		    		return true;
		    	}
		    }
			return false;
	    }

	    bool 	HasAlarm()
	    {
		    for(auto curr: DatchikList)
		    {
		    	if(curr->Modify){
		    		return true;
		    	}
		    }
			return false;
	    }

	    bool IsEnable;

		vector<IDatchik *>    DatchikList;

		TSimplEtype Et;

		Word Address;

		string Oid;

		string SerialNumber;

		IModbusRegisterList *cmd;

	private:

	};


	//=============================================================================
	class TFaseEnergy:public IParams
	{
			  public:

			  TFaseEnergy(Byte pt, Byte pn):IParams(pt,pn),Ea(0),Er(0),Ea_minus(0),Er_minus(0),Efa(0),Efb(0),Efc(0)
			  	  {
				  	  Size = 7;
				  }

				 ~TFaseEnergy(){}

				  string GetValueString(void)
				  {
					return string(">,"+
								  WordToString(Pt.Ptype, 0)+','+
								  WordToString(Pt.Pname, 0)+','+
								  FloatToString(Ea, 0, 3)+','+
								  FloatToString(Er, 0, 3)+','+
								  FloatToString(Ea_minus, 0, 3)+','+
							      FloatToString(Er_minus, 0, 3)+','+
								  FloatToString(Efa, 0, 3)+','+
								  FloatToString(Efb, 0, 3)+','+
							      FloatToString(Efc, 0, 3)+',');
				  }
				  string GetSimpleValueString(void)
				  {
					return string(">,"+
								  WordToString(Pt.Ptype, 0)+','+
								  WordToString(Pt.Pname, 0)+','+
								  FloatToString(Ea, 0, 3)+','+
								  FloatToString(Er, 0, 3)+',');
				  }

				  void ClearValues()
				  {
						 Ea 		= 0;
					     Ea_minus	= 0;
						 Er			= 0;
						 Er_minus	= 0;
						 Efa		= 0;
						 Efb		= 0;
						 Efc		= 0;
				  }


				 float Ea;
			     float Ea_minus;
				 float Er;
				 float Er_minus;
				 float Efa;
				 float Efb;
				 float Efc;
	};

	//=============================================================================
	class TVoltage:public IParams
	{
	      public:
	        TVoltage():IParams(42,1)
	        {
	          Params.push_back(&Ua);
	          Params.push_back(&Ub);
	          Params.push_back(&Uc);
	          Size = 3*3;
	        }

	        TVoltage(int pt, int pn):IParams(pt,pn)
	        {
	          Params.push_back(&Ua);
	          Params.push_back(&Ub);
	          Params.push_back(&Uc);
	          Size = 3*3;
	        }

	       ~TVoltage(){}
	        TMamStruct Ua;
	        TMamStruct Ub;
	        TMamStruct Uc;
	};
	//=============================================================================
	class TCurrent:public IParams
	{
	 	 public:
	        TCurrent():IParams(43,1)
	        {
	          Params.push_back(&Ia);
	          Params.push_back(&Ib);
	          Params.push_back(&Ic);
	          Size = 3*3;
	        }
	       ~TCurrent(){}
	        TMamStruct Ia;
	        TMamStruct Ib;
	        TMamStruct Ic;
	 };
	//=============================================================================
	class TPower:public IParams
	{
	 	 public:
			TPower():IParams(44,1)
	           {
	             Params.push_back(&P);
	             Params.push_back(&Pa);
	             Params.push_back(&Pb);
	             Params.push_back(&Pc);
	             Size = 3*4;
	           }
	        ~TPower(){}
	           TMamStruct P;
	           TMamStruct Pa;
	           TMamStruct Pb;
	           TMamStruct Pc;
	 };
	//=============================================================================
	class TRpower:public IParams
	{
	         public:
	           TRpower():IParams(45,1)
	           {
	             Params.push_back(&Q);
	             Params.push_back(&Qa);
	             Params.push_back(&Qb);
	             Params.push_back(&Qc);
	             Size = 3*4;
	           }
	          ~TRpower(){}
	           TMamStruct Q;
	           TMamStruct Qa;
	           TMamStruct Qb;
	           TMamStruct Qc;
	};
	//=============================================================================
	class TSpower:public IParams
	{
	         public:
	           TSpower():IParams(46,1)
	           {
	             Params.push_back(&S);
	             Params.push_back(&Sa);
	             Params.push_back(&Sb);
	             Params.push_back(&Sc);
	             Size = 3*4;
	           }
	          ~TSpower(){}
	           TMamStruct S;
	           TMamStruct Sa;
	           TMamStruct Sb;
	           TMamStruct Sc;
	};
	//=============================================================================
	class TCoefficient:public IParams
	{
	         public:
	           TCoefficient():IParams(47,1)
	           {
	             Params.push_back(&KM);
	             Params.push_back(&KMa);
	             Params.push_back(&KMb);
	             Params.push_back(&KMc);
	             Size = 3*4;
	           }
	          ~TCoefficient() {}
	           TMamStruct KM;
	           TMamStruct KMa;
	           TMamStruct KMb;
	           TMamStruct KMc;
	};
	//=============================================================================
	class TFrequency:public IParams
	{
		public:
			TFrequency():IParams(48,1)
	           {
	             Params.push_back(&F);
	             Size = 3*1;
	           }
	           TFrequency(int pt, int pn):IParams(pt, pn)
	           {
	             Params.push_back(&F);
	             Size = 3*1;
	           }

	          ~TFrequency() {}
	           TMamStruct    F;
	         };
	//=============================================================================
	class TIbpAlarms:public IParams
	{
	      public:
	  	  TIbpAlarms(Byte cnt):IParams(200, 1)
	      {
	  		  for(int i = 0; i < cnt; i++)
	  		  {
	  			  Alarms.push_back( new THistBoolParam() );
	  		  }
	  		  Size = Alarms.size();
	  		  CurrentIndex = 0;
	        }
	  	  TIbpAlarms(Byte cnt, Byte pt, Byte pn):IParams(pt, pn)
	  	  {
	  		  for(int i = 0; i < cnt; i++){
	  			  Alarms.push_back( new THistBoolParam() );
	  		  }
	  		  Size = Alarms.size();
	  		CurrentIndex = 0;
	        }
	       ~TIbpAlarms()
	       {
	    	   for(auto curr: Alarms){
	    		   if(curr != NULL)
	    			   delete curr;
	    	   }
	    	   Alarms.clear();
	       }

	       int size(){
	    	   return Alarms.size();
	       }

	       void clear(){
	    	   Alarms.clear();
	       }

	       THistBoolParam* operator[]( int ix ) const
	       {
	    	   return Alarms[ix];
	       }

	       string GetValueString(void)
	       {
	    	   if(Alarms.size() == 0)
	    		   return "";

	         string ReturnStr = "\r\n>,"+
	                             WordToString(Pt.Ptype)+','+
	                             WordToString(Pt.Pname)+',';
	         string al = "";
	         bool st = false;
	  		 for(auto curr: Alarms)
	  		 {
	  			 //st = curr;//if one is true st=true
	  			 if(curr->Value)
	  				 st = true;
	  			 al += WordToString(curr->Value)+',';
	  		 }
	  		 ReturnStr += WordToString(st)+",\r\n"+al;


	  		 if(TextAlarms.size() > 0)
	  		 {
		          ReturnStr += "\r\n>,"+
		                             WordToString(Pt.Ptype)+','+
		                             WordToString(Pt.Pname+1)+',';
	 	  		for(auto curr: TextAlarms)
	 	  		{
	 	  			ReturnStr += curr + ',';
	 	  		}
	  		 }
	         return ReturnStr;
	       }

	       int 						CurrentIndex;
	       vector<THistBoolParam*> Alarms;

	       vector<string>     TextAlarms;
	};
	//=============================================================================
	class TUnPtype:public IParams
	{
	    public:
	    	TUnPtype(Byte pt, Byte pn, Byte si):IParams(pt,pn)
	    	{
	    		for( int i=0; i < si;i++)
	    		{
	    			Params.push_back(new THistStringParam());
	    		}
	    	}

	    	~TUnPtype()
		       {
		    	   for(auto curr: Params){
		    		   if(curr != NULL)
		    			   delete curr;
		    	   }
		    	   Params.clear();
		       }

		    int size(){
		    	   return Params.size();
		    }

		    THistStringParam* operator[]( int ix ) const
		    {
		    	   return Params[ix];
		    }
	    	string GetValueString(void)
	    	{
	    		string ReturnStr = "\r\n>,"+
	    		                          WordToString(Pt.Ptype)+','+
	    		                          WordToString(Pt.Pname)+',';
	    		for( int i=0; i < Params.size();i++)
	    		 {
	    		    ReturnStr += Params[i]->Value+',';
	    		 }
	    		 return ReturnStr;

	    	}
	    	vector<THistStringParam* > Params;
	};
	//=============================================================================
	class TUfloatPtype:public IParams
	{
	    public:
	    	TUfloatPtype(Byte pt, Byte pn, Byte si):IParams(pt,pn)
	    	{
	    		for( int i=0; i < si;i++){
	    			Params.push_back(new THistFloatParam());
	    		}
	    		CurrentIndex = 0;
	    	}

	    	~TUfloatPtype()
		       {
		    	   for(auto curr: Params){
		    		   if(curr != NULL)
		    			   delete curr;
		    	   }
		    	   Params.clear();
		       }

		    int size(){
		    	   return Params.size();
		    }

		    THistFloatParam* operator[]( int ix ) const
		    {
		    	   return Params[ix];
		    }

	    	string GetValueString(void)
	    	{
	    		string ReturnStr = "";
	    		if(Params.size() > 0)
	    		{
	    			ReturnStr="\r\n>,"+WordToString(Pt.Ptype)+','+WordToString(Pt.Pname)+',';
					for( int i=0; i < Params.size();i++){
						ReturnStr += FloatToString( Params[i]->Value, 0, 1 )+',';
					 }
	    		}
	    		return ReturnStr;
	    	}

	    	int 						CurrentIndex;
	    	vector<THistFloatParam *> 	Params;
	};
	//=============================================================================
	class TULongPtype:public IParams
	{
	    public:
	    	TULongPtype(Byte pt, Byte pn, Byte si):IParams(pt,pn)
	    	{
	    		for( int i=0; i < si;i++)
	    		{
	    			Params.push_back(0);
	    		}
	    	}

	    	~TULongPtype()
	    	{
	    		Params.clear();
	    	}

	    	string GetValueString(void)
	    	{
	    		string ReturnStr = "\r\n>,"+WordToString(Pt.Ptype)+','+WordToString(Pt.Pname)+',';
	    		for( int i=0; i < Params.size();i++)
	    		{
	    		    ReturnStr += toString( Params[i] )+',';
	    		}
	    		return ReturnStr;
	    	}
	    	vector<unsigned long> Params;
	};
	//=============================================================================
	class TRectifier:public TSimplPtype
	{
		public:
			TRectifier(Byte Pt, Byte Pn):TSimplPtype(Pt, Pn)
			{
				Enable = true;
				Alarms = 0;
				Interface = false;
			}

	    	string GetValueString(void)
	    	{
	    		string ReturnStr = "\r\n>,"+WordToString(Ptype)+','+WordToString(Pname)+',';

	    		ReturnStr += toString(Enable)+','+toString(Alarms)+',';
		        TMamStruct *CurrMam = &I;
		        ReturnStr += FloatToString(CurrMam->Aver, 0, 2)+','+
		                       FloatToString(CurrMam->Min, 0, 2)+','+
		                       FloatToString(CurrMam->Max, 0, 2)+',';
		        CurrMam->ClrDivider();
	    		return ReturnStr;
	    	}

	    	bool Enable;
	    	bool Interface;
	    	Word Alarms;
	    	TMamStruct I;
	    	TMamStruct Uac;
	    	TMamStruct Udc;
	    	TMamStruct Temp;

	};
	//=============================================================================
	class TRectifiersInfo:public TSimplPtype
	{
		public:
			TRectifiersInfo(Byte Pt, Byte Pn):TSimplPtype(Pt, Pn)
			{
				All = 0;
				Alarm = 0;
				Normal = 0;
			}

	    	string GetValueString(void)
	    	{
	    		string ReturnStr = "\r\n>,"+WordToString(Ptype)+','+WordToString(Pname)+',';

	    		ReturnStr += toString(All)+','+toString(Alarm)+','+toString(Normal);
	    		return ReturnStr;
	    	}
	    	Word All;
	    	Word Alarm;
	    	Word Normal;
	};
	//=============================================================================
	class TSiteInfo:public IParams
	{
	      public:
	    	TSiteInfo():IParams(49,1)
	        {
	            Customer = "0";
	            Location = "0";
	            SerialNo = "0";
	            CU_No = "0";
	            SwVers = "0";
	            BattType = "0";
	            Size = 6;
	            //Blank = false;
	            ModulesNum = 0;
	            ModulesCap = 0;
	        }
	       ~TSiteInfo() {}


	        string GetValueString(void)
	        {
	        	string ret = "";
	        	//if(!Blank)
	        	{
					ret += "\r\n>,";
					ret += WordToString(Pt.Ptype)+',';
					ret += WordToString(Pt.Pname)+',';
					ret += Customer+',';
					ret += SerialNo+',';
					ret += CU_No+',';
					ret += Location+',';
					ret += SwVers+',';
					ret += BattType+',';
	        	}
	            return ret;
	        }
	        string Customer;
	        string Location;
	        string SerialNo;
	        string CU_No;
	        string SwVers;
	        string BattType;

	        int ModulesNum;
	        int ModulesCap;

	        //bool Blank;
	};
	//=============================================================================
	class SelfTestingSettings
	{
		public:
			int						udpPort;
			int						RecvTimeout;
			int						SendTimeout;
			vector<IManager*>  		managers;
			vector<IManager*>  		modmanagers;
		private:
	};
	//=============================================================================
	class IDevice
	{
		friend class TIbpManager;
		public:
			IDevice(Byte etype, Byte ename, Byte type)
			{
					Port 		= NULL;
					Et 			= etype;
					En 			= ename;
					Type 		= type;
					Modify 		= false;
					IsEnable 		= false;
					BetweenTimeout = 100;
					AckCnt 		= 0;
					ErrCnt 		= 0;
			}

			virtual ~IDevice()
			{
				  if(Port !=NULL)
				  {
					  Port->Close();
					  delete Port;
				  }
			}

			virtual  Word    CreateCMD(Byte CMD, Byte *Buffer)	 					= 0;
			virtual  Byte    GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)	= 0;
			virtual  sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen )		= 0;
			virtual  bool    ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)		= 0;

			virtual  bool    ChangeState(Byte newState)								= 0;
			virtual  bool    ChangeParameters(vector<string> parameters)			= 0;

			virtual  string  GetStringValue(void)									= 0;
			virtual  string  GetAdvStringValue(void)								= 0;
			virtual  string  GetStringCfgValue(void)								= 0;
			virtual  bool    DetectStates(void)										= 0;
			virtual void 	 ClearValues(void)										= 0;
			virtual void     InitGPIO(void)											= 0;

        	Byte          		Et;
        	Byte          		En;
        	Byte          		Type;
        	bool          		Modify;
        	bool          		IsEnable;
        	IPort          	   *Port;
			Word 				BetweenTimeout;
        	DWord				AckCnt;
        	DWord				ErrCnt;

        	TSiteInfo          	SiteInfo;

		protected:
			sync_deque<string> 	CmdMessages;
	};
	//=============================================================================
	  extern Data_Params          Configuration;
	  extern TDateTime			  SystemTime;
	  extern Word64			 	  SecCount;
	  extern Word64			      MilSecCount;
	  extern TFastTimer           ObjectFunctionsTimer;
	  extern TFifo<string>        FifoBuffer;
	  extern TGetVersion          GetVersionObject;
	  //=============================================================================


#endif /* CLASSES_H_ */

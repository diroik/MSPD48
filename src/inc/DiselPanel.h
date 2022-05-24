/*
 * DiselPanel.h
 *
 *  Created on: Nov 14, 2016
 *      Author: user
 */

#ifndef DISELPANEL_H_
#define DISELPANEL_H_
#include <CommPort.h>
#include <SocketPort.h>
#include <Classes.h>
#include <math.h>
#include <stdio.h>
#include <ModbusRTU.h>


using namespace std;
using namespace log4net;
using namespace MyClassTamlates;

//=============================================================================
class Ipanel
{
	friend class TDiselPanelManager;
	public:
		Ipanel(Byte Etype, Byte Ename, Byte adr);
		virtual ~Ipanel();
	      virtual string  GetStringValue(void);
	      virtual string  GetStringCfgValue(void);
	      virtual void    SetPeriod(Word Period);

	      virtual Word    CreateCMD(Byte CMD, Byte *Buffer) = 0;
	      virtual Byte    GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len) = 0;
	      virtual void    ManageData(void){
	    	  /*
				if(HState != State){
					HState = State;
					Modify = true;
				}

				if(HMode != Mode){
					HMode = Mode;
					Modify = true;
				}

				for(int i = 0; i < PointSteps.Params.size(); i++){
					if(PointSteps.Params[i] != HPointSteps.Params[i]){
						HPointSteps.Params[i] = PointSteps.Params[i];
						Modify = true;
					}
				}
				for(int i = 0; i < Alarms.Alarms.size(); i++){
					if(Alarms.Alarms[i] != HAlarms.Alarms[i]){
						HAlarms.Alarms[i] = Alarms.Alarms[i];
						Modify = true;
					}
				}

				for(int i = 0; i < Warnings.Alarms.size(); i++){
					if(Warnings.Alarms[i] != HWarnings.Alarms[i]){
						HWarnings.Alarms[i] = Warnings.Alarms[i];
						Modify = true;
					}
				}*/
			}

	      virtual sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	      virtual sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
	      virtual bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	      virtual bool    ChangeState(short newState);
	      virtual bool    ChangeParameters(vector<string> parameters);

	      virtual Word    GetState(Word state)
	      {
	  		Word ret = state;
	  		return ret;
	      }

	      virtual bool  NeedResetPeriod(void)
	      {
	    	  if(State == 0 || State == 1 || State == 4)
	    		  return false;
	    	  else
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
	        	for(auto curr: Warnings.Alarms){
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

	        	if(HState != State){
	        		HState = State;
	        		ret = true;
	        	}

				if(HMode != Mode){
					HMode = Mode;
					ret = true;
				}

				if(HEngMode != EngMode){
					HEngMode = EngMode;
					ret = true;
				}

	        	return ret;
	      }


	      Byte          Et;
	      Byte          En;
	      bool          Modify;
	      bool          IsEnable;

	      Byte 			Buffer[1000];
	      Word 			DataLen;

	      string 		SerialNo;
	      string 		ModelNo;

          TVoltage      U;
          TVoltage      Unet;

          TCurrent      I;
          TPower        P;
          TRpower       Q;
          TSpower       S;
          TCoefficient  KM;

          TFrequency    F;
          TFrequency    Fnet;

	      TMamStruct Vbatt;
	      TMamStruct Vgen;

	      TMamStruct OilPressure;
	      TMamStruct CoolTemp;
	      TMamStruct OilTemp;
	      TMamStruct OutTemp;
	      TMamStruct Fuel;
	      TMamStructWord RPM;
	      TSiteInfo  SiteInfo;


	      Word 		BetweenTimeout;
	      Word      NeedInitTimeout;

	     float		Ktran;

	      DWord		AckCnt;
	      DWord		ErrCnt;

	      //vector<ModbusRegCmd*> CmdList;


	  protected:

	      static void transportData(void *data, void *ptr);

	      bool NeedInit;
	      sync_deque<string> CmdMessages;

	      Byte 		Mode;
	      Byte 		HMode;

	      Byte 		EngMode;
	      Byte 		HEngMode;

	      Byte 		State;
	      Byte 		HState;



	    TIbpAlarms 	Alarms;
	    TIbpAlarms 	Warnings;
	  	TUfloatPtype PointSteps;
	  	TULongPtype  RunHours;


	    IPort        	*Port;
	    int			  	type;
	    Byte          	address;
	    TModbusManager 	*driver;
		CommSettings  	settings;

		Word modbus_cmd_size;

		//Byte last_subfase;
};
//=============================================================================
class Pw2Panel:public Ipanel
{
public:
	Pw2Panel(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
	~Pw2Panel();

	Word    CreateCMD(Byte CMD, Byte *Buffer);
	Byte 	GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
	sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
	bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	bool    ChangeParameters(vector<string> parameters);
	bool    ChangeState(short newState);
	string  GetStringValue(void);

	void    ManageData(void){
		/*
		if(HState != State){
			HState = State;
			this->Modify = true;
		}*/
	}

protected:
	//static void transportData(void *data, void *ptr);
};
//=============================================================================
class Dse7xxxPanel: public Ipanel
{
public:
	Dse7xxxPanel(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
	~Dse7xxxPanel();

	Word    CreateCMD(Byte CMD, Byte *Buffer);
	Byte 	GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
	sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
	bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	bool    ChangeParameters(vector<string> parameters);
	bool    ChangeState(short newState);

	bool    DetectStates(){

		bool ret = false;
		if(HAlarmCondition != AlarmCondition){
			HAlarmCondition = AlarmCondition;
			ret = true;
		}
		return Ipanel::DetectStates() || ret;
	}


	Word    GetState(Word state){
		Word ret = state;
		switch(state)
		{
		case 0:
			ret = 4;
			break;
		case 2:
			ret = 3;
			break;
		case 3:
			ret = 2;
			break;
		case 4:
			ret = 5;
			break;
		}
		return ret;
	}

	string  GetStringValue(void);

	void    ManageData(void){
		/*
		if(HState != State){
			HState = State;
			Modify = true;
		}
		if(HMode != Mode){
			HMode = Mode;
			Modify = true;
		}*/
	}

protected:
	//static void transportData(void *data, void *ptr);

	string AlarmCondition;
	string HAlarmCondition;

};
//=============================================================================
class Blc200Panel:public Ipanel
{
public:
	Blc200Panel(Byte Etype, Byte Ename, Byte adr);
	~Blc200Panel();

	Word    CreateCMD(Byte CMD, Byte *Buffer);
	Byte 	GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
	//void    ManageData(void);

	//sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase);
	bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	bool    ChangeParameters(vector<string> parameters);
	bool    ChangeState(short newState);
	string  GetStringValue(void);



protected:

	string CheckSum(Byte *BUF, Word Len)
	{
	    int ires = 0;

	    for (int i = 0; i < Len; i++)
	    {
	        ires += (int)BUF[i];
	        if (ires > 255) { ires -= 255; }
	    }

	    return IntToString(ires, 3);
	}

	string       GetDataFromMessage( Byte *BUF, Word Len ){
		string retVal;
		//Log::DEBUG("GetDataFromMessage="+string((char*)BUF, Len)+" Len="+toString(Len));
		int start = TBuffer::find_first_of( BUF, Len, 'A' );
		if(start >=0){
			BUF += start;
			Len -= start;
			start = TBuffer::find_first_of( BUF, Len-(start+1), 'D' );
			if(start >=0){
				BUF +=  start+1;
				Len -= (start+1);
				if(Len >= 5){
					retVal = string((char*)BUF, 5);
				}
			}
		}
		return retVal;
	}
};
//=============================================================================
class Telys2Panel:public Ipanel
{
public:
	Telys2Panel(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
	~Telys2Panel();

	Word    CreateCMD(Byte CMD, Byte *Buffer);
	Byte 	GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
	sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
	bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	bool    ChangeParameters(vector<string> parameters);
	bool    ChangeState(short newState);
	string  GetStringValue(void);

	void    ManageData(void){
		/*
		if(HState != State){
			HState = State;
			this->Modify = true;
		}*/
	}

protected:
	//static void transportData(void *data, void *ptr);
};
//=============================================================================
class VruTm251:public Ipanel
{
public:
	VruTm251(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
	~VruTm251();

	Word    CreateCMD(Byte CMD, Byte *Buffer);
	Byte 	GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
	sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
	bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	bool    ChangeParameters(vector<string> parameters);
	bool    ChangeState(short newState);
	string  GetStringValue(void);

	void    ManageData(void){
	}

protected:
	//static void transportData(void *data, void *ptr);

	THistShortParam *Data[40];
};
//=============================================================================
class Ats022:public Ipanel
{
public:
	Ats022(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
	~Ats022();

	Word    CreateCMD(Byte CMD, Byte *Buffer);
	Byte 	GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
	sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
	bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	bool    ChangeParameters(vector<string> parameters);
	bool    ChangeState(short newState);
	string  GetStringValue(void);

	void    ManageData(void){
	}

	bool    DetectStates(){
		bool ret = false;
		if(HLN1_status != LN1_status){
			HLN1_status = LN1_status;
			ret = true;
		}
		if(HLN2_status != LN2_status){
			HLN2_status = LN2_status;
			ret = true;
		}
		if(HSW_status != SW_status){
			HSW_status = SW_status;
			ret = true;
		}
		if(HGEN_status != GEN_status){
			HGEN_status = GEN_status;
			ret = true;
		}
		return Ipanel::DetectStates() || ret;
	}


protected:
    Byte 		LN1_status;
    Byte 		HLN1_status;

    Byte 		LN2_status;
    Byte 		HLN2_status;

    Byte 		SW_status;
    Byte 		HSW_status;

    Byte 		GEN_status;
    Byte 		HGEN_status;
};
//=============================================================================
class CVM:public Ipanel
{
public:
	CVM(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
	~CVM();

	Word    CreateCMD(Byte CMD, Byte *Buffer);
	Byte 	GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
	sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
	bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	bool    ChangeParameters(vector<string> parameters);
	bool    ChangeState(short newState);
	string  GetStringValue(void);

	void    ManageData(void){
	}

protected:
	TFaseEnergy       E;
};
//=============================================================================
class PM5xxx:public Ipanel
{
public:
	PM5xxx(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
	~PM5xxx();

	Word    CreateCMD(Byte CMD, Byte *Buffer);
	Byte 	GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
	sWord   SendData( IPort* Port, Byte *Buf, Word Len );
	sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
	bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
	bool    ChangeParameters(vector<string> parameters);
	bool    ChangeState(short newState);
	string  GetStringValue(void);

	void    ManageData(void){
	}

protected:
	TFaseEnergy       E;
};

//=============================================================================
class TDiselPanelManager:public TFastTimer, public IManager
{
public:
	TDiselPanelManager();
	~TDiselPanelManager();

    void Init( void *config, void *driver=NULL );

    void CreateCMD();
    void SendCMD();
    void RecvCMD();
    void ExecCMD();
    void DoECMD(void *Par = NULL);

    void DetectStates(void);
    void CreateMessageCMD(void *Par = NULL);

    bool Enable( void );
    IPort *GetPort();

    void ChangeState(Word controllerIndex, Word newState);
    void ChangeParameters(Word controllerIndex, vector<string> parameters);

    Byte                Fase;
    bool                FirstInitFlg;

    void ChangeController(void)
    {
  	   int size = Controllers.size();
  	   if( size > 0)
  	    {
	      if( ++ControllerIndex >=  Controllers.size()){
	    	  ControllerIndex = 0; }
  	      Ipanel *CurrController = GetSimple(ControllerIndex);//new
  	      if(CurrController != NULL){
  	    	  if( CurrController->Port != this->Port ){
  	    		  if(this->Port != NULL){
  	    			this->Port->Close();
  	    		  }
  	    		  Port = CurrController->Port;//change
  	    	  }
  	      }
  	    }
  	  	//SetTimer( COMMON_TIMER, 1000);
  	  	SubFase = 0;
    	Fase    = CREATE_CMD;
    }

    string GetSelfTestData(int et, int en){
  	  string ret = "";

  	  for(auto curr: Controllers){
		  if(curr->Et == et && curr->En == en)
		  {
			  ret += curr->GetStringCfgValue();
			  /*
			  ret += "{";
			  ret += "\"Et\":"+toString((int)curr->Et)+",";
			  ret += "\"En\":"+toString((int)curr->En)+",";
			  ret += "\"Data0\":"+toString((int)curr->State)+",";
			  ret += "\"Data1\":"+toString((int)curr->Mode)+",";
			  ret += "\"Data2\":"+toString((int)curr->RPM.Value)+",";
			  ret += "\"Data3\":"+toString((int)curr->Fuel.Value)+",";
			  ret += "\"Data4\":"+toString((int)curr->U.Ua.Value)+",";
			  ret += "\"Data5\":"+toString((int)curr->U.Ub.Value)+",";
			  ret += "\"Data6\":"+toString((int)curr->U.Uc.Value)+",";
			  ret += "\"Data7\":"+toString((int)curr->P.P.Value)+",";
			  ret += "\"Address\":"+toString((int)curr->address)+",";
			  ret += "\"Number\":\""+curr->SerialNo+"\""+",";
			  ret += "\"IsEnable\":"+toString(curr->IsEnable)+",";
			  ret += "\"AckCnt\":"+toString((int)curr->AckCnt)+",";
			  ret += "\"ErrCnt\":"+toString((int)curr->ErrCnt);
			  ret+="},";
			  */
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

    Ipanel *GetSimple( Word index );

    TRtcTime            DateTime;
    vector<Ipanel*> 	Controllers;
    Word                ControllerIndex;

    IPort *Port;
    Byte Buffer[1000];
    Word DataLen;
    Byte SubFase;

    Word Period;
    Word Aperiod;
    Word Operiod;

    DWord ModifyTime;
    pthread_mutex_t sych;
};




#endif /* DISELPANEL_H_ */

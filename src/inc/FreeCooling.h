/*
 * FreeCooling.h
 *
 *  Created on: Oct 25, 2014
 *      Author: user
 */

#ifndef FREECOOLING_H_
#define FREECOOLING_H_

#include <CommPort.h>
#include <Classes.h>
#include <math.h>
#include <stdio.h>
#include <ModbusRTU.h>


using namespace std;
using namespace log4net;
using namespace MyClassTamlates;
//=============================================================================
class TFreeCoolingManager;
//=============================================================================
class ScenarioStep
{
public:
	ScenarioStep(Byte face, Byte subface)
	{
		scrFace 	= face;
		scrSubFace = subface;
		ssFace = "";
		newValue = "";
		isEnterOK = false;
	}
	ScenarioStep(Byte face, Byte subface, string ss)
	{
		scrFace 	= face;
		scrSubFace = subface;
		ssFace = ss;
		newValue = "";
		isEnterOK = false;
	}
	ScenarioStep(Byte face, Byte subface, string ss, string nv)
	{
		scrFace 	= face;
		scrSubFace = subface;
		ssFace = ss;
		newValue = nv;
		isEnterOK = false;
	}

    Byte   scrFace;
    Byte   scrSubFace;

    bool   isEnterOK;

    string ssFace;
    string newValue;

    string GetName()
    {
    	return toString((int)scrFace)+"."+toString((int)scrSubFace);
    }

    string GetSmallName()
    {
    	char ch[2] {(char)(0x40+scrSubFace), 0};

    	return string(ch);
    }

    string GetFullName()
    {
    	//char ch = 0x40+scrSubFace;
    	char ch[2] {(char)(0x40+scrSubFace), 0};
    	return string(ch)+"."+ssFace;
    }

    vector<string> GetDataFromName()
	{
    	string fn = GetFullName();
    	//Log::DEBUG("["+fn+"]");
    	return TBuffer::Split(fn, ".");
	}
};
//=============================================================================
class Scenario
{
public:
	Scenario(string name)
	{
		Name 		= name;
		stepIndex 	= 0;
		IsComplited = false;
		IsAdditional = false;
		SetCmd       = 0;
		SetValue     = "";
	}
	~Scenario()
	{
		for(int i=0; i < Steps.size(); i++)
		{
			delete Steps[i];
		}
		Steps.clear();
	}


	string Name;
	vector<ScenarioStep*> Steps;
	ScenarioStep * GetCurrStep()
	{
    	if(stepIndex < Steps.size())
    		return Steps[stepIndex];
    	else
    		return NULL;
	}

	void IncStepIndex()
	{
    	if(stepIndex < Steps.size())
    	{
    		Steps[stepIndex]->isEnterOK = false;
    	}
		stepIndex++;
		//if(++stepIndex >= Steps.size())
		//	stepIndex = 0;
	}

	bool IsComplited;

	Word stepIndex;

	bool IsAdditional;

	Word SetCmd;
	string SetValue;
private:

};
//=============================================================================
class Ifree:public TFastTimer
{
	  friend class TFreeCoolingManager;
	public:

	  Ifree(Byte Ename, Byte adr);
	  Ifree(Byte Etype, Byte Ename, Byte adr);
	  Ifree(Byte Ename, Byte adr, TFreeCoolingManager* fp);

	  virtual ~Ifree();
      virtual string  GetStringValue(void);
      virtual string  GetStringCfgValue(void);
      virtual void    SetPeriod(Word Period);

      virtual Word    CreateCMD(Byte CMD, Byte *Buffer) = 0;
      virtual Byte    GetDataFromMessage(Byte *Buffer, Word Len, Byte &subFacePtr, void *data = NULL) = 0;
      virtual void    ManageData(void){

      }
      virtual void    ManageScenario(void) 	= 0;
      virtual void 	  Restart(void) 		= 0;
      virtual sWord   SendData( IPort* Port, Byte *Buf, Word Len );
      virtual sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );
      virtual bool    ParsingAnswer( Byte *BUF, Word Len);

      virtual bool    ChangeState(bool newState){
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

        	for(auto curr: Warnings.Alarms){
        		if(curr->HValue != curr->Value){
        			curr->HValue = curr->Value;
        			ret = true;
        		}
        	}

        	for(auto curr: PointsSteps.Params){
        		if(curr->HValue != curr->Value){
        			curr->HValue = curr->Value;
        			ret = true;
        		}
        	}

        	if(HState != State){
        		HState = State;
        		ret = true;
        	}
        	return ret;
      }

      TMamStruct 		RoomTemp;//input
      TMamStruct 		RoomTempOut;
      TMamStruct 		ExtTemp;
      TMamStruct 		AdvTemp;

      TMamStructWord    Humidity;
      TMamStruct    	AirPressure;

      TMamStruct	Vdc;
      TMamStruct	Vac;

      TMamStruct    If1;
      TMamStruct    If2;
//Power in Watt
      TMamStruct    Pf1;
      TMamStruct    Pf2;
// Speed in rpm
      TMamStructWord    Sf1;
      TMamStructWord    Sf2;

      TIbpAlarms 	Alarms;
      TIbpAlarms 	Warnings;
      TUnPtype      PointsSteps;
      TUnPtype      SetPoint;

      string 		SerialNo;
      string 		Customer;
  	  string		CU_No;

      Byte 			State;
      Byte 			HState;

      /////////////////////
      Byte 			AdvState;
      Byte 			HAdvState;
      //////////////////////

      Byte          Et;
      Byte          En;

      bool          Modify;
      bool          IsEnable;

      int			Type;
      DWord			AckCnt;
      DWord			ErrCnt;

	Byte Buffer[500];
	Word DataLen;

  protected:

    Byte          		Addr;

    CommSettings 		comm_settings;

    TFreeCoolingManager  *freemngptr;

	static void 		transportData(void *data, void *ptr);

	Word 				modbus_cmd_size;

};
//=============================================================================
class Envicool: public Ifree
{
	public:
		Envicool(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
		~Envicool();

		void Restart(void){
			Log::DEBUG("Envicool::Restart");
		}

		Word    CreateCMD(Byte CMD, Byte *Buffer){
			//Word DataLen = 1;
			//Log::DEBUG("Envicool::CreateCMD");
			//return DataLen;

			Word DataLen = 1;
			if(CMD > modbus_cmd_size  && CMD < 98){
				DataLen = 0;
			}
			return DataLen;
		}

		Byte    GetDataFromMessage(Byte *Buffer, Word Len, Byte &subFacePtr, void *data);

		void    ManageData(void){}

		sWord   SendData( IPort* Port, Byte *Buf, Word Len ){

			//Log::DEBUG("Envicool::SendData");
			return 0;
		}

		sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );

		bool    ParsingAnswer( Byte *BUF, Word Len){
			if(Len == 0) return true;
			return Len >= 4;
		}

		void    ManageScenario(void){

		}

	    bool    ChangeState(bool newState){
	    	  bool ret = false;
	    	  if(driver!=NULL){
	    		  driver->AddToDeviceList(address, 6, 0x901,  0, this, newState, MODBUS_RTU);
	    	  }
	    	  return ret;
	    }

	    bool    ChangeParameters(vector<string> parameters){
	    	  bool ret = false;
	    	  if(driver != NULL){
	    		  for(int i=0; i < parameters.size(); i++){
	    			  short newVal = (short)(atof( parameters[i].c_str() )*10);
	    			  Log::DEBUG("reg="+toString(i)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
		    		  driver->AddToDeviceList(address, 6, i,  0, this, newVal, MODBUS_RTU);
	    		  }
	    	  }
	    	  return ret;
	    }

	    bool    DetectStates();

		string GetStringValue(void);

		TULongPtype RunHours;

	protected:

		TUfloatPtype PointSteps;

		Byte 			address;
		TModbusManager 	*driver;
		CommSettings 	settings;

	private:
};
//=============================================================================
class EnvicoolHC: public Ifree
{
	public:
		EnvicoolHC(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
		~EnvicoolHC();

		void Restart(void){
			Log::DEBUG("EnvicoolHC::Restart");
		}

		Word    CreateCMD(Byte CMD, Byte *Buffer){
			Word DataLen = 1;
			if(CMD > modbus_cmd_size  && CMD < 98){
				DataLen = 0;
			}
			return DataLen;
		}

		Byte    GetDataFromMessage(Byte *Buffer, Word Len, Byte &subFacePtr, void *data);

		void    ManageData(void){}

		sWord   SendData( IPort* Port, Byte *Buf, Word Len ){

			//Log::DEBUG("Envicool::SendData");
			return 0;
		}

		sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );

		bool    ParsingAnswer( Byte *BUF, Word Len){
			if(Len == 0) return true;
			return Len >= 4;
		}

		void    ManageScenario(void){

		}

	    bool    ChangeState(bool newState)
	    {
	    	  bool ret = false;
	    	  if(driver!=NULL){
	    		  //driver->AddToDeviceList(address, 6, 0x202,  0, this, newState);
	    	  }
	    	  return ret;
	    }

	    bool    ChangeParameters(vector<string> parameters)
	    {
	    	  bool ret = false;
	    	  if(driver != NULL)
	    	  {
	    		  unsigned short start_reg = 10;
	    		  for(int i=0; i < parameters.size(); i++)
	    		  {
	    			  short newVal = (short)(atof( parameters[i].c_str() )*10);
	    			  unsigned short reg = start_reg + i*2;

	    			  Log::DEBUG("reg="+toString(reg)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
		    		  //driver->AddToDeviceList(address, 6, i,  0, newVal);
	    		  }
	    	  }
	    	  return ret;
	    }

	    bool    DetectStates();

		string GetStringValue(void);

		TULongPtype RunHours;


	      Byte 			IndoorState;
	      Byte 			HIndoorState;

	      Byte 			OutdoorState;
	      Byte 			HOutdoorState;

	      Byte 			CompressorState;
	      Byte 			HCompressorState;


	protected:

		TUfloatPtype PointSteps;

		Byte 			address;
		TModbusManager 	*driver;
		CommSettings 	settings;


	private:
};
//=============================================================================
class Enisey2000: public Ifree
{
	public:
		Enisey2000(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr);
		~Enisey2000();

		void Restart(void){
			Log::DEBUG("Enisey2000::Restart");
		}

		Word    CreateCMD(Byte CMD, Byte *Buffer){
			Word DataLen = 1;
			if(CMD > modbus_cmd_size  && CMD < 98){
				DataLen = 0;
			}
			return DataLen;
		}

		Byte    GetDataFromMessage(Byte *Buffer, Word Len, Byte &subFacePtr, void *data);

		void    ManageData(void){}

		sWord   SendData( IPort* Port, Byte *Buf, Word Len ){
			//Log::DEBUG("Enisey2000::SendData");
			return 0;
		}

		sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen );

		bool    ParsingAnswer( Byte *BUF, Word Len){
			if(Len == 0) return true;
			return Len >= 4;
		}

		void    ManageScenario(void){

		}

	    bool    ChangeState(bool newState){
	    	//Log::DEBUG("Enisey2000::ChangeState");
	    	  bool ret = false;
	    	  int ns = newState;// ? 0xFF00 : 0x0000;//set bit
	    	  //Log::DEBUG("Enisey2000::newState="+toString(ns));
	    	  if(driver!=NULL){
	    		  driver->AddToDeviceList(address, 5, 1, 0, this, ns, MODBUS_RTU);
	    	  }
	    	  return ret;
	    }

	    bool ChangeParameters(vector<string> parameters){
	    	 //Log::DEBUG("Enisey2000::ChangeParameters");
	    	  bool ret = false;
	    	  unsigned short start_reg = 1;
	    	  if(driver != NULL){
	    		  for(int i=0; i < parameters.size(); i++){
	    			  short newVal = (short)(atof( parameters[i].c_str() )*10);
	    			  unsigned short reg = start_reg+i;
	    			  //Log::DEBUG("Enisey2000::ChangeParameters reg="+toString(reg)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
		    		  driver->AddToDeviceList(address, 6, reg,  0, this, newVal, MODBUS_RTU);
	    		  }
	    	  }
	    	  return ret;
	    }

	    bool    DetectStates();

		string GetStringValue(void);

		TULongPtype RunHours;

	protected:

		TUfloatPtype PointSteps;

		Byte 			address;
		TModbusManager 	*driver;
		CommSettings 	settings;

	private:
};
//=============================================================================

class Shtultz: public Ifree
{
	public:
		Shtultz(Byte Ename, Byte adr);
		Shtultz(Byte Ename, Byte adr, TFreeCoolingManager* fp);
	    ~Shtultz();

        Word    CreateCMD(Byte CMD, Byte *Buffer);
        Byte    GetDataFromMessage(Byte *Buffer, Word Len, Byte &subFacePtr, void *data = NULL);
        void    SetScreen(Word value);
        void    ManageData(void);
        void    ManageScenario(void);
        string  GetStringValue(void);
        Blong   GetScreen(void);
        Byte    GetSubScreen(void);
        void 	Restart(void);
        void 	AddAdditionalScenario(Scenario* sc)
        {
        	AdditionalScenarioList.push_back(sc);
        	clearCurrScenario();
        	getAddScenario();
        }
        void  	clearCurrScenario()
        {
        	if(currentScenario != NULL)
        	{
        		delete currentScenario;
        		currentScenario = NULL;
        	}
        	else
        	{
        		screenNumber.Data_s 	= 0;
        		newScreenNumber.Data_s 	= 0;
        		ScenarioIndex 			= 0;
        		currentScenario 		= NULL;

        		for(int i =0; i < ScenarioList.size(); i++)
        		{
        			Scenario* scen = ScenarioList[i];
        			for(int j =0; j < scen->Steps.size(); j++)
        				scen->Steps[j]->isEnterOK = false;
        			scen->stepIndex 	= 0;
        			scen->IsComplited 	= false;
        			scen->IsAdditional 	= false;
        		}
        		/*
				if(ScenarioIndex < ScenarioList.size())
				{
					ScenarioList[ScenarioIndex]->stepIndex = 0;;
					//ScenarioIndex = 0;
				}
				ScenarioIndex = 0;
				*/
        	}
        }

        Bshort screenNumber;
        Bshort newScreenNumber;
        vector<Scenario*>   ScenarioList;

    protected:

        bool    GetDataFromScreen(void);
        bool    CompareScreens();
		pthread_mutex_t sych;
        vector<string>      Screen;
        vector<string>      OldScreen;
        vector<string>      addFace;
        Byte                scrFace;
        Byte                scrSubFace;
        Byte                hasWarn;
        Byte				hasAlarm;
        Byte                hScrSubFace;
        Word 				ScenarioIndex;
        string             ScreenValue;
        Scenario*          getCurrScenario()
        {
        	if(currentScenario != NULL)
        	{
        		return currentScenario;
        	}
        	else
        	{
				if(ScenarioIndex < ScenarioList.size())
					return ScenarioList[ScenarioIndex];
				else
					return NULL;
        	}
        }
        Scenario*          getAddScenario()
        {
        	if(currentScenario != NULL)
        	{
        		delete currentScenario;
        		currentScenario = NULL;
        	}
        	if(AdditionalScenarioList.size() > 0)
        	{
        		currentScenario = AdditionalScenarioList.front();
        		AdditionalScenarioList.pop_front();
        		currentScenario->IsAdditional = true;
        		Log::DEBUG("AdditionalScenario name="+currentScenario->Name);
        		return currentScenario;
        	}
        	return NULL;
        }
        Scenario* 				currentScenario;
        sync_deque<Scenario*>   AdditionalScenarioList;
    	void IncScenarioIndex()
    	{
        	if(currentScenario != NULL)
        	{
        		delete currentScenario;
        		currentScenario = NULL;
        	}
        	else
        	{
				if(++ScenarioIndex >= ScenarioList.size())
					ScenarioIndex = 0;
        	}
    	}
        void SetBack();
        void SetEnter();
        void SetUp();
        void SetDown();
        void SetPassword();
        void SwitchState();
};

//=============================================================================

//=============================================================================
class TFreeCoolingManager:public TFastTimer, public IManager
{
public:
	TFreeCoolingManager();
	~TFreeCoolingManager();

    void Init( IPort *CurrCOM, void *config, void *driver=NULL );

    void FREE_CreateCMD();
    void FREE_SendCMD();
    void FREE_RecvCMD();
    void FREE_ExecCMD();
    void FREE_DoECMD();

    void FREE_DetectStates(void);
    void FREE_CreateMessageCMD(void *Par = NULL);

    void FREE_Manage();
    void FREE_Restart(void);

    //void SetScreen(Word value);
    //Word GetScreen();

    //void AddScenario(string name, Byte face, Byte sface, string strFace, string nv, Word cmd);
    void EditScenario(string name, Byte face, Byte sface, string strFace, string nv, bool clear=false);
    //void AddPasswordScenario();

    void ChangeState(Word newState);
    void ChangeParameters(vector<string> parameters);

    Byte                Fase;
    bool                FirstInitFlg;

    bool 				Enable()
    {
    	return Controller != NULL;
    }
    bool                NeedRestart;

    string GetSelfTestData(int et, int en){
  	  string ret = "";
  	  Ifree* curr = Controller;
  	  if(curr != NULL){
		  if(curr->Et == et && curr->En==en)
		  {
			  ret += "{";
			  ret += "\"Et\":"+toString((int)curr->Et)+",";
			  ret += "\"En\":"+toString((int)curr->En)+",";
			  ret += "\"Address\":"+toString((int)curr->Addr)+",";
			  ret += "\"Number\":\""+curr->SerialNo+"\""+",";
			  ret += "\"IsEnable\":"+toString(curr->IsEnable)+",";
			  ret += "\"AckCnt\":"+toString((int)curr->AckCnt)+",";
			  ret += "\"ErrCnt\":"+toString((int)curr->ErrCnt);
			  ret+="},";
		  }
  	  }
  	  return ret;
    }

    void Stop(void)
    {
    	if(Port != NULL)
    		Port->Close();
    }

  private:

    TRtcTime            DateTime;
    Ifree*   			Controller;
    ComPort *Port;
    Byte Buffer[500];
    Word DataLen;
    Byte SubFase;
    Word Period;
    DWord ModifyTime;
};




#endif /* FREECOOLING_H_ */

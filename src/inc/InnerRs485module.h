/*
 * InnerRs485module.h
 *
 *  Created on: Mar 15, 2017
 *      Author: user
 */

#ifndef INNERRS485MODULE_H_
#define INNERRS485MODULE_H_

#include <CommPort.h>
#include <Classes.h>
#include <math.h>
#include <stdio.h>
#include <SocketPort.h>
#include <ModbusRTU.h>


using namespace std;
using namespace log4net;
using namespace MyClassTamlates;

	//=============================================================================
	class Imodul
	{
		friend class TInnerRs485Manager;
		public:
			Imodul(Byte et, Byte en);
			virtual ~Imodul();
		      virtual vector<string>  	GetStringValue(void);
		      virtual string  			GetStringCfgValue(void);
		      virtual void    			SetPeriod(Word Period);
		      virtual Word 				GetPeriod(void);

		      virtual Word    CreateCMD(Byte CMD, Byte *Buffer) = 0;
		      virtual Byte    GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len) = 0;
		      virtual void    ManageData(void) = 0;

		      virtual void    InitGPIO();
		      virtual sWord   SendData( IPort* Port, Byte *Buf, Word Len );
		      virtual sWord   RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase );
		      virtual bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
		      virtual bool    ChangeParameter( Word reg, Word fun, string val );
		      virtual bool    DetectStates(void);

			  string  			GetDatchikValue(Word et, Word en, Word pt, Word pn);
			  TSimpleDevice 	*GetDevice(Word et, Word en)
			  {
				  TSimpleDevice *ret = NULL;
					for(auto dev: Devices)
					{
						if(dev->Et.Etype == et && dev->Et.Ename)
						{
							ret = dev;
						}
					}
					return ret;
			  }


		      Byte    GetLastCmd()
		      {
		    	  return Devices.size() > 0 ? Devices.size()-1 : 0;
		      }

		      void    SetModify(bool val)
			  {
			    for(auto curr: Devices)
			    	curr->SetModify(val);
			    for(auto curr: ConfigDevices)
			    	curr->SetModify(val);

			    if(val == true)
			    {
			    	DWord tim = SystemTime.GetGlobalSeconds();
		    		modifyTime = ((DWord)( (DWord)(tim / period) + 1)) * period;
		    		if(modifyTime > MAX_SECOND_IN_DAY) modifyTime = period;
			    }

			  }
		      void    SetModify(Byte CMD, bool val)
			  {
			    if(CMD >= 0 && CMD < Devices.size())
			    {
			    	Devices[CMD]->SetModify(val);
			    }
			  }
			  bool 	  GetModify()
			  {
			    for(auto curr: Devices)
			    {
			    	if(curr->GetModify())
			    		return true;
			    }
			    /*// need inspect
			    for(auto curr: ConfigDevices)
			    {
			    	if(curr->GetModify())
			    		return true;
			    }*/

			    return false;
			  }

			  bool 	  HasAlarm()
			  {
			    return hasalarm;
			  }

		      bool GetIsEnable(Byte CMD)
		      {
		    	bool ret = false;
		    	if(CMD >= 0 && CMD < Devices.size())
		    	{
		    		ret = Devices[CMD]->IsEnable;
		    		//Log::DEBUG("TInnerRs485Manager::GetIsEnable="+toString((int)ret)+" CMD="+toString((int)CMD));
		    	}
		    	return ret;
		      }
		      void SetIsEnable(Byte CMD, bool value)
		      {
			    if(CMD >= 0 && CMD < Devices.size())
			    {
			    	Devices[CMD]->IsEnable = value;
			    	//Log::DEBUG("TInnerRs485Manager::SetIsEnable="+toString((int)Devices[CMD]->IsEnable)+" CMD="+toString((int)CMD));
			    }
		      }

		      Byte          Et;
		      Byte          En;

		      Word 			BetweenTimeout;

		      DWord			AckCnt;
		      DWord			ErrCnt;


		      Word 			MaxErrors;
		      Word 			ResetPort;
		      Word 			SelValue;
		      Word 			SelTimeout;
		      string 		SelPorts;

		      IModbusRegisterList *CurrentCmd;

		  protected:
		      //vector<string>  configs;

		      vector<TSimpleDevice*> Devices;
		      vector<TSimpleDevice*> ConfigDevices;

		      Word 			period;
		      DWord         modifyTime;

		      bool          modify;
		      bool			hasalarm;
		      int		    type;
		      IPort        *Port;
		      //CommSettings  settings;

		      void InitDevices(vector<string> cmds);
		      void InitConfigs(vector<string> cfgs);

		      vector<TSimpleDevice*> analizeCommands(vector<string> cmds);
		      IModbusRegisterList 	*GetCmdList(int addr, int func, int reg, int len, short newData=0 )
		      {
					if(func == 1)
						return new ModbusCoilStatusRegistersList( addr, reg, len, NULL) ;
					else if(func == 2)
						return new ModbusDiscreteInputsList( addr, reg, len, NULL );
					else if(func == 3)
						return new ModbusHoldingRegistersList( addr, reg, len, NULL );
					else if(func == 4)
						return new ModbusInputRegistersList( addr, reg, len, NULL );
					else if(func == 6)
						return new ModbusWriteSingleRegister( addr, reg, newData, NULL );
					else
						return (IModbusRegisterList*)NULL;
				}
	};

	//=============================================================================
	class ModbusAscii:public Imodul
	{
		public:
			ModbusAscii(Byte et, Byte en, vector<string> cmds, vector<string> cfg);
			~ModbusAscii();

			Word    		CreateCMD(Byte CMD, Byte *Buffer);
			Byte 			GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
			void    		ManageData(void);
			sWord   		RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase);
			bool    		ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
			//vector<string>  GetStringValue(void);
			bool    		ChangeParameter( Word reg, Word fun, string val );

		protected:

			Byte 		CheckSum(Byte *BUF, Word Len)
			{
			    int ires = 0;
			    //Log::DEBUG("CheckSum["+string((char*)BUF, Len) +"] Len="+toString(Len));
			    for (int i = 0; i < Len; i+=2){
			        ires += HexToByte(&BUF[i]);
			    }
			    return (0x100-ires);
			}
			string 		GetDataFromMessage( Byte *BUF, Word Len ){
				string retVal = string((char*)BUF, Len);
				//Log::DEBUG("GetDataFromMessage befor=["+retVal+"]");
				retVal = TBuffer::clear_start_end(retVal, ':', '\r' );
				//Log::DEBUG("GetDataFromMessage after=["+retVal+"]");
				return retVal;
			}
	};

	//=============================================================================
	class ModbusRtu:public Imodul
	{
	public:
		ModbusRtu(Byte et, Byte en, vector<string> cmds, vector<string> cfg);
		~ModbusRtu();

		Word    		CreateCMD(Byte CMD, Byte *Buffer);
		Byte 			GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
		void    		ManageData(void);
		sWord   		RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase);
		bool    		ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
		//vector<string>  GetStringValue(void);

	protected:

		Word 		CheckSum(Byte *BUF, Word Len)
		{
			return Crc16(BUF, Len);
		}
	};
	//=============================================================================
	class ModbusTcp:public Imodul
	{
		public:
			ModbusTcp(Byte et, Byte en, vector<string> cmds, vector<string> cfg);
			~ModbusTcp();

			Word    		CreateCMD(Byte CMD, Byte *Buffer);
			Byte 			GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len);
			void    		ManageData(void);
			sWord   		RecvData( IPort* Port, Byte *Buf, Word MaxLen, Byte subfase);
			bool    		ParsingAnswer( Byte *BUF, Word Len, Byte subFase);
			//bool    		ChangeParameter( Word reg, Word fun, string val );

		protected:

			Bshort			TransactionId;
			Bshort			ProtocolId;
			Bshort 			Len;
	};


	//=============================================================================
	class TInnerRs485Manager:public TFastTimer, public IManager
	{
	public:
		TInnerRs485Manager();
		~TInnerRs485Manager();

	    void Init( void *config, void *driver = NULL );
	    void CreateCMD();
	    void SendCMD();
	    void RecvCMD();
	    void ExecCMD();

	    void DetectStates(void);
	    void CreateMessageCMD(void *Par = NULL);
	    void ChangeParameters(Word et, Word en, vector<tuple<int, int,string>> parameters);
	    void ChangeController(void);

	    bool Enable( void );
	    IPort *GetPort();

	    Byte                Fase;
	    bool                FirstInitFlg;

        string GetSelfTestData(int et, int en)
        {
        	  string ret = "";
        	  for(auto curr: Controllers)
        	  {
        		  for(auto cm: curr->Devices)
        		  {
        			  if(cm->Et.Etype == et && cm->Et.Ename==en)
        			  {
        				  ret += "{";
        				  ret += "\"Et\":"+toString((int)cm->Et.Etype)+",";
        				  ret += "\"En\":"+toString((int)cm->Et.Ename)+",";
        				  ret += "\"Address\":"+toString((int)cm->Address)+",";
        				  ret += "\"Number\":\""+cm->SerialNumber+"\""+",";
        				  ret += "\"IsEnable\":"+toString(cm->IsEnable)+",";
        				  ret += "\"AckCnt\":"+toString((int)curr->AckCnt)+",";
        				  ret += "\"ErrCnt\":"+toString((int)curr->ErrCnt)+",";
        				  ret += "\"DatchikList\":[";

        				  for(auto dat: cm->DatchikList){
        					  ret+= dat->GetSimpleValueString();
        				  }

						  if(ret.c_str()[ret.size()-1]==',')
							ret=ret.substr(0,ret.size()-1);
        				  ret += "]";
        				  ret+="},";
        			  }
        		  }
				  /*
				  ret += "{"+
      					  toString((int)cm->Et.Etype )+":"+
      					  toString((int)cm->Et.Ename)+":"+
      					  toString((int)cm->Address)+":"+
      					  cm->SerialNumber+":"+
      					  toString(curr->IsEnable)+":"+
      					  toString(curr->AckCnt)+":"+
      					  toString(curr->ErrCnt)+":";
				  for(auto dat: cm->DatchikList){
					  ret+= toString((int)dat->Pt.Ptype)+":";
					  ret+= toString((int)dat->Pt.Pname)+":";
					  ret+= dat->GetSimpleValueString()+":";
				  }
				  ret+="},";

				  */
        	  }
      	  return ret;
        }

	  private:

	    Imodul *GetSimple( Word index );


	    DWord               ModifyTime;
	    TRtcTime            DateTime;
	    vector<Imodul*>     Controllers;
	    Word                ControllerIndex;

	    IPort *Port;
	    Byte Buffer[500];
	    sWord DataLen;
	    Byte SubFase;
	    Word Period;
	    Word Aperiod;

	    pthread_mutex_t sych;
	};
	//=============================================================================


#endif /* INNERRS485MODULE_H_ */

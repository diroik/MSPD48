/*
 * CmdMessages.h
 *
 *  Created on: Oct 3, 2014
 *      Author: user
 */

#ifndef CMDMESSAGES_H_
#define CMDMESSAGES_H_

#include <tuple>
#include <Classes.h>
#include <Ecounter.h>
#include <Ibp.h>
#include <FreeCooling.h>
#include <DiselPanel.h>
#include <InnerRs485module.h>
#include <ModbusRTU.h>
#include <Climat.h>
#include <InputOutputManager.h>
//=======================================================
	extern TEnergyCounterManager EnergyCounterManager;
	extern TIbpManager 			 IbpManager;
	extern TFreeCoolingManager   FreeCoolingManager;
	extern TMkkClimatManager     MkkClimatManager;
	extern TDiselPanelManager    DiselPanelManager;
	extern TModbusManager        ModbusManager1;
	extern TModbusManager        ModbusManager2;
	extern TExternClimatManager  ExternClimatManager;
	extern TInnerRs485Manager    InnerRs485Manager;
	extern TIOManager    		 IOManager;

//====================================
	class TCmd_msg:public ICmd
	{
	public:
			   TCmd_msg():Fase(WAIT_MES), CalcCRC(0)
			   {
				 BaseCMD         = this;
				 Addr            = "0";
				 ID              = "0";
				 CmdText         = "";
				_needReset = _needAnswer = false;
				 LastErrorIndex  = 0;
				_errorCount      = 0;
				 MainMessages               = NULL;
				 //AlternateMessages          = NULL;
				 PasswordPtr                = "";
				 AddrPtr                    = "";
				 ReloadConfig   = NULL;
				_currNeedCmdAnswerMessage   = NULL;
				 OnAnswerErrorExecute       = NULL;
				 OnGetDataFrom485       = NULL;
				 OnGetDataFrom485_1     = NULL;
				 OnGetDataFrom485_2     = NULL;
				 OnGetDataFrom485_3     = NULL;
				 OnGetDataFrom485_4     = NULL;
				 REPEAT_COUNT 			= 10;
				 MAX_ERRORS_TO_RESET 	= 30;//
			   }
			  ~TCmd_msg()  {
				  for(auto curr: Controllers){
					  if(curr.second != NULL){
						  delete curr.second;
					  }
				  }
			  }

			  ICmd*              GET_Message(void* p1 = 0,void* p2 = 0);
			  ICmd*              CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);
			  ICmd*              CMD_Recv( void* p1,void* p2 );
			  ICmd*              CMD_Send( void* p1,void* p2 );

	bool       		   	    ConnectOK(void* p1 = 0);
	void                    AddToCMD_Messages(TCmd_msg* msg);
	TCmd_msg*        		GetCMDbyKey(string key);
	Byte                    GetFase(void);
	void                    SetFase(Byte NewFase);
	bool                    NeedReset(void);
	bool                    NeedAnswer(void);
	bool                    GetBroadcastAddr(void);
	void                    SendRESET(void* p1 = 0,void* p2 = 0);
	void                    ClearErrorCount(void);
	PFVint                  ReloadConfig;
	PFV                     OnAnswerErrorExecute;
	PFV                     OnGetDataFrom485;
	PFV                     OnGetDataFrom485_1;
	PFV                     OnGetDataFrom485_2;
	PFV                     OnGetDataFrom485_3;
	PFV                     OnGetDataFrom485_4;
	string                  CmdText;
	sync_deque<TCmdMessage*>    *MainMessages;
	//sync_deque<TCmdMessage*>    *AlternateMessages;
	vector<sync_deque<TCmdMessage*>*> AltMessList;
	Byte                    LastErrorIndex;
	string                  LastDate;
	string                  LastTime;
	string                  LastCMD;
	string                  LastState;
	string                  ID;
	string                  Addr;
	vector<string>          LastData;
	string                  PasswordPtr;
	string                  AddrPtr;
	TCmdMessage            *GetCurrNeedCmdAnswerMessage(void);
	void                    SetCurrNeedCmdAnswerMessage(TCmdMessage *NewMess);

	Word REPEAT_COUNT;
	Word MAX_ERRORS_TO_RESET;

	string Info;
	map<Word, ICmdController*> Controllers;


		TCmd_msg               *BaseCMD;

	protected:
		string 					ConvertPtype(string data);
		void                    SetNeedReset(void);
		void                    SetNeedAnswer(void);
		Word                    CreateCMD(	  TBuffer 			*Buffer,
											  string 			CMD,
											  string 			INDICATION,
											  vector<string> 	Data,
											  bool 				WithQuotes = true,
											  TRtcTime 			*DateTime=NULL,
											  string 			Passw="",
											  string 			Addr=""
										 );
		ICmd *                  ParsingAnswer( TBuffer *Buffer );
		bool                    PushToAlternateMessages(string &Pass, string &Addr, string IND, bool WithQuotes = true);
		bool                    PushToMainMessages(string& Addr, string CMD);
		void                    ClearAlternateMessages(void);
		void                    ClearMainAndCurrMessages(void);
		void                    ClearLastData(void);
		void                    PushBackToLastData(string data);
		bool                    CMD_CmpCRC(Byte  *RcvCRC, Word &CRC);
		vector<TEt>             GetTypes( vector<string> &Data );
		vector<TEt>             GetTypes( string &Data );



	private:
		TCmdMessage              *_currNeedCmdAnswerMessage;
		Word                      _errorCount;
		bool                      _needReset;
		bool                      _needAnswer;
		Byte                      Fase;
		Word                      CalcCRC;
		vector<TCmd_msg*>         CMD_Messages;
		//int							_waitCnt = 0;
	};
	//====================================
		class TCmd_ERROR:public TCmd_msg
		{
		public:
		   TCmd_ERROR(TCmd_msg *baseCMD, string cmdText)
		  {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd_ERROR(){}
		   ICmd*       CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd_UnknownCMD:public TCmd_msg
		{
		public:
		   TCmd_UnknownCMD(TCmd_msg *baseCMD, string cmdText)
		  {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd_UnknownCMD(){}
		   ICmd*       CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd_Data:public TCmd_msg
		{
		public:
		  TCmd_Data(TCmd_msg *baseCMD, string cmdText)
		  {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd_Data(){}
		   ICmd*        CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd_SetData:public TCmd_msg
		{
		public:
		  TCmd_SetData(TCmd_msg *baseCMD, string cmdText)
		  {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd_SetData(){}
		   ICmd*        CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};

		//====================================
		class TCmd_SetCfg:public TCmd_msg
		{
		public:
		   TCmd_SetCfg(TCmd_msg *baseCMD, string cmdText)
		   {
		    CmdText = cmdText;
		    BaseCMD = baseCMD;
		  }
		  ~TCmd_SetCfg(){}
		   ICmd*  CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd_GetCfg:public TCmd_msg
		{
		public:
		   TCmd_GetCfg(TCmd_msg *baseCMD, string cmdText)
		   {
		    CmdText = cmdText;
		    BaseCMD = baseCMD;
		  }
		  ~TCmd_GetCfg(){}
		   ICmd*  CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd_UB:public TCmd_msg
		{
		public:
		  TCmd_UB(TCmd_msg *baseCMD, string cmdText)
		   {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd_UB(){}
		   ICmd*  CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd_UD:public TCmd_msg
		{
		public:
		  TCmd_UD(TCmd_msg *baseCMD, string cmdText)
		   {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd_UD(){}
		   ICmd*  CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd_UE:public TCmd_msg
		{
		public:
		  TCmd_UE(TCmd_msg *baseCMD, string cmdText)
		   {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd_UE(){}
		   ICmd*  CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd485_ERROR:public TCmd_msg
		{
		public:
		   TCmd485_ERROR(TCmd_msg *baseCMD, string cmdText)
		  {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd485_ERROR(){}
		   ICmd*       CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd485_UnknownCMD:public TCmd_msg
		{
		public:
		   TCmd485_UnknownCMD(TCmd_msg *baseCMD, string cmdText)
		  {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd485_UnknownCMD(){}
		   ICmd*       CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
		//====================================
		class TCmd485_Data:public TCmd_msg
		{
		public:
		   TCmd485_Data(TCmd_msg *baseCMD, string cmdText)
		  {
		     CmdText = cmdText;
		     BaseCMD = baseCMD;
		   }
		  ~TCmd485_Data(){}
		   ICmd*       CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

		private:
		};
	//====================================
	class TCmd_GetData:public TCmd_msg
	{
	public:
	  TCmd_GetData(TCmd_msg *baseCMD, string cmdText)
	  {
		 CmdText = cmdText;
		 BaseCMD = baseCMD;
	   }
	  ~TCmd_GetData(){}
	   ICmd*        CMD_Execute(void* p1 = 0,void* p2 = 0, void* p3 = 0);

	private:
	};

#endif /* CMDMESSAGES_H_ */

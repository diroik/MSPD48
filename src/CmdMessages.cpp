/*
 * CmdMessages.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: user
 */

#include <CmdMessages.h>

//=============================================================================
//***TCmd_msg class ***********************************************************
//=============================================================================
ICmd *TCmd_msg::CMD_Recv(void* p1,void* p2)
{
    IPort         *Port  =   (IPort*)p1;
    TBuffer       *Buffer =  (TBuffer*)p2;
    ICmd          *ExecutableCMD = this;
	try
	{
		Buffer->DataLen = Port->RecvTo( Buffer->BUF, Buffer->MaxSize, ETX );
		//Log::DEBUG( "[TCmd_msg] RecvTo "+ BaseCMD->Info +" DataLen="+toString(Buffer->DataLen ));
		if(  Buffer->DataLen > 2 ) //2 bytes send server for test connection
		{
		  SetFase( EXE_CMD );
		  ExecutableCMD = BaseCMD;
		  //if(BaseCMD->Info != "Router")Log::DEBUG( "[TCmd_msg] RecvTo OK DataLen="+toString(Buffer->DataLen ) +" Subfase="+toString( (int)GetFase())  +" \r\nData ["+ string((char*)Buffer->BUF, Buffer->DataLen )+"]");
		}
		else //if(Buffer->DataLen == 0)//timeout
		{
			//if(BaseCMD->Info == "Router") Log::DEBUG( "[TCmd_msg] timeout");
			SetFase(WAIT_MES);
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "TCmd_msg::CMD_Recv " + toString( e.what() ) );
		ExecutableCMD = BaseCMD;
		SetFase(WAIT_MES);
	}
    return ExecutableCMD;
}
//=============================================================================
ICmd *TCmd_msg::CMD_Send( void* p1,void* p2 )
{
	//Log::DEBUG( "[TCmd_msg] CMD_Send");
  IPort       	*Port  			= (IPort*)p1;
  TBuffer       *Buffer 		= (TBuffer*)p2;
  ICmd          *ExecutableCMD 	= this;
  try
  {
	  int Len = Port->Send(Buffer->BUF, Buffer->DataLen);
	  if( Len > 0 )
	  {
		  if(BaseCMD->Info == "Router")
		  {
			  bool needRX = false;
			  TCmdMessage *CurrMessage = GetCurrNeedCmdAnswerMessage();
			  if(CurrMessage != NULL && CurrMessage->NeedCmdAnswer == true)
				  needRX = true;

			  if(!needRX)
			  {
				  if(Port->GetBytesInBuffer() > 0){
					  needRX = true;
					  Log::DEBUG( "[TCmd_msg] Port->GetBytesInBuffer() > 0");
				  }
			  }
			  needRX == true ? SetFase( RX_CMD ) : SetFase(WAIT_MES);

			  string mess = needRX == true ? "RX_CMD": "WAIT_MES";
			  //Log::DEBUG( "[TCmd_msg] Router CMD_Send SetFase=" + mess );
		  }
		  else{
			  SetFase( RX_CMD );
		  }
		  ExecutableCMD  		= BaseCMD;
		  TCmd_msg::_needAnswer = false;
	  }
  }
  catch(exception &e)
  {
		Log::ERROR( "TCmd_msg::CMD_Send " + toString( e.what() ) );
		ExecutableCMD = BaseCMD;
		SetFase(WAIT_MES);
  }
  return ExecutableCMD;
}
//=============================================================================

/*
ICmd *TCmd_msg::GET_Message(void* p1,void* p2)
{
	//Log::DEBUG( "[CounterTask] GET_Message");
  static Word TryCount = 0;
  IPort       *Port  =   (IPort*)p1;
  TBuffer     *Buffer =  (TBuffer*)p2;
  ICmd        *ExecutableCMD  = this;

  if( BaseCMD->MainMessages != NULL )
  {
    TCmdMessage *CurrMessage = GetCurrNeedCmdAnswerMessage();
    if(CurrMessage != NULL)//
    {
      if(TryCount++ >= REPEAT_RECV_COUNT)
      {
        TryCount = 0;
        if(BaseCMD->_errorCount++ < MAX_ERRORS_TO_RESET_ROUTER)
        {
          Buffer->DataLen = CreateCMD(Buffer,
                                      CurrMessage->CMD,
                                      CurrMessage->State,
                                      CurrMessage->Data,
                                      CurrMessage->WithQuotes,
                                      CurrMessage->DateTime,
                                      CurrMessage->Password,
                                      CurrMessage->Addr);


          Log::DEBUG("[TCmd_msg::GET_Message] send curr message repeat Data.size="+toString(CurrMessage->Data.size()));
          for(auto curr: CurrMessage->Data)
        	  Log::DEBUG("[TCmd_msg::GET_Message] Data="+curr);
          if(BaseCMD->_errorCount%2 == 0)
        	  Port->Close();

          SetFase(TX_CMD);
        }
        else
        {
          // delete currMessage
          Log::DEBUG("[TCmd_msg::GET_Message] send curr message error!!!");
          CurrMessage->Data.clear();
          if(CurrMessage->DateTime != NULL) { delete CurrMessage->DateTime;}
          delete CurrMessage;
          SetCurrNeedCmdAnswerMessage(NULL);
          //
          BaseCMD->ClearErrorCount();
          if(BaseCMD->OnAnswerErrorExecute != NULL)
          {
            BaseCMD->OnAnswerErrorExecute(NULL);
          }
        }
        return ExecutableCMD;
      }
    }
    else if( !BaseCMD->MainMessages->empty() )
    {
     //Log::DEBUG( "GET_Message MainMessages.size="+toString(BaseCMD->MainMessages->size()));
      CurrMessage = BaseCMD->MainMessages->front();
      BaseCMD->MainMessages->pop_front();
      Buffer->DataLen = CreateCMD(Buffer,
                                  CurrMessage->CMD,
                                  CurrMessage->State,
                                  CurrMessage->Data,
                                  CurrMessage->WithQuotes,
                                  CurrMessage->DateTime,
                                  CurrMessage->Password,
                                  CurrMessage->Addr);
      if(CurrMessage->NeedCmdAnswer == false)
      {
        CurrMessage->Data.clear();
        if(CurrMessage->DateTime != NULL) { delete CurrMessage->DateTime;}
        delete CurrMessage;
      }
      else
      {
        SetCurrNeedCmdAnswerMessage(CurrMessage);
        BaseCMD->_errorCount = 0;
        TryCount = 0;
      }
      SetFase(TX_CMD);
      return ExecutableCMD;
    }
  }
  SetFase(RX_CMD);
  return ExecutableCMD;
}*/

ICmd *TCmd_msg::GET_Message(void* p1,void* p2)
{

  static Word TryCount = 0;
  IPort       *Port  =   (IPort*)p1;
  TBuffer     *Buffer =  (TBuffer*)p2;
  ICmd        *ExecutableCMD  = this;

  if( BaseCMD->MainMessages != NULL )
  {
	//if(BaseCMD->Info == "Router") Log::DEBUG( "[CounterTask] GET_Message");

    TCmdMessage *CurrMessage = GetCurrNeedCmdAnswerMessage();
    if(CurrMessage != NULL)//
    {
      if(TryCount++ >= REPEAT_COUNT)
      {//change messages
        TryCount = 0;
        if(CurrMessage->ErrorCnt++ >=  MAX_ERRORS_TO_RESET)
        {// delete currMessage
            Log::DEBUG("[TCmd_msg::GET_Message] send curr message error, delete message!!!");
            CurrMessage->Data.clear();
            if(CurrMessage->DateTime != NULL)
            	delete CurrMessage->DateTime;
            delete CurrMessage;
            SetCurrNeedCmdAnswerMessage(NULL);
            BaseCMD->ClearErrorCount();

            if(BaseCMD->OnAnswerErrorExecute != NULL)
              BaseCMD->OnAnswerErrorExecute(NULL);
        }
        else
        {//push_back currMessage
            SetCurrNeedCmdAnswerMessage(NULL);
            BaseCMD->MainMessages->push_back(CurrMessage);

            Log::DEBUG("[TCmd_msg::GET_Message] recv confirm err push_back CurrMessage to MainMessages");
            for(auto curr: CurrMessage->Data)
          	  Log::DEBUG("[TCmd_msg::GET_Message] Data="+curr);

          	//Port->Close();
        }
        return ExecutableCMD;
      }
      //Log::DEBUG("[TCmd_msg::GET_Message] TryCount="+toString(TryCount));
    }
    else if( !BaseCMD->MainMessages->empty() )
    {
     //Log::DEBUG( "GET_Message MainMessages.size="+toString(BaseCMD->MainMessages->size()));
      CurrMessage = BaseCMD->MainMessages->front();
      BaseCMD->MainMessages->pop_front();
      Buffer->DataLen = CreateCMD(Buffer,
                                  CurrMessage->CMD,
                                  CurrMessage->State,
                                  CurrMessage->Data,
                                  CurrMessage->WithQuotes,
                                  CurrMessage->DateTime,
                                  CurrMessage->Password,
                                  CurrMessage->Addr);
      if(CurrMessage->NeedCmdAnswer == false || Buffer->DataLen == 0)
      {
        CurrMessage->Data.clear();
        if(CurrMessage->DateTime != NULL) { delete CurrMessage->DateTime;}
        delete CurrMessage;
      }
      else
      {
        SetCurrNeedCmdAnswerMessage(CurrMessage);
        BaseCMD->_errorCount = 0;
        TryCount = 0;
      }
      SetFase(TX_CMD);
      return ExecutableCMD;
    }
  }
  SetFase(RX_CMD);
  return ExecutableCMD;
}

//=============================================================================
ICmd *TCmd_msg::CMD_Execute(void* p1,void* p2, void* p3)
{
  IPort    	*Port  		= (IPort*)p1;
  TBuffer  	*BufferB  	= (TBuffer*)p2;
  ICmd		*CmdPtr 	= this;
  try
  {
	 BufferB->BUF[BufferB->DataLen] = 0;
 	 CmdPtr = ParsingAnswer(BufferB);
  }
  catch(exception &e)
  {
	  Log::ERROR( "TCmd_msg::CMD_Execute " + toString( e.what() ) );
  }
  if(CmdPtr == NULL)
  {
	  Log::ERROR( "[TCmd_msg] ParsingAnswer ERROR DataLen="+toString(BufferB->DataLen) + " go to WAIT_MES");
	  SetFase(WAIT_MES);
	  CmdPtr = (ICmd*)BaseCMD;
	 //if(BaseCMD->Info == "RS485") Log::DEBUG( "[TCmd_msg] CMD_Execute SubFase=="+toString((int)GetFase())+" CmdPtr="+toString((int)CmdPtr));
  }
  else
  {
	  //if(BaseCMD->Info == "RS485") Log::DEBUG("TCmd_msg::CMD_Execute ParsingAnswer OK");
  }
  return CmdPtr;
}
//=============================================================================
bool TCmd_msg::ConnectOK(void* p1)
{
  IPort      *Port  =   (IPort*)p1;

  //Log::DEBUG( "ConnectOKK" );
  return Port->ConnectOK();
}
//=============================================================================
Word TCmd_msg::CreateCMD(TBuffer *Buffer, string CMD,string INDICATION, vector<string> Data,
																			bool WithQuotes, TRtcTime *DateTime, string Passw, string Addr)
{
  Word index = 0;
  Data_Params const  *ptr = &Configuration;
  TRtcTime            Time = SystemTime.GetTime();

  if(DateTime != NULL){ Time = *DateTime;}


  Buffer->BUF[index++] = STX;
  Buffer->BUF[index++] = ',';
  if( Passw != "")
  { index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)Passw.c_str());}
  else
  { index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)ptr->PassWord.c_str());}
  Buffer->BUF[index++] = ',';

  if( Addr != "")
  { index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)Addr.c_str());}
  else
  { index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)ptr->DevAddress.c_str());}
  Buffer->BUF[index++] = ',';

  index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)WordToString(Time.Day,2).c_str());
  Buffer->BUF[index++] = '.';
  index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)WordToString(Time.Month,2).c_str());
  Buffer->BUF[index++] = '.';
  index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)WordToString(Time.Year).c_str());
  Buffer->BUF[index++] = ',';
  index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)WordToString(Time.Hour,2).c_str());
  Buffer->BUF[index++] = ':';
  index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)WordToString(Time.Minute,2).c_str());
  Buffer->BUF[index++] = ':';
  index += TBuffer::CopyString(&Buffer->BUF[index], (Byte*)WordToString(Time.Second,2).c_str());
  Buffer->BUF[index++] = ',';
  index += TBuffer::CopyString( &Buffer->BUF[index], (Byte*)CMD.c_str() );
  Buffer->BUF[index++] = ',';

  if( INDICATION !="" )
  {
      index += TBuffer::CopyString( &Buffer->BUF[index], (Byte*)INDICATION.c_str() );
      Buffer->BUF[index++] = ',';
  }
  if(!Data.empty())
  {
    int sz = Data.size();

      for(int i=0; i < sz; i++)
      {
        if(WithQuotes == true)
        {
          Buffer->BUF[index++] = '\r';
          Buffer->BUF[index++] = '\n';
          Buffer->BUF[index++] = '>';
          Buffer->BUF[index++] = '>';
          Buffer->BUF[index++] = ',';
        }
        string estr = ConvertPtype(Data[i]);
        if(index + estr.size() >= (Buffer->MaxSize+10))
        	return 0;
        index += TBuffer::CopyString( &Buffer->BUF[index], (Byte*)estr.c_str() );
        Data[i].clear();
        if(Buffer->BUF[index-1] != ',')
        {  Buffer->BUF[index++] = ',';  }
      }

      if(WithQuotes == true)
      {
        Buffer->BUF[index++] = '>';
        Buffer->BUF[index++] = '>';
        Buffer->BUF[index++] = ',';
      }
  }
  Word CRC = TBuffer::Crc( Buffer->BUF, index);
  Byte Buf[5] = {0,0,0,0,0};
  WordToHex(Buf, CRC);
  index += TBuffer::CopyString( &Buffer->BUF[index], Buf );
  Buffer->BUF[index++] = ',';
  Buffer->BUF[index++] = ETX;
  return index;
}
//=============================================================================
string TCmd_msg::ConvertPtype(string data)
{
	string orig = "\r\n";
	string rep = "";
    string tmpstr = data;

	for (int pos = tmpstr.find(orig); pos != string::npos; pos = tmpstr.find(orig, pos))
	{
	   tmpstr = tmpstr.replace(pos, orig.length(), rep);
	   pos += rep.length() - orig.length() + 1;
	}
	orig = ">";
	rep = "\r\n>";

	//Log::DEBUG("ConvertPtype data="+data+" orig="+orig+" rep="+rep);
	for (int pos = tmpstr.find(orig); pos != string::npos; pos = tmpstr.find(orig, pos))
	{
		tmpstr = tmpstr.replace(pos, orig.length(), rep);
		pos += rep.length() - orig.length() + 1;
	}
	//Log::DEBUG("ConvertPtype return="+tmpstr);

	return tmpstr;
}
//=============================================================================
ICmd* TCmd_msg::ParsingAnswer( TBuffer *Buffer )
{
	//Log::DEBUG( "Start TCmd_msg::ParsingAnswer DataLen="+toString(Buffer->DataLen));
  ICmd *returnValue             = NULL;
  char stx = STX;
  char etx = ETX;
  int start = TBuffer::find_first_of(Buffer, stx);
  int end   = TBuffer::find_first_of(Buffer, etx);

  //Log::DEBUG( "TCmd_msg::ParsingAnswer start="+toString(start)+ " end="+toString(end) );
  if(start >= 0 && end > 0 && start < end)
  {//ok
    Buffer->DataLen = Buffer->DataLen-2;
    int index = TBuffer::find_last_of(Buffer, ',');
    if(index < 0 || index >= end)
    {
    	Log::DEBUG( "TCmd_msg::ParsingAnswer find_last_of(,)="+toString(index));
    	BaseCMD->LastCMD = "";
    	returnValue = NULL;
    	return returnValue;
    }
    index++;
    //Log::DEBUG( "TCmd_msg::ParsingAnswer go next index="+toString(index));
    Buffer->BUF[Buffer->DataLen] = 0;
    Byte  *CRC        = &Buffer->BUF[index];
    Word   RecvCRC    = HexToWord( CRC );
    Word   CalcCRC    = TBuffer::Crc(&Buffer->BUF[start], (index-start));
    //if(BaseCMD->Info == "RS485")Log::DEBUG( "TCmd_msg::ParsingAnswer end CalcCRC");
    while( *CRC <' ' ){CalcCRC+=*CRC++;}
    //if(BaseCMD->Info == "RS485") Log::DEBUG( "RecvCRC="+toString(RecvCRC)+" CalcCRC="+toString(CalcCRC));

    if(RecvCRC!=CalcCRC)
    {
    	Log::ERROR( "TCmd_msg::ParsingAnswer wrong CRC! RecvCRC="+toString(RecvCRC)+" CalcCRC="+toString(CalcCRC));
        BaseCMD->LastCMD = "";
        returnValue = NULL;
        return returnValue;
    }
    Buffer->DataLen = index;
    Buffer->DataLen = TBuffer::clear_all_nulls( (char*)Buffer->BUF, Buffer->DataLen );
    ClearLastData();
    TBuffer::Split(Buffer->BUF, Buffer->DataLen, ",", BaseCMD->LastData, 6);//5
    int size = BaseCMD->LastData.size();
    if(  size >= 5 )
    {
          string PASSWORD   = BaseCMD->LastData[0];
          string ID         = BaseCMD->LastData[1];
          if( ID.size() == 9 ){
            BaseCMD->Addr          = ID.substr(7,2);
            ID                     = ID.substr(0,7);
          }
          else if(ID.size() == 2){
        	  BaseCMD->Addr          = ID;
          }
          else{
        	  BaseCMD->Addr = "0";
          }
          BaseCMD->LastDate       = BaseCMD->LastData[2];
          BaseCMD->LastTime       = BaseCMD->LastData[3];
          BaseCMD->LastCMD        = BaseCMD->LastData[4];
          string PS="";
          if(size>=6)
          {  PS         = BaseCMD->LastData[5];}

          string pass = BaseCMD->PasswordPtr;
          string adr  = ID;
          if(BaseCMD->AddrPtr != "")
          { adr = BaseCMD->AddrPtr;}
          DWord flashAddr = atol(adr.c_str());
          DWord recvAddr  = atol(ID.c_str());
          if( (PASSWORD !=  pass && PASSWORD !=  "TM_DPGSM") )
          {
            BaseCMD->LastErrorIndex  = PASW_ER_IND;
            returnValue = GetCMDbyKey("ERROR");
            return returnValue;
          }
          else if( recvAddr != flashAddr && recvAddr != 9999999  )
          {
            BaseCMD->LastErrorIndex  = ID_ER_IND;
            returnValue = GetCMDbyKey("ERROR");
            return returnValue;
          }
          else
          {//ok
              if(BaseCMD->AddrPtr != "")//
              {
                if(PS!="NS")
                {
                  if(SystemTime.GetSynchAct())
                  {
                	  SystemTime.SyncDateTime(BaseCMD->LastDate, BaseCMD->LastTime);
                  }
                  else
                  {
                	SystemTime.SetDateTime(BaseCMD->LastDate, BaseCMD->LastTime);
                	SystemTime.SetSystemTimeOfDay(SystemTime.DateTime);//set localtime of linux
                    SystemTime.SetSynchAct(true);
                    Log::DEBUG("SetSynchAct");
                  }

                }
              }
              BaseCMD->ID = ID;
              returnValue = GetCMDbyKey( BaseCMD->LastCMD );

              if( returnValue == NULL )
            	  returnValue = GetCMDbyKey( "UnknownCMD" );
              //Log::DEBUG("GetCMDbyKey cmd=" + toString((int)returnValue));
              if(returnValue == NULL)
              {
                ClearLastData();
                return returnValue;
              }
              else
              {
                BaseCMD->LastState = "";
                if(size >= 6)
                { BaseCMD->LastState = BaseCMD->LastData[5]; }
                this->ClearLastData();
                index = TBuffer::find(Buffer, ">>,");
                if(index >= 0)
                {
                  int lenth = Buffer->DataLen - index;
                  if(lenth > 0){
                    TBuffer::Split(&Buffer->BUF[index], lenth, ">>,", BaseCMD->LastData);
                  }
                }
                else
                {
                	int indS = TBuffer::find(Buffer, "[,");
                	int indE = TBuffer::find(Buffer, ",]");
                	if(indS >= 0 && indE >= 0)
                	{
                		indS +=2;
                		indE -=1;
                		if(indE > indS)
                		{
							int l = indE - indS;
							string bf = string((char*)&Buffer->BUF[indS], l);
							vector<string> retVect;
							TBuffer::Split(bf, ",", retVect);
							if(retVect.size() > 0)
							{
						        if(BaseCMD->OnGetDataFrom485 != NULL)
						        {//obrabotka dannih ot MKK
						          BaseCMD->OnGetDataFrom485(&retVect);
						        }
							}
                		}
                	}
                }
                BaseCMD->LastErrorIndex = OK_IND;
                return returnValue;
              }
          }
    }
    else
    {
        ClearLastData();
        BaseCMD->LastErrorIndex = UNKNOWN_ER_IND;
        returnValue = GetCMDbyKey("ERROR");
        return returnValue;
    }
  }
  else
  {
      //SetFase(RX_CMD);
      //Buffer->DataLen = 0;
	  ///////////////////////////
	  Byte bf[2] {0,0};
	  string recv = "";
	  for(int i = 0; i < Buffer->DataLen; i++)
	  {
		  ByteToHex(bf, Buffer->BUF[i]);
		  recv += "0x"+string((char*)bf, 2)+" ";
	  }
	  //Log::DEBUG( "[CounterManager] RecvData ok: [" + recv +"] RecvLen="+toString(RecvLen)+ " Len="+toString(Len)+" SubFase="+toString((int)SubFase));
	  string mess = string((char*)Buffer->BUF, (int)Buffer->DataLen);
	  //////////////////////
      Log::ERROR( "TCmd_msg::ParsingAnswer ERR start="+toString(start)+ " end="+toString(end) + "\r\n bytes=[" +  recv + "]\r\n text=" + mess);
  }
  return returnValue;
}
//=======================================
bool TCmd_msg::PushToAlternateMessages(string &Pass, string &Addr, string IND, bool WithQuotes)
{
	bool flg = false;
	for(auto curr: BaseCMD->AltMessList)
	{
	    if( curr->size() < MAX_MESS_IN_ALTERNATE)
	    {
	      TCmdMessage *NewMessage = new TCmdMessage();
	      NewMessage->CMD         =  BaseCMD->LastCMD;
	      NewMessage->Data        =  BaseCMD->LastData;
	      NewMessage->Password    =  Pass;
	      NewMessage->Addr        =  Addr;
	      NewMessage->State       =  IND;
	      NewMessage->WithQuotes  = WithQuotes;
	      curr->push_front(NewMessage);
	      flg = true;
	    }
	}
  ClearLastData();
  return flg;
}
//=======================================
bool TCmd_msg::PushToMainMessages(string& Addr, string CMD)
{
  if( BaseCMD->MainMessages != NULL )
  {
    if( BaseCMD->MainMessages->size() < MAX_MESS_IN_MAIN)
    {
      TCmdMessage *NewMessage = new TCmdMessage();
      NewMessage->Password    = PasswordPtr;
      NewMessage->Addr        = Addr;
      NewMessage->CMD         = CMD;

      BaseCMD->MainMessages->push_front(NewMessage);
      ClearLastData();
      return true;
    }
  }
  ClearLastData();
  return false;
}
//=======================================
void TCmd_msg::ClearAlternateMessages(void)
{
	for(auto curr: BaseCMD->AltMessList)
	{
		  if( curr != NULL )
		  {
		    int size = curr->size();
		    for(int i = 0; i < size; i++)
		    {
		      TCmdMessage *CurrMessage = curr->front();//operator[](i);
		      if(CurrMessage != NULL)
		      {
		          CurrMessage->Data.clear();
		          if(CurrMessage->DateTime != NULL) { delete CurrMessage->DateTime;}
		        //  delete CurrMessage;
		      }
		      curr->pop_front();
		    }
		    curr->clear();
		  }
	}
}
//=======================================
void TCmd_msg::ClearMainAndCurrMessages(void)
{
  if( BaseCMD->MainMessages != NULL )
  {
    TCmdMessage *CurrMessage = GetCurrNeedCmdAnswerMessage();
    if(CurrMessage != NULL)
    {
        CurrMessage->Data.clear();
        if(CurrMessage->DateTime != NULL) { delete CurrMessage->DateTime;}
        delete CurrMessage;
        SetCurrNeedCmdAnswerMessage(NULL);
        BaseCMD->ClearErrorCount();
    }

    for(int i = 0; i < BaseCMD->MainMessages->size(); i++)
    {
      CurrMessage = BaseCMD->MainMessages->operator[](i);
      if(CurrMessage != NULL)
      {
          CurrMessage->Data.clear();
          if(CurrMessage->DateTime != NULL) { delete CurrMessage->DateTime;}
          delete CurrMessage;
      }
    }

    BaseCMD->MainMessages->clear();
  }
}
//=======================================
void TCmd_msg::ClearLastData(void)
{
  BaseCMD->LastData.clear();
}
//=======================================
void TCmd_msg::PushBackToLastData(string data)
{
  BaseCMD->LastData.push_back(data);
}
//=============================================================================
bool TCmd_msg::CMD_CmpCRC(Byte  *RcvCRC, Word &CRC)
{
  Byte          CalcCRC[] = {0,0,0,0,0};


    while(*RcvCRC < 0x20)
    {

      if(*RcvCRC == 0) return false;
      CRC +=*RcvCRC++;
    }
    WordToHex(CalcCRC,CRC);
    return TBuffer::StrCmp(RcvCRC,CalcCRC);
}
//=============================================================================
TCmd_msg* TCmd_msg::GetCMDbyKey(string key)
{
  const char* Buf = key.c_str();
  while(*Buf <' ')
  {Buf++;}
  key = Buf;
  TCmd_msg* ReturnMsg = NULL;
  for(auto curr: BaseCMD->CMD_Messages)
  {
    if(curr->CmdText == key)
    {
      ReturnMsg = curr;
      break;
    }
  }
  return ReturnMsg;
}
//=============================================================================
void  TCmd_msg::AddToCMD_Messages(TCmd_msg* msg)
{
  BaseCMD->CMD_Messages.push_back(msg);
}
//=============================================================================
Byte TCmd_msg::GetFase(void)
{
  return this->BaseCMD->Fase;
}
//=============================================================================
void TCmd_msg::SetFase(Byte NewFase)
{
  BaseCMD->Fase = NewFase;
}
//=======================================
bool TCmd_msg::NeedReset(void)
{
  return this->BaseCMD->_needReset;
}
//=======================================
void TCmd_msg::SetNeedReset(void)
{
  this->BaseCMD->_needReset = true;
}
//=======================================
bool TCmd_msg::NeedAnswer(void)
{
  return this->BaseCMD->_needAnswer;
}
//=======================================
void TCmd_msg::SetNeedAnswer(void)
{
  this->BaseCMD->_needAnswer = true;
}
///=======================================
void TCmd_msg::SendRESET(void* p1,void* p2)
{
	/*
 TComPort   *Port             = (TComPort*)p1;
 TBuffer    *Buffer           = (TBuffer*)p2;
 string     AnswerState       = string((char*)ERROR_MASS[OK_IND]);

  Tlist<string>          Data;
  //string Msg              = CreateCMD("RESET", AnswerState, Data);
  //Buffer->DataLen         = TBuffer::CopyString(Buffer->BUF, (Byte*)Msg.c_str());
  Buffer->DataLen = CreateCMD(Buffer, "RESET", AnswerState, Data);
  Port->ClrStxFlag();
  SetFase(TX_CMD);
  this->ClearLastData();*/
  return;
}
//=======================================
vector<TEt>  TCmd_msg::GetTypes(vector<string> &Data)
{
 string         DataString;
 vector<string> PList;
 vector<TEt>    ReturnValue;
 vector<string> TmpList;
 int size = Data.size();


    for(int i = 0; i< size;i++)
    {
      DataString = Data[i];
      PList.clear();
      TBuffer::Split(DataString, ",>,", PList);

      TmpList.clear();

      if(PList.size() == 0)
      { return ReturnValue;}
      string Pstring = PList[0];

      TBuffer::Split(Pstring, ",", TmpList);
      int tmpsize = TmpList.size();
      if( tmpsize!= 2)
      { return ReturnValue;}
      TEt CurrEt( atoi(TmpList[0].c_str()), atoi(TmpList[1].c_str()) );
      int SubSize = PList.size();
      for(int j = 1; j < SubSize; j++)
      {
        TmpList.clear();
        DataString = PList[j];
        TBuffer::Split(DataString, ",", TmpList);
        int s = TmpList.size();
        if(s < 2)
        { return ReturnValue;}
        TPt CurrPt( atoi(TmpList[0].c_str()), atoi(TmpList[1].c_str()) );//

        for(int k = 2; k < s; k++)//
        {CurrPt.Value.push_back( TmpList[k] );}
        CurrEt.Ptypes.push_back( CurrPt );
      }
      ReturnValue.push_back( CurrEt );
    }

    return ReturnValue;
}
//=======================================
vector<TEt>  TCmd_msg::GetTypes(string &Data)
{
 string         DataString;
 vector<string> PList;
 vector<TEt>    ReturnValue;
 vector<string> TmpList;


    DataString = Data;
    PList.clear();
    TBuffer::Split(DataString, ",>,", PList);
    TmpList.clear();
    if(PList.size() == 0)
    { return ReturnValue;}
    string Pstring = PList[0];
    TBuffer::Split(Pstring, ",", TmpList);
    int tmpsize = TmpList.size();
    if( tmpsize!= 2)
    { return ReturnValue;}
    TEt CurrEt( atoi(TmpList[0].c_str()), atoi(TmpList[1].c_str()) );
    int SubSize = PList.size();
    for(int j = 1; j < SubSize; j++)
    {
        TmpList.clear();
        DataString = PList[j];
        TBuffer::Split(DataString, ",", TmpList);
        int s = TmpList.size();
        if(s < 2)
        { return ReturnValue;}
        TPt CurrPt( atoi(TmpList[0].c_str()), atoi(TmpList[1].c_str()) );//

        for(int k = 2; k < s; k++)//
        {CurrPt.Value.push_back( TmpList[k] );}
        CurrEt.Ptypes.push_back( CurrPt );
    }
    ReturnValue.push_back( CurrEt );

    return ReturnValue;
}
//=============================================================================
TCmdMessage *TCmd_msg::GetCurrNeedCmdAnswerMessage(void)
{
  return BaseCMD->_currNeedCmdAnswerMessage;
}
//=============================================================================
void TCmd_msg::SetCurrNeedCmdAnswerMessage(TCmdMessage *NewMess)
{
  BaseCMD->_currNeedCmdAnswerMessage = NewMess;
}
//=============================================================================
void TCmd_msg::ClearErrorCount(void)
{
  BaseCMD->_errorCount = 0;
}

//*****************************************************************************
ICmd* TCmd_ERROR::CMD_Execute(void* p1,void* p2, void* p3)
{
 TBuffer        *Buffer   = (TBuffer*)p2;
 string AnswerState       = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
 ICmd *ExecutableCMD      = BaseCMD;

  this->ClearLastData();
  if( this->BaseCMD->LastCMD == "")
  { this->BaseCMD->LastCMD = "ERROR";}
  Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
  SetFase(TX_CMD);
  return ExecutableCMD;
}
//*****************************************************************************
//***TCmd_UnknownCMD
//*****************************************************************************
ICmd* TCmd_UnknownCMD::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 ICmd *ExecutableCMD      = BaseCMD;

 //Log::DEBUG("TCmd_UnknownCMD::CMD_Execute");
  if( atoi( BaseCMD->Addr.c_str() ) == 0 )
  {
    TCmdMessage *CurrMessage = GetCurrNeedCmdAnswerMessage();
    if(CurrMessage == NULL)
    {
      if(BaseCMD->LastCMD != "CfData")
      {
        BaseCMD->LastErrorIndex = CMD_ER_IND;
        ExecutableCMD = GetCMDbyKey("ERROR");
        return ExecutableCMD;
      }
    }
    else
    {
      if(SystemTime.GetSynchAct())
      {
        string neededCmd = "Cf"+CurrMessage->CMD;
        if( BaseCMD->LastCMD == neededCmd)
        {
          CurrMessage->Data.clear();
          if(CurrMessage->DateTime != NULL) { delete CurrMessage->DateTime;}
          delete CurrMessage;
          SetCurrNeedCmdAnswerMessage(NULL);
          BaseCMD->ClearErrorCount();
          //Log::DEBUG("TCmd_UnknownCMD");
        }
      }
      else
      {//
          BaseCMD->ClearErrorCount();
      }
    }
    SetFase(WAIT_MES);
  }
  else
  {
    //ClearAlternateMessages();
    string Pass = ptr->PassWord485;
    PushToAlternateMessages(Pass, BaseCMD->Addr, "");
    ClearLastData();
    SetFase(WAIT_MES);//RX_CMD
  }
  return ExecutableCMD;
}
//*****************************************************************************
//***TCmd_Data
//*****************************************************************************
ICmd* TCmd_Data::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 TBuffer *Buffer          = (TBuffer*)p2;
 BaseCMD->LastErrorIndex  = OK_IND;
 ICmd *ExecutableCMD      = BaseCMD;

 	 //Log::DEBUG("TCmd_Data::CMD_Execute");
  if( atoi( BaseCMD->Addr.c_str() ) == 0 )
  {
      if(BaseCMD->LastData.size() > 0)
      {
    	string type = BaseCMD->LastData[0];
    	vector<TEt> Equipments = GetTypes(type);
    	int size = Equipments.size();
    	if(size > 0){
    		TEt et = Equipments[0];
    		if(et.EType == 0 && et.EName == 120)//installer
    		{
    			ClearLastData();
    			string installer = uci::Get(CONFIG_FILE, "main", "installer");
    			if(isNullOrWhiteSpace(installer))
    				installer = "~";
    			BaseCMD->LastData.push_back("0,120, >,120,1,"+installer);
    		}
    	}
      }
      else
      {
    	    ClearLastData();
      }
      string AnswerState      = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
      Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
      SetFase(TX_CMD);
  }
  else
  {
	string Pass = ptr->PassWord485;
    PushToAlternateMessages(Pass, BaseCMD->Addr, "");
    ClearLastData();
    SetFase(RX_CMD);
  }
  return ExecutableCMD;
}

//*****************************************************************************
//***TCmd_SetData
//*****************************************************************************
ICmd* TCmd_SetData::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 TBuffer *Buffer          = (TBuffer*)p2;
 BaseCMD->LastErrorIndex  = OK_IND;
 ICmd *ExecutableCMD      = BaseCMD;

  if( atoi( BaseCMD->Addr.c_str() ) == 0 )
  {
	  //Log::DEBUG("TCmd_SetData end EnergyCounterManager.FirstInitFlg="+toString(EnergyCounterManager.FirstInitFlg));
	  vector<EcounterCmdParameter> cmdList;
	  vector<IbpCmdParameter> icmdList;
      for(int i = 0; i < BaseCMD->LastData.size(); i++)
      {
    	string type = BaseCMD->LastData[i];
    	vector<TEt> Equipments = GetTypes(type);
    	int size = Equipments.size();
    	if(size > 0)
    	{
    		TEt et = Equipments[0];
    		if(et.EType == 0)
    		{
    			if(et.Ptypes.size() == 1)
    			{
    				TPt pt = et.Ptypes[0];
    				if(pt.Value.size() == 2)
    				{
    					if(pt.Value[0] == "reboot" && pt.Value[1] == "123qwe45RTY")
    					{
    						Log::INFO("router reboot by CMD");
    						string rebootStr = "ubus call system watchdog '{\"stop\": true}'";/// "/sbin/reboot";//
    						sendToConsole(rebootStr);
    						Log::INFO("router reboot OK");
    					}
    				}
    			}
    		}
    		else if(et.EType == 41)//ecounter
    		{//>>,41,0,>,0,1,Addr,cmd,subcmd,par
    			int index;
    			if(et.Ptypes.size() > 1)
    				index = 1;
    			Log::INFO("Set ECMD size="+toString(et.Ptypes.size()));
    			for(auto pt: et.Ptypes)
    			{
    				if(pt.PType == 0)
    				{
    					if(pt.Value.size() >= 4)
    					{
    						EcounterCmdParameter epar;

    						epar.Et 		= et.EType;
    						epar.En 		= et.EName;
    						epar.addr 		= atoi(pt.Value[0].c_str());
    						epar.cmd 		= atoi(pt.Value[1].c_str());
    						epar.sub_cmd 	= atoi(pt.Value[2].c_str());
    						epar.index      = index >= et.Ptypes.size() ? 23 : index;
    						epar.parameters.push_back( atoi(pt.Value[3].c_str()) );
    						cmdList.push_back(epar);
    						Log::INFO("Set ECMD epar.index="+toString(epar.index));
    					}
    				}
    				else if(pt.PType == 51)
    				{//>>,41,0,>,51,1,1,3
    					vector<string> params;
       					if(pt.Value.size() >= 1)
       					{
       						Word st = atoi(pt.Value[0].c_str());//st=0 - get time; st=1 - set time; st=2 - getProfil
       						for(int i = 1; i < pt.Value.size(); i++)
       						{
       							params.push_back(pt.Value[i]);
       						}
       						EnergyCounterManager.ExeCmd(et.EType, et.EName, st, params, p3);
       						if(st != 2)
       						{
       							Log::DEBUG("TCmd_SetData st="+toString(st) + " wait answer");
       							TReturnValue rval = EnergyCounterManager.GetReturnValue();


       						}
       						else{
       							Log::DEBUG("TCmd_SetData st="+toString(st) + "don't wait answer");
       						}
       					}
    				}
    				index++;
    			}
    		}
    		else if(et.EType == 26)//ibp
    		{
    			vector<string> newParameters;
    			for(auto pt: et.Ptypes)
    			{
    				if(pt.PType == 201){// on|off free
    					if(pt.Value.size() >= 1){
    						Word newState = atoi(pt.Value[0].c_str());
    						IbpManager.ChangeState((et.EName-1), newState);
    					}
    				}else if(pt.PType == 230){//
    					if(pt.Value.size() >= 1){
    						//float newVal = atof(pt.Value[0].c_str());
    						newParameters.push_back( pt.Value[0] );
    					}
    				}
    			}
    			if(newParameters.size() > 0){
    				IbpManager.ChangeParameters((et.EName-1), newParameters);
    			}
    		}
    		else if(et.EType == 27)//dgu
    		{
    			vector<string> newParameters;
    			for(auto pt: et.Ptypes){
    				//Log::DEBUG("pt.PType ="+toString( (int)pt.PType )+ " pt.Value="+toString(pt.Value.size()));

    				if(pt.PType == 201){// on|off free
    					if(pt.Value.size() >= 1){
    						Log::DEBUG("pt.Value.size() >= 1");
    						Word newState = atoi(pt.Value[0].c_str());
    						DiselPanelManager.ChangeState( (et.EName-1), newState);
    					}
    				}
    				else if(pt.PType == 230){//
    					if(pt.Value.size() >= 1){
    						newParameters.push_back( pt.Value[0] );
    					}
    				}
    			}
    			if(newParameters.size() > 0){
    				DiselPanelManager.ChangeParameters( (et.EName-1), newParameters);
    			}
    		}
    		else if(et.EType == 28 || et.EType == 29)//pvv
    		{
    			vector<string> newParameters;
    			for(auto pt: et.Ptypes)
    			{
    				if(pt.PType == 201){// on|off free
    					if(pt.Value.size() >= 1){
    						Word newState = atoi(pt.Value[0].c_str());
    						FreeCoolingManager.ChangeState(newState);
    					}
    				}else if(pt.PType == 230){//
    					if(pt.Value.size() >= 1){
    						//float newVal = atof(pt.Value[0].c_str());
    						newParameters.push_back( pt.Value[0] );
    					}
    				}
    			}

    			if(newParameters.size() > 0){
    				FreeCoolingManager.ChangeParameters(newParameters);
    			}
    		}

    		/*
    		else if(et.EType == 29)//freecool
    		{
    			for(auto pt: et.Ptypes)
    			{
    				if(pt.PType == 201)
    				{// on|off free
    					if(pt.Value.size() >= 1)
    					{
    						//FreeCoolingManager.AddScenario("SwitchFree", 1, 1, "", "", 2048);
    						Word newState = atoi(pt.Value[0].c_str());
    						FreeCoolingManager.ChangeState(newState);
    						//FreeCoolingManager.EditScenario("DefaultData", 1, 1, "", "2048", true);
    					}
    				}
    				else if(pt.PType == 230)
    				{//
    					if(pt.Value.size() >= 1) // pt.Pname=index, pt.Value[0]=new value
    					{
    						//FreeCoolingManager.AddPasswordScenario();
    						string st = "1.3."+ toString((int)pt.PName);
    						FreeCoolingManager.EditScenario("PassWord",3,5,st,pt.Value[0]);
    					}
    				}
    				else if(pt.PType == 231)
    				{//
    					if(pt.Value.size() >= 1)
    					{
    						FreeCoolingManager.EditScenario("PassWord",3,2, "1", pt.Value[0]);
    					}
    				}
    				else if(pt.PType == 222)//!!!!!!!!!!!!!!!!1
    				{//
    					if(pt.Value.size() >= 1)
    					{
    						FreeCoolingManager.NeedRestart = true;
    					}
    				}
    			}
    		}*/

       		else if(et.EType == 52)//mkdd
        	{
       			Log::DEBUG("TCmd_SetData mkdd et.Ptypes.size="+toString(et.Ptypes.size()));
        		vector< tuple<int, int,string> > newParameters;
        		for(auto pt: et.Ptypes)
        		{
        			if(pt.Value.size() >= 1){
        				auto p = std::make_tuple( (int)pt.PName, (int)pt.PType, pt.Value[0] );
        				//tuple<int, int,string> p((int)pt.PName, (int)pt.PType, pt.Value[0]);
        				newParameters.push_back( p );
        			}
        		}
        		if(newParameters.size() > 0){

        			Log::DEBUG("TCmd_SetData mkdd newParameters.size="+toString(newParameters.size()));
        			InnerRs485Manager.ChangeParameters(et.EType, et.EName, newParameters);
        		}
        	}


    		else if(et.EType == 60)//mkk2
    		{
    			Log::DEBUG("TCmd_SetData mkk2 et.Ptypes.size="+toString(et.Ptypes.size()));
    			bool reload_flg = false;
    			vector<string> newParameters;
    			for(auto pt: et.Ptypes)
    			{
    				if(pt.PType == 201)
    				{// on|off free
    					if(pt.Value.size() >= 1){
    						Word newState = atoi(pt.Value[0].c_str());
    						MkkClimatManager.ChangeState(newState);
    						reload_flg = true;
    					}
    				}
    				else if(pt.PType == 230)
    				{//
    					if(pt.Value.size() >= 1){
    						//float newVal = atof(pt.Value[0].c_str());
    						newParameters.push_back( pt.Value[0] );
    					}
    				}
    			}

    			if(newParameters.size() > 0){
    				MkkClimatManager.ChangeParameters(newParameters);
    				reload_flg = true;
    			}
    			if(reload_flg == true){
    				MkkClimatManager.ReloadConfig();
    			}
    		}
    		else if(et.EType == 61)
    		{
    			for(auto pt: et.Ptypes)
    			{
    				if(pt.PType == 99)
    				{// send to modBus
    					if(pt.Value.size() >= 4)
    					{
    						ModbusManager1.Stop();
    						IModbusRegisterList *cmd = NULL;
    						try
    						{
    							int addr = atoi(pt.Value[0].c_str());
    							int func = atoi(pt.Value[1].c_str());
    							int reg  = atoi(pt.Value[2].c_str());
    							int data = atoi(pt.Value[3].c_str());

    							bool st = false;
    							switch(func)
    							{
    							case 6://WriteSingleRegister
    								cmd = new ModbusWriteSingleRegister(ModbusManager1.GetCommSettings(), addr, reg, data, NULL );
    								break;
    							default:
    								break;
    							};
    							if(cmd != NULL)
    							{	st = cmd->DoCmd();}
								if(st == false)
								{	BaseCMD->LastErrorIndex = UNKNOWN_ER_IND;}
								else
								{
									ExternClimatManager.NeedModify.SetData(true);
									//ExternClimatManager.FirstInitFlg.SetData(true);
								}

							}
							catch(exception &e)
							{
								Log::ERROR( "[TCmd_SetData] " + toString( e.what() ) );
							}
							if(cmd != NULL) delete cmd;
    						ModbusManager1.Start();
    					}
    				}
    			}
    		}
    	}
      }
      ClearLastData();

      if(cmdList.size() > 0)
      {
    	  EnergyCounterManager.SetNewEcmd(cmdList);
    	  for(auto epar: cmdList)
    	  {
			if(epar.cmd == 40)
			{
				TReturnValue rval = EnergyCounterManager.GetReturnValue();
				BaseCMD->LastData = rval.Data;
			}
    	  }
      }

    string AnswerState      = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
    Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
    SetFase(TX_CMD);
    Log::DEBUG("TCmd_SetData end EnergyCounterManager.FirstInitFlg="+toString(EnergyCounterManager.FirstInitFlg));
  }
  else
  {
    string Pass = ptr->PassWord485;
    PushToAlternateMessages(Pass, BaseCMD->Addr, "");
    ClearLastData();
    SetFase(RX_CMD);
  }
  return ExecutableCMD;
}
//*****************************************************************************
//***TCmd_GetData
//*****************************************************************************
ICmd* TCmd_GetData::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 TBuffer *Buffer          = (TBuffer*)p2;
 BaseCMD->LastErrorIndex  = OK_IND;
 ICmd *ExecutableCMD      = BaseCMD;

 Log::DEBUG("GetData BaseCMD->Addr"+BaseCMD->Addr);

  int addr =  atoi( BaseCMD->Addr.c_str() );
  if( addr == 0 )
  {
    ClearLastData();

    Log::DEBUG("GetData set GetVersionObject.FirstInitFlg");
    GetVersionObject.FirstInitFlg 		= true;

    Log::DEBUG("GetData set EnergyCounterManager.FirstInitFlg");
    EnergyCounterManager.FirstInitFlg 	= true;

    Log::DEBUG("GetData set GetVersionObject.FirstInitFlg");
    IbpManager.FirstInitFlg 			= true;

    Log::DEBUG("GetData set FreeCoolingManager.FirstInitFlg");
    FreeCoolingManager.FirstInitFlg 	= true;

    Log::DEBUG("GetData set DiselPanelManager.FirstInitFlg");
    DiselPanelManager.FirstInitFlg 		= true;

    Log::DEBUG("GetData set ExternClimatManager.FirstInitFlg");
    ExternClimatManager.FirstInitFlg.SetData(true);

    Log::DEBUG("GetData set InnerRs485Manager.FirstInitFlg");
    InnerRs485Manager.FirstInitFlg 		= true;


    Log::DEBUG("GetData set IOManager.FirstInitFlg");
    IOManager.FirstInitFlg 		= true;


    string AnswerState      = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
    Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
    SetFase(TX_CMD);
    Log::DEBUG("GetData set all FirstInitFlg");
  }
  else
  {
	    if(addr == 2)//climat
	    {
	    	//Log::DEBUG("BaseCMD->ReloadConfig");
	        if(BaseCMD->ReloadConfig != NULL)
	        {
	          //Log::DEBUG("BaseCMD->ReloadConfig!=NULL");
	          BaseCMD->ReloadConfig();
	        }
	    }

	string Pass = ptr->PassWord485;
    PushToAlternateMessages(Pass, BaseCMD->Addr, "");
    ClearLastData();
    SetFase(RX_CMD);
  }
  sleep(2);//wait
  return ExecutableCMD;
}
//*****************************************************************************
//***TCmd_SetCfg
//*****************************************************************************
ICmd* TCmd_SetCfg::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 TBuffer *Buffer          = (TBuffer*)p2;
 BaseCMD->LastErrorIndex  = OK_IND;
 ICmd *ExecutableCMD      = BaseCMD;

  if( atoi( BaseCMD->Addr.c_str() ) == 0 )
  {
      for(int i = 0; i < BaseCMD->LastData.size(); i++)
      {
    	  string type = BaseCMD->LastData[i];
    	  vector<TEt> Equipments = GetTypes(type);
    	  int size = Equipments.size();
    	  if(size > 0)
    	  {
    		  TEt et = Equipments[0];
    		  if(et.EType == 41)
    		  {//ecounter
    			  for(auto pt: et.Ptypes)
    			  {
    				  if(pt.PType == 0){
    					  if(pt.Value.size() == 1){
    						  DWord time = atol(pt.Value[0].c_str());
    						   if(time < 1800000){
    							   EnergyCounterManager.SetTimer(ADDITIONAL_TIMER, time);
    							   Log::DEBUG("TCmd_SetCfg set EnergyCounterManager ADDITIONAL_TIMER value=" + toString(time));
    						   }
    					  }
    				  }
    			  }
    		  }
    		  else if(et.EType == 26)
    		  {//ibp
    			  for(auto pt: et.Ptypes)
    			  {
    				  if(pt.PType == 0){
    					  if(pt.Value.size() == 1){
    						  DWord time = atol(pt.Value[0].c_str());
    						   if(time < 1800000){
    							   IbpManager.SetTimer(ADDITIONAL_TIMER, time);
    							   Log::DEBUG("TCmd_SetCfg set IbpManager ADDITIONAL_TIMER value=" + toString(time));
    						   }
    					  }
    				  }
    			  }
    		  }
    		  else if(et.EType == 27)
    		  {//dgu
    			  for(auto pt: et.Ptypes)
    			  {
    				  if(pt.PType == 0){
    					  if(pt.Value.size() == 1){
    						  DWord time = atol(pt.Value[0].c_str());
    						   if(time < 1800000){
    							   DiselPanelManager.SetTimer(ADDITIONAL_TIMER, time);
    							   Log::DEBUG("TCmd_SetCfg set DiselPanelManager ADDITIONAL_TIMER value=" + toString(time));
    						   }
    					  }
    				  }
    			  }
    		  }
    	  }
      }
      ClearLastData();
      string AnswerState      = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
      Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
      SetFase(TX_CMD);
  }
  else
  {
	  string Pass = ptr->PassWord485;
	  PushToAlternateMessages(Pass, BaseCMD->Addr, "");
	  ClearLastData();
	  SetFase(RX_CMD);
  }
  return ExecutableCMD;
}
//*****************************************************************************
//***TGet_Cfg
//*****************************************************************************
ICmd* TCmd_GetCfg::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 TBuffer    *Buffer           = (TBuffer*)p2;
 BaseCMD->LastErrorIndex      = OK_IND;
 ICmd *ExecutableCMD          = BaseCMD;

  if( atoi( BaseCMD->Addr.c_str() ) == 0 )
  {
    ClearLastData();

    string tmp = string(


      "0,1,\r\n>,99,0,"+              //
      	  	  FloatToString(VERSION)+","+toString(Configuration.SerialNumber)+","
                    //WordToString(ptr->SubVersion)+","+
      "\r\n>,103,1,"+
                    WordToString(ptr->RS485BaudRate)+","+
                    WordToString(ptr->RS485RecvTimeout)+","+
      "\r\n>,105,1,"+
                    WordToString(ptr->SocketRecvTimeout)+","+
                    WordToString(MAX_ERRORS_TO_RESET_ROUTER)+","+
                    WordToString(REPEAT_RECV_COUNT)+","+
      "\r\n>,113,1,"+             //
                    WordToString(ptr->EcounterPeriod)+","+
                    WordToString(ptr->DataTaskPeriod) );

    PushBackToLastData(tmp);

    string AnswerState      = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
    Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
    ClearLastData();
    SetFase(TX_CMD);
  }
  else
  {
    string Pass = ptr->PassWord485;
    PushToAlternateMessages(Pass, BaseCMD->Addr, "");
    ClearLastData();
    SetFase(RX_CMD);
  }
  return ExecutableCMD;
}
//*****************************************************************************
//***TCmd_UB
//*****************************************************************************
ICmd* TCmd_UB::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 TBuffer    *Buffer           = (TBuffer*)p2;
 BaseCMD->LastErrorIndex      = OK_IND;
 ICmd *ExecutableCMD          = BaseCMD;

  if( atoi( BaseCMD->Addr.c_str() ) == 0 )
  {
	    this->ClearLastData();
	    string AnswerState      = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
	    Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
	    SetFase(TX_CMD);
  }
  else
  {
    ClearAlternateMessages();
    ClearMainAndCurrMessages();
    string Pass = ptr->PassWord485;
    PushToAlternateMessages(Pass, BaseCMD->Addr, "", false);
    ClearLastData();
    SetFase(RX_CMD);
    ObjectFunctionsTimer.SetTimer(USB_RS485_ACTIV_WAIT_TIMER, 30);
  }
  return ExecutableCMD;
}
//*****************************************************************************
//***TCmd_UD
//*****************************************************************************
ICmd* TCmd_UD::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 TBuffer    *Buffer           = (TBuffer*)p2;
 BaseCMD->LastErrorIndex      = OK_IND;
 ICmd *ExecutableCMD          = BaseCMD;

  if( atoi( BaseCMD->Addr.c_str() ) == 0 )
  {
    this->ClearLastData();
    string AnswerState      = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
    Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
    SetFase(TX_CMD);
  }
  else
  {
    ClearAlternateMessages();
    ClearMainAndCurrMessages();
    string Pass = ptr->PassWord485;
    PushToAlternateMessages(Pass, BaseCMD->Addr, "", false);
    ClearLastData();
    SetFase(RX_CMD);
    ObjectFunctionsTimer.SetTimer(USB_RS485_ACTIV_WAIT_TIMER, 30);
  }
  return ExecutableCMD;
}
//*****************************************************************************
//*** TCmd_UE
//*****************************************************************************
ICmd* TCmd_UE::CMD_Execute(void* p1,void* p2, void* p3)
{
 Data_Params const   *ptr = &Configuration;
 TBuffer    *Buffer           = (TBuffer*)p2;
 BaseCMD->LastErrorIndex      = OK_IND;
 ICmd *ExecutableCMD          = BaseCMD;

  if( atoi( BaseCMD->Addr.c_str() ) == 0 )
  {
    this->ClearLastData();
    string AnswerState      = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);
    Buffer->DataLen = CreateCMD(Buffer, BaseCMD->LastCMD, AnswerState, BaseCMD->LastData);
    SetFase(TX_CMD);
  }
  else
  {
    ClearAlternateMessages();
    ClearMainAndCurrMessages();
    string Pass = ptr->PassWord485;
    PushToAlternateMessages(Pass, BaseCMD->Addr, "", false);
    ClearLastData();
    SetFase(RX_CMD);
    ObjectFunctionsTimer.SetTimer(USB_RS485_ACTIV_WAIT_TIMER, 5);
  }
  return ExecutableCMD;
}
//*****************************************************************************
//***TCmd_UnknownCMD
//*****************************************************************************
ICmd* TCmd485_UnknownCMD::CMD_Execute(void* p1,void* p2, void* p3)
{
 ICmd *ExecutableCMD      = BaseCMD;
    string Pass = "";
    string Addr = "";
    TCmdMessage *CurrMessage = GetCurrNeedCmdAnswerMessage();
    if(CurrMessage != NULL)
    {
      if(BaseCMD->LastCMD == "CfData")
      {
          CurrMessage->Data.clear();
          if(CurrMessage->DateTime != NULL) { delete CurrMessage->DateTime;}
          delete CurrMessage;
          SetCurrNeedCmdAnswerMessage(NULL);
          BaseCMD->ClearErrorCount();
      }
    }
    string AnswerState  = BaseCMD->LastState;
    //if(AnswerState == "" && BaseCMD->LastErrorIndex == OK_IND)
    //{ AnswerState = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);}
    PushToAlternateMessages( Pass, Addr, AnswerState);
    ClearLastData();
    SetFase(RX_CMD);
    return ExecutableCMD;
}
//*****************************************************************************
//***TCmd485_Data
//*****************************************************************************
ICmd* TCmd485_Data::CMD_Execute(void* p1,void* p2, void* p3)
{
 ICmd *ExecutableCMD          = BaseCMD;

  if(BaseCMD->LastData.size() > 0)
  {
    //string Pass = "";
    //string Addr = "";
    string AnswerState  = string((char*)ERROR_MASS[this->BaseCMD->LastErrorIndex]);

      int addr =  atoi( BaseCMD->Addr.c_str() );

      //Log::DEBUG("Controllers size="+toString(BaseCMD->Controllers.size()));
      map<Word, ICmdController*>::iterator it = BaseCMD->Controllers.find(addr);
      if( it == BaseCMD->Controllers.end())
    	  BaseCMD->Controllers[addr] = new ICmdController(addr);
      BaseCMD->Controllers[addr]->RcvCnt++;



      TRtcTime dt = SystemTime.GetTime();
      string dtStr = BaseCMD->LastDate+","+BaseCMD->LastTime+">>";
      for(int i = 0; i < BaseCMD->LastData.size(); i++)
      {
    	string type = BaseCMD->LastData[i];
    	vector<TEt> Equipments = GetTypes(type);
    	int size = Equipments.size();
    	if(size > 0)
    	{
    		//Log::DEBUG("Controllers addr="+toString((long)&BaseCMD->Controllers));
    		BaseCMD->Controllers[addr]->GetEquipmentsData(Equipments);
    		TEt et = Equipments[0];
    		if(et.EType == 41 && et.EName == 50 && addr == 2)
    		{
		        if(BaseCMD->OnGetDataFrom485_1 != NULL)
		        {//obrabotka dannih ot MKK
		          BaseCMD->OnGetDataFrom485_1(&et);
		        }
    			continue;
    		}
    		else if(et.EType == 50 && et.EName == 1 && addr == 2)
    		{
		        if(BaseCMD->OnGetDataFrom485_2 != NULL)
		        {//obrabotka dannih ot MKK
		          BaseCMD->OnGetDataFrom485_2(&et);
		        }
    			continue;
    		}
    		else if(et.EType == 61 && et.EName == 1 && addr == 2)
    		{
		        if(BaseCMD->OnGetDataFrom485_3 != NULL)
		        {//obrabotka dannih ot MKK
		          BaseCMD->OnGetDataFrom485_3(&et);
		        }
    			continue;
    		}
    		else if( (et.EType == 26 || et.EType == 0) && (addr == 11 || addr == 12 || addr == 13 || addr == 14))
    		{
		        if(BaseCMD->OnGetDataFrom485_4 != NULL)
		        {//obrabotka dannih ot MKAB
		          BaseCMD->OnGetDataFrom485_4(&et);
		        }
    			continue;
    		}
    	}
        string framStr = dtStr + BaseCMD->LastData[i];// + "a=" + BaseCMD->ID;
        if(framStr.size() > 0)
        {
        	FifoBuffer.push(framStr);
        }
        framStr.clear();
      }
      ClearLastData();
      PushToMainMessages(BaseCMD->ID, "CfData");
      SetFase(WAIT_MES);
  }
  else
  {
    SetFase(RX_CMD);
  }
  return ExecutableCMD;
}

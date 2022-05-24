/*
 * Classes.cpp
 *
 *  Created on: Sep 23, 2014
 *      Author: user
 */


#include <Classes.h>
#include <regex>

//=======================================
//*** TBuffer class *********************
//=======================================
Short TBuffer::Crc(Byte const * pcBlock, sWord len)
{
	Short crc = 0;

	if(len > 20000){
		Log::DEBUG("TBuffer::Crc !!!!big len = " + toString(len));
	}

	if(len > 0){
		while (len--){
		  crc += *pcBlock++;
		}
	}
	else{
		Log::DEBUG("TBuffer::Crc len = " + toString(len));
	}


    return crc;
}
//=======================================
Byte TBuffer::Crc8(Byte const * pcBlock, sWord len)
{
	Byte crc = 0;

//string tmp="";
//string l = toString(len);
	if(len > 0){
		while (len--){
			Byte ttt = *pcBlock++;
			//tmp += toString((int)ttt) + ",";
			crc += ttt;
		}
	}
    //Log::DEBUG("len ="+l+" crc8=["+tmp+"]");

    return crc;
}
//=======================================
void TBuffer::XOR_Crypt(Byte* in, Word inlen, string pass)
{
  for ( Word x = 0; x < inlen; x++)
  {
      in[x] = (Byte)( in[x] ^ pass[x%pass.length()] );

  }
}
//=======================================
void   TBuffer::ClrBUF(Byte *BUF, sWord Len)
{
	if(Len > 0){
	  for (Word i=0;i<Len;i++)
	  {
		BUF[i] = 0;
	  }
	}
}
//=======================================
string TBuffer::DateTimeToString(TRtcTime *datetime)
{
  return  string(
          WordToString(datetime->Day)+'.'+
          WordToString(datetime->Month)+'.'+
          WordToString(datetime->Year)+','+
          WordToString(datetime->Hour)+':'+
          WordToString(datetime->Minute)+':'+
          WordToString(datetime->Second)
                );

}
//=======================================
TRtcTime *TBuffer::StringToDateTime(string strdt)
{
vector<string> results;

  TBuffer::Split(strdt, "," , results);
  if(results.size() != 2){ return NULL;}
  string date = results[0];
  string time = results[1];

  results.clear();
  TBuffer::Split(date, "." , results);
  if(results.size() != 3){ return NULL;}

  TRtcTime dt;

  dt.Day    = atoi( results[0].c_str() );
  dt.Month  = atoi( results[1].c_str() );
  dt.Year   = atoi( results[2].c_str() );

  results.clear();
  TBuffer::Split(time, ":" , results);
  if(results.size() != 3){ return NULL;}
  dt.Hour    = atoi( results[0].c_str() );
  dt.Minute  = atoi( results[1].c_str() );
  dt.Second  = atoi( results[2].c_str() );

  TRtcTime *rdt = new TRtcTime();
  *rdt = dt;
  return rdt;
}
//=======================================
Word TBuffer::CopyString(Byte* DestStr, Byte* SrcStr)
{
  Word len = 0;
  while(*SrcStr)
  {
    *DestStr++ = *SrcStr++;
    len++;
  }
  *DestStr = 0;
  return len;
}
//=======================================
Word TBuffer::CopyBuffer(Byte* DestStr, Byte* SrcStr, Word Len)
{
	for(int i=0; i < Len; i++)
	{
		DestStr[i] = SrcStr[i];
	}
	return Len;
}

//=======================================
bool TBuffer::StrCmp(Byte const* Str1, Byte const* Str2)
{

  while(*Str1)
  {
    if(*Str1++ != *Str2++)
    {  return false;}
  }

  if(*Str2 != 0)
  {  return false;}
  return true;
}
//=======================================
void  TBuffer::Split(string &str, string separator, vector<string> &results, int Len)
{
    unsigned int found;
    found = str.find(separator);
    while(found != string::npos)
    {
        if(found > 0)
        {
          results.push_back(str.substr(0,found));
          if(Len > 0 && (int)results.size()>= Len)
          { return;}
        }
        str = str.substr(found+separator.size());
        found = str.find(separator);
    }
    if(str.length() > 0)
    { results.push_back(str);}
}
//=======================================
vector<string>  TBuffer::Split(string str, char sep)
{
	char rb[2] {sep, 0};
	string separator = string(rb);
	vector<string> results;
	results.clear();
    unsigned int found;
    found = str.find(separator);
    while(found != string::npos)
    {
        if(found > 0)
        {
          results.push_back(str.substr(0,found));
        }
        str = str.substr(found+separator.size());
        found = str.find(separator);
    }
    if(str.length() > 0)
    { results.push_back(str);}
    return results;
}
//=======================================
vector<string>  TBuffer::Split(string str, string separator)
{
	vector<string> results;
	results.clear();
    unsigned int found;
    found = str.find(separator);
    while(found != string::npos)
    {
        if(found > 0)
        {
          results.push_back(str.substr(0,found));
        }
        str = str.substr(found+separator.size());
        found = str.find(separator);
    }
    if(str.length() > 0)
    { results.push_back(str);}
    return results;
}
//=======================================
void  TBuffer::Split(Byte *BUFFER, int Size, string separator, vector<string> &results, int Len)
{
    int found;
    found = TBuffer::find(BUFFER, Size, separator);
    while( found != -1 && Size > 0)
    {

        if(found > 0)
        {
          string tmp = string((char*)BUFFER, found );
          results.push_back( tmp );
          if(Len > 0 && (int)results.size()>= Len)
          { return;}
        }
        BUFFER = (Byte*)( (Word)BUFFER+found+separator.size() );
        Size = Size - (found+separator.size());
        found = TBuffer::find(BUFFER, Size, separator);
    }
    if(Size > 0)
    { results.push_back( string((char*)BUFFER, Size ) );}
}
//=======================================
int  TBuffer::find_first_of(TBuffer *BUFFER, Byte simb)
{
  int returnVal = -1;

  for(int i=0; i < BUFFER->DataLen; i++)
  {
    if(BUFFER->BUF[i] == simb)
    {
      returnVal = i;
      break;
    }
  }

return returnVal;
}
//=======================================
int  TBuffer::find_first_of(Byte *BUFFER, Word Len, Byte simb)
{
  int returnVal = -1;

  for(Word i=0; i < Len; i++)
  {
    if(BUFFER[i] == simb)
    {
      returnVal = i;
      break;
    }
  }

return returnVal;
}
//=======================================
int  TBuffer::find(TBuffer *BUFFER, string str)
{
  int returnVal = -1;
  Byte simb = str.c_str()[0];

  for(int i=0; i < BUFFER->DataLen; i++)
  {
    if(BUFFER->BUF[i] == simb)
    {
      string tmp = string( (char*)&BUFFER->BUF[i], str.size() );
      if(tmp == str)
      {
        returnVal = i;
        break;
      }
    }
  }

return returnVal;
}
//=======================================
int  TBuffer::find(Byte *BUFFER, Word Len, string str)
{
  int returnVal = -1;
  Byte simb = str.c_str()[0];

  for(Word i=0; i < Len; i++)
  {
    if(BUFFER[i] == simb)
    {
      string tmp = string( (char*)&BUFFER[i], str.size() );
      if(tmp == str)
      {
        returnVal = i;
        break;
      }
    }
  }

return returnVal;
}
//=======================================
int  TBuffer::find_last_of(TBuffer *BUFFER, Byte simb)
{
  int returnVal = -1;

  for(int i=(BUFFER->DataLen-1); i > 0; i--)
  {
    if(BUFFER->BUF[i] == simb)
    {
      returnVal = i;
      break;
    }
  }

return returnVal;
}
//=======================================
Word TBuffer::clear_all_nulls(char *_ptr, Word _size)
{
        //
        char *ptrDst;
        char *ptrSrc;

        for(Word i = 0; i <= _size; i++)
        {
          ptrDst = &_ptr[i];
          if( *ptrDst < ' ')
          {
            ptrSrc = &_ptr[i+1];
            int rsz = (_size-i);
            for(int j = 0; j < rsz; j++)
            {
              ptrDst[j] = ptrSrc[j];
            }
            if(rsz > 0)
            {
              _size--;
              i--;
            }
          }
        }

        _ptr[_size] = 0;
        return _size;
}
//=======================================
//********** TDateTime class ************
//=======================================
//DWord 			TDateTime::GlobalSeconds;
//bool 			TDateTime::TimeSynchIsActual;
//pthread_mutex_t TDateTime::sych;
//=======================================
TDateTime::TDateTime()
{
    TRtcTime time;
      time.Year  = 2010;
      time.Month = 1;
      time.Day   = 1;
      time.Hour  = 0;
      time.Minute= 0;
      time.Second= 1;
      GlobalSeconds = 1;
      SetTime(time);

    TimeSynchIsActual=false;
}
//=======================================
TDateTime::TDateTime(TRtcTime time)
{
    SetTime(time);
}
//=======================================
void TDateTime::SetSynchAct(bool flg)
{
	pthread_mutex_lock(&sych);
      TimeSynchIsActual=flg;
    pthread_mutex_unlock(&sych);
}
//=======================================
bool TDateTime::GetSynchAct(void)
{
	pthread_mutex_lock(&sych);
    	bool ret =  TimeSynchIsActual;
    pthread_mutex_unlock(&sych);
    return ret;
}
//=======================================
TDateTime::~TDateTime()
{}
//=======================================
void TDateTime::SyncDateTime(string &date, string &time)
{
	TDateTime dt;
	vector<string> tmpvect;

	  TRtcTime DateTime;
	  TBuffer::Split(date,".",tmpvect);
	  if(tmpvect.size() != 3){ return;}
	  DateTime.Day    = atoi(tmpvect[0].c_str());
	  DateTime.Month  = atoi(tmpvect[1].c_str());
	  DateTime.Year   = atoi(tmpvect[2].c_str());
	  tmpvect.clear();

	  TBuffer::Split(time,":",tmpvect);
	  if(tmpvect.size() != 3){ return;}
	  DateTime.Hour     = atoi(tmpvect[0].c_str());
	  DateTime.Minute   = atoi(tmpvect[1].c_str());
	  DateTime.Second   = atoi(tmpvect[2].c_str());
	  dt.SetTime(DateTime);

	  long div = dt.GetGlobalSeconds() - GetGlobalSeconds();
	  if(div > 43200 || div < -43200)//12h (was 1 h then 8h)
	  {
		  Log::DEBUG("TDateTime::SetDateTime old="+ToString() +" GlobalSeconds="+toString(GlobalSeconds));
		  SetTime(DateTime);

		  //Log::DEBUG("TDateTime::SetDateTime new="+ToString() +" GlobalSeconds="+toString(GlobalSeconds));
		  //SetSystemTimeOfDay(DateTime);//set localtime of linux
	  }
	  else
	  {
		  long tdiv = GetTimeDiv();
		  if(tdiv > 300 || tdiv < -300)//5m
		  {
			  tm sync_time = Now();
			  SetTime(&sync_time);//from rtc
			  Log::DEBUG("TDateTime::SyncDateTime sync="+TimeToString(sync_time));
		  }
		  else
		  {
			  if(tdiv > 0)
				  IncSecond();
			  else if(tdiv < 0)
				  DecSecond();
		  }
	  }
}
//=======================================
long TDateTime::GetTimeDiv(void)
{

	Log::DEBUG("TDateTime::GetTimeDiv enter");
	tm time = Now();

	Log::DEBUG("TDateTime::GetTimeDiv Now="+ string(asctime(&time)));

	TRtcTime dt;
	dt.Day 		= time.tm_mday;
	dt.Month 	= time.tm_mon + 1;
	dt.Year     = time.tm_year + 1900;
	dt.Hour 	= time.tm_hour;
	dt.Minute 	= time.tm_min;
	dt.Second 	= time.tm_sec;


	//Log::DEBUG("TDateTime::GetTimeDiv before set time");
	TDateTime linuxtime;
	linuxtime.SetTime(dt);

	long ret = linuxtime.GetGlobalSeconds() - GetGlobalSeconds();
	//Log::DEBUG("TDateTime::GetTimeDiv after set time, diff="+toString(ret));
	return ret;
}

//=======================================
void TDateTime::SetDateTime(string &date, string &time)
{
	vector<string> tmpvect;
	  TRtcTime DateTime;
	  TBuffer::Split(date,".",tmpvect);
	  if(tmpvect.size() != 3){ return;}
	  DateTime.Day    = atoi(tmpvect[0].c_str());
	  DateTime.Month  = atoi(tmpvect[1].c_str());
	  DateTime.Year   = atoi(tmpvect[2].c_str());
	  tmpvect.clear();
	  TBuffer::Split(time,":",tmpvect);
	  if(tmpvect.size() != 3){ return;}
	  DateTime.Hour     = atoi(tmpvect[0].c_str());
	  DateTime.Minute   = atoi(tmpvect[1].c_str());
	  DateTime.Second   = atoi(tmpvect[2].c_str());
	  SetTime(DateTime);

}
//=======================================
void TDateTime::SetSystemTimeOfDay(TRtcTime &dt)
{
		struct tm time;
		time.tm_mday 	= dt.Day;
		time.tm_mon 	= dt.Month - 1;
		time.tm_year 	= dt.Year - 1900;
		time.tm_hour 	= dt.Hour;
		time.tm_min 	= dt.Minute;
		time.tm_sec 	= dt.Second;

		struct timeval tval;
		tval.tv_sec 	= mktime(&time);
		tval.tv_usec 	= 0;

		Log::INFO("SetSystemTimeOfDay:"+ string(asctime(&time)));

		tm n = Now();
		Log::INFO("SetSystemTimeOfDay Now: "+toString(n.tm_mday)+"."+toString(n.tm_mon+1)+"."+toString(n.tm_year+1900)+"T"+toString(n.tm_hour)+":"+toString(n.tm_min)+
				":"+toString(n.tm_sec)+"Z"+toString(n.tm_zone));//GlobalSeconds

		settimeofday(&tval, NULL);

		n = Now();
		Log::INFO("SetSystemTimeOfDay NewNow: "+toString(n.tm_mday)+"."+toString(n.tm_mon+1)+"."+toString(n.tm_year+1900)+"T"+toString(n.tm_hour)+":"+toString(n.tm_min)+
				":"+toString(n.tm_sec)+"Z"+toString(n.tm_zone));//GlobalSeconds
}
//=======================================
TRtcTime TDateTime::GetTime(void)
{
	//pthread_mutex_lock(&sych);
		TRtcTime ret = DateTime;
    //pthread_mutex_unlock(&sych);
    return ret;
}
//=======================================
struct tm TDateTime::Now(void)
{
	time_t seconds = time(NULL);
	struct tm *timeinfo =  localtime(&seconds);
	//tm ret = *timeinfo;
	//delete timeinfo;

    return *timeinfo;
}
//=======================================
struct tm TDateTime::NowUtc(void)
{
	time_t seconds = time(NULL);
	struct tm *timeinfo =  gmtime(&seconds);
	//tm ret = *timeinfo;
	//delete timeinfo;

    return *timeinfo;
}

//=======================================
void TDateTime::SetTime(TRtcTime dateTime)
{
	//pthread_mutex_lock(&sych);
      DateTime      = dateTime;
      GlobalSeconds = DateTime.Hour*3600L+DateTime.Minute*60L+DateTime.Second;
    //pthread_mutex_unlock(&sych);
}
//=======================================
void TDateTime::SetTime(struct tm *dateTime)
{
	//pthread_mutex_lock(&sych);
      DateTime.Second      = dateTime->tm_sec;
      DateTime.Minute      = dateTime->tm_min;
      DateTime.Hour        = dateTime->tm_hour;
      DateTime.Day         = dateTime->tm_mday;
      DateTime.Month      = dateTime->tm_mon + 1;
      DateTime.Year      = dateTime->tm_year+1900;
      GlobalSeconds = DateTime.Hour*3600L+DateTime.Minute*60L+DateTime.Second;
    //pthread_mutex_unlock(&sych);
}
//=======================================
DWord TDateTime::GetGlobalSeconds(void)
{
	//pthread_mutex_lock(&sych);
    	DWord ret =  GlobalSeconds;
    //pthread_mutex_unlock(&sych);
    return ret;
}
//=======================================
void TDateTime::DecSecond(void)
{
		if(--DateTime.Second < 0)
		{
		  DateTime.Second = 59;
		  DecMinute();
		}
		GlobalSeconds = DateTime.Hour*3600L+DateTime.Minute*60L+DateTime.Second;
}
//=======================================
void TDateTime::DecMinute(void)
{
    if(--DateTime.Minute < 0)
    {
      DateTime.Minute = 59;
      DecHour();
    }
}
//=======================================
void TDateTime::DecHour(void)
{
    if(--DateTime.Hour < 0)
    {
      DateTime.Hour = 23;
      DecDay();
    }
}
//=======================================
void TDateTime::DecDay(void)
{
   Byte DaysInMonth = 31;

    if(--DateTime.Day <= 0 )
    {
      DecMonth();

      if(DateTime.Month == 4 || DateTime.Month == 6 || DateTime.Month == 9 || DateTime.Month == 11 )
      	DaysInMonth = 30;
      else if( DateTime.Month == 2)
      {
        if(IsLeapYear( DateTime.Year ) == true)
      	  DaysInMonth = 29;
        else
      	  DaysInMonth = 28;
      }
      DateTime.Day = DaysInMonth;
    }
}
//=======================================
void TDateTime::DecMonth(void)
{
    if(--DateTime.Month <= 0)
    {
      DateTime.Month = 12;
      DateTime.Year--;
    }
}
//=======================================
void TDateTime::IncSecond(void)
{
	//pthread_mutex_lock(&sych);
		if(++DateTime.Second >= 60)
		{
		  DateTime.Second = 0;
		  IncMinute();
		}
		//if(GlobalSeconds++ > 86400L)//
		//{ GlobalSeconds = DateTime.Hour*3600L+DateTime.Minute*60L+DateTime.Second;}
		GlobalSeconds = DateTime.Hour*3600L+DateTime.Minute*60L+DateTime.Second;
		//Log::DEBUG("GlobalSeconds="+toString(GlobalSeconds));
    //pthread_mutex_unlock(&sych);
}
//=======================================
void TDateTime::IncMinute(void)
{
    if(++DateTime.Minute >= 60)
    {
      DateTime.Minute = 0;
      IncHour();
    }
}
//=======================================
void TDateTime::IncHour(void)
{
    if(++DateTime.Hour >= 24)
    {
      DateTime.Hour = 0;
      IncDay();
    }
}
//=======================================
void TDateTime::IncDay(void)
{
   Byte DaysInMonth = 31;
    if(DateTime.Month == 4 || DateTime.Month == 6 || DateTime.Month == 9 || DateTime.Month == 11 )
    { DaysInMonth = 30;}
    else if( DateTime.Month == 2)
    {
      if(IsLeapYear( DateTime.Year ) == true)
      { DaysInMonth = 29;}
      else
      { DaysInMonth = 28;}
    }
    if(++DateTime.Day > DaysInMonth)
    {
      DateTime.Day = 1;
      IncMonth();
    }
}
//=======================================
void TDateTime::IncMonth(void)
{
    if(++DateTime.Month > 12)
    {
      DateTime.Month = 1;
      DateTime.Year++;
    }
}
//=======================================
bool TDateTime::IsLeapYear(Word Year)
{
    if( (Year%4 == 0) && ((Year % 100 != 0) || (Year % 400 == 0)) )
    {  return true;} //
    else
    {  return false;} //
}
//=======================================
string TDateTime::ToString()
{
	return toString(DateTime.Day)+"."+toString(DateTime.Month)+"."+toString(DateTime.Year)+"T"+toString(DateTime.Hour)+":"+toString(DateTime.Minute )+":"+toString(DateTime.Second);
}
//*****************************************************************************
//*** TGetVersion
//*****************************************************************************
TGetVersion::TGetVersion():TFastTimer(1,&SecCount),Modify(false),Period(600)
{
  SetTimer(PERIUD_TIMER, 5);
  FirstInitFlg 		= true;
  simnumber 		= "";
  interfaceaddr1 	= "";
  interfaceaddr2 	= "";
  ModifyTime 		= Period;
  NeedFirstFlg 		= true;
}
//=============================================================================
TGetVersion::~TGetVersion()
{
}
//=============================================================================
/*void TGetVersion::VER_DetectStates( void  )
    {
      bool flag = false;
        if( GetTimValue(PERIUD_TIMER) <= 0 )
        {
          if(FirstInitFlg == true)
          {
            FirstInitFlg = false;
            flag     = true;
          }
          //Log::DEBUG(toString(SystemTime.GetGlobalSeconds()));
          if( SystemTime.GetGlobalSeconds()%Period  == 0)
          {
            flag     = true;
            SetTimer( PERIUD_TIMER, 2 );

          }
          else
          {
        	  SetTimer( PERIUD_TIMER, 1 );
          }

        }
        if( flag == true)
        {
        	//Log::DEBUG("flag == true ");
          DateTime = SystemTime.GetTime();
          Modify = true;
        }
    }*/

void TGetVersion::VER_DetectStates( void  )
{
	bool flag = false;
	long long tim = GetTimValue(PERIUD_TIMER);
	if( tim <= 0 )
	{
	    DWord tim = SystemTime.GetGlobalSeconds();
	    if(FirstInitFlg == true)
	    {
	    		Log::DEBUG("VER_DetectStates FirstInitFlg="+toString(FirstInitFlg));
	    		FirstInitFlg = false;
	    		flag     = true;
	    		ModifyTime = ((DWord)( (DWord)(tim / Period) + 1)) * Period;
	    		if(ModifyTime > MAX_SECOND_IN_DAY)
	    			ModifyTime = Period;
	    }
	    else
	    {
	    		long err = ModifyTime-tim;
	    		//Log::DEBUG("VER_DetectStates err="+toString(err)+" ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
	    		if( (ModifyTime <= tim) || (err > Period*2) )
	    		{
	    			flag     = true;
	    			ModifyTime = ((DWord)((DWord)(tim / Period) + 1)) * Period;
	    			if(ModifyTime > MAX_SECOND_IN_DAY)
	    				ModifyTime = Period;
	    		}
	    }
		if( flag == true)
		{
			  Log::DEBUG("VER_DetectStates ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
	          DateTime = SystemTime.GetTime();
	          Modify = true;
			  SetTimer( PERIUD_TIMER, 10 );//2!!!  SecCount
		}
		else
		{
			  SetTimer( PERIUD_TIMER, 1 );//1!!!  SecCount
		}
	}
}

//=============================================================================
void TGetVersion::VER_CreateMessageCMD( void *Par )
    {
      if( Par != NULL)
      {
        if(ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0)
        {
            if( Modify == true )
            {
            	  Log::DEBUG( "[TGetVersion::VER_CreateMessageCMD]");
                  string framStr = TBuffer::DateTimeToString( &this->DateTime )+">>"+"0,1,\r\n>,99,0,"+
                		  FloatToString(VERSION)+"," + toString(Configuration.SerialNumber);

                  if(NeedFirstFlg)
                  {
					  if(Configuration.AsduNumber.size() > 0){
						framStr +=","+Configuration.AsduNumber;
					  }
					  if(Configuration.MkddNumber.size() > 0){
						framStr +=","+Configuration.MkddNumber;
					  }
					  if(Configuration.Installer.size() > 0)
					  {
						  ////////////////
						  Byte bf[2] {0,0};
						  string sended = "";
						  for(int i = 0; i < Configuration.Installer.size(); i++){
							ByteToHex(bf, Configuration.Installer.c_str()[i]);
							sended += string((char*)bf, 2)+":";
						  }
						  Log::DEBUG( "[TGetVersion::VER_CreateMessageCMD] Configuration.Installer= [" + sended +"] len="+toString(Configuration.Installer.size()));
						  //////////////////////////
						  if(!isNullOrWhiteSpace(sended)){
							if(sended.c_str()[sended.size()-1] == ':')
								sended.erase(sended.size()-1, 1);
							framStr+=",\r\n>,99,2,"+ sended;
						  }
					  }
					  if(!isNullOrWhiteSpace(simnumber)){
						  framStr+=",\r\n>,99,3,"+ simnumber;
					  }
					  else{
						  simnumber = GetSimNumber();
						  Log::DEBUG("GetSimNumber simnumber=["+simnumber+"] size="+toString(simnumber.size()));
						  if(!isNullOrWhiteSpace(simnumber))
							  framStr+=",\r\n>,99,3,"+ simnumber;
					  }
					  if(!isNullOrWhiteSpace(Configuration.Commissioning)){
						  framStr+=",\r\n>,99,4,"+ Configuration.Commissioning;
					  }
                  }

                  //////////////
                  interfaceaddr1 = GetInterfaceAddr(GPRS_PORT);
                  if(!isNullOrWhiteSpace(interfaceaddr1))
                	  framStr+=",\r\n>,99,6,"+ interfaceaddr1;

                  interfaceaddr2 = GetInterfaceAddr(ETH_PORT);
                  if(!isNullOrWhiteSpace(interfaceaddr2))
                	  framStr+=",\r\n>,99,7,"+ interfaceaddr2;
                  //////////////


                  framStr+=",\r\n>,99,5,0";
                  if(framStr.size() > 0)
                  {
                	  framStr += ">>NeedCmdAnswer=0";
                	  TFifo<string> *framFifo = (TFifo<string> *)Par;
                	  framFifo->push( framStr );
                  }

                  Log::DEBUG( "[TGetVersion::VER_CreateMessageCMD] framStr="+framStr);
                  framStr.clear();
                  Modify = false;
                  NeedFirstFlg = false;
            }
        }
      }
    }
//=============================================================================
void TGetVersion::Init(Word period)
{
  Period        = period;
}
//=============================================================================
string TGetVersion::GetSimNumber()
{
	string ret = "";
	if(Configuration.SmsPort.size() > 0)
	{
		string sim = sendToConsole("ls "+Configuration.SmsPort);
		if(Configuration.SmsPort == sim)
		{
			string cmd = "gcom -d "+sim;
			Log::DEBUG("TGetVersion::GetSimNumber cmd=["+cmd+"]");
			string answer = sendToConsole(cmd);
			Log::DEBUG("TGetVersion::GetSimNumber answer=["+answer+"]");

			sleep(5);

			sendToConsole("echo '#!/bin/sh' > /tmp/simnumber.sh");
			sendToConsole("echo 'rm -f /tmp/sig' >> /tmp/simnumber.sh");
			sendToConsole("echo 'cat " + sim + " > /tmp/sig &' >> /tmp/simnumber.sh");
			sendToConsole("echo 'sleep 1' >> /tmp/simnumber.sh");
			sendToConsole("echo 'echo -e \"AT+CIMI\\r\\n\" > " + sim +"' >> /tmp/simnumber.sh");
			sendToConsole("echo 'sleep 1 && killall -9 cat' >> /tmp/simnumber.sh");
			sendToConsole("echo 'r=`cat /tmp/sig | sed /250/!d | cut -c 4-15`' >> /tmp/simnumber.sh");
			sendToConsole("echo 'echo $r' >> /tmp/simnumber.sh");
			sendToConsole("echo 'exit 0' >> /tmp/simnumber.sh");
			sendToConsole("chmod 755 /tmp/simnumber.sh");
			ret = sendToConsole("/tmp/simnumber.sh");
		}
		else
		{
			Log::DEBUG("Do not need GetSimNumber, wrong SmsPort=["+Configuration.SmsPort+"]");
		}
	}
	if(ret.size()> MAX_SIM_NUMBER_LEN){
		ret = ret.substr(0, MAX_SIM_NUMBER_LEN);
	}
	return ret;
}
string TGetVersion::GetInterfaceAddr(string name)
{
	string ret = "";
	if(name.size() > 0)
	{
		Log::DEBUG("GetInterfaceAddr. Start detect IP for interface name=["+ name + "]");
		string strto = "ip -4 addr | grep inet | grep " + name + " | grep -E -o '(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?).(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)'";

		Log::DEBUG("GetInterfaceAddr. sendToConsole=["+ strto + "]");
		string tmp = sendToConsole(strto);
		Log::DEBUG("GetInterfaceAddr. Tmp result=["+ tmp + "]");


		if(tmp.size() > 0)
		{
			  ///////////////////////////
			  Byte bf[2] {0,0};
			  string recv = "";
			  for(int i = 0; i < tmp.size(); i++)
			  {
				  ByteToHex(bf, tmp.c_str()[i]);
				  recv += string((char*)bf, 2)+" ";
			  }
			  Log::DEBUG("GetInterfaceAddr. byte result=["+ recv + "]");
			  ///////////////////////////
			  ret = tmp;
		}
	}
	else{
		Log::DEBUG("GetInterfaceAddr. Wrong interface name=["+ name + "]");
	}
	return ret;
}



//*****************************************************************************
//***TGetCurrentDataTask
TGetCurrentDataTask::TGetCurrentDataTask( void ):TFastTimer(1,&SecCount)
{
    //Config = ListConfig;
	SetTimer(PERIUD_TIMER, 5);
    MainMessages  = NULL;
    Controllers = NULL;
}
//=======================================
TGetCurrentDataTask::~TGetCurrentDataTask()
{ }
//=======================================
void TGetCurrentDataTask::Init(TaskListSettings ListConfig)
{
  if( ListConfig.mainMessages != NULL)
  {
    MainMessages 		= (sync_deque<TCmdMessage*> *)ListConfig.mainMessages;
  }
  Config 				= ListConfig;
}
//=======================================
int TGetCurrentDataTask::AddrCount(void)
{
    return Config.DeviceList.size();
}
//=======================================
DeviceOnLineSettings* TGetCurrentDataTask::GetAddress( int ix )
{
    int count = AddrCount();
    if( count > 0)
    {
        if( ix >= count)
        	return &Config.DeviceList[count-1];
        else
        	return &Config.DeviceList[ix];
    }
    else
    	return NULL;
}
//=======================================
void TGetCurrentDataTask::FASE_PullingRS_485( void )
{
  if( MainMessages != NULL)
  {
    if( MainMessages->empty() == true  &&
    		ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0 )
    {
      for(int i = 0; i < AddrCount(); i++)
      {
    	  DeviceOnLineSettings* dev = GetAddress(i);
    	  if(dev != NULL)
    	  {
    		  int Val = dev->address;

			  TCmdMessage *NewMessage = new TCmdMessage();
			  NewMessage->CMD = "Data";
			  NewMessage->Password    = Config.PassWord;
			  NewMessage->Addr        = WordToString(Val, 2);
			  MainMessages->push_back(NewMessage);
			  //Log::DEBUG("pulling: "+NewMessage->Addr);


			  if(dev->AckCnt++ > 1000)
			  {
				  //Log::DEBUG("FASE_PullingRS_485: clear AckCnt");
				  dev->AckCnt = 1;
				  if(Controllers != NULL)
				  {
					  for (auto it = Controllers->begin(); it != Controllers->end(); ++it)
					  {
						  if(it->second != NULL)
						  {
							  it->second->AckCnt=0;
							  it->second->RcvCnt=0;
						  }
					  }
				  }
			  }
    	  }
      }
      SetTimer(PERIUD_TIMER, 1);
    }
  }
}
//*****************************************************************************
//*** IAnalogDatchik
//*****************************************************************************
TAnalogDatchik::TAnalogDatchik(Byte Pt, Byte Pn, bool enable, bool IsVisible)
:IDatchik(Pt, Pn)
{
	  Type = 3;
	  Enable = enable;
	  Visible = IsVisible;
	  MaxPorog = 100.0;
	  MinPorog = -1;
	  Params.push_back(&Value);

	  Multyplier =0.1;
	  errCnt = 0;
	  MinError = -99.0;
	  MaxError = 199.0;
	  gisteresis = 1.0;
}
//=======================================

TAnalogDatchik::TAnalogDatchik(Byte Pt, Byte Pn, float max, float min, float multy, float def, float maxerr, float minerr, bool enable, bool IsVisible)
:TAnalogDatchik(Pt, Pn, enable, IsVisible)
{
	  //Type = 3;//in TAnalogDatchik
	  Multyplier =multy;
	  MaxPorog = max;
	  MinPorog = min;

	  MinError = minerr;
	  MaxError = maxerr;

	  if(MaxPorog >= MaxError)
		  MaxError = MaxPorog + 1;

	  if(MinPorog <= MinError)
		  MinError = MinPorog - 1;

	  Value.ClrDivider();
	  Value.CalcMinAverMax(def);
	  Value.ClrDivider();
}
//=======================================
TAnalogDatchik::~TAnalogDatchik()
{

}
//=======================================
string TAnalogDatchik::GetValueString(void)
{
	string ReturnStr = "";
	if(Visible && Enable)
	{
		ReturnStr = IParams::GetValueString();
		ReturnStr += FloatToString(MinPorog, 0, 1)+',';
		ReturnStr += FloatToString(MaxPorog, 0, 1)+',';
		ReturnStr += FloatToString(MaxPorog, 0, 1)+',';
		ReturnStr += FloatToString(Multyplier, 0, 1)+',';
		ReturnStr += toString(Visible)+',';
		ReturnStr += toString(Enable)+',';
	}
    return ReturnStr;
}
//=======================================
string TAnalogDatchik::GetSimpleValueString(void)
{
	string ret = "";
	if(Visible && Enable)
	{
		ret += "{";
		  ret += "\"Pt\":"+toString((int)Pt.Ptype)+",";
		  ret += "\"Pn\":"+toString((int)Pt.Pname)+",";
		  ret += "\"Value\":"+toString((int)(Value.Value/Multyplier)); //*10);
		ret+="},";
	}
    return ret;
}
//=======================================
string TAnalogDatchik::GetTinyValueString(void)
{
	//Log::DEBUG("TAnalogDatchik::GetTinyValueString");

    return toString(Value.Value);
}
//=======================================
DETECTING_RESULT  TAnalogDatchik::DetectPorogs(float Value)
{
	//Log::DEBUG("IAnalogDatchik::DetectPorogs(float Value) newVal="+toString(Value));
//GISTERESIS!!!!!

	if(Value >= MaxError || Value <= MinError)
		return NO_RESULT;

	float max = MaxPorog;
	float min = MinPorog;

	switch(StateValue)
	{
	case MAX_DETECTED:
		max-=gisteresis;
		break;

	case MIN_DETECTED:
		min+=gisteresis;
		break;
	}

	if(Value >= max)
		return MAX_DETECTED;
	if(Value <= min)
		return MIN_DETECTED;
	return NO_RESULT;
}
//=======================================
void TAnalogDatchik::CalculateValue(float newVal)
{
	//Log::DEBUG("IAnalogDatchik::CalculateValue(float newVal) newVal="+toString(newVal));
	if( !IsWrong(newVal) )
	{
		Value.CalcMinAverMax(newVal);
		errCnt = 0;
	}
	else
	{
		if(errCnt++ >= 5)
		{
			Log::DEBUG("IAnalogDatchik::CalculateValue IsWrong=true newVal="+toString(newVal)+" errCnt="+toString(errCnt));
			errCnt = 0;
			if(newVal >= MaxError)
				newVal = MaxError;
			else if(newVal <= MinError)
				newVal = MinError;

			Value.ClrDivider();
			Value.CalcMinAverMax(newVal);
			Value.ClrDivider();
		}
	}
}
//=======================================
bool TAnalogDatchik::IsWrong(float val)
{
	return val >= MaxError || val <= MinError;
}

//=======================================
bool TAnalogDatchik::IsWrong(void*  ptrToval)
{
	float val = *((float*)ptrToval);
	bool ret = IsWrong(val);

	//Log::DEBUG("TAnalogDatchik::IsWrong(ptr) val="+toString(val)+ " IsWrong="+toString((int)ret));

	return ret;
}

//=======================================
//*****************************************************************************
//*** TDiscreteDatchik
//*****************************************************************************
TDiscreteDatchik::TDiscreteDatchik(Byte Pt, Byte Pn, bool inversia, bool enable, bool IsVisible)
:IDatchik(Pt, Pn)
{
	  Enable 	= enable;
	  Visible = IsVisible;
	  MaxPorog 	= 1;
	  MinPorog 	= 0;
	  Value 	= false;
	  Inversia 	= inversia;
	  Type 		= 2;
}
//=======================================
TDiscreteDatchik::~TDiscreteDatchik()
{

}
//=======================================
string TDiscreteDatchik::GetValueString(void)
{
	string ReturnStr = "";
	if(Visible && Enable){
	   ReturnStr = "\r\n>,"+
	    	WordToString(Pt.Ptype)+','+
	        WordToString(Pt.Pname)+',';
	   ReturnStr += toString(Value)+',';
	}
    return ReturnStr;
}
//=======================================
string TDiscreteDatchik::GetSimpleValueString(void)
{
	string ret = "";
	if(Visible && Enable){
		ret += "{";
		  ret += "\"Pt\":"+toString((int)Pt.Ptype)+",";
		  ret += "\"Pn\":"+toString((int)Pt.Pname)+",";
		  ret += "\"Value\":"+toString((int)Value);
		ret+="},";
	}
    return ret;
}
//=======================================
string TDiscreteDatchik::GetTinyValueString(void)
{
    return Value ? "1": "0";
}
//=======================================
DETECTING_RESULT  TDiscreteDatchik::DetectPorogs(bool Value)
{
	//Log::DEBUG("IAnalogDatchik::DetectPorogs(float Value) newVal="+toString(Value));
	if(Value >= MaxPorog)
		return MAX_DETECTED;
	if(Value <= MinPorog)
		return MIN_DETECTED;
	return NO_RESULT;
}
//=======================================
void TDiscreteDatchik::CalculateValue(bool newVal)
{
	Value = newVal;
}

//*****************************************************************************
//*** TConstantDatchik
//*****************************************************************************
TConstantDatchik::TConstantDatchik(Byte Pt, Byte Pn, bool enable, bool IsVisible)
:IDatchik(Pt, Pn)
{
	  Type = 1;
	  Enable = enable;
	  Visible = IsVisible;
	  Multyplier = 1;
	  Value 	= 0;
	  MinError = -32760.0;
	  MaxError = 32760.0;
	  errCnt = 0;
}
//=======================================
TConstantDatchik::TConstantDatchik(Byte Pt, Byte Pn, float multy, float maxerr, float minerr, bool enable, bool IsVisible)
:TConstantDatchik(Pt, Pn, enable, IsVisible)
{
	  MinError = minerr;
	  MaxError = maxerr;
	  Multyplier = multy;
}
//=======================================
TConstantDatchik::~TConstantDatchik()
{

}
//=======================================
string TConstantDatchik::GetValueString(void)
{
	string ReturnStr = "";
	if(Visible && Enable){
	   ReturnStr = "\r\n>,"+
	    	WordToString(Pt.Ptype)+','+
	        WordToString(Pt.Pname)+',';
	   ReturnStr += (Multyplier == 1 ? toString((int)Value): toString(Value))  +',';
	}
    return ReturnStr;
}
//=======================================
string TConstantDatchik::GetSimpleValueString(void)
{
	string ret = "";
	if(Visible && Enable)
	{
		ret += "{";
		  ret += "\"Pt\":"+toString((int)Pt.Ptype)+",";
		  ret += "\"Pn\":"+toString((int)Pt.Pname)+",";
		  ret += "\"Value\":"+toString((int)(Value/Multyplier));
		ret+="},";
	}
    return ret;
}
//=======================================
string TConstantDatchik::GetTinyValueString(void)
{
	return toString(Value);
}
//=======================================
void TConstantDatchik::CalculateValue(float newVal)
{
	//Log::DEBUG("TConstantDatchik::CalculateValue(float newVal) newVal="+toString(newVal));
	if( !IsWrong(newVal) )
	{
		Value = newVal;
		errCnt = 0;
	}
	else
	{
		if(errCnt++ >= 5)
		{
			Log::DEBUG("TConstantDatchik::CalculateValue IsWrong=true newVal="+toString(newVal)+" errCnt="+toString(errCnt));

			errCnt = 0;
			if(newVal >= MaxError)
				newVal = MaxError;
			else if(newVal <= MinError)
				newVal = MinError;
			Value = newVal;
		}
	}
}
//=======================================
bool TConstantDatchik::IsWrong(float val)
{
	return val >= MaxError || val <= MinError;
}

//=======================================
bool TConstantDatchik::IsWrong(void*  ptrToval)
{
	float val = *((float*)ptrToval);
	bool ret = IsWrong(val);
	return ret;
}

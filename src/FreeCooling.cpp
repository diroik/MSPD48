/*
 * FreeCooling.cpp
 *
 *  Created on: Oct 25, 2014
 *      Author: user
 */

#include <FreeCooling.h>
//*****************************************************************************
//***
//*****************************************************************************
//=============================================================================
Ifree::Ifree(Byte Etype, Byte Ename, Byte adr):TFastTimer(2, &MilSecCount),
									Addr(adr), Et(Etype), En(Ename), Modify(false),
									IsEnable(false),Alarms(5,200,1), Warnings(5,200,2),
									PointsSteps(230, 1, 4),SetPoint(231, 1, 1)
{
	Log::DEBUG("Ifree");

	State 		= 0;
	HState      = 0;

	freemngptr  = NULL;
	Type = 0;
	SerialNo = "0";
	ErrCnt = 0;
	AckCnt = 0;
	AdvState = 0;
	HAdvState = 0;
    Customer = "";
	CU_No = "";
	DataLen = 0;
	modbus_cmd_size = 0;
}
//=============================================================================
Ifree::Ifree(Byte Ename, Byte adr):TFastTimer(2,&MilSecCount),
									Addr(adr), Et(29), En(Ename), Modify(false),
									IsEnable(false),
									Alarms(5,200,1),Warnings(5,200,2),
									PointsSteps(230, 1, 4), SetPoint(231, 1, 1)
{
	State 		= 0;
	HState      = 0;

	freemngptr  = NULL;
	Type = 0;
	SerialNo = "0";
	ErrCnt = 0;
	AckCnt = 0;
	AdvState = 0;
	HAdvState = 0;
    Customer = "";
	CU_No = "";
	DataLen = 0;
	modbus_cmd_size = 0;
}
//=============================================================================
Ifree::Ifree(Byte Ename, Byte adr, TFreeCoolingManager* fp):TFastTimer(2,&MilSecCount),
									Addr(adr), Et(29), En(Ename), Modify(false),
									IsEnable(false),Alarms(5,200,1),Warnings(5,200,2),
									PointsSteps(230, 1, 4), SetPoint(231, 1, 1)
{
	State 		= 0;
	HState      = 0;

	freemngptr  = fp;
	Type = 0;
	ErrCnt = 0;
	AckCnt = 0;
	AdvState = 0;
	HAdvState = 0;
    Customer = "";
	CU_No = "";
	DataLen = 0;
	modbus_cmd_size = 0;
}
//=============================================================================
Ifree::~Ifree()
{

}
//=============================================================================
void Ifree::SetPeriod(Word Period)
{
  //UE.Period=U.Period= Period;
}
//=============================================================================
string Ifree::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	/*ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(RoomTemp.Aver, 0, 2)+','+FloatToString(RoomTemp.Min, 0, 2)+','+
			FloatToString(RoomTemp.Max, 0, 2)+',';

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';
	ReturnString += FloatToString(ExtTemp.Aver, 0, 2)+','+FloatToString(ExtTemp.Min, 0, 2)+','+
			FloatToString(ExtTemp.Max, 0, 2)+',';

	ReturnString +=Alarms.GetValueString();
	ReturnString +=Warnings.GetValueString();

	ReturnString += "\r\n>,201,1,"+ WordToString(State)+',';


	RoomTemp.ClrDivider();
	ExtTemp.ClrDivider();*/
							  //UE.GetValueString()+
								//U.GetValueString();
	return ReturnString;
}
//=============================================================================
string Ifree::GetStringCfgValue(void)
{
	 string ReturnString = "";
	 return ReturnString;
}
//=============================================================================
sWord  Ifree::SendData(IPort* port, Byte *Buf, Word Len)
{
	return port->Send(Buf, Len);
}

//=============================================================================
sWord   Ifree::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;
	//sWord bytes = 0;
	Word datalen = MaxLen;

	Byte rb[1] {0};
    int bytes;
    do
    {
        bytes = port->Recv( rb, 1);
		if(bytes == 1)
		{//ok
			Buf[RecvLen++] = rb[0];
			if(RecvLen >= 3)
			{
				//Log::DEBUG("RecvData RecvLen="+toString(RecvLen));
				datalen = Buf[2] + 4;
			}
		}
		else
		{
			//Log::DEBUG("RecvData bytes="+toString(bytes));
			usleep(30000);
			break;
		}
    }
    while ( RecvLen < datalen );

	//RecvLen = port->Recv( Buf, MaxLen );
	/*
	bytes = port->Recv( Buf, MaxLen );
	if(bytes >= 4 )
	{
		if(Buf[0] == 0x01)
		{
			RecvLen=bytes;
		}
	}*/
	return RecvLen;
}
//=============================================================================
bool Ifree::ParsingAnswer( Byte *BUF, Word Len )
{
	if(Len < 5) return false;
	Byte MessCRC = BUF[Len-1];
	Byte CalcCRC = TBuffer::Crc8( BUF, Len-1);

	Log::DEBUG("Len="+toString(Len)+" Iibp::ParsingAnswer CalcCRC="+toString((int)CalcCRC) + " MessCRC="+toString((int)MessCRC) );
    //////////////////////////



	/*
   		  Byte bf[2] {0,0};
    		  string recv = "";
    		  for(int i = 0; i < Len; i++)
    		  {
    			  ByteToHex(bf, Buffer[i]);
    			  recv += "0x"+string((char*)bf, 2)+" ";
    		  }
    Log::DEBUG( "[TFreeCoolingManager] ParsingAnswer: [" + recv +"] len="+toString(Len) );*/


	return MessCRC == CalcCRC;
}

//=============================================================================
void Ifree::transportData(void *data, void *ptr)
{
	void *ret = NULL;
	try
	{
		if(data != NULL && ptr != NULL)
		{
			IModbusRegisterList *curr = (IModbusRegisterList *)data;
			Ifree			*_this = (Ifree*)ptr;
			Log::DEBUG( "[Ifree::transportData] fun="+ toString(curr->fun));
			_this->DataLen = 0;

			Bshort tmp;
			Word len = 0;
			tmp.Data_s = curr->reg;
			_this->Buffer[len++] = curr->fun;
			_this->Buffer[len++] = tmp.Data_b[0];
			_this->Buffer[len++] = tmp.Data_b[1];

			if(curr->fun == 3 || curr->fun == 4)
			{
				vector<short> *data = (vector<short> *)curr->GetData();
				int size = data->size();

				Log::DEBUG( "[Ifree::transportData] size="+ toString(size));
				for(int i = 0; i < size; i++)
				{
					tmp.Data_s = data->operator [](i);
					_this->Buffer[len++] = tmp.Data_b[0];
					_this->Buffer[len++] = tmp.Data_b[1];

					Log::INFO("[Ifree::transportData] " + toString(curr->reg+i) + string("=") + toString( tmp.Data_s ));
				}
				_this->DataLen = len;
			}
			else if(curr->fun == 6){
				//_this->Modify = true;
			}
			else if(curr->fun == 1 || curr->fun == 2)
			{
					vector<Byte> *data = (vector<Byte> *)curr->GetData();
					int size = data->size();
					for(int i = 0; i < size; i++){
						_this->Buffer[len++] = data->operator [](i);
					}
					_this->DataLen = len;
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "[Ifree::transportData] "+string(e.what()) );
	}
}
//*****************************************************************************
//***
//*****************************************************************************
Shtultz::Shtultz(Byte Ename, Byte adr):Ifree(Ename, adr)
		//,Alarms(5,200,1), Warnings(5,200,2)
{
	screenNumber.Data_s = 0;
	newScreenNumber.Data_s = 0;
	scrFace 	= 0;
	scrSubFace 	= 0;
	hScrSubFace = 0;
	hasWarn 	= 0;
	hasAlarm 	= 0;


	Screen.push_back("");
	Screen.push_back("");
	Screen.push_back("");
	Screen.push_back("");

	OldScreen.push_back("");
	OldScreen.push_back("");
	OldScreen.push_back("");
	OldScreen.push_back("");

	ScenarioIndex = 0;

	Scenario* scen = new Scenario("DefaultData");
	scen->Steps.push_back(new ScenarioStep(1,1));
	scen->Steps.push_back(new ScenarioStep(1,2));
	scen->Steps.push_back(new ScenarioStep(1,3));
	scen->Steps.push_back(new ScenarioStep(1,4));
	scen->Steps.push_back(new ScenarioStep(1,5));
	scen->Steps.push_back(new ScenarioStep(1,6));
	scen->Steps.push_back(new ScenarioStep(1,7));
	ScenarioList.push_back(scen);

	scen = new Scenario("PassWord");
	scen->Steps.push_back(new ScenarioStep(2,1));
	scen->Steps.push_back(new ScenarioStep(2,2));
	scen->Steps.push_back(new ScenarioStep(2,3));

	scen->Steps.push_back(new ScenarioStep(3,1));//A
	scen->Steps.push_back(new ScenarioStep(3,2,"1"));//B
	scen->Steps.push_back(new ScenarioStep(3,3,"7"));
	scen->Steps.push_back(new ScenarioStep(3,4));
	scen->Steps.push_back(new ScenarioStep(3,5,"1.3.1"));//start point step 1
	scen->Steps.push_back(new ScenarioStep(3,5,"1.3.2"));//max point step 1
	scen->Steps.push_back(new ScenarioStep(3,5,"1.3.5"));//start point step 2
	scen->Steps.push_back(new ScenarioStep(3,5,"1.3.6"));//max point step 2

	ScenarioList.push_back(scen);

	//SerialNo = "00000000";

	currentScenario = NULL;
	AdditionalScenarioList.clear();

	Type = 2;
}
//=============================================================================
Shtultz::Shtultz(Byte Ename, Byte adr, TFreeCoolingManager* fp):Ifree(Ename, adr, fp)
		//,Alarms(5,200,1), Warnings(5,200,2)
{
	screenNumber.Data_s = 0;
	newScreenNumber.Data_s = 0;
	scrFace 	= 0;
	scrSubFace 	= 0;
	hScrSubFace = 0;
	hasWarn 	= 0;
	hasAlarm 	= 0;


	Screen.push_back("");
	Screen.push_back("");
	Screen.push_back("");
	Screen.push_back("");

	OldScreen.push_back("");
	OldScreen.push_back("");
	OldScreen.push_back("");
	OldScreen.push_back("");

	ScenarioIndex = 0;

	Scenario* scen = new Scenario("DefaultData");
	scen->Steps.push_back(new ScenarioStep(1,1));
	scen->Steps.push_back(new ScenarioStep(1,2));
	scen->Steps.push_back(new ScenarioStep(1,3));
	scen->Steps.push_back(new ScenarioStep(1,4));
	scen->Steps.push_back(new ScenarioStep(1,5));
	scen->Steps.push_back(new ScenarioStep(1,6));
	scen->Steps.push_back(new ScenarioStep(1,7));
	ScenarioList.push_back(scen);

	scen = new Scenario("PassWord");
	scen->Steps.push_back(new ScenarioStep(2,1));
	scen->Steps.push_back(new ScenarioStep(2,2));
	scen->Steps.push_back(new ScenarioStep(2,3));

	scen->Steps.push_back(new ScenarioStep(3,1));//A
	scen->Steps.push_back(new ScenarioStep(3,2,"1"));//B
	scen->Steps.push_back(new ScenarioStep(3,3,"7"));
	scen->Steps.push_back(new ScenarioStep(3,4));
	scen->Steps.push_back(new ScenarioStep(3,5,"1.3.1"));//start point step 1
	scen->Steps.push_back(new ScenarioStep(3,5,"1.3.2"));//max point step 1
	scen->Steps.push_back(new ScenarioStep(3,5,"1.3.5"));//start point step 2
	scen->Steps.push_back(new ScenarioStep(3,5,"1.3.6"));//max point step 2

	ScenarioList.push_back(scen);

	//SerialNo = "00000000";

	currentScenario = NULL;
	AdditionalScenarioList.clear();
	Type = 2;
}
//=============================================================================
Shtultz::~Shtultz()
{

}
//=============================================================================
void Shtultz::Restart(void)
{
	Log::DEBUG("Shtultz::Restart start");

	screenNumber.Data_s 	= 0;
	newScreenNumber.Data_s 	= 0;
	ScenarioIndex 			= 0;
	currentScenario 		= NULL;

	for(int i =0; i < ScenarioList.size(); i++)
	{
		Scenario* scen = ScenarioList[i];
		for(int j =0; j < scen->Steps.size(); j++)
		{
			scen->Steps[j]->isEnterOK = false;
		}
		scen->stepIndex 	= 0;
		scen->IsComplited = false;
		scen->IsAdditional = false;
		scen->SetCmd       = 0;
		scen->SetValue     = "";
	}
	Log::DEBUG("Shtultz::Restart end");
}
//=============================================================================
Word Shtultz::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Bshort       Tmp;
	 Byte         Crc = 0;
	 switch(CMD)
	 {
     case 0:
         DataLen = 0;//01 FF 01 01 02
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x01;
         Crc = TBuffer::Crc8(Buffer, DataLen);
         Buffer[DataLen++] = Crc;
       break;

     case 1:

    	 if(newScreenNumber.Data_s != 65535 )
    	 {
    		 screenNumber.Data_s = newScreenNumber.Data_s;
    		 SetScreen(65535);
    		 //newScreenNumber.Data_s = 65535;
    	 }
         DataLen = 0;//
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x21;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = screenNumber.Data_b[0];//0x08;//0x00
         Buffer[DataLen++] = screenNumber.Data_b[1];;
         Crc = TBuffer::Crc8(Buffer, DataLen);
         Buffer[DataLen++] = Crc;

         if(screenNumber.Data_s != 0)
         {
          screenNumber.Data_s = 0x00;
         }
    	 break;


    case 2:
    	 DataLen = 0;// 01 21 00 (22)
    	 Buffer[DataLen++] = 0x01;
    	 Buffer[DataLen++] = 0x21;
    	 Buffer[DataLen++] = 0x00;
    	 Crc = TBuffer::Crc8(Buffer, DataLen);
    	 Buffer[DataLen++] = Crc;
    	 break;

     case 3:
         DataLen = 0;// 01 05 08 50 45 5F 4E 53 55 30 31 (59)
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x05;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = 0x50;
         Buffer[DataLen++] = 0x45;
         Buffer[DataLen++] = 0x5F;
         Buffer[DataLen++] = 0x4E;
         Buffer[DataLen++] = 0x53;
         Buffer[DataLen++] = 0x55;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x31;
         Crc = TBuffer::Crc8(Buffer, DataLen);
         Buffer[DataLen++] = Crc;
    	 break;


     default:
       DataLen = 0;
       break;
	 }

	 return DataLen;
}
//=============================================================================
void Shtultz::SetScreen(Word value)
{
	pthread_mutex_lock(&sych);
		newScreenNumber.Data_s = value;
	pthread_mutex_unlock(&sych);
}
//=============================================================================
void Shtultz::SetBack()
{
	SetScreen(64);
}
//=============================================================================
void Shtultz::SetEnter()
{
	SetScreen(128);
}
//=============================================================================
void Shtultz::SetUp()
{
	SetScreen(4);
}
//=============================================================================
void Shtultz::SetDown()
{
	SetScreen(1024);
}
//=============================================================================
void Shtultz::SetPassword()
{
	SetScreen(8);
}
//=============================================================================
void Shtultz::SwitchState()
{
	SetScreen(2048);
}
//=============================================================================
void Shtultz::ManageData(void)
{
	if(GetTimValue(1) > 0) return;
	SetTimer(COMMON_TIMER, 250);//enter 1 per second//500
	if(CompareScreens() == false)
	{
		OldScreen = Screen;
		Blong 	st 		= GetScreen();
		for(int i = 0; i < 4; i++)
		{
			string pstr = toString(i)+":		%s\n";
			printf( pstr.c_str(), OldScreen[i].c_str() );
		}
		Byte 	sFace 	= st.Data_b[0];
		Byte    ssFace 	= st.Data_b[1];
		Byte    hWarn   = st.Data_b[2];
		Byte    hAlarm  = st.Data_b[3];

		bool flg = false;
		if(sFace != scrFace)
		{
			scrFace = sFace;
			flg = true;
		}
		if(ssFace != scrSubFace)
		{
			scrSubFace = ssFace;
			flg = true;
		}
		if(flg)
		{
			Log::DEBUG("scrFase:"+toString((int)scrFace));
			Log::DEBUG("scrSubFace:"+toString((int)scrSubFace));
		}



		if(hWarn != hasWarn)
		{
			hasWarn = hWarn;
			Log::DEBUG("hasWarn:"+toString((int)hasWarn));
			//Warnings[0]->Value = hasWarn;
			Warnings[0]->SetValue(hasWarn);
			Modify = true;
		}
		if(hAlarm != hasAlarm)
		{
			hasAlarm = hAlarm;
			Log::DEBUG("hasAlarm:"+toString((int)hasAlarm));
			//Alarms[0]->Value = hasAlarm;
			Alarms[0]->SetValue(hasAlarm);
			Modify = true;
		}

		if(HState != State)
		{
			HState = State;
			Log::DEBUG("HState:"+toString((int)HState));
			this->Modify = true;
		}
		//GetDataFromScreen();
	}
	//
}
//=============================================================================
// ret.Data_b[0] - screen
// ret.Data_b[1] - subscreen
// ret.Data_b[2] - haswarning
// ret.Data_b[3] - hasalarm
//=============================================================================
Blong Shtultz::GetScreen(void)
{
	Blong ret;// = 0;
	ret.Data_b[0] = 0;
	ret.Data_b[1] = 0;
	ret.Data_b[2] = hasWarn;
	ret.Data_b[3] = hasAlarm;
	vector<string> temp1;
	vector<string> temp2;
	addFace.clear();

	if(isNullOrWhiteSpace(Screen[0]))
	{
		if(Screen[1].find("Password") != string::npos)
		{
			ret.Data_b[0] = SH_PASSWORD;

			//Log::DEBUG( "orig=[" + Screen[3] + "], len="+toString(Screen[3].size()));
			if(Screen[3].size() > 11)
			{
				string val = Screen[3].substr(8,3);
				//Log::DEBUG(  "pass=[" + val + "], len="+toString( val.size() )  );
				ScreenValue = val;
				ret.Data_b[1] = hScrSubFace;
				for(int i = 0; i < val.size(); i++)
				{

					if(((Byte*)val.c_str())[i] < 0x30)
					{
						ret.Data_b[1] = i+1;
						hScrSubFace = ret.Data_b[1];
						break;
					}
				}
			}
		}
		else if(Screen[1].find("WARNING") != string::npos)
		{
			ret.Data_b[0] = SH_MAIN;
			ret.Data_b[2] = 1;//haswarning
		}
		else if(Screen[1].find("ALARM") != string::npos)
		{
			ret.Data_b[0] = SH_MAIN;
			ret.Data_b[3] = 1;//hasalarm
		}
	}
	else
	{
		if(Screen[0].find("CONTROLLER") != string::npos)
		{
			ret.Data_b[0] = SH_MAIN;
			if( Screen[3].find("SYSTEM")	!= string::npos)
			{
				string ss = Screen[3].substr(Screen[3].find("SYSTEM")+6);
				vector<string> rr = TBuffer::Split(ss, " ");
				if(rr.size() > 0)
				{
					if(rr[0] == "ON")
						State = 1;

					else
						State = 0;
				}
			}
			else if( Screen[3].find("WARNING")	!= string::npos)
			{
				State = 1;
			}
			else if( Screen[3].find("ALARM")	!= string::npos)
			{
				State = 1;
			}


			if( Screen[1].find("Room temp.:")!= string::npos &&
				Screen[2].find("Ext. temp.:")!= string::npos)
			{
				ret.Data_b[1] = 1;
				temp1 = TBuffer::Split(Screen[1], " ");
				temp2 = TBuffer::Split(Screen[2], " ");
				float t1 = atof(temp1[2].c_str());
				float t2 = atof(temp2[2].c_str());
				//Log::DEBUG( "Room temp:[" + toString(t1)+"]");
				//Log::DEBUG( "Ext temp:[" + toString(t2)+"]");
				RoomTemp.CalcMinAverMax(t1);
				ExtTemp.CalcMinAverMax(t2);
			}
			else if( Screen[1].find("AC1:")!= string::npos &&
					 Screen[2].find("TM1:")!= string::npos)
			{
				ret.Data_b[1] = 2;
			}
			else if( Screen[1].find("AC3:")!= string::npos &&
					 Screen[2].find("DAMPER:")!= string::npos)
			{
				ret.Data_b[1] = 3;
			}
			else if( Screen[1].find("FAN 1:")!= string::npos &&
					 Screen[2].find("FAN 2:")!= string::npos)
			{
				ret.Data_b[1] = 4;
			}
			else if( Screen[1].find("Vdc:")!= string::npos &&
					 Screen[2].find("If1:")!= string::npos)
			{
				temp1 = TBuffer::Split(Screen[1], " ");
				temp2 = TBuffer::Split(Screen[2], " ");
				if(temp1.size() >= 1)
				{
					float vdc = atof(temp1[1].c_str());
					Vdc.CalcMinAverMax(vdc);
					//Log::DEBUG( "Vdc:[" + toString(vdc) +"]");
				}
				if(temp2.size() >= 2)
				{
					float i1 = atof(temp2[1].c_str());
					If1.CalcMinAverMax(i1);
					//Log::DEBUG( "if1:[" + toString(i1) +"]");
				}
				if(temp2.size() >= 4 )
				{
					if( temp2[3].find("---")== string::npos )
					{
						float i2 = atof(temp2[3].c_str());
						If2.CalcMinAverMax(i2);
						//Log::DEBUG( "if2:[" + toString(i2) +"]");
					}
				}
/*//////////////////////
				for(int i=0; i < temp2.size();i++)
				{
					Log::DEBUG( "temp2 ind"+ toString(i)  +":[" + temp2[i] +"]");
				}
//////////////////////*/
				ret.Data_b[1] = 5;
			}
			else if( Screen[1].find("Pf1:")!= string::npos &&
					 Screen[2].find("Pf2:")!= string::npos)
			{
				temp1 = TBuffer::Split(Screen[1], " ");
				temp2 = TBuffer::Split(Screen[2], " ");
				if(temp1.size() >= 2)
				{
					float pf1 = atof(temp1[1].c_str());
					Pf1.CalcMinAverMax(pf1);
					//Log::DEBUG( "Pf1:[" + toString(pf1) +"]");
				}
				if(temp2.size() >= 2 )
				{
					if( temp2[1].find("---")== string::npos )
					{
						float pf2 = atof(temp2[1].c_str());
						Pf2.CalcMinAverMax(pf2);
						//Log::DEBUG( "pf2:[" + toString(pf2) +"]");
					}
				}

				ret.Data_b[1] = 6;
			}
			else if( Screen[1].find("Sf1:")!= string::npos &&
					 Screen[2].find("Sf2:")!= string::npos)
			{
				temp1 = TBuffer::Split(Screen[1], " ");
				temp2 = TBuffer::Split(Screen[2], " ");
				if(temp1.size() >= 2)
				{
					float sw1 = atof(temp1[1].c_str());
					Sf1.CalcMinAverMax(sw1);
					//Log::DEBUG( "sf1:[" + toString(sw1) +"]");
				}
				if(temp2.size() >= 2 )
				{
					if( temp2[1].find("---")== string::npos )
					{
						float sw2 = atof(temp2[1].c_str());
						Sf2.CalcMinAverMax(sw2);
						//Log::DEBUG( "sf2:[" + toString(sw2) +"]");
					}
				}

				ret.Data_b[1] = 7;
			}
		}
		else if(Screen[0].find("SERIAL NUMBER") != string::npos)
		{
			SerialNo = Screen[2];
			SerialNo = trim(SerialNo);
			//Log::DEBUG("SERIAL NUMBER ["+SerialNo+"]");

		}
		else
		{
			temp1 = TBuffer::Split(Screen[0], " ");
			if(temp1.size() >= 2)
			{
				temp2 = TBuffer::Split(temp1[0], ".");
				if(temp2.size() > 0 && temp2[0].length() == 1 && temp2[0] >= "A" && temp2[0] <= "Z")//
				{
					addFace = temp2;
					//Log::DEBUG("temp2.size="+toString(temp2.size())+" ");
					ret.Data_b[0] = SH_MAIN_MENU;
					char ch = temp2[0].c_str()[0];
					ret.Data_b[1] = ch - 0x40;

					if(!isNullOrWhiteSpace(Screen[3]))
					{
						//Log::DEBUG("!isNullOrWhiteSpace(Screen[3]) !!");
						if(Screen[1].find("Start point step 1") != string::npos)
						{
							vector<string> rrr = TBuffer::Split(trim(Screen[3]), (char)223);
							if(rrr.size() >=2) {
								PointsSteps[0]->Value = rrr[0];
							}
						}
						else if(Screen[1].find("Start point step 2") != string::npos)
						{
							vector<string> rrr = TBuffer::Split(trim(Screen[3]), (char)223);
							if(rrr.size() >=2) {
								PointsSteps[2]->Value = rrr[0];
							}
						}
						else if(Screen[1].find("Max point step 1") != string::npos)
						{
							vector<string> rrr = TBuffer::Split(trim(Screen[3]), (char)223);
							if(rrr.size() >=2) {
								PointsSteps[1]->Value = rrr[0];
							}
						}
						else if(Screen[1].find("Max point step 2") != string::npos)
						{
							vector<string> rrr = TBuffer::Split(trim(Screen[3]), (char)223);
							if(rrr.size() >=2) {
								PointsSteps[3]->Value = rrr[0];
							}
						}
						else if(Screen[1].find("Cooling set point") != string::npos)
						{
							vector<string> rrr = TBuffer::Split(trim(Screen[3]), (char)223);
							//Log::DEBUG("Cooling set point!!!!!!["+rrr[0]+"]");
							if(rrr.size() >=2) {
								SetPoint[0]->Value = rrr[0];
								//Log::DEBUG("Cooling set point ["+SetPoint.Params[0]+"]");
							}
						}
					}
				}
			}
		}
	}

	return ret;
}
//=============================================================================
Byte Shtultz::GetSubScreen(void)
{
	return 0;
}
//=============================================================================
Byte Shtultz::GetDataFromMessage(Byte *Buffer, Word DataLen, Byte &subFacePtr, void *data)
{
	Byte Fase = RECV_CMD;
	Byte addr 		= Buffer[0];
	Byte cmd  		= Buffer[1];
	Byte dataLen  	= Buffer[2];
	Byte data0      = Buffer[3];
	if(DataLen != (dataLen+4))
	{
    	Log::DEBUG( "[TFreeCoolingManager] Format command ERROR");
		Fase    = RECV_CMD;
	}
	else
	{
		switch(cmd)
		{
		case 0x21:
			if(data0 == 0x0B && dataLen == 4)
			{
				subFacePtr = 0;
			}
			else if(data0 == 0x01 && dataLen == 1)
			{
				subFacePtr = 1;
			}

			else if(data0 == 0x04 && dataLen == 0x18)
			{//screen mess
    			string logstr = string((char*)&Buffer[7], 20);
    			int screen_index = (int)Buffer[5];
    			if(screen_index > 0 && screen_index <=4)
    			{
    				Screen[screen_index-1] = logstr;
    			}
    			/*
    			for(int i = 0; i < 4; i++)
    			{
    				string pstr = toString(i)+":		%s\n";
    				printf( pstr.c_str(), Screen[i].c_str() );
    			}*/
    			subFacePtr = 2;
			}
			break;

		case 0x05:
			if(data0 == 0x00 && dataLen == 3)
			{
				subFacePtr = 0;
			}
			break;

		default:
			subFacePtr = 0;
			break;
		}
		Fase    = CREATE_CMD;
	}
	return Fase;
}
//=============================================================================
bool Shtultz::GetDataFromScreen(void)
{
	if(scrFace > 0)
	{
		if(scrFace == SH_MAIN )
		{
			if(scrSubFace > 0)
			{
				if( Screen[3].find("SYSTEM")	!= string::npos)
				{
					string ss = Screen[3].substr(Screen[3].find("SYSTEM")+6);
					vector<string> rr = TBuffer::Split(ss, " ");
					if(rr.size() > 0)
					{
						if(rr[0] == "ON")
						{
							State = 1;
						}
						else
						{
							State = 0;
						}
						//Log::DEBUG( "SYSTEM=[" + toString(rr[0])+"]" );
					}

				}
				else if( Screen[3].find("WARNING")	!= string::npos)
				{
					State = 1;
				}
				else if( Screen[3].find("ALARM")	!= string::npos)
				{
					State = 1;
				}
			}
			else
			{

			}
		}


	}
	return 0;
}
//=============================================================================
void Shtultz::ManageScenario(void)
{
	if(GetTimValue(0) > 0) return;
	SetTimer(0, 500);//enter 1 per second//1000

	static Word errorIndex = 0;
	static Word maxStepIndex = 0;
	static bool needExit = false;

	Scenario* curr = getCurrScenario();
	if(curr != NULL)
	{
		ScenarioStep * currStep = curr->GetCurrStep();
		if(currStep != NULL)
		{
			switch(currStep->scrFace)
			{
			case 1:
				if(currStep->scrFace == scrFace)
				{
					if(scrSubFace > 0)
					{
						errorIndex = 0;
						if(scrSubFace > currStep->scrSubFace)
							SetDown();
						else if(scrSubFace < currStep->scrSubFace)
							SetUp();
						else
						{
							if( !isNullOrWhiteSpace(currStep->newValue) )
							{
								Word _cmd = atoi(currStep->newValue.c_str());
								SetScreen(_cmd);
								Log::DEBUG("currStep->newValue="+toString(_cmd));
								currStep->newValue = "";
								Modify = true;
							}
							maxStepIndex = 0;
							curr->IncStepIndex();
							Log::DEBUG("StepIndex="+toString(curr->stepIndex));
						}
					}
					else
						errorIndex++;
				}
				else
					SetBack();
				break;
			case 2:
				if(currStep->scrFace == scrFace)
				{
					if(scrSubFace > 0 && ScreenValue.size() == 3)
					{
						errorIndex = 0;

						Byte ch = ((Byte*)ScreenValue.c_str())[scrSubFace-1];
						if(scrSubFace > currStep->scrSubFace)
						{
							if(ch== 0x20) SetBack();
						}
						else if(scrSubFace < currStep->scrSubFace)
						{
							if(ch== 0x20) SetEnter();
						}
						else
						{
							if(ScreenValue.size() == 3)
							{
								if(scrSubFace > 3)
									SetBack();//errorIndex++;
								else
								{
									//Log::DEBUG("ScreenValue=["+ScreenValue+"]");
									//Log::DEBUG("ch="+toString((int)ch));
									if(ch > 0x20 && ch <= 0x39)
									{
										if(ch > 0x31)
											SetDown();
										else if (ch < 0x31)
											SetUp();
										else
										{
											maxStepIndex = 0;
											ScreenValue = "   ";
											hScrSubFace = 0;
											curr->IncStepIndex();
											Log::DEBUG("StepIndex_="+toString(curr->stepIndex));
										}
									}
								}
							}
							else
								SetBack();//errorIndex++;
						}
					}
					else
						errorIndex++;
				}
				else
				{
					errorIndex++;
					SetPassword();
				}
				break;
			case 3:
				if(currStep->scrFace == scrFace)
				{
					//Log::DEBUG("MainMenu");
					if(scrSubFace > 0)
					{
						errorIndex = 0;
						if(maxStepIndex++ > 100)//////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						{
							Log::ERROR("maxStepIndex > 100");
							maxStepIndex = 0;
							errorIndex = 999;
							break;
						}
						vector<string> needed  = currStep->GetDataFromName();
						vector<string> current = addFace;
						if(needExit)
						{
							if(current.size() > 1)
								SetBack();
							else
							{
								needExit = false;
								maxStepIndex = 0;//!!!
							}
						}
						else
						{
							if(scrSubFace > currStep->scrSubFace)
								SetDown();
							else if(scrSubFace < currStep->scrSubFace)
								SetUp();
							else
							{
								bool flg = false;
								Log::DEBUG("needed.size="+toString(needed.size()) + "  " +"current.size="+toString(current.size()));
								if(current.size() < 2)
								{
									if(current.size() < needed.size())
										SetEnter();
									else  if(current.size() > needed.size())
										SetBack();
									else
									{
										//Log::DEBUG("!!needed.size=current.size!!");
										if(current.size() > 0 && needed.size() > 0)
										{
											int curr = atoi(current[current.size()-1].c_str());
											int need = atoi(needed[needed.size()-1].c_str());
											if(curr < need)
												SetUp();
											else if(curr > need)
												SetDown();
											else
											{
												//SetEnter();
												flg 		= true;
												needExit 	= true;
											}
										}
									}
								}
								else
								{
									Log::DEBUG("current.size() > 1");
									Word index = current.size() -1;
									if(current.size() > index && needed.size() > index)
									{
										int curr = atoi(current[index].c_str());
										int need = atoi(needed[index].c_str());
										if(curr < need)
											SetUp();
										else if(curr > need)
											SetDown();
										else
										{
											if(current.size() < needed.size())
												SetEnter();
											else  if(current.size() > needed.size())
												SetBack();
											else
											{
												if( !isNullOrWhiteSpace(currStep->newValue) )
												{
													if(currStep->isEnterOK)
													{
														Log::DEBUG("NewVal="+currStep->newValue);
														if(!isNullOrWhiteSpace(Screen[3]))
														{
															vector<string> rrr = TBuffer::Split(trim(Screen[3]), (char)223);
															if(rrr.size() >= 2)
															{
																float currV = atof( rrr[0].c_str() );
																float newV  = atof( currStep->newValue.c_str() );
																if(currV < newV)
																	SetUp();
																else if(currV > newV)
																	SetDown();
																else
																{
																	SetEnter();
																	currStep->newValue = "";
																	currStep->isEnterOK = false;
																	this->Modify = true;
																}
															}
														}
													}
													else
													{
														SetEnter();
														currStep->isEnterOK = true;

													}
												}
												else
												{
													flg 		= true;
													needExit 	= true;
												}
											}
										}
									}
								}
								if(flg == true)
								{
									SetEnter();///
									curr->IncStepIndex();
									Log::DEBUG("StepIndex="+toString(curr->stepIndex));
								}
							}
						}
					}
					else
						errorIndex++;
				}
				else
				{//zashli posle parolia -> enter mainmenu
					errorIndex++;
					SetEnter();
				}
				break;

			default:
				Log::ERROR("Error in currStep->scrFace="+toString((int)currStep->scrFace));
				errorIndex++;
				SetBack();////
				break;
			}
		}
		else
		{
			curr->IsComplited 	= true;
			curr->stepIndex 	= 0;
		}
		if(curr->IsComplited == true)
		{
			if(curr->SetCmd != 0)
			{
				Log::DEBUG("curr->SetCmd="+toString(curr->SetCmd));
				SetScreen(curr->SetCmd);
				curr->SetCmd = 0;
			}
			curr->stepIndex = 0;
			//Log::DEBUG("ScenarioIndex="+toString(ScenarioIndex)+" IsComplited");
			curr->IsComplited = false;
			IncScenarioIndex();
			//getAddScenario();//!!!
		}
		///////////
		if(errorIndex > 5)
		{
			errorIndex 		= 0;
			ScenarioIndex 	= 0;
			curr->stepIndex = 0;
			SetBack();
			Log::DEBUG("Error errorIndex!");//FreeCoolingManager.NeedRestart
			if(freemngptr != NULL)
				freemngptr->NeedRestart;
			//FreeCoolingManager.NeedRestart = true;
			/*
			SetTimer(0, 2000);
			if(currentScenario != NULL)
			{
				delete currentScenario;
				currentScenario = NULL;
			}
			*/
		}
		///////////
	}
	else
		ScenarioIndex = 0;
}
//=============================================================================
bool Shtultz::CompareScreens(void)
{
	bool ret = true;
	if(Screen.size() != OldScreen.size())
		return false;
	int size = Screen.size();
	for(int i=0;i<size;i++)
	{
		if(Screen[i] != OldScreen[i])
			return false;
	}
	return ret;
}
//=============================================================================
string Shtultz::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(RoomTemp.Aver, 0, 2)+','+FloatToString(RoomTemp.Min, 0, 2)+','+
			FloatToString(RoomTemp.Max, 0, 2)+',';
	RoomTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';
	ReturnString += FloatToString(ExtTemp.Aver, 0, 2)+','+FloatToString(ExtTemp.Min, 0, 2)+','+
			FloatToString(ExtTemp.Max, 0, 2)+',';
	ExtTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(80)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vdc.Aver, 0, 2)+','+FloatToString(Vdc.Min, 0, 2)+','+ FloatToString(Vdc.Max, 0, 2)+',';
	Vdc.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(60)+','+WordToString(1)+',';
	ReturnString += FloatToString(If1.Aver, 0, 2)+','+FloatToString(If1.Min, 0, 2)+','+ FloatToString(If1.Max, 0, 2)+',';
	ReturnString += "\r\n>,"+WordToString(60)+','+WordToString(2)+',';
	ReturnString += FloatToString(If2.Aver, 0, 2)+','+FloatToString(If2.Min, 0, 2)+','+ FloatToString(If2.Max, 0, 2)+',';
	If1.ClrDivider();
	If2.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(44)+','+WordToString(1)+',';
	ReturnString += FloatToString(Pf1.Aver, 0, 2)+','+FloatToString(Pf1.Min, 0, 2)+','+ FloatToString(Pf1.Max, 0, 2)+',';
	ReturnString += "\r\n>,"+WordToString(44)+','+WordToString(2)+',';
	ReturnString += FloatToString(Pf2.Aver, 0, 2)+','+FloatToString(Pf2.Min, 0, 2)+','+ FloatToString(Pf2.Max, 0, 2)+',';
	Pf1.ClrDivider();
	Pf2.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(140)+','+WordToString(1)+',';
	ReturnString += WordToString(Sf1.Aver)+','+WordToString(Sf1.Min)+','+ WordToString(Sf1.Max)+',';
	ReturnString += "\r\n>,"+WordToString(140)+','+WordToString(2)+',';
	ReturnString += WordToString(Sf2.Aver)+','+WordToString(Sf2.Min)+','+ WordToString(Sf2.Max)+',';
	Sf1.ClrDivider();
	Sf2.ClrDivider();

	ReturnString +=Alarms.GetValueString();
	ReturnString +=Warnings.GetValueString();

	ReturnString += "\r\n>,201,1,"+ WordToString(State)+',';

	ReturnString += PointsSteps.GetValueString();
	ReturnString += SetPoint.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";
    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,99,1,"+WordToString(Type)+',';


	RoomTemp.ClrDivider();
	ExtTemp.ClrDivider();

	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
Envicool::Envicool(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ifree(Etype, Ename, adr),
PointSteps(230, 1, 24),RunHours(232, 1 ,10)
{
	Log::DEBUG("Envicool");

	address 	= adr;
	Type 		= 3;
	DataLen 	= 0;
	driver 		= (TModbusManager *)dr;
	settings 	= *((CommSettings*)sett);

	//Log::DEBUG("Envicool driver="+toString( (int)driver ));
	if(driver != NULL){

		driver->Init(settings);
		//driver->ClearDeviceList();
		int slen = driver->DeviceList.size();
		driver->AddToDeviceList(address, 3, 0x0,   24, this);
		driver->AddToDeviceList(address, 3, 0x200, 10, this);
		driver->AddToDeviceList(address, 3, 0x300, 14, this);
		driver->AddToDeviceList(address, 3, 0x400, 26, this);
		driver->AddToDeviceList(address, 3, 0x500, 20, this);
		driver->AddToDeviceList(address, 3, 0x901,  1, this);
		//driver->outPtr = this;
		driver->OnCmdComplited = transportData;
		Log::DEBUG( "Envicool::Envicool driver->OnCmdComplited="+toString((long)driver->OnCmdComplited));
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();

	}
	else{
		Log::DEBUG("Envicool driver is NULL!!!");
	}

	int cnt = 26-Alarms.Alarms.size();
	while(cnt-- > 0)
	{
		Alarms.Alarms.push_back(new THistBoolParam());
	}
	Customer = "Envicool";
	CU_No = "indoor";
}
//=============================================================================
Envicool::~Envicool()
{

}
//=============================================================================
Byte Envicool::GetDataFromMessage(Byte *Buffer, Word Len, Byte &subFacePtr, void *data)
{
			sBshort tmp;
			Blong   long_tmp;
			Word ind = 0;

			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];

			Log::DEBUG("Envicool::GetDataFromMessage reg = "+toString(tmp.Data_s));

			switch(tmp.Data_s){
			case 0:
				for(int alInd=0; alInd < PointSteps.size(); alInd++){
					tmp.Data_b[0] = Buffer[ind++];
					tmp.Data_b[1] = Buffer[ind++];
					PointSteps[alInd]->Value =  (float)( (float)tmp.Data_s/10.0 );
					Log::DEBUG("ind="+toString(alInd)+" val="+toString(tmp.Data_s) );
				}
				break;

			case 0x200:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				State = tmp.Data_s;
				break;

			case 0x300:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				RoomTemp.CalcMinAverMax( tmp.Data_s/10.0 );

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				RoomTempOut.CalcMinAverMax( tmp.Data_s/10.0 );

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				ExtTemp.CalcMinAverMax( tmp.Data_s/10.0 );

				ind+=4;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Humidity.CalcMinAverMax( tmp.Data_s/10.0 );

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Vdc.CalcMinAverMax( tmp.Data_s/10.0 );

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Vac.CalcMinAverMax( tmp.Data_s/10.0 );

				ind+=4;
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				AirPressure.CalcMinAverMax( tmp.Data_s/10.0 );

				break;

			case 0x400:
				for(int alInd=0; alInd < Alarms.Alarms.size(); alInd++){
					tmp.Data_b[0] = Buffer[ind++];
					tmp.Data_b[1] = Buffer[ind++];
					//Alarms.Alarms[alInd]->Value = tmp.Data_s == 0x20 ? 0 : tmp.Data_s;
					Alarms[alInd]->SetValue(tmp.Data_s == 0x20 ? 0 : tmp.Data_s);
					Log::DEBUG("Alarms: ind="+toString(alInd)+" val="+toString(tmp.Data_s) );
				}
				break;

			case 0x500:
				for(int alInd=0; alInd < RunHours.Params.size(); alInd++){
					long_tmp.Data_b[2] = Buffer[ind++];
					long_tmp.Data_b[3] = Buffer[ind++];
					long_tmp.Data_b[0] = Buffer[ind++];
					long_tmp.Data_b[1] = Buffer[ind++];
					RunHours.Params[alInd] = long_tmp.Data_l;
					Log::DEBUG("ind="+toString(alInd)+" val="+toString(tmp.Data_s) );
				}
				break;

			default:
				break;
			}


			Byte Fase    = CREATE_CMD;
			return Fase;
		}
//=============================================================================
sWord Envicool::RecvData( IPort* Port, Byte *Buf, Word MaxLen )
{
	//Log::DEBUG("Envicool::RecvData");
	int timeout_cnt = settings.RecvTimeout/100;
	if(timeout_cnt <= 0 ) timeout_cnt = 1;
	while(timeout_cnt--)
	{
		usleep(100000);//was 1000 mks=1ms
		if(this->DataLen > 0)
		{
			Word cnt = this->DataLen > MaxLen ? MaxLen: this->DataLen;
			for(Word i=0; i < cnt; i++){
				Buf[i] = Buffer[i];
			}
			this->DataLen = 0;
			return cnt;
		}
	}
	return 0;
}
//=============================================================================
bool Envicool::DetectStates()
{
    	bool ret = false;

    	//Log::DEBUG("Envicool::DetectStates Alarms.Alarms.size()=" + toString(Alarms.Alarms.size()));
    	//Alarms.Alarms.size()
    	//int index = 0;
    	for(auto curr: Alarms.Alarms)
    	{
    		//Log::DEBUG("Envicool::DetectStates index==" + toString(index++));
    		if(curr->HValue != curr->Value){
    			curr->HValue = curr->Value;
    			ret = true;
    		}
    	}

    	for(auto curr: Warnings.Alarms)
    	{
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
    	return ret;
  }
//=============================================================================
string Envicool::GetStringValue(void)
{

	Log::DEBUG("Envicool::GetStringValue");

	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(RoomTemp.Aver, 0, 2)+','+FloatToString(RoomTemp.Min, 0, 2)+','+FloatToString(RoomTemp.Max, 0, 2)+',';
	RoomTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';
	ReturnString += FloatToString(ExtTemp.Aver, 0, 2)+','+FloatToString(ExtTemp.Min, 0, 2)+','+FloatToString(ExtTemp.Max, 0, 2)+',';
	ExtTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(3)+',';
	ReturnString += FloatToString(RoomTempOut.Aver, 0, 2)+','+FloatToString(RoomTempOut.Min, 0, 2)+','+FloatToString(RoomTempOut.Max, 0, 2)+',';
	RoomTempOut.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(71)+','+WordToString(1)+',';
	ReturnString += FloatToString(Humidity.Aver, 0, 2)+','+FloatToString(Humidity.Min, 0, 2)+','+ FloatToString(Humidity.Max, 0, 2)+',';
	Humidity.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(76)+','+WordToString(1)+',';
	ReturnString += FloatToString(AirPressure.Aver, 0, 2)+','+FloatToString(AirPressure.Min, 0, 2)+','+ FloatToString(AirPressure.Max, 0, 2)+',';
	AirPressure.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(80)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vdc.Aver, 0, 2)+','+FloatToString(Vdc.Min, 0, 2)+','+ FloatToString(Vdc.Max, 0, 2)+',';
	Vdc.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(42)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vac.Aver, 0, 2)+','+FloatToString(Vac.Min, 0, 2)+','+ FloatToString(Vac.Max, 0, 2)+',';
	Vac.ClrDivider();

	ReturnString +=Alarms.GetValueString();

	ReturnString += "\r\n>,201,1,"+ WordToString(State)+',';


	ReturnString += PointSteps.GetValueString();
	ReturnString += RunHours.GetValueString();

	ReturnString += "\r\n>,149,1,"+ Customer +',';
	if(CU_No.length() > 0)
		ReturnString += CU_No +',';

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";
    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,99,1,"+WordToString(Type)+',';
	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
EnvicoolHC::EnvicoolHC(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ifree(Etype, Ename, adr),
PointSteps(230, 1, 12),RunHours(232, 1 ,4)
{
	Log::DEBUG("EnvicoolHC");

	address = adr;
	Type = 5;

	driver = (TModbusManager *)dr;
	settings = *((CommSettings*)sett);

	Log::DEBUG("EnvicoolHC driver="+toString( (int)driver ));
	if(driver != NULL)
	{

		driver->Init(settings);
		//driver->ClearDeviceList();
		int slen = driver->DeviceList.size();
		driver->AddToDeviceList(address, 3, 0x1000,   51, this);
		driver->AddToDeviceList(address, 3, 0x0A, 	  23, this);
		driver->AddToDeviceList(address, 3, 0x300, 	  17, this);

		//driver->outPtr = this;
		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();
	}

	int cnt = 17-Alarms.Alarms.size();
	while(cnt-- > 0)
	{
		Alarms.Alarms.push_back(new THistBoolParam());
	}
    IndoorState = 0;
    HIndoorState = 0;

    OutdoorState = 0;
    HOutdoorState = 0;

    CompressorState = 0;
    HCompressorState = 0;

    Customer = "Envicool";
    CU_No = "outdoor";
}
//=============================================================================
EnvicoolHC::~EnvicoolHC()
{

}
//=============================================================================
sWord EnvicoolHC::RecvData( IPort* Port, Byte *Buf, Word MaxLen )
{
	//Log::DEBUG("EnvicoolHC::RecvData");
	int timeout_cnt = settings.RecvTimeout/100;
	if(timeout_cnt <= 0 ) timeout_cnt = 1;
	while(timeout_cnt--)
	{
		usleep(100000);//was 1000 mks=1ms
		if(this->DataLen > 0)
		{
			Word cnt = this->DataLen > MaxLen ? MaxLen: this->DataLen;
			for(Word i=0; i < cnt; i++){
				Buf[i] = Buffer[i];
			}
			this->DataLen = 0;
			return cnt;
		}
	}
	return 0;
}
//======================================================================
Byte EnvicoolHC::GetDataFromMessage(Byte *Buffer, Word Len, Byte &subFacePtr, void *data)
{
			sBshort tmp;
			Blong   long_tmp;
			Word ind = 0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];

			switch(tmp.Data_s)
			{
			case 0x0A:
				for(int alInd=0; alInd < PointSteps.size(); alInd++)
				{
					tmp.Data_b[0] = Buffer[ind++];
					tmp.Data_b[1] = Buffer[ind++];
					PointSteps[alInd]->Value =  (float)( (float)tmp.Data_s/10.0 );
					ind+=2;

					//Log::DEBUG("PointSteps ind="+toString(alInd)+" val="+toString(tmp.Data_s) );
				}
				break;


			case 0x1000:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				State = tmp.Data_s;
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				IndoorState = tmp.Data_s;
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				OutdoorState = tmp.Data_s;
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				CompressorState = tmp.Data_s;
				ind+=2;



				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				RoomTemp.CalcMinAverMax( tmp.Data_s/10.0 );//indoor temp
				ind+=2;

				ind+=4;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				ExtTemp.CalcMinAverMax( tmp.Data_s/10.0 );//outdoor temp
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				RoomTempOut.CalcMinAverMax( tmp.Data_s/10.0 );//condenser temp
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				AdvTemp.CalcMinAverMax( tmp.Data_s/10.0 );//evaporator temp
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Sf1.CalcMinAverMax( tmp.Data_s/10.0 );//indoor fan speed
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Sf2.CalcMinAverMax( tmp.Data_s/10.0 );//outdoor fan speed
				ind+=2;


				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Vac.CalcMinAverMax( tmp.Data_s/10.0 );
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Vdc.CalcMinAverMax( tmp.Data_s/10.0 );
				ind+=2;

				ind+=4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];//equipment runing time
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				RunHours.Params[0] = long_tmp.Data_l;
				ind+=4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];//compressor runing time
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				RunHours.Params[1] = long_tmp.Data_l;
				ind+=4;


				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];//indoor fan runing time
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				RunHours.Params[2] = long_tmp.Data_l;
				ind+=4;


				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];//compressor action times
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				RunHours.Params[3] = long_tmp.Data_l;
				ind+=4;
				break;


			case 0x300:
				for(int alInd=0; alInd < Alarms.Alarms.size(); alInd++)
				{
					tmp.Data_b[0] = Buffer[ind++];
					tmp.Data_b[1] = Buffer[ind++];
					//Alarms.Alarms[alInd]->Value = tmp.Data_s == 0xFF ? 3 : tmp.Data_s;
					Alarms[alInd]->SetValue(tmp.Data_s == 0xFF ? 3 : tmp.Data_s);
					//Log::DEBUG("Alarms ind="+toString(alInd)+" val="+toString(tmp.Data_s) );
				}
				break;

			default:
				break;
			}


			Byte Fase    = CREATE_CMD;
			return Fase;
		}
//=============================================================================
bool EnvicoolHC::DetectStates()
{
    	bool ret = false;

    	//Log::DEBUG("EnvicoolHC::DetectStates Alarms.Alarms.size()=" + toString(Alarms.Alarms.size()));
    	//Alarms.Alarms.size()
    	//int index = 0;
    	for(auto curr: Alarms.Alarms){
    		//Log::DEBUG("Envicool::DetectStates index==" + toString(index++));
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

    	if(HIndoorState != IndoorState){
    		HIndoorState = IndoorState;
    		ret = true;
    	}

    	if(HOutdoorState != OutdoorState){
    		HOutdoorState = OutdoorState;
    		ret = true;
    	}

    	if(HCompressorState != CompressorState){
    		HCompressorState = CompressorState;
    		ret = true;
    	}

    	return ret;
  }
//=============================================================================
string EnvicoolHC::GetStringValue(void)
{

	string ReturnString = WordToString(Et)+","+WordToString(En)+",";

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(RoomTemp.Aver, 0, 2)+','+FloatToString(RoomTemp.Min, 0, 2)+','+FloatToString(RoomTemp.Max, 0, 2)+',';
	RoomTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';
	ReturnString += FloatToString(ExtTemp.Aver, 0, 2)+','+FloatToString(ExtTemp.Min, 0, 2)+','+FloatToString(ExtTemp.Max, 0, 2)+',';
	ExtTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(3)+',';//condenser temp
	ReturnString += FloatToString(RoomTempOut.Aver, 0, 2)+','+FloatToString(RoomTempOut.Min, 0, 2)+','+FloatToString(RoomTempOut.Max, 0, 2)+',';
	RoomTempOut.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(4)+',';//evaporator temp
	ReturnString += FloatToString(AdvTemp.Aver, 0, 2)+','+FloatToString(AdvTemp.Min, 0, 2)+','+FloatToString(AdvTemp.Max, 0, 2)+',';
	AdvTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(140)+','+WordToString(1)+',';//indoor fan speed
	ReturnString += WordToString(Sf1.Aver)+','+WordToString(Sf1.Min)+','+ WordToString(Sf1.Max)+',';
	Sf1.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(140)+','+WordToString(2)+',';//outdoor fan speed
	ReturnString += WordToString(Sf2.Aver)+','+WordToString(Sf2.Min)+','+ WordToString(Sf2.Max)+',';
	Sf2.ClrDivider();


	ReturnString += "\r\n>,"+WordToString(80)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vdc.Aver, 0, 2)+','+FloatToString(Vdc.Min, 0, 2)+','+ FloatToString(Vdc.Max, 0, 2)+',';
	Vdc.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(42)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vac.Aver, 0, 2)+','+FloatToString(Vac.Min, 0, 2)+','+ FloatToString(Vac.Max, 0, 2)+',';
	Vac.ClrDivider();

	ReturnString +=Alarms.GetValueString();

	ReturnString += "\r\n>,201,1,"+ WordToString(State)+','+ WordToString(IndoorState)+','+ WordToString(OutdoorState)+','+ WordToString(CompressorState)+',';

	ReturnString += PointSteps.GetValueString();

	ReturnString += RunHours.GetValueString();


	ReturnString += "\r\n>,149,1,"+ Customer +',';
	if(CU_No.length() > 0)
		ReturnString += CU_No +',';

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";
    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,99,1,"+WordToString(Type)+',';
	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
Enisey2000::Enisey2000(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ifree(Etype, Ename, adr),
		PointSteps(230, 1, 1),RunHours(232, 1 ,0)
{
	address = adr;
	Type = 4;

	driver = (TModbusManager *)dr;
	settings = *((CommSettings*)sett);

	Log::DEBUG("Enisey2000 driver="+toString( (int)driver ));
	if(driver != NULL)
	{
		driver->Init(settings);
		//driver->ClearDeviceList();
		int slen = driver->DeviceList.size();
		driver->AddToDeviceList(address, 3, 1,  1, this);//temp setting
		driver->AddToDeviceList(address, 4, 2,  10, this);//analog val input reg = 30003-30001=2
		driver->AddToDeviceList(address, 2, 3,  10, this);
		driver->AddToDeviceList(address, 2, 52, 14, this);

		driver->AddToDeviceList(address, 4, 129, 1, this); 	// state
		driver->AddToDeviceList(address, 1, 1,   1, this);    //on|off

		//driver->outPtr = this;
		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();
	}

	int cnt = 14-Alarms.Alarms.size();
	while(cnt-- > 0)
	{
		Alarms.Alarms.push_back(new THistBoolParam());
	}
	cnt = 10-Warnings.Alarms.size();
	while(cnt-- > 0)
	{
		Warnings.Alarms.push_back(new THistBoolParam());
	}


}
//=============================================================================
Enisey2000::~Enisey2000()
{

}
//=============================================================================
Byte Enisey2000::GetDataFromMessage(Byte *Buffer, Word Len, Byte &subFacePtr, void *data)
{
			sBshort tmp;
			Blong   long_tmp;
			Word 	ind = 0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];

			Word reg = tmp.Data_s;

		    //////////////////////////
			 //Log::DEBUG( "Enisey2000 GetDataFromMessage ok: func="+toString(func)+" reg="+toString(reg));
		   		  Byte bf[2] {0,0};
		    		  string recv = "";
		    		  for(int i = 0; i < Len; i++)
		    		  {
		    			  ByteToHex(bf, Buffer[i]);
		    			  recv += "0x"+string((char*)bf, 2)+" ";
		    		  }
		    //Log::DEBUG( "Enisey2000[" + recv +"] len="+toString(Len));
		    //////////////////////

		    if(func == 3 && reg == 1){
				for(int alInd=0; alInd < PointSteps.size(); alInd++){
					if(ind >= Len) break;
					tmp.Data_b[0] = Buffer[ind++];
					tmp.Data_b[1] = Buffer[ind++];
					PointSteps[alInd]->Value =  (float)( (float)tmp.Data_s/10.0 );
					//Log::DEBUG("ind="+toString(alInd)+" val="+toString(tmp.Data_s) );
				}
		    }
		    if(func == 4 && reg == 2)//analog val input reg = 30003-30001=2
		    {
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Humidity.CalcMinAverMax( tmp.Data_s/10.0 );

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				RoomTemp.CalcMinAverMax( tmp.Data_s/10.0 );

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				RoomTempOut.CalcMinAverMax( tmp.Data_s/10.0 );

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				ExtTemp.CalcMinAverMax( tmp.Data_s/10.0 );

				ind+=6;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Sf1.CalcMinAverMax( tmp.Data_s/10.0 );
		    }
		    else if(func == 4 && reg == 129){
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				AdvState = tmp.Data_s;//(tmp.Data_s == 1 || tmp.Data_s == 3) ? 1 : 0;
		    }
		    else if(func == 1 && reg == 1)//on|off
		    {
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				State = tmp.Data_s;//(tmp.Data_s == 1 || tmp.Data_s == 3) ? 1 : 0;
				//Log::DEBUG( "[Enisey2000] tmp.Data_s="+toString(tmp.Data_s));

		    }
		    else if(func == 2 && reg == 3)
		    {
				for(int alInd=0; alInd < Warnings.Alarms.size(); alInd++){
					if(ind >= Len) break;
					//Warnings[alInd]->Value =   Buffer[ind++];
					Warnings[alInd]->SetValue(Buffer[ind++]);
				}
		    }
		    else if(func == 2 && reg == 52)
		    {
				for(int alInd=0; alInd < Alarms.Alarms.size(); alInd++){
					if(ind >= Len) break;
					//Alarms[alInd]->Value =   Buffer[ind++];
					Alarms[alInd]->SetValue(Buffer[ind++]);
					if(alInd == 2)
						Alarms[alInd]->Value = 0;//exclude not using datchik
				}
		    }

			Byte Fase    = CREATE_CMD;
			return Fase;
		}
//=============================================================================
sWord Enisey2000::RecvData( IPort* Port, Byte *Buf, Word MaxLen )
{
	//Log::DEBUG("Enisey2000::RecvData");
	int timeout_cnt = settings.RecvTimeout/100;
	if(timeout_cnt <= 0 ) timeout_cnt = 1;
	while(timeout_cnt--)
	{
		usleep(100000);//was 1000 mks=1ms
		if(this->DataLen > 0)
		{
			Word cnt = this->DataLen > MaxLen ? MaxLen: this->DataLen;
			for(Word i=0; i < cnt; i++){
				Buf[i] = Buffer[i];
			}
			this->DataLen = 0;
			return cnt;
		}
	}
	return 0;
}
//=============================================================================
bool Enisey2000::DetectStates()
{
    	bool ret = false;
    	//Log::DEBUG("Enisey2000::DetectStates Alarms.Alarms.size()=" + toString(Alarms.Alarms.size()));
    	for(auto curr: Alarms.Alarms){
    		//Log::DEBUG("Envicool::DetectStates index==" + toString(index++));
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

    	if(HAdvState != AdvState){
    		HAdvState = AdvState;
    		ret = true;
    	}

    	return ret;
  }
//=============================================================================
string Enisey2000::GetStringValue(void)
{

	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(RoomTemp.Aver, 0, 2)+','+FloatToString(RoomTemp.Min, 0, 2)+','+FloatToString(RoomTemp.Max, 0, 2)+',';
	RoomTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';
	ReturnString += FloatToString(ExtTemp.Aver, 0, 2)+','+FloatToString(ExtTemp.Min, 0, 2)+','+FloatToString(ExtTemp.Max, 0, 2)+',';
	ExtTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(3)+',';
	ReturnString += FloatToString(RoomTempOut.Aver, 0, 2)+','+FloatToString(RoomTempOut.Min, 0, 2)+','+FloatToString(RoomTempOut.Max, 0, 2)+',';
	RoomTempOut.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(71)+','+WordToString(1)+',';
	ReturnString += FloatToString(Humidity.Aver, 0, 2)+','+FloatToString(Humidity.Min, 0, 2)+','+ FloatToString(Humidity.Max, 0, 2)+',';
	Humidity.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(140)+','+WordToString(1)+',';
	ReturnString += WordToString(Sf1.Aver)+','+WordToString(Sf1.Min)+','+ WordToString(Sf1.Max)+',';
	Sf1.ClrDivider();

	ReturnString +=Alarms.GetValueString();

	ReturnString +=Warnings.GetValueString();

	ReturnString += "\r\n>,201,1,"+ WordToString(State)+','+ WordToString(AdvState)+',';


	ReturnString += PointSteps.GetValueString();
	//ReturnString += RunHours.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";
    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,99,1,"+WordToString(Type)+',';
	return ReturnString;
}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//***
//*****************************************************************************
TFreeCoolingManager::TFreeCoolingManager( void ):TFastTimer(3,&MilSecCount),
												//Fase(CREATE_CMD),
												Fase(RECV_CMD),
												SubFase(0),
												Period(600),
												DataLen(0),
												//fd(NULL),
												Port(NULL),Controller(NULL),NeedRestart(false)
{
	  SetTimer(PERIUD_TIMER, 50000);
	  SetTimer(ADDITIONAL_TIMER, 5000);
	  FirstInitFlg = true;
	  ModifyTime = Period;
}
//=============================================================================
TFreeCoolingManager::~TFreeCoolingManager()
{
	try
	{
		if(Controller != NULL)
		{
			delete Controller;
		}
	}
	catch(exception &e)
	{
		Log::ERROR( e.what() );
	}
}
//=============================================================================
void TFreeCoolingManager::Init( IPort *CurrCOM, void *config, void *driver )
{
  Port = (ComPort *)CurrCOM;
  vector<FreeSettings> sett = *((vector<FreeSettings> *)config);

  int name = 1;
  Log::DEBUG("Init FreeCoolingManager sett.size()=" + toString(sett.size()));
  if(sett.size() == 1)
  {
	  FreeSettings curr = sett[0];
	  Log::DEBUG("Init FreeCoolingManager curr.type=" + toString((int)curr.type) + " driver="+toString((int)driver));
	  if(curr.type == 2)
	  {
		  Port->settings 			= curr.portsettings;
		  Shtultz *sht 				= new Shtultz(curr.ename, curr.address, this);
		  sht->screenNumber.Data_s  = curr.screen;
		  Controller				= sht;
	  }
	  else if(curr.type == 3)
	  {
		  Port->SetVirtual(true);
		  Log::DEBUG("Init FreeCoolingManager new Envicool");
		  Envicool *env 			= new Envicool(28, curr.ename, curr.address, &curr.portsettings, driver);
		  Controller				= env;
		  Log::DEBUG("Init FreeCoolingManager Controller="+toString((int)Controller));
	  }
	  else if(curr.type == 4)
	  {
		  Port->SetVirtual(true);
		  Log::DEBUG("Init FreeCoolingManager new Enisey2000");
		  Enisey2000 *enis 			= new Enisey2000(29, curr.ename, curr.address, &curr.portsettings, driver);
		  Controller				= enis;
	  }
	  else if(curr.type == 5)
	  {
		  Port->SetVirtual(true);
		  Log::DEBUG("Init FreeCoolingManager new EnvicoolHC");
		  EnvicoolHC *envHC 		= new EnvicoolHC(28, curr.ename, curr.address, &curr.portsettings, driver);
		  Controller				= envHC;
		  Log::DEBUG("Init FreeCoolingManager Controller="+toString((int)Controller));
	  }
	  /*
	  else if(curr.type == 51)
	  {
		  Port->SetVirtual(true);
		  Log::DEBUG("Init FreeCoolingManager new vru");
		  Controller 		= new VruTm251(curr.etype, curr.ename, curr.address, &curr.portsettings, driver);

		  Log::DEBUG("Init FreeCoolingManager Controller="+toString((int)Controller));
	  }*/

	  Period 			=  curr.period;
	  Controller->comm_settings = curr.portsettings;
	  Controller->SetPeriod( Period );
  }
}
//=============================================================================
void TFreeCoolingManager::FREE_Restart( void)
{
	Log::DEBUG("FREE_Restart start");

	Fase = RECV_CMD;
	SubFase = 0;
	DataLen = 0;
	Controller->Restart();

	Log::DEBUG("FREE_Restart end");
}
//=============================================================================
void TFreeCoolingManager::FREE_CreateCMD( void)
{
    if(Controller != NULL)
    {
      DataLen = Controller->CreateCMD(SubFase, Buffer);

      //Log::DEBUG( "[TFreeCoolingManager] CreateCMD len="+toString(DataLen));

      if(DataLen == 0)
      {
    	  SubFase = 0;
    	  return;
      }
    }
    Fase    = SEND_CMD;
}
//=============================================================================
void TFreeCoolingManager::FREE_SendCMD( void )
{
	  if( Controller->SendData(Port, Buffer, DataLen) > 0 )
	  {//
		  /////////////////////

		  //Log::DEBUG( "[IbpManager] Send ok: [" + toString(Buffer) +"]"+" SubFase="+toString((int)SubFase));
		  Byte bf[2] {0,0};
		  string sended = "";
		  for(int i = 0; i < DataLen; i++)
		  {
			  ByteToHex(bf, Buffer[i]);
			  sended += "0x"+string((char*)bf, 2)+" ";
		  }
		  //Log::DEBUG( "[TFreeCoolingManager] send ok: [" + sended +"] len="+toString(DataLen)+" SubFase="+toString((int)SubFase));

		  ////////////////
		TBuffer::ClrBUF(Buffer, DataLen);
		Fase    = RECV_CMD;
	  }
	else
	{
		Fase    = EXEC_CMD;
		//Log::DEBUG("TDiselPanelManager::SendCMD DataLen="+toString(DataLen));
	}
}
//=============================================================================
void TFreeCoolingManager::FREE_RecvCMD( void )
{
	static int 	_answerErrorCNT = 0;
	sWord RecvLen = Controller->RecvData(Port, Buffer, sizeof(Buffer));
	if(RecvLen > 0 )
	{
	  DataLen     = RecvLen;
	  Fase        = EXEC_CMD;
	  _answerErrorCNT = 0;
	  //Log::DEBUG( "[TFreeCoolingManager] RecvData ok: [" + toString(Buffer) +"] len="+toString(RecvLen)+" SubFase="+toString((int)SubFase));
	    //////////////////////////
	   		  Byte bf[2] {0,0};
	    		  string recv = "";
	    		  for(int i = 0; i < RecvLen; i++)
	    		  {
	    			  ByteToHex(bf, Buffer[i]);
	    			  recv += "0x"+string((char*)bf, 2)+" ";
	    		  }
	    //Log::DEBUG( "[TFreeCoolingManager] RecvData ok: [" + recv +"] len="+toString(RecvLen)+" SubFase="+toString((int)SubFase));
	}
	else
	{
		if(_answerErrorCNT++ >= 10)
		{
			_answerErrorCNT       	= 0;
			Controller->ErrCnt++;
			if( Controller->IsEnable == true)
			{
				Controller->Modify     	= true;
				Controller->IsEnable 		= false;
				DateTime = SystemTime.GetTime();
			}

		    Port->Close();
		    Log::DEBUG( "[TFreeCoolingManager]::IBP_RecvCMD _answerErrorCNT >= 10, wait 5s");
		    SetTimer( COMMON_TIMER, 5000);
		}
		DataLen = 0;
		Fase    = CREATE_CMD;
	}
}
//=============================================================================
void TFreeCoolingManager::FREE_ExecCMD( void )
{
    if( Controller->ParsingAnswer(Buffer, DataLen) )
    {
    	Fase = Controller->GetDataFromMessage(Buffer, DataLen, SubFase );
        if( Controller->IsEnable == false)
        {
        	if(!FirstInitFlg)
        	{
        		 DateTime = SystemTime.GetTime();
        		 Controller->Modify = true;
        	 }
        	Controller->IsEnable   			  = true;
        }
        Controller->AckCnt++;
    	SetTimer( COMMON_TIMER, 500);
    }
    else
    {
    	Log::DEBUG( "[TFreeCoolingManager] ParsingAnswer ERROR");
    	Fase    = RECV_CMD;
    }
}
//=============================================================================
void TFreeCoolingManager::FREE_DoECMD( void )//Mercury only
{
	Fase    = RECV_CMD;
}
//=============================================================================
void TFreeCoolingManager::FREE_DetectStates( void  )
{

	if( !Enable()) return;

	  bool flag = false;
	  if( GetTimValue(PERIUD_TIMER) <= 0 )
	  {
	    	DWord tim = SystemTime.GetGlobalSeconds();
	    	if(FirstInitFlg == true)
	    	{
	    		Log::DEBUG("FREE_DetectStates FirstInitFlg="+toString(FirstInitFlg));
	    		FirstInitFlg = false;
	    		flag     = true;
	    		ModifyTime = ((DWord)( (DWord)(tim / Period) + 1)) * Period;
	    		if(ModifyTime > MAX_SECOND_IN_DAY)
	    			ModifyTime = Period;
	    	}
	    	else
	    	{
	    		long err = ModifyTime-tim;
	    		if( (ModifyTime <= tim) || (err > Period*2) )
	    		{
	    			//Log::DEBUG("FREE_DetectStates err="+toString(err)+" ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
	    			flag     = true;
		    		ModifyTime = ((DWord)( (DWord)(tim / Period) + 1)) * Period;
		    		if(ModifyTime > MAX_SECOND_IN_DAY)
		    			ModifyTime = Period;
	    		}
	    	}
		  if( flag == true )
		  {
			  DateTime = SystemTime.GetTime();
			  //Log::DEBUG("FREE_DetectStates Complite ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
			  Controller->Modify = true;//new

			  SetTimer( PERIUD_TIMER, 10000 );
		  }
		  else
		  {
			  Controller->Modify = Controller->DetectStates();//new
			  SetTimer( PERIUD_TIMER, 500 );
		  }
	  }
}
//=============================================================================
void TFreeCoolingManager::FREE_CreateMessageCMD( void *Par )
{
if( Par != NULL && Enable())
  {
    if(ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0)
    {
      string framStr = "";
      if( Controller->Modify == true )
      {
    	  Log::DEBUG( "[FREE_CreateMessageCMD] IBP_CreateMessageCMD");
    	  if(DateTime.Year == 0){
    		  DateTime = SystemTime.GetTime();}
    	  framStr += TBuffer::DateTimeToString( &DateTime )+">>";
    	  DateTime.Year  = 0;
    	  framStr += Controller->GetStringValue();
    	  if(framStr.size() > 0){
    		  TFifo<string> *framFifo = (TFifo<string> *)Par;
    		  framFifo->push( framStr );
    	  }
    	  framStr.clear();
    	  Controller->Modify = false;
      }
    }
  }
}
//=============================================================================
/*void TFreeCoolingManager::SetScreen(Word value)
{
	if(Controller != NULL)
	{
		//Shtultz *sht = (Shtultz *)Controller;
		//sht->newScreenNumber.Data_s = value;
		Controller->SetScreen(value);
	}
}
//=============================================================================
Word TFreeCoolingManager::GetScreen()
{
	if(Controller != NULL)
	{
		Shtultz *sht = (Shtultz *)Controller;
		return sht->newScreenNumber.Data_s;
	}
	return 0;
}*/
//=============================================================================
/*void TFreeCoolingManager::AddScenario(string name, Byte face, Byte sface, string strFace, string nv, Word cmd)
{
	if(Controller != NULL)
	{
		Scenario* scen = new Scenario(name);
		scen->Steps.push_back(new ScenarioStep(face, sface, strFace, nv));
		scen->SetCmd 	= cmd;
		Shtultz *sht 	= (Shtultz *)Controller;
		sht->AddAdditionalScenario(scen);
	}
}*/
//=============================================================================
void TFreeCoolingManager::EditScenario(string name, Byte face, Byte sface, string strFace, string nv, bool clear)
{
	if(Controller != NULL){
		if(Controller->Type == 2){
			Shtultz *sht 	= (Shtultz *)Controller;
			for(auto currSc: sht->ScenarioList){
				if(currSc->Name == name){
					Log::DEBUG("EditScenario name="+name);
					for(auto currSt: currSc->Steps){
						if(currSt->scrFace==face && currSt->scrSubFace==sface && currSt->ssFace==strFace){
							currSt->newValue = nv;
							if(clear){
								Log::DEBUG("clearCurrScenario");
								sht->clearCurrScenario();
							}
							break;
						}
					}
				}
			}
		}
		else if(Controller->Type == 3){

		}
	}
}
//=============================================================================
void TFreeCoolingManager::ChangeState(Word newState)
{
	if(Controller != NULL){
		if(Controller->Type == 2)
		{
			Shtultz *sht 	= (Shtultz *)Controller;
			Word currState = sht->State;
			if(currState != newState){
				sht->Modify = true;
				EditScenario("DefaultData", 1, 1, "", "2048", true);
				Log::DEBUG("ChangeState, newState="+toString(newState));
			}
		}
		else// if(Controller->Type == 3)
		{
			Word currState = Controller->State;
			//if(currState != newState)
			{
				Controller->ChangeState(newState);
				//Log::DEBUG("ChangeState, newState="+toString(newState));
			}
		}
	}
}
//=============================================================================
void TFreeCoolingManager::ChangeParameters(vector<string> parameters)
{
	if(Controller != NULL){
		//Log::DEBUG("TFreeCoolingManager::ChangeParameters Controller->Type="+toString((int)Controller->Type));
		if(Controller->Type > 2){
			Controller->ChangeParameters(parameters);
		}
	}
}
/*
//=============================================================================
void TFreeCoolingManager::AddPasswordScenario()
{
	if(Controller != NULL)
	{
		Scenario *scen = new Scenario("TmpPassWord");
		scen->Steps.push_back(new ScenarioStep(2,1));
		scen->Steps.push_back(new ScenarioStep(2,2));
		scen->Steps.push_back(new ScenarioStep(2,3));
		Shtultz *sht 	= (Shtultz *)Controller;
		sht->AddAdditionalScenario(scen);
	}
}*/
//=============================================================================
void TFreeCoolingManager::FREE_Manage()
{
	if(Controller != NULL){
		Controller->ManageData();
		if(GetTimValue(ADDITIONAL_TIMER) <= 0){
			Controller->ManageScenario();
		}
	}
}
//=============================================================================

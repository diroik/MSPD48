/*
 * DiselPanel.cpp
 *
 *  Created on: Nov 14, 2016
 *      Author: user
 */

#include <DiselPanel.h>


//*****************************************************************************
//***
//*****************************************************************************
//=============================================================================
Ipanel::Ipanel(Byte Etype, Byte Ename, Byte adr):address(adr), Et(Etype), En(Ename),
	Modify(false),IsEnable(false), Unet(42,2),Fnet(48, 2),
	PointSteps(230, 1, 0), RunHours(232, 1 ,1),
	Alarms(0,200,1),Warnings(0,200,101)
{
	DataLen = 0;
	driver = NULL;

	EngMode 		= 0;
	HEngMode       = 0;

	Mode 		= 0;
	HMode       = 0;

	State 		= 0;
	HState      = 0;

	type = 0;
	SerialNo = "0";
	Port = NULL;
	BetweenTimeout = 10;
	NeedInitTimeout = 100;
	NeedInit = false;
	ErrCnt = 0;
	AckCnt = 0;
	modbus_cmd_size = 0;

	Ktran = 1.0;
	//last_subfase = 0;
}
//=============================================================================
Ipanel::~Ipanel()
{

}
//=============================================================================
void Ipanel::SetPeriod(Word Period)
{
  //UE.Period=U.Period= Period;
}
//=============================================================================
string Ipanel::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += U.GetValueString();
	ReturnString += I.GetValueString();
	ReturnString += P.GetValueString();
	ReturnString += F.GetValueString();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(CoolTemp.Aver, 0, 2)+','+FloatToString(CoolTemp.Min, 0, 2)+','+FloatToString(CoolTemp.Max, 0, 2)+',';
	CoolTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(85)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vbatt.Aver, 0, 2)+','+FloatToString(Vbatt.Min, 0, 2)+','+FloatToString(Vbatt.Max, 0, 2)+',';
	Vbatt.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(87)+','+WordToString(1)+',';
	ReturnString += WordToString(RPM.Aver)+','+WordToString(RPM.Min)+','+WordToString(RPM.Max)+',';
	RPM.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(88)+','+WordToString(1)+',';
	ReturnString += FloatToString(OilPressure.Aver, 0, 2)+','+FloatToString(OilPressure.Min, 0, 2)+','+FloatToString(OilPressure.Max, 0, 2)+',';
	OilPressure.ClrDivider();

	ReturnString +=Alarms.GetValueString();
	ReturnString += "\r\n>,201,1,"+ WordToString(State)+','+WordToString(Mode)+',';

	ReturnString += PointSteps.GetValueString();
	ReturnString += RunHours.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";

    ReturnString += WordToString(IsEnable)+',';
    ReturnString += "\r\n>,99,1,"+WordToString(type)+',';


	return ReturnString;
}
//=============================================================================
string Ipanel::GetStringCfgValue(void)
{
	 string ret = "";
	  ret += "{";
	  ret += "\"Et\":"+toString((int)Et)+",";
	  ret += "\"En\":"+toString((int)En)+",";
	  ret += "\"Data0\":"+toString((int)State)+",";
	  ret += "\"Data1\":"+toString((int)Mode)+",";
	  ret += "\"Data2\":"+toString((int)RPM.Value)+",";
	  ret += "\"Data3\":"+toString((int)Fuel.Value)+",";
	  ret += "\"Data4\":"+toString((int)U.Ua.Value)+",";
	  ret += "\"Data5\":"+toString((int)U.Ub.Value)+",";
	  ret += "\"Data6\":"+toString((int)U.Uc.Value)+",";
	  ret += "\"Data7\":"+toString((int)P.P.Value)+",";
	  ret += "\"Address\":"+toString((int)address)+",";
	  ret += "\"Number\":\""+SerialNo+"\""+",";
	  ret += "\"IsEnable\":"+toString(IsEnable)+",";
	  ret += "\"AckCnt\":"+toString((int)AckCnt)+",";
	  ret += "\"ErrCnt\":"+toString((int)ErrCnt);
	  ret+="},";
	 return ret;
}
//=============================================================================
sWord  Ipanel::SendData(IPort* port, Byte *Buf, Word Len)
{
	return port->Send(Buf, Len);
}
//=============================================================================
sWord   Ipanel::RecvData(IPort* port, Byte *Buf, Word MaxLen, Byte subfase)
{
	sWord RecvLen = 0;
	return RecvLen;
}
//=============================================================================
bool Ipanel::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len < 5) return false;
	Byte MessCRC = BUF[Len-1];
	Byte CalcCRC = TBuffer::Crc8( BUF, Len-1);

	return MessCRC == CalcCRC;
}
//=============================================================================
bool Ipanel::ChangeState( short newState )
{
	return true;
}
//=============================================================================
bool Ipanel::ChangeParameters( vector<string> parameters )
{
	return true;
}
//=============================================================================
void Ipanel::transportData(void *data, void *ptr)
{
	void *ret = NULL;
	try
	{
		if(data != NULL && ptr != NULL)
		{
			IModbusRegisterList *curr = (IModbusRegisterList *)data;
			Ipanel			*_this = (Ipanel*)ptr;

			_this->DataLen = 0;
			Bshort tmp;
			Word len = 0;
			tmp.Data_s = curr->reg;
			_this->Buffer[len++] = curr->fun;
			_this->Buffer[len++] = tmp.Data_b[0];
			_this->Buffer[len++] = tmp.Data_b[1];

			if(curr->fun == 3)
			{
				vector<short> *data = (vector<short> *)curr->GetData();
				int size = data->size();
				for(int i = 0; i < size; i++)
				{
					tmp.Data_s = data->operator [](i);
					_this->Buffer[len++] = tmp.Data_b[0];
					_this->Buffer[len++] = tmp.Data_b[1];
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
		Log::ERROR( "[Ipanel::transportData] "+string(e.what()) );
	}
}
//*****************************************************************************
//***
//*****************************************************************************
Pw2Panel::Pw2Panel(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ipanel(Etype, Ename, adr)
{
	//Log::DEBUG("Pw2Panel driver="+toString( (int)dr ));
	address = adr;
	type = 3;
	driver = (TModbusManager *)dr;
	settings = *((CommSettings*)sett);

	if(driver != NULL)
	{
		driver->Init(settings);
		//driver->ClearDeviceList();
		int slen = driver->DeviceList.size();
		driver->AddToDeviceList(address, 3, 199,  	7,  this);
		driver->AddToDeviceList(address, 3, 101,   15,  this);	//!!!
		driver->AddToDeviceList(address, 3, 203,  	2,  this);
		driver->AddToDeviceList(address, 3, 212,  	2,  this);

		driver->AddToDeviceList(address, 3, 116,  	28, this);
		driver->AddToDeviceList(address, 3, 300,  	1,  this);

		driver->AddToDeviceList(address, 3, 0x4fb,  6,  this);
		driver->AddToDeviceList(address, 3, 0x14e,  1,  this);
		driver->AddToDeviceList(address, 3, 0x41c,  1,  this);

		//driver->outPtr = this;
		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();
		ModelNo = "PW2.0";
	}

	for(int i = 0; i < 8; i++){
		Warnings.Alarms.push_back(new THistBoolParam());
	}
	RunHours.Params.push_back(0);//!!!
}
//=============================================================================
Pw2Panel::~Pw2Panel() {
}
//=============================================================================
Word Pw2Panel::CreateCMD(Byte CMD, Byte *Buffer){
	Word DataLen = 1;

	if(CMD > modbus_cmd_size  && CMD < 98){
		DataLen = 0;
	}

	//Log::DEBUG("Dse7xxxPanel::CreateCMD");

	return DataLen;
}
//=============================================================================
Byte Pw2Panel::GetDataFromMessage(Byte subFase, Byte *Buffer, Word Len)
{
	if(Len > 3)
	{
			sBshort tmp;
			Blong   long_tmp;
			Word ind = 0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];
			//Log::DEBUG("Pw2Panel::GetDataFromMessage Len="+toString(Len)+ " reg="+toString(tmp.Data_s));
			switch(tmp.Data_s){
			case 199:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				OilPressure.CalcMinAverMax( (float)tmp.Data_s*0.125/100.0);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				CoolTemp.CalcMinAverMax( (float)tmp.Data_s*0.03125-273.0);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Vbatt.CalcMinAverMax( (float)tmp.Data_s*0.05);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				RPM.CalcMinAverMax( tmp.Data_s*0.125 );
				Mode = RPM.Value > 10 ? 1 : 0;
				break;

			case 203:
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				RunHours.Params[0] = long_tmp.Data_l*0.05;
				break;

			case 212:
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				RunHours.Params[0] = long_tmp.Data_l*0.05;
				break;

			case 101:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				F.F.CalcMinAverMax( (float)tmp.Data_s/128.0);

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				KM.KM.CalcMinAverMax( ((float)tmp.Data_s/16384.0)-1.0);

				ind+=4;
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.P.CalcMinAverMax( Ktran*(long_tmp.Data_l-2000000000)/1000.0 );
				ind+=6;
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];

				I.Ia.CalcMinAverMax( Ktran*(float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				I.Ib.CalcMinAverMax( Ktran*(float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				I.Ic.CalcMinAverMax( Ktran*(float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];

				U.Ua.CalcMinAverMax( (float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				U.Ub.CalcMinAverMax( (float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				U.Uc.CalcMinAverMax( (float)tmp.Data_s);
				//Mode = (U.Ua.Value+U.Ub.Value+U.Uc.Value) > 1;
				break;

			case 116:
				long_tmp.Data_b[2] = Buffer[ind++];//116
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pa.CalcMinAverMax( Ktran*(long_tmp.Data_l-2000000000)/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];//118
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pb.CalcMinAverMax( Ktran*(long_tmp.Data_l-2000000000)/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];//120
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pc.CalcMinAverMax( Ktran*(long_tmp.Data_l-2000000000)/1000.0 );

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				S.Sa.CalcMinAverMax( Ktran*(long_tmp.Data_l-2000000000)/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				S.Sb.CalcMinAverMax( Ktran*(long_tmp.Data_l-2000000000)/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				S.Sc.CalcMinAverMax( Ktran*(long_tmp.Data_l-2000000000)/1000.0 );

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				Q.Qa.CalcMinAverMax( Ktran*((sDWord)long_tmp.Data_l-2000000000)/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				Q.Qb.CalcMinAverMax( Ktran*((sDWord)long_tmp.Data_l-2000000000)/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				Q.Qc.CalcMinAverMax( Ktran*((sDWord)long_tmp.Data_l-2000000000)/1000.0 );
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];

				KM.KMa.CalcMinAverMax( ((float)tmp.Data_s/16384.0)-1.0);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				KM.KMb.CalcMinAverMax( ((float)tmp.Data_s/16384.0)-1.0);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				KM.KMc.CalcMinAverMax( ((float)tmp.Data_s/16384.0)-1.0);

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				S.S.CalcMinAverMax( Ktran*(long_tmp.Data_l-2000000000)/1000.0 );
				ind+=2;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				Q.Q.CalcMinAverMax( Ktran*((sDWord)long_tmp.Data_l-2000000000)/1000.0 );
				break;

			case 300:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				State = tmp.Data_s;
				break;

			case 0x4fb://serial 1275
				{
					Log::DEBUG("Pw2Panel::GetDataFromMessage Len="+toString(Len)+ " reg="+toString(tmp.Data_s));
					int len = Len - ind;
					if(len > 0)
					{
						string tstr = string((char*)&Buffer[ind], len);
						tstr = trimAllNull(tstr);
						Log::DEBUG("Pw2Panel::GetDataFromMessage tstr="+toString(tstr));
						if(!isNullOrWhiteSpace(tstr)) //isNullOrWhiteSpace
						{
							SerialNo = tstr;
						}
					}
				}
				break;

			case 0x41c://Engine status 1052
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				HEngMode = tmp.Data_s;
				break;

			case 0x14e://lamp 334
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Log::DEBUG("Dse7xxxPanel::GetDataFromMessage reg=334 value="+toString((Short)tmp.Data_s));

				for(int i=0; i < Warnings.size(); i++){
					//Warnings[i]->SetValue((bool)(((Short)tmp.Data_s >> i*2) & 3));
					Warnings[i]->SetValue((bool)(((Short)tmp.Data_s >> i) & 1));
				}
				break;

			default:
				break;
			}

	}
	return subFase;//99;
}
//=============================================================================
sWord Pw2Panel::SendData( IPort* Port, Byte *Buf, Word Len )
{

	//Log::DEBUG("Pw2Panel::SendData");
	return Len;//0
}
//=============================================================================
sWord Pw2Panel::RecvData( IPort* Port, Byte *Buf, Word MaxLen , Byte subfase)
{
	//Log::DEBUG("Pw2Panel::RecvData");
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
bool Pw2Panel::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len == 0) return true;
	return Len >=4;
}
//=============================================================================
bool Pw2Panel::ChangeState(short newState)
{//0-stop 1-auto 2-run
	  bool ret = false;
	  if(driver!=NULL){

		  uint16_t st = newState;
		  uint16_t data[2];

		  switch(st)
		  {
		  case 0:
		  case 1:
		  case 2:
			  driver->AddToDeviceList(address, 6, 301,  0, this, st);
			  break;

		  case 7:
			  st = 1;
			  driver->AddToDeviceList(address, 6, 0x0134,  0, this, st);
			  break;

		  };


	  }
	  return ret;
}
//=============================================================================
bool Pw2Panel::ChangeParameters(vector<string> parameters){
	  bool ret = false;
	  if(driver != NULL){
		  /*
		  for(int i=0; i < parameters.size(); i++){
			  short newVal = (short)(atof( parameters[i].c_str() )*10);
			  Log::DEBUG("reg="+toString(i)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
    		  driver->AddToDeviceList(address, 6, i,  0, this, newVal);
		  }
		  */
	  }
	  return ret;
}
//=============================================================================
string Pw2Panel::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += U.GetValueString();
	ReturnString += I.GetValueString();
	ReturnString += P.GetValueString();
	//ReturnString += Q.GetValueString();
	ReturnString += S.GetValueString();
	ReturnString += KM.GetValueString();
	ReturnString += F.GetValueString();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(CoolTemp.Aver, 0, 2)+','+FloatToString(CoolTemp.Min, 0, 2)+','+FloatToString(CoolTemp.Max, 0, 2)+',';
	CoolTemp.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(85)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vbatt.Aver, 0, 2)+','+FloatToString(Vbatt.Min, 0, 2)+','+FloatToString(Vbatt.Max, 0, 2)+',';
	Vbatt.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(87)+','+WordToString(1)+',';
	ReturnString += WordToString(RPM.Aver)+','+WordToString(RPM.Min)+','+WordToString(RPM.Max)+',';
	RPM.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(88)+','+WordToString(1)+',';
	ReturnString += FloatToString(OilPressure.Aver, 0, 2)+','+FloatToString(OilPressure.Min, 0, 2)+','+FloatToString(OilPressure.Max, 0, 2)+',';
	OilPressure.ClrDivider();

	ReturnString += Warnings.GetValueString();
	ReturnString += "\r\n>,201,1,"+ WordToString(State)+','+ WordToString(Mode)+',';
	//ReturnString += "\r\n>,201,2,"+ WordToString(Mode)+',';
	ReturnString += "\r\n>,201,3,"+ WordToString(HEngMode)+',';

	ReturnString += PointSteps.GetValueString();
	ReturnString += RunHours.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";

    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,50,1,"+ModelNo+",";

    ReturnString += "\r\n>,99,1,"+WordToString(type)+',';


	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
Dse7xxxPanel::Dse7xxxPanel(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ipanel(Etype, Ename, adr)
{
	//Log::DEBUG("Dse7xxxPanel driver="+toString( (int)dr ));
	ModelNo = "DSE7xxx";
	address 	= adr;
	type 		= 4;
	DataLen 	= 0;
	driver 		= (TModbusManager *)dr;
	settings 	= *((CommSettings*)sett);

	bool ip_flg = false;
	unsigned long ipadr;
	ipadr = inet_addr(settings.DeviceName.c_str());
	if (ipadr != INADDR_NONE) ip_flg = true;

	if(driver != NULL)
	{
		driver->Init(settings);
		//driver->ClearDeviceList();
		int slen = driver->DeviceList.size();

		driver->AddToDeviceList(address, 3, 1024,   7,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);
		driver->AddToDeviceList(address, 3, 1031,   7,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);
		driver->AddToDeviceList(address, 3, 1044,   14, this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);
		driver->AddToDeviceList(address, 3, 1059,   7,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);
		driver->AddToDeviceList(address, 3, 768,    7,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);
		driver->AddToDeviceList(address, 3, 1798,   12, this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);
		driver->AddToDeviceList(address, 3, 2048,   9,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);

		//driver->outPtr = this;
		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();

		Log::DEBUG("Dse7xxxPanel ip_flg="+toString( (int)ip_flg ) + " ip="+settings.DeviceName.c_str()+" adr="+toString( (int)adr ));

	}
	for(int i = 0; i < 10; i++){
		Alarms.Alarms.push_back(new THistBoolParam());
	}
	RunHours.Params.push_back(0);

	AlarmCondition = "";
	HAlarmCondition = "";
}
//=============================================================================
Dse7xxxPanel::~Dse7xxxPanel()
{
}
//=============================================================================
Word Dse7xxxPanel::CreateCMD(Byte CMD, Byte *Buffer){
	Word DataLen = 1;

	if(CMD > modbus_cmd_size  && CMD < 98){
		DataLen = 0;
	}

	//Log::DEBUG("Dse7xxxPanel::CreateCMD");

	return DataLen;
}
//=============================================================================
Byte Dse7xxxPanel::GetDataFromMessage(Byte subFase, Byte *Buffer, Word Len)
{
	if(Len > 3)
	{
			sBshort tmp;
			Blong   long_tmp;
			Word 	ind = 0;
			float 	fu = 0.0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];
			//Log::DEBUG("Dse7xxxPanel::GetDataFromMessage Len="+toString(Len)+ " reg="+toString(tmp.Data_s));

			switch(tmp.Data_s)
			{
			case 768:


				ind+=2;
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				if(tmp.Data_s > 0 && (Short)tmp.Data_s < 65530){
					ModelNo = "DSE"+toString(tmp.Data_s);
				}
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];

				//Log::DEBUG("Dse7xxxPanel::SerialNo Data_l="+toString(long_tmp.Data_l));
				if(long_tmp.Data_l > 0 && long_tmp.Data_l <= 999999999){
					SerialNo = toString(long_tmp.Data_l);
					Log::DEBUG("Dse7xxxPanel::SerialNo="+ toString(SerialNo));
				}

				//Log::DEBUG("Dse7xxxPanel::SerialNo="+toString(long_tmp.Data_l));

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				State = GetState((Short)tmp.Data_s);
				ind+=2;
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				for(int i=0;i<Alarms.size();i++){
					//Alarms[i]->Value = (bool)(((Short)tmp.Data_s >> i+8) & 1);
					Alarms[i]->SetValue((bool)(((Short)tmp.Data_s >> i+8) & 1));
				}
				break;

			case 1024:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				OilPressure.CalcMinAverMax( (Short)tmp.Data_s/1.0);

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				CoolTemp.CalcMinAverMax( (Short)tmp.Data_s/1.0);

				//ind+=2;//reserv
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				OilTemp.CalcMinAverMax( (Short)tmp.Data_s/1.0);


				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				if( tmp.Data_s < 0 || tmp.Data_s > 199 )
				{	tmp.Data_s = 199;}
				//Log::DEBUG("fuel tmp.Data_s="+toString(tmp.Data_s));
				fu = tmp.Data_s/1.0;
				if(fu > 100.0)
					fu = 100.0;
				Fuel.CalcMinAverMax(fu);

				//ind+=2;
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Vgen.CalcMinAverMax( (Short)tmp.Data_s/10.0);

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Vbatt.CalcMinAverMax( (Short)tmp.Data_s/10.0);

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				RPM.CalcMinAverMax( (Short)tmp.Data_s/1.0);
				break;

			case 1031:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				F.F.CalcMinAverMax( (Short)tmp.Data_s/10.0);

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				U.Ua.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 );

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				U.Ub.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 );

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				U.Uc.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 );
				Mode = (U.Ua.Value+U.Ub.Value+U.Uc.Value) > 1;

				Log::DEBUG("Dse7xxxPanel::Mode="+toString((int)Mode)+" U.Ua.Value="+toString(U.Ua.Value)+" U.Ub.Value="+toString(U.Ub.Value)+" U.Uc.Value="+toString(U.Uc.Value));

				break;

			case 1044:
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				I.Ia.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 *Ktran);

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				I.Ib.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 *Ktran);

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				I.Ic.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 *Ktran);

				ind+=4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pa.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 *Ktran);

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pb.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 *Ktran);

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pc.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 *Ktran);

				P.P.CalcMinAverMax(P.Pa.Value+P.Pb.Value+P.Pc.Value);
				break;

			case 1059:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Fnet.F.CalcMinAverMax( (Short)tmp.Data_s/10.0);

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				Unet.Ua.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 );

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				Unet.Ub.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 );

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				Unet.Uc.CalcMinAverMax( (DWord)(long_tmp.Data_l)/10.0 );
				break;

			case 1798:
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				RunHours.Params[0] = long_tmp.Data_l/3600;
				ind+=16;
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				RunHours.Params[1] = long_tmp.Data_l;
				break;

			case 2048:
				//Log::DEBUG("Dse7xxxPanel::GetDataFromMessage Len="+toString(Len)+ " reg="+toString(tmp.Data_s));
				{
			    		ind += 2;//skip cnt
			    		int len = Len - (ind + 2);

			    		string ss = "";
			    		if(len > 0)
			    		{
			    			for(int i=0; i < len; i++)
			    			{
			    				Byte d = Buffer[ind++];
			    				ss+= toString((int)(d & 0x0F) )  + "," + toString((int)( (d >> 4) & 0x0F) )  + ",";
			    			}
			    		}
			    		AlarmCondition = ss;
			    		Log::DEBUG( "Dse7xxxPanel::GetDataFromMessage len = "+toString(len) + " ss="+ ss);
				}
				break;


			default:
				break;
			}

	}
	return subFase;//98;//99;
}
//=============================================================================
sWord Dse7xxxPanel::SendData( IPort* Port, Byte *Buf, Word Len )
{

	//Log::DEBUG("Dse7xxxPanel::SendData");
	return Len;//0;
}
//=============================================================================
sWord Dse7xxxPanel::RecvData( IPort* Port, Byte *Buf, Word MaxLen , Byte subfase)
{
	//Log::DEBUG("Dse7xxxPanel::RecvData");
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
bool Dse7xxxPanel::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len == 0) return true;
	return Len >= 4;
}
//=============================================================================
bool Dse7xxxPanel::ChangeState(short newState){
	  bool ret = false;
	  if(driver!=NULL){

		  uint16_t st = newState;
		  uint16_t data[2];

		  switch(st)
		  {

			  case 7://reset alarms
				data[0] = (uint16_t)35707;
				data[1] = (uint16_t)(65535-35707);
				driver->AddToDeviceList(address, 16, 0x1008,  2, this, (long)data);
				break;

		  	  case 1://auto mode
		  		data[0] = (uint16_t)35701;
		  		data[1] = (uint16_t)(65535-35701);
		  		driver->AddToDeviceList(address, 16, 0x1008,  2, this, (long)data);
		  		break;
		  	  case 2://test on load
			  	data[0] = (uint16_t)35703;
			  	data[1] = (uint16_t)(65535-35703);
			  	driver->AddToDeviceList(address, 16, 0x1008,  2, this, (long)data);

				data[0] = (uint16_t)35705;
				data[1] = (uint16_t)(65535-35705);
				driver->AddToDeviceList(address, 16, 0x1008,  2, this, (long)data);
		  		break;
		  	  case 3://manual mode + start
				data[0] = (uint16_t)35702;
				data[1] = (uint16_t)(65535-35702);
				driver->AddToDeviceList(address, 16, 0x1008,  2, this, (long)data);

				data[0] = (uint16_t)35705;
				data[1] = (uint16_t)(65535-35705);
				driver->AddToDeviceList(address, 16, 0x1008,  2, this, (long)data);
		  		break;
		  	  case 4://stop mode
				data[0] = (uint16_t)35700;
				data[1] = (uint16_t)(65535-35700);
				driver->AddToDeviceList(address, 16, 0x1008,  2, this, (long)data);
		  		break;
		  }
		  //Log::DEBUG("newState="+toString(newState)+" st="+toString(st));
	  }
	  return ret;
}
//=============================================================================
bool Dse7xxxPanel::ChangeParameters(vector<string> parameters){
	  bool ret = false;
	  if(driver != NULL)
	  {
		  for(int i=0; i < parameters.size(); i++)
		  {
			  short newVal = (short)(atof( parameters[i].c_str() )*10);
			  Log::DEBUG("reg="+toString(i)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
    		  //driver->AddToDeviceList(address, 6, i,  0, this, newVal);
		  }
	  }
	  return ret;
}
//=============================================================================
string Dse7xxxPanel::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += U.GetValueString();
	ReturnString += Unet.GetValueString();
	ReturnString += P.GetValueString();
	ReturnString += F.GetValueString();
	ReturnString += Fnet.GetValueString();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(CoolTemp.Aver, 0, 2)+','+FloatToString(CoolTemp.Min, 0, 2)+','+FloatToString(CoolTemp.Max, 0, 2)+',';
	CoolTemp.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';
	ReturnString += FloatToString(OilTemp.Aver, 0, 2)+','+FloatToString(OilTemp.Min, 0, 2)+','+FloatToString(OilTemp.Max, 0, 2)+',';
	OilTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(85)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vbatt.Aver, 0, 2)+','+FloatToString(Vbatt.Min, 0, 2)+','+FloatToString(Vbatt.Max, 0, 2)+',';
	Vbatt.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(85)+','+WordToString(2)+',';
	ReturnString += FloatToString(Vgen.Aver, 0, 2)+','+FloatToString(Vgen.Min, 0, 2)+','+FloatToString(Vgen.Max, 0, 2)+',';
	Vgen.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(86)+','+WordToString(1)+',';
	ReturnString += WordToString(Fuel.Aver)+','+WordToString(Fuel.Min)+','+WordToString(Fuel.Max)+',';
	Fuel.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(87)+','+WordToString(1)+',';
	ReturnString += WordToString(RPM.Aver)+','+WordToString(RPM.Min)+','+WordToString(RPM.Max)+',';
	RPM.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(88)+','+WordToString(1)+',';
	ReturnString += FloatToString(OilPressure.Aver, 0, 2)+','+FloatToString(OilPressure.Min, 0, 2)+','+FloatToString(OilPressure.Max, 0, 2)+',';
	OilPressure.ClrDivider();



	string ttt = "\r\n>,201,1,"+ WordToString(State)+','+ WordToString(Mode)+',';
	ReturnString += ttt;
	Log::DEBUG("Dse7xxxPanel::GetStringValue Mode="+toString((int)Mode) + " State="+toString((int)State) + " ttt=["+ttt+"]");

	ReturnString +=Alarms.GetValueString();

	if(!isNullOrWhiteSpace(AlarmCondition))
		ReturnString += "\r\n>,200,3,"+ AlarmCondition +',';

	ReturnString += PointSteps.GetValueString();
	ReturnString += RunHours.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0"){
		snumb = "0";

	}
	Log::DEBUG("Dse7xxxPanel::GetStringValue SerialNo="+toString(SerialNo) );

    ReturnString += "\r\n>,49,1,"+snumb+",";
    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,50,1,"+ModelNo+",";

    ReturnString += "\r\n>,99,1,"+WordToString(type)+',';
	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
Blc200Panel::Blc200Panel(Byte Etype, Byte Ename, Byte adr):Ipanel(Etype, Ename, adr)
{
	type = 2;
	driver = NULL;

	for(int i = 0; i < 22; i++){
		Alarms.Alarms.push_back(new THistBoolParam());
	}
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}
	RunHours.Params.push_back(0);
}
//=============================================================================
Blc200Panel::~Blc200Panel() {

}
//=============================================================================
Word Blc200Panel::CreateCMD(Byte CMD, Byte *Buffer){
	 Word DataLen = 0;
	 string Tmp;


	 if(NeedInit){
		 Buffer[DataLen++] = 0x43;
	 }
	 else
	 {
		 switch(CMD)//Subfase
		 {
			case 0://init
				Buffer[DataLen++] = 0x43;
				break;

			case 1://F net
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x38;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 2://F gen
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x39;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 3://L1N
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x33;
				Buffer[DataLen++] = 0x33;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 4://L2N
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x33;
				Buffer[DataLen++] = 0x34;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 5://L2N
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x33;
				Buffer[DataLen++] = 0x35;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 6://L1G
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x33;
				Buffer[DataLen++] = 0x36;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 7://L2G
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x33;
				Buffer[DataLen++] = 0x37;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 8://L3G
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x33;
				Buffer[DataLen++] = 0x38;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 9://Fuel
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x33;
				Buffer[DataLen++] = 0x39;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 10://WL1N
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x34;
				Buffer[DataLen++] = 0x31;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 11://WL2N
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x34;
				Buffer[DataLen++] = 0x32;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 12://WL3N
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x34;
				Buffer[DataLen++] = 0x33;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 13://alarm
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x32;
				Buffer[DataLen++] = 0x34;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 14://motohours
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x31;
				Buffer[DataLen++] = 0x32;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 15://gen work state
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x34;
				Buffer[DataLen++] = 0x37;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 16://U akb
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x35;
				Buffer[DataLen++] = 0x36;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 17://motor temp
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x35;
				Buffer[DataLen++] = 0x38;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 18:// limit < V  PointStep
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x31;
				Buffer[DataLen++] = 0x31;
				Buffer[DataLen++] = 0x31;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 19:// limit > V  PointStep
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x31;
				Buffer[DataLen++] = 0x31;
				Buffer[DataLen++] = 0x32;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 20:// V min fail duration   PointStep
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x39;
				Buffer[DataLen++] = 0x31;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 21:// V recovery duration   PointStep
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x39;
				Buffer[DataLen++] = 0x32;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 22://out temp
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x35;
				Buffer[DataLen++] = 0x37;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 23://start count
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x31;
				Buffer[DataLen++] = 0x30;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;



			case 97:
				//Log::DEBUG("SubFase=97");
				if(CmdMessages.size() > 0){
					string mess = CmdMessages.front();
					CmdMessages.pop_front();
					vector<string>  cmd = TBuffer::Split(mess, " ");
					Log::DEBUG("CmdMessages mess=["+mess+"] split size="+toString(cmd.size()));
					if(cmd.size() > 1){
						for(int i=0; i < cmd.size(); i++){
							sWord b = HexToByte((char*)cmd[i].c_str());
							if(b > 0){
								Buffer[DataLen++] = b;
							}
						}
						/////
						/*
							  Byte bf[2] {0,0};
							  string send = "";
							  for(int i = 0; i < DataLen; i++)
							  {
								  ByteToHex(bf, Buffer[i]);
								  send += "0x"+string((char*)bf, 2)+" ";
							  }
							Log::DEBUG( "[TDiselPanelManager] New send CMD=[" + send +"]");
							*/
						/////
					}
				}
				break;

			case 98://Work status
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x30;
				Buffer[DataLen++] = 0x32;
				Buffer[DataLen++] = 0x30;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			case 99://ver
				Buffer[DataLen++] = 0x52;
				Buffer[DataLen++] = 0x32;
				Buffer[DataLen++] = 0x32;
				Buffer[DataLen++] = 0x33;
				Tmp = CheckSum(Buffer, DataLen);
				for(int i =0; i < Tmp.size(); i++)
					Buffer[DataLen++] = Tmp.c_str()[i];
				break;

			default:
			  DataLen = 0;
			  break;
		 }

	 }

	 return DataLen;
}
//=============================================================================
Byte Blc200Panel::GetDataFromMessage(Byte subFase,Byte *Buffer, Word Len)
{
	if(Len > 3)
	{
		if(!NeedInit)
		{
			string value = GetDataFromMessage(Buffer, Len);
			switch(subFase)
			{
			case 0://open|init
				break;

			case 1://F net
				if(! isNullOrWhiteSpace(value)){
					Fnet.F.CalcMinAverMax(atof(value.c_str())/100.0);
				}
				break;

			case 2://F gen
				if(! isNullOrWhiteSpace(value)){
					F.F.CalcMinAverMax(atof(value.c_str())/100.0);
				}
				break;

			case 3://L1N
				if(! isNullOrWhiteSpace(value)){
					Unet.Ua.CalcMinAverMax(atof(value.c_str()));
				}
				break;

			case 4://L2N
				if(! isNullOrWhiteSpace(value)){
					Unet.Ub.CalcMinAverMax(atof(value.c_str()));
				}
				break;

			case 5://L3N
				if(! isNullOrWhiteSpace(value)){
					Unet.Uc.CalcMinAverMax(atof(value.c_str()));
				}
				break;

			case 6://L1G
				if(! isNullOrWhiteSpace(value)){
					U.Ua.CalcMinAverMax(atof(value.c_str()));
				}
				break;

			case 7://L2G
				if(! isNullOrWhiteSpace(value)){
					U.Ub.CalcMinAverMax(atof(value.c_str()));
				}
				break;

			case 8://L3G
				if(! isNullOrWhiteSpace(value)){
					U.Uc.CalcMinAverMax(atof(value.c_str()));
				}
				break;

			case 9:////Fuel
				if(! isNullOrWhiteSpace(value)){
					Fuel.CalcMinAverMax(atof(value.c_str()));
				}
				break;

			case 10://WL1N
				if(! isNullOrWhiteSpace(value)){
					P.Pa.CalcMinAverMax(atof(value.c_str())*100.0);
				}
				break;

			case 11://WL2N
				if(! isNullOrWhiteSpace(value)){
					P.Pb.CalcMinAverMax(atof(value.c_str())*100.0);
				}
				break;

			case 12://WL3N
				if(! isNullOrWhiteSpace(value)){
					P.Pc.CalcMinAverMax(atof(value.c_str())*100.0);
				}
				P.P.CalcMinAverMax(P.Pa.Value + P.Pb.Value + P.Pc.Value);
				break;

			case 13://alarm
				if(! isNullOrWhiteSpace(value)){////////// TODO: check !!
				 int al = atoi(value.c_str());
				 if(al <= Alarms.size()){
					 for(int i = 0; i < Alarms.size(); i++){
						 //Alarms[i]->Value  = false;
						 Alarms[i]->SetValue(false);
					 }
					 if(al > 0){
						 //Alarms[al-1]->Value = true;
						 Alarms[al-1]->SetValue(true);
					 }
				 }
				}
				break;

			case 14://motorhours
				if(! isNullOrWhiteSpace(value)){
					RunHours.Params[0] = atoi(value.c_str());
				}
				break;

			case 15://gen work state
				if(! isNullOrWhiteSpace(value)){
					//State = atoi(value.c_str());
					Mode = atoi(value.c_str());
				}
				break;

			case 16://U akb
				if(! isNullOrWhiteSpace(value)){
					Vbatt.CalcMinAverMax( atof(value.c_str())/10.0);
				}
				break;

			case 17://motor temp
				if(! isNullOrWhiteSpace(value)){
					CoolTemp.CalcMinAverMax( atof(value.c_str())/10.0);
				}
				break;

			case 18://limit < V  PointStep
				if(! isNullOrWhiteSpace(value)){
					PointSteps[0]->Value = (atof(value.c_str()));
				}
				break;

			case 19://limit > V  PointStep
				if(! isNullOrWhiteSpace(value)){
					PointSteps[1]->Value = (atof(value.c_str()));
				}
				break;

			case 20://V min fail duration   PointStep
				if(! isNullOrWhiteSpace(value)){
					PointSteps[2]->Value = (atof(value.c_str()));
				}
				break;

			case 21://V recovery duration   PointStep
				if(! isNullOrWhiteSpace(value)){
					PointSteps[3]->Value = (atof(value.c_str()));
				}
				break;

			case 22://
				if(! isNullOrWhiteSpace(value)){
					OutTemp.CalcMinAverMax( atof(value.c_str())/10.0);
				}
				break;

			case 23://start count
				if(! isNullOrWhiteSpace(value)){
					RunHours.Params[1] = atoi(value.c_str());
				}
				break;


			case 98://Work status
				if(! isNullOrWhiteSpace(value)){
					//Mode = atoi(value.c_str());
					State = atoi(value.c_str());
				}
				break;

			case 99://close
				if(! isNullOrWhiteSpace(value)){
					ModelNo = "BLC200";
					SerialNo = toString( atoi(value.c_str())/100.0 );
				}
				break;

			default:
				//subFase = 99;
				//Log::DEBUG("Blc200Panel::GetDataFromMessage default subFase="+toString((int)subFase));
				break;
			}

		}
	}
	return subFase;
}
//=============================================================================
sWord Blc200Panel::RecvData(IPort* port, Byte *Buf, Word MaxLen, Byte subfase)
{
	sWord RecvLen = 0;

	if(NeedInit){
		RecvLen = port->Recv( Buf, 3);
	}
	else
	{
		switch(subfase)
		{
			case 0:
				  RecvLen = port->Recv( Buf, 3);
				  break;

				  /*
			case 1:
				  RecvLen = port->RecvTo( Buf, MaxLen, 0x04);
				  break;*/

			default:
				RecvLen = port->RecvTo( Buf, MaxLen, 0x04);
				break;
		};
	}
	return RecvLen;
}
//=============================================================================
bool Blc200Panel::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
   bool ret = true;

   if(NeedInit){
	   if(Len != 3){
		   ret = false;
	   }
	   else{
		   string tmp = string((char*)BUF, Len);
		   if(tmp != "BLC")
			   return false;
	   }
   }
   else
   {
	   switch(subFase)
	   {
	   case 0:
		   if(Len != 3){
			   ret = false;
		   }
		   else{
			   string tmp = string((char*)BUF, Len);
			   if(tmp != "BLC")
				   return false;
		   }
		   break;

	   default:
		   if(Len > 0 && Len < 11)//11
				ret = false;
		   else{
			   if(BUF[0] == 'A' && BUF[4] == 'D')
				   return true;
			   else
				   return false;
		   }
		   break;

	   }
   }
   return ret;
}
//=============================================================================
bool Blc200Panel::ChangeState( short newState )
{
	Byte buf[100];
	Word DataLen = 0;
	Word csind = 0;
	buf[DataLen++] = 0x57;
	buf[DataLen++] = 0x30;
	buf[DataLen++] = 0x32;
	buf[DataLen++] = 0x30;
	string Tmp = CheckSum(buf, DataLen);
	for(int i =0; i < Tmp.size(); i++)
		buf[DataLen++] = Tmp.c_str()[i];
	csind = DataLen;
	Tmp = IntToString(newState, 5);
	for(int i =0; i < Tmp.size(); i++)
		buf[DataLen++] = Tmp.c_str()[i];
	Tmp = CheckSum(&buf[csind], DataLen-csind);
	for(int i =0; i < Tmp.size(); i++)
		buf[DataLen++] = Tmp.c_str()[i];

	string newCmd = "";
	Byte bf[2] {0,0};
	for(int i=0; i < DataLen; i++){
		ByteToHex(bf, buf[i]);
		newCmd += string((char*)bf, 2)+" ";
	}
	CmdMessages.push_back(newCmd);
	Log::DEBUG("ChangeState newCmd mess=["+newCmd+"]");
	return true;
}
//=============================================================================
bool Blc200Panel::ChangeParameters( vector<string> parameters )
{
	for(int j=0; j < parameters.size(); j++)
	{
		Byte buf[100];
		Word DataLen = 0;
		Word csind = 0;
		buf[DataLen++] = 0x57;
		switch(j){
			case 0:
				buf[DataLen++] = 0x31;
				buf[DataLen++] = 0x31;
				buf[DataLen++] = 0x31;
				break;
			case 1:
				buf[DataLen++] = 0x31;
				buf[DataLen++] = 0x31;
				buf[DataLen++] = 0x32;
				break;
			case 2:
				buf[DataLen++] = 0x30;
				buf[DataLen++] = 0x39;
				buf[DataLen++] = 0x31;
				break;
			case 3:
				buf[DataLen++] = 0x30;
				buf[DataLen++] = 0x39;
				buf[DataLen++] = 0x32;
				break;
		};
		string Tmp = CheckSum(buf, DataLen);
		for(int i =0; i < Tmp.size(); i++)
			buf[DataLen++] = Tmp.c_str()[i];
		csind = DataLen;
		int newState = atoi(parameters[j].c_str());
		Tmp = IntToString(newState, 5);
		for(int i =0; i < Tmp.size(); i++)
			buf[DataLen++] = Tmp.c_str()[i];
		Tmp = CheckSum(&buf[csind], DataLen-csind);
		for(int i =0; i < Tmp.size(); i++)
			buf[DataLen++] = Tmp.c_str()[i];

		string newCmd = "";
		Byte bf[2] {0,0};
		for(int i=0; i < DataLen; i++){
			ByteToHex(bf, buf[i]);
			newCmd += string((char*)bf, 2)+" ";
		}
		CmdMessages.push_back(newCmd);
		Log::DEBUG("ChangeParameters newCmd mess=["+newCmd+"]");
	}
	return true;
}
//=============================================================================
string Blc200Panel::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += U.GetValueString();
	ReturnString += Unet.GetValueString();
	ReturnString += P.GetValueString();
	ReturnString += F.GetValueString();
	ReturnString += Fnet.GetValueString();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(CoolTemp.Aver, 0, 2)+','+FloatToString(CoolTemp.Min, 0, 2)+','+FloatToString(CoolTemp.Max, 0, 2)+',';
	CoolTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';
	ReturnString += FloatToString(OutTemp.Aver, 0, 2)+','+FloatToString(OutTemp.Min, 0, 2)+','+FloatToString(OutTemp.Max, 0, 2)+',';
	OutTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(85)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vbatt.Aver, 0, 2)+','+FloatToString(Vbatt.Min, 0, 2)+','+FloatToString(Vbatt.Max, 0, 2)+',';
	Vbatt.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(86)+','+WordToString(1)+',';
	ReturnString += WordToString(Fuel.Aver)+','+WordToString(Fuel.Min)+','+WordToString(Fuel.Max)+',';
	Fuel.ClrDivider();

	ReturnString +=Alarms.GetValueString();
	//ReturnString += "\r\n>,201,1,"+ WordToString(Mode)+','+WordToString(State)+',';
	ReturnString += "\r\n>,201,1,"+ WordToString(State)+','+WordToString(Mode)+',';
	ReturnString += PointSteps.GetValueString();
	ReturnString += RunHours.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";


    ReturnString += WordToString(IsEnable)+',';
    ReturnString += "\r\n>,50,1,"+ModelNo+",";

    ReturnString += "\r\n>,99,1,"+WordToString(type)+',';
	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
Telys2Panel::Telys2Panel(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ipanel(Etype, Ename, adr)
{
	//Log::DEBUG("Pw2Panel driver="+toString( (int)dr ));
	address = adr;
	type = 5;
	driver = (TModbusManager *)dr;
	settings = *((CommSettings*)sett);

	if(driver != NULL){
		driver->Init(settings);
		//driver->ClearDeviceList();

		int slen = driver->DeviceList.size();
		driver->AddToDeviceList(address, 3, 256,   16, this);//Statuses
		driver->AddToDeviceList(address, 3, 512,   49, this);//Measurement parameters
		driver->AddToDeviceList(address, 3, 2824,   40, this);//serial numbers
		//driver->AddToDeviceList(address, 3, 300,  1, this);
		//driver->outPtr = this;
		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();
	}
}
//=============================================================================
Telys2Panel::~Telys2Panel() {
}
//=============================================================================
Word Telys2Panel::CreateCMD(Byte CMD, Byte *Buffer){
	Word DataLen = 1;

	if(CMD > modbus_cmd_size  && CMD < 98){
		DataLen = 0;
	}

	//Log::DEBUG("Telys2Panel::CreateCMD");

	return DataLen;
}
//=============================================================================
Byte Telys2Panel::GetDataFromMessage(Byte subFase, Byte *Buffer, Word Len)
{
	if(Len > 3)
	{
			sBshort tmp;
			Blong   long_tmp;
			Word ind = 0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];
			Log::DEBUG("Telys2Panel::GetDataFromMessage Len="+toString(Len)+ " reg="+toString(tmp.Data_s));
			switch(tmp.Data_s){
			case 256:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				OilPressure.CalcMinAverMax( (float)tmp.Data_s*0.125/100.0);
				break;

			case 512:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				F.F.CalcMinAverMax( (float)tmp.Data_s/128.0);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				KM.KM.CalcMinAverMax( ((float)tmp.Data_s/16384.0)-1.0);
				ind+=4;
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.P.CalcMinAverMax( (long_tmp.Data_l-2000000000)/1000.0 );
				ind+=6;
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				I.Ia.CalcMinAverMax( (float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				I.Ib.CalcMinAverMax( (float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				I.Ic.CalcMinAverMax( (float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				U.Ua.CalcMinAverMax( (float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				U.Ub.CalcMinAverMax( (float)tmp.Data_s);
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				U.Uc.CalcMinAverMax( (float)tmp.Data_s);
				break;

			default:
				break;
			}

	}
	return subFase;//99;
}
//=============================================================================
sWord Telys2Panel::SendData( IPort* Port, Byte *Buf, Word Len )
{

	//Log::DEBUG("Pw2Panel::SendData");
	return Len;//0
}
//=============================================================================
sWord Telys2Panel::RecvData( IPort* Port, Byte *Buf, Word MaxLen , Byte subfase)
{
	//Log::DEBUG("Telys2Panel::RecvData");
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
bool Telys2Panel::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len == 0) return true;
	return Len >=4;
}
//=============================================================================
bool Telys2Panel::ChangeState(short newState){
	  bool ret = false;
	  if(driver!=NULL){
		  driver->AddToDeviceList(address, 6, 0x450,  0, this, newState);
	  }
	  return ret;
}
//=============================================================================
bool Telys2Panel::ChangeParameters(vector<string> parameters){
	  bool ret = false;
	  if(driver != NULL){
		  /*
		  for(int i=0; i < parameters.size(); i++){
			  short newVal = (short)(atof( parameters[i].c_str() )*10);
			  Log::DEBUG("reg="+toString(i)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
    		  driver->AddToDeviceList(address, 6, i,  0, newVal);
		  }
		  */
	  }
	  return ret;
}
//=============================================================================
string Telys2Panel::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += U.GetValueString();
	ReturnString += I.GetValueString();
	ReturnString += P.GetValueString();
	ReturnString += Q.GetValueString();
	ReturnString += S.GetValueString();
	ReturnString += KM.GetValueString();
	ReturnString += F.GetValueString();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(CoolTemp.Aver, 0, 2)+','+FloatToString(CoolTemp.Min, 0, 2)+','+FloatToString(CoolTemp.Max, 0, 2)+',';
	CoolTemp.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(85)+','+WordToString(1)+',';
	ReturnString += FloatToString(Vbatt.Aver, 0, 2)+','+FloatToString(Vbatt.Min, 0, 2)+','+FloatToString(Vbatt.Max, 0, 2)+',';
	Vbatt.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(87)+','+WordToString(1)+',';
	ReturnString += WordToString(RPM.Aver)+','+WordToString(RPM.Min)+','+WordToString(RPM.Max)+',';
	RPM.ClrDivider();
	ReturnString += "\r\n>,"+WordToString(88)+','+WordToString(1)+',';
	ReturnString += FloatToString(OilPressure.Aver, 0, 2)+','+FloatToString(OilPressure.Min, 0, 2)+','+FloatToString(OilPressure.Max, 0, 2)+',';
	OilPressure.ClrDivider();

	ReturnString +=Alarms.GetValueString();
	ReturnString += "\r\n>,201,1,"+ WordToString(State)+',';

	ReturnString += PointSteps.GetValueString();
	ReturnString += RunHours.GetValueString();


	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";


    ReturnString += WordToString(IsEnable)+',';
    ReturnString += "\r\n>,99,1,"+WordToString(type)+',';


	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
VruTm251::VruTm251(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ipanel(Etype, Ename, adr)
{
	Log::DEBUG("VruTm251 Etype="+toString((int)Etype) + " Ename="+toString((int)Ename));
	address 	= adr;
	type 		= 51;
	DataLen 	= 0;
	driver 		= (TModbusManager *)dr;
	settings 	= *((CommSettings*)sett);

	if(driver != NULL)
	{
		Log::DEBUG("VruTm251 IP="+settings.DeviceName+ ":"+toString(settings.BaudRate));

		driver->Init(settings);
		//driver->ClearDeviceList();

		int slen = driver->DeviceList.size();
		driver->AddToDeviceList(address, 3, 1000,  40, this, 0, MODBUS_TCP);
		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();
	}

	SiteInfo.Customer = "Schneider";
	SiteInfo.CU_No = "TM251";

	int size = sizeof(Data)/sizeof(THistShortParam);
	for(int i = 0; i < size; i++)
	{
		Data[i] = new THistShortParam();
	}
}
//=============================================================================
VruTm251::~VruTm251() {
}
//=============================================================================
Word VruTm251::CreateCMD(Byte CMD, Byte *Buffer)
{
	Word DataLen = 1;
	if(CMD > modbus_cmd_size  && CMD < 98){
		DataLen = 0;
	}
	return DataLen;
}
//=============================================================================
Byte VruTm251::GetDataFromMessage(Byte subFase, Byte *Buffer, Word Len)
{
	if(Len > 3)
	{
			sBshort tmp;
			Blong   long_tmp;
			Word ind = 0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];
			int size = sizeof(Data)/sizeof(THistShortParam);

			Log::DEBUG("VruTm251::GetDataFromMessage Len="+toString(Len)+ " reg="+toString(tmp.Data_s) + " subFase="+toString((int)subFase));

			switch(tmp.Data_s)
			{
			case 1000:
				for(int i = 0; i < size; i++)
				{
					tmp.Data_b[0] = Buffer[ind++];
					tmp.Data_b[1] = Buffer[ind++];
					Data[i]->Value = tmp.Data_s;
				}
				break;

			default:
				break;
			}

	}
	return subFase;//99;
}
//=============================================================================
sWord VruTm251::SendData( IPort* Port, Byte *Buf, Word Len )
{

	//Log::DEBUG("Pw2Panel::SendData");
	return Len;//0
}
//=============================================================================
sWord VruTm251::RecvData( IPort* Port, Byte *Buf, Word MaxLen , Byte subfase)
{
	//Log::DEBUG("VruTm251::RecvData");
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
bool VruTm251::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len == 0) return true;
	return Len >=4;
}
//=============================================================================
bool VruTm251::ChangeState(short newState){
	  bool ret = false;
	  if(driver!=NULL){
		  //driver->AddToDeviceList(address, 6, 0x450,  0, this, newState);
	  }
	  return ret;
}
//=============================================================================
bool VruTm251::ChangeParameters(vector<string> parameters){
	  bool ret = false;
	  if(driver != NULL){
		  /*
		  for(int i=0; i < parameters.size(); i++){
			  short newVal = (short)(atof( parameters[i].c_str() )*10);
			  Log::DEBUG("reg="+toString(i)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
    		  driver->AddToDeviceList(address, 6, i,  0, this, newVal);
		  }
		  */
	  }
	  return ret;
}
//=============================================================================
string VruTm251::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	int size = sizeof(Data)/sizeof(THistShortParam);

	for(int i = 0; i < size; i++)
	{
		ReturnString += "\r\n>,"+WordToString(3)+','+WordToString(i+1)+',';
		ReturnString += WordToString(Data[i]->Value) +',';
	}

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+ SiteInfo.Customer +','+snumb+","+SiteInfo.CU_No +',';
    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,99,1,";
    ReturnString += WordToString(type)+',';

	ReturnString += "\r\n>,0,1,";
	ReturnString += settings.DeviceName+":"+toString(settings.BaudRate);
	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
Ats022::Ats022(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ipanel(Etype, Ename, adr)
{
	//Log::DEBUG("Ats022 Etype="+toString((int)Etype) + " Ename="+toString((int)Ename));
	address 	= adr;
	type 		= 52;
	DataLen 	= 0;
	driver 		= (TModbusManager *)dr;
	settings 	= *((CommSettings*)sett);

	bool ip_flg = false;
	unsigned long ipadr;
	ipadr = inet_addr(settings.DeviceName.c_str());
	if (ipadr != INADDR_NONE) ip_flg = true;

	if(driver != NULL)
	{
		driver->Init(settings);
		int slen = driver->DeviceList.size();

		driver->AddToDeviceList(address, 3, 40,   1,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//CmdList.push_back(new ModbusRegCmd(address, 3, 40));
		driver->AddToDeviceList(address, 3, 54,   1,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//CmdList.push_back(new ModbusRegCmd(address, 3, 54));
		driver->AddToDeviceList(address, 3, 58,   10, this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//CmdList.push_back(new ModbusRegCmd(address, 3, 58));
		driver->AddToDeviceList(address, 3, 150,  6,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//CmdList.push_back(new ModbusRegCmd(address, 3, 150));
		driver->AddToDeviceList(address, 3, 164,  6,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//CmdList.push_back(new ModbusRegCmd(address, 3, 164));
		driver->AddToDeviceList(address, 3, 250,  3,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//CmdList.push_back(new ModbusRegCmd(address, 3, 250));
		driver->AddToDeviceList(address, 3, 622,  1,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//CmdList.push_back(new ModbusRegCmd(address, 3, 622));

		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();
	}
	for(int i = 0; i < 13; i++){
		Alarms.Alarms.push_back(new THistBoolParam());
	}
	for(int i = 0; i < 10; i++){
		Warnings.Alarms.push_back(new THistBoolParam());
	}

	SiteInfo.Customer = "ABB";
	SiteInfo.CU_No = "ATS022";

	ModelNo = SiteInfo.Customer + " " + SiteInfo.CU_No;

	LN1_status = LN2_status = SW_status = GEN_status = 0;
	HLN1_status = HLN2_status = HSW_status = HGEN_status = 0;
}
//=============================================================================
Ats022::~Ats022() {
}
//=============================================================================
Word Ats022::CreateCMD(Byte CMD, Byte *Buffer)
{
	Word DataLen = 1;
	if(CMD > modbus_cmd_size  && CMD < 98){
		DataLen = 0;
	}
	return DataLen;
}
//=============================================================================
Byte Ats022::GetDataFromMessage(Byte subFase, Byte *Buffer, Word Len)
{
	if(Len > 3)
	{
			sBshort tmp;
			//Blong   long_tmp;
			Word ind = 0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];

			Log::DEBUG("Ats022::GetDataFromMessage Len="+toString(Len)+ " address="+toString((int)address) +  " reg="+toString(tmp.Data_s) + " subFase="+toString((int)subFase));

			switch(tmp.Data_s)
			{
			case 40:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];

				LN1_status = tmp.Data_s & 7;//first 3 bits
				LN2_status = (tmp.Data_s >> 3) & 7;//second 3 bits
				SW_status  = (tmp.Data_s >> 6) & 7;
				GEN_status = (tmp.Data_s >> 9) & 1;
				break;

			case 54:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				for(int i=0;i< Alarms.size(); i++){
					//Alarms[i]->Value = (bool)(((Short)tmp.Data_s >> i) & 1);
					Alarms[i]->SetValue((bool)(((Short)tmp.Data_s >> i) & 1));
				}
				break;

			case 58:
				for(int i=0;i< Warnings.size(); i++)
				{
					tmp.Data_b[0] = Buffer[ind++];
					tmp.Data_b[1] = Buffer[ind++];
					//Warnings[i]->Value = (bool)((Short)tmp.Data_s & 1);
					Warnings[i]->SetValue((bool)((Short)tmp.Data_s & 1));
				}
				break;

			case 150:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				U.Ua.CalcMinAverMax( (DWord)(tmp.Data_s)/10.0 );
				ind+=2;
				Log::DEBUG("Ats022::GetDataFromMessage Len="+toString(Len)+ " address="+toString((int)address) + " Ua="+ toString(U.Ua.Value));


				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				U.Ub.CalcMinAverMax( (DWord)(tmp.Data_s)/10.0 );
				ind+=2;
				Log::DEBUG("Ats022::GetDataFromMessage Len="+toString(Len)+ " address="+toString((int)address) + " Ub="+ toString(U.Ub.Value));


				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				U.Uc.CalcMinAverMax( (DWord)(tmp.Data_s)/10.0 );
				ind+=2;
				Log::DEBUG("Ats022::GetDataFromMessage Len="+toString(Len)+ " address="+toString((int)address) + " Uc="+ toString(U.Uc.Value));
				break;

			case 164:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Unet.Ua.CalcMinAverMax( (DWord)(tmp.Data_s)/10.0 );
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Unet.Ub.CalcMinAverMax( (DWord)(tmp.Data_s)/10.0 );
				ind+=2;

				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Unet.Uc.CalcMinAverMax( (DWord)(tmp.Data_s)/10.0 );
				ind+=2;
				break;


			case 250:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				F.F.CalcMinAverMax( (DWord)(tmp.Data_s)/10.0 );
				ind+=2;
				Log::DEBUG("Ats022::GetDataFromMessage Len="+toString(Len)+ " address="+toString((int)address) + " F="+ toString(F.F.Value));


				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				Fnet.F.CalcMinAverMax( (DWord)(tmp.Data_s)/10.0 );
				Log::DEBUG("Ats022::GetDataFromMessage Len="+toString(Len)+ " address="+toString((int)address) + " Fnet="+ toString(Fnet.F.Value));
				break;

			case 622:
				tmp.Data_b[0] = Buffer[ind++];
				tmp.Data_b[1] = Buffer[ind++];
				State = tmp.Data_s;
				break;

			default:
				break;
			}

	}
	return subFase;//99;
}
//=============================================================================
sWord Ats022::SendData( IPort* Port, Byte *Buf, Word Len )
{

	//Log::DEBUG("Pw2Panel::SendData");
	return Len;//0
}
//=============================================================================
sWord Ats022::RecvData( IPort* Port, Byte *Buf, Word MaxLen , Byte subfase)
{
	//Log::DEBUG("Ats022::RecvData subfase="+toString((int)subfase));
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
bool Ats022::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len == 0) return true;
	return Len >=4;
}
//=============================================================================
bool Ats022::ChangeState(short newState){
	  bool ret = false;
	  if(driver!=NULL){
		  //driver->AddToDeviceList(address, 6, 0x450,  0, this, newState);
	  }
	  return ret;
}
//=============================================================================
bool Ats022::ChangeParameters(vector<string> parameters){
	  bool ret = false;
	  if(driver != NULL){
		  /*
		  for(int i=0; i < parameters.size(); i++){
			  short newVal = (short)(atof( parameters[i].c_str() )*10);
			  Log::DEBUG("reg="+toString(i)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
    		  driver->AddToDeviceList(address, 6, i,  0, this, newVal);
		  }
		  */
	  }
	  return ret;
}
//=============================================================================
string Ats022::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += U.GetValueString();
	ReturnString += Unet.GetValueString();
	ReturnString += F.GetValueString();
	ReturnString += Fnet.GetValueString();

	ReturnString += "\r\n>,201,1,"+
			WordToString(State)+','+
			WordToString(LN1_status)+','+
			WordToString(LN2_status)+','+
			WordToString(SW_status)+','+
			WordToString(GEN_status)+',';

	ReturnString +=Alarms.GetValueString();
	ReturnString +=Warnings.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0"){
		snumb = "0";
	}
    ReturnString += "\r\n>,49,1,"+snumb+",";
    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,50,1,"+ModelNo+",";

    ReturnString += "\r\n>,99,1,"+WordToString(type)+',';
	return ReturnString;


}
//*****************************************************************************
//***
//*****************************************************************************
CVM::CVM(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ipanel(Etype, Ename, adr), E(40,1)
{
	//Log::DEBUG("CVM Etype="+toString((int)Etype) + " Ename="+toString((int)Ename));
	address 	= adr;
	type 		= 53;
	DataLen 	= 0;
	driver 		= (TModbusManager *)dr;
	settings 	= *((CommSettings*)sett);

	bool ip_flg = false;
	unsigned long ipadr;
	ipadr = inet_addr(settings.DeviceName.c_str());
	if (ipadr != INADDR_NONE) ip_flg = true;

	if(driver != NULL)
	{
		driver->Init(settings);
		int slen = driver->DeviceList.size();

		driver->AddToDeviceList(address, 3, 0,    70,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//
		//driver->AddToDeviceList(address, 3, 94,   42,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//t1 E
		//driver->AddToDeviceList(address, 3, 136,  42,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//t2 E
		//driver->AddToDeviceList(address, 3, 178,  42,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//t3 E
		driver->AddToDeviceList(address, 3, 220,  42,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//summ E

		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();
	}
	for(int i = 0; i < 13; i++){
		Alarms.Alarms.push_back(new THistBoolParam());
	}
	for(int i = 0; i < 10; i++){
		Warnings.Alarms.push_back(new THistBoolParam());
	}

	SiteInfo.Customer = "Circutor";
	SiteInfo.CU_No = "CVM-C10";

	ModelNo = SiteInfo.Customer + " " + SiteInfo.CU_No;
}
//=============================================================================
CVM::~CVM() {
}
//=============================================================================
Word CVM::CreateCMD(Byte CMD, Byte *Buffer)
{
	Word DataLen = 1;
	if(CMD > modbus_cmd_size  && CMD < 98){
		DataLen = 0;
	}
	return DataLen;
}
//=============================================================================
Byte CVM::GetDataFromMessage(Byte subFase, Byte *Buffer, Word Len)
{
	if(Len > 3)
	{
			sBshort tmp;
			Blong   long_tmp;
			float 	float_tmp = 0;
			Word ind = 0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];

			Log::DEBUG("CVM::GetDataFromMessage Len="+toString(Len)+ " address="+toString((int)address) +  " reg="+toString(tmp.Data_s) + " subFase="+toString((int)subFase));

			switch(tmp.Data_s)
			{
			case 0:
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				U.Ua.CalcMinAverMax(long_tmp.Data_l/10.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				I.Ia.CalcMinAverMax(long_tmp.Data_l/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pa.CalcMinAverMax( long_tmp.Data_l/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp = long_tmp.Data_l/1000.0;
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp -= long_tmp.Data_l/1000.0;
				Q.Qa.CalcMinAverMax( float_tmp );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				S.Sa.CalcMinAverMax( long_tmp.Data_l/1000.0 );

				ind += 4;
				/*
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				KM.KMa.CalcMinAverMax( long_tmp.Data_l/100.0 );*/
				ind +=4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				U.Ub.CalcMinAverMax(long_tmp.Data_l/10.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				I.Ib.CalcMinAverMax(long_tmp.Data_l/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pb.CalcMinAverMax( long_tmp.Data_l/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp = long_tmp.Data_l/1000.0;
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp -= long_tmp.Data_l/1000.0;
				Q.Qb.CalcMinAverMax( float_tmp );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				S.Sb.CalcMinAverMax( long_tmp.Data_l/1000.0 );

				ind += 4;
				/*
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				KM.KMb.CalcMinAverMax( long_tmp.Data_l/100.0 );*/
				ind +=4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				U.Uc.CalcMinAverMax(long_tmp.Data_l/10.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				I.Ic.CalcMinAverMax(long_tmp.Data_l/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.Pc.CalcMinAverMax( long_tmp.Data_l/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp = long_tmp.Data_l/1000.0;
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp -= long_tmp.Data_l/1000.0;
				Q.Qc.CalcMinAverMax( float_tmp );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				S.Sc.CalcMinAverMax( long_tmp.Data_l/1000.0 );

				ind += 4;
				/*
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				KM.KMc.CalcMinAverMax( long_tmp.Data_l/100.0 );*/
				ind += 4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				P.P.CalcMinAverMax( long_tmp.Data_l/1000.0 );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp = long_tmp.Data_l/1000.0;
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp -= long_tmp.Data_l/1000.0;
				Q.Q.CalcMinAverMax( float_tmp );
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				S.S.CalcMinAverMax( long_tmp.Data_l/1000.0 );

				ind += 4;
				/*
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				KM.KM.CalcMinAverMax( long_tmp.Data_l/100.0 );*/
				ind += 4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				F.F.CalcMinAverMax( long_tmp.Data_l/100.0 );
				break;

			case 220:
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				E.Ea = long_tmp.Data_l/1.0;//active energy
				ind += 4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp = long_tmp.Data_l/1.0;
				ind += 4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				float_tmp -= long_tmp.Data_l/1.0;
				//E.Er = float_tmp;
				ind += 4;

				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				E.Ea_minus = long_tmp.Data_l/1.0;//full energy
				break;

			default:
				break;
			}

	}
	return subFase;//99;
}
//=============================================================================
sWord CVM::SendData( IPort* Port, Byte *Buf, Word Len )
{

	//Log::DEBUG("Pw2Panel::SendData");
	return Len;//0
}
//=============================================================================
sWord CVM::RecvData( IPort* Port, Byte *Buf, Word MaxLen , Byte subfase)
{
	//Log::DEBUG("CVM::RecvData subfase="+toString((int)subfase));
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
bool CVM::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len == 0) return true;
	return Len >=4;
}
//=============================================================================
bool CVM::ChangeState(short newState){
	  bool ret = false;
	  if(driver!=NULL){
		  //driver->AddToDeviceList(address, 6, 0x450,  0, this, newState);
	  }
	  return ret;
}
//=============================================================================
bool CVM::ChangeParameters(vector<string> parameters){
	  bool ret = false;
	  if(driver != NULL){
		  /*
		  for(int i=0; i < parameters.size(); i++){
			  short newVal = (short)(atof( parameters[i].c_str() )*10);
			  Log::DEBUG("reg="+toString(i)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
    		  driver->AddToDeviceList(address, 6, i,  0, this, newVal);
		  }
		  */
	  }
	  return ret;
}
//=============================================================================
string CVM::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";

	ReturnString += E.GetValueString();
	ReturnString += U.GetValueString();
	ReturnString += I.GetValueString();
	ReturnString += P.GetValueString();
	//ReturnString += Q.GetValueString();
	ReturnString += S.GetValueString();
	ReturnString += F.GetValueString();
	//ReturnString += KM.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0"){
		snumb = "0";
	}
    ReturnString += "\r\n>,49,1,"+snumb+",";
    ReturnString += WordToString(IsEnable)+',';

    ReturnString += "\r\n>,50,1,"+ModelNo+",";

    ReturnString += "\r\n>,99,1,"+WordToString(type)+',';
	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
PM5xxx::PM5xxx(Byte Etype, Byte Ename, Byte adr, void *sett, void *dr):Ipanel(Etype, Ename, adr), E(40,1)
{
	//Log::DEBUG("PM5xxx Etype="+toString((int)Etype) + " Ename="+toString((int)Ename));
	address 	= adr;
	type 		= 54;
	DataLen 	= 0;
	driver 		= (TModbusManager *)dr;
	settings 	= *((CommSettings*)sett);

	bool ip_flg = false;
	unsigned long ipadr;
	ipadr = inet_addr(settings.DeviceName.c_str());
	if (ipadr != INADDR_NONE) ip_flg = true;

	if(driver != NULL)
	{
		driver->Init(settings);
		int slen = driver->DeviceList.size();

		driver->AddToDeviceList(address, 3, 49, 	20,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//model
		driver->AddToDeviceList(address, 3, 2999,  	94,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//Meter Data (Basic)
		driver->AddToDeviceList(address, 3, 2699, 	24,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//all E
		driver->AddToDeviceList(address, 3, 129, 	11,  this, 0, ip_flg ? MODBUS_TCP : MODBUS_RTU);//serial


		driver->OnCmdComplited = transportData;
		modbus_cmd_size = driver->DeviceList.size() - slen;
		driver->Start();
	}
	for(int i = 0; i < 10; i++){
		Alarms.Alarms.push_back(new THistBoolParam());
	}
	for(int i = 0; i < 10; i++){
		Warnings.Alarms.push_back(new THistBoolParam());
	}

	SiteInfo.Customer = "Schneider";
	SiteInfo.CU_No = "PM5xxx";

	ModelNo = SiteInfo.Customer + " " + SiteInfo.CU_No;
}
//=============================================================================
PM5xxx::~PM5xxx() {
}
//=============================================================================
Word PM5xxx::CreateCMD(Byte CMD, Byte *Buffer)
{
	Word DataLen = 1;
	if(CMD > modbus_cmd_size  && CMD < 98){
		DataLen = 0;
	}
	return DataLen;
}
//=============================================================================
Byte PM5xxx::GetDataFromMessage(Byte subFase, Byte *Buffer, Word Len)
{
	if(Len > 3)
	{
			sBshort tmp;
			Bfloat  float_tmp;
			Blong   long_tmp;

			Word ind = 0;
			int func = Buffer[ind++];
			tmp.Data_b[0] = Buffer[ind++];
			tmp.Data_b[1] = Buffer[ind++];

			Log::DEBUG("PM5xxx::GetDataFromMessage Len="+toString(Len)+ " address="+toString((int)address) +  " reg="+toString(tmp.Data_s) + " subFase="+toString((int)subFase));

			switch(tmp.Data_s)
			{
			case 2999:
				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				I.Ia.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				I.Ib.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				I.Ic.CalcMinAverMax( float_tmp.Data_f );
				ind+=44;

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				U.Ua.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				U.Ub.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				U.Uc.CalcMinAverMax( float_tmp.Data_f );

				ind+=36;
				ind+=4;

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				P.Pa.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				P.Pb.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				P.Pc.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				P.P.CalcMinAverMax( float_tmp.Data_f );

				Log::DEBUG( "PM5xxx::GetDataFromMessage Pa="+toString(P.Pa.Value)+ " Pb="+toString(P.Pb.Value) +  " Pc="+toString(P.Pc.Value) + " Psumm="+toString(P.P.Value) +" ind="+toString(ind));


				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				Q.Qa.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				Q.Qb.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				Q.Qc.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				Q.Q.CalcMinAverMax( float_tmp.Data_f );

				Log::DEBUG( "PM5xxx::GetDataFromMessage Qa="+toString(Q.Qa.Value)+ " Qb="+toString(Q.Qb.Value) +  " Qc="+toString(Q.Qc.Value) + " Qsumm="+toString(Q.Q.Value) +" ind="+toString(ind));

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				S.Sa.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				S.Sb.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				S.Sc.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				S.S.CalcMinAverMax( float_tmp.Data_f );

				Log::DEBUG( "PM5xxx::GetDataFromMessage Sa="+toString(S.Sa.Value)+ " Sb="+toString(S.Sb.Value) +  " Sc="+toString(S.Sc.Value) + " Ssumm="+toString(S.S.Value) +" ind="+toString(ind));


				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				KM.KMa.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				KM.KMb.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				KM.KMc.CalcMinAverMax( float_tmp.Data_f );

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				KM.KM.CalcMinAverMax( float_tmp.Data_f );

				Log::DEBUG( "PM5xxx::GetDataFromMessage KMa="+toString(KM.KMa.Value)+ " KMSb="+toString(KM.KMb.Value) +  " KMc="+toString(KM.KMc.Value) + " KMsumm="+toString(KM.KM.Value) +" ind="+toString(ind));


				ind+=48;

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				F.F.CalcMinAverMax( float_tmp.Data_f );

				Log::DEBUG( "PM5xxx::GetDataFromMessage F="+toString(F.F.Value) +" ind="+toString(ind));
				break;

			case 2699:
				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				E.Ea = float_tmp.Data_f ;//Active Energy Delivered (Into Load)

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				//E.Ea_minus = float_tmp.Data_f ;//Active Energy Received (Out of Load)

				ind += 8;

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				E.Er = float_tmp.Data_f ;//Reactive Energy Delivered (Into Load)

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				//E.Er_minus = float_tmp.Data_f ;//Reactive Energy Received (Out of Load)

				ind += 8;

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				E.Ea_minus = float_tmp.Data_f ;//Apparent Energy Delivered

				float_tmp.Data_b[2] = Buffer[ind++];
				float_tmp.Data_b[3] = Buffer[ind++];
				float_tmp.Data_b[0] = Buffer[ind++];
				float_tmp.Data_b[1] = Buffer[ind++];
				//Apparent Energy Received

				break;

			case 129:
				long_tmp.Data_b[2] = Buffer[ind++];
				long_tmp.Data_b[3] = Buffer[ind++];
				long_tmp.Data_b[0] = Buffer[ind++];
				long_tmp.Data_b[1] = Buffer[ind++];
				if(long_tmp.Data_l > 0){
					SerialNo = toString(long_tmp.Data_l);
					Log::DEBUG("PM5xxx::GetDataFromMessage SerialNo="+SerialNo + " subFase="+toString((int)subFase));
				}
				break;

			case 49:
				{
					string tstr = string((char*)&Buffer[ind]);
					if(!isNullOrWhiteSpace(tstr)) //isNullOrWhiteSpace
					{
						string sernStr = "";
						int len = tstr.length();
						for(int ind = 0; ind < len; ind++)
						{
							if(ind % 2 > 0){
								sernStr += tstr.c_str()[ind];
								sernStr += tstr.c_str()[ind-1];
							}
						}
						//Log::DEBUG("PM5xxx::GetDataFromMessage CU_No, tstr="+tstr + " sernStr="+sernStr);
						SiteInfo.CU_No = sernStr;
					}
				//SerialNo = toString(long_tmp.Data_l);
				}
				break;

			default:
				break;
			}

	}
	return subFase;//99;
}
//=============================================================================
sWord PM5xxx::SendData( IPort* Port, Byte *Buf, Word Len )
{

	//Log::DEBUG("PM5xxx::SendData");
	return Len;//0
}
//=============================================================================
sWord PM5xxx::RecvData( IPort* Port, Byte *Buf, Word MaxLen , Byte subfase)
{
	//Log::DEBUG("PM5xxx::RecvData subfase="+toString((int)subfase));
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
bool PM5xxx::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len == 0) return true;
	return Len >=4;
}
//=============================================================================
bool PM5xxx::ChangeState(short newState){
	  bool ret = false;
	  if(driver!=NULL){
		  //driver->AddToDeviceList(address, 6, 0x450,  0, this, newState);
	  }
	  return ret;
}
//=============================================================================
bool PM5xxx::ChangeParameters(vector<string> parameters){
	  bool ret = false;
	  if(driver != NULL){
		  /*
		  for(int i=0; i < parameters.size(); i++){
			  short newVal = (short)(atof( parameters[i].c_str() )*10);
			  Log::DEBUG("reg="+toString(i)+ ": parameter="+parameters[i]+" newVal="+toString(newVal));
    		  driver->AddToDeviceList(address, 6, i,  0, this, newVal);
		  }
		  */
	  }
	  return ret;
}
//=============================================================================
string PM5xxx::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+WordToString(En)+",";

	ReturnString += E.GetValueString();
	ReturnString += U.GetValueString();
	ReturnString += I.GetValueString();
	ReturnString += P.GetValueString();
	ReturnString += Q.GetValueString();
	ReturnString += S.GetValueString();
	ReturnString += F.GetValueString();
	ReturnString += KM.GetValueString();

	string snumb = "000000";
	if(!IsEnable && SerialNo == "0"){
		snumb = "0";
	}
	else{
		snumb = SerialNo;
	}
    ReturnString += "\r\n>,49,1,"+snumb+",";
    ReturnString += WordToString(IsEnable)+',';

    ModelNo = SiteInfo.Customer + " " + SiteInfo.CU_No;
    ReturnString += "\r\n>,50,1,"+ModelNo+",";

    ReturnString += "\r\n>,99,1,"+WordToString(type)+',';
	return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
TDiselPanelManager::TDiselPanelManager( void ):TFastTimer(3, &MilSecCount),
												//Fase(CREATE_CMD),
												Fase(RECV_CMD),
												SubFase(0),
												Period(600),
												DataLen(0),
												//fd(NULL),
												Port(NULL)
{
		ControllerIndex = 0;
		SetTimer(PERIUD_TIMER, 55000);
		SetTimer(ADDITIONAL_TIMER, 0);
		FirstInitFlg = true;
		ModifyTime = Period;

		Aperiod = 60;
		Operiod = Period;
}
//=============================================================================
TDiselPanelManager::~TDiselPanelManager()
{
	try
	{
		for(auto curr: Controllers){
			if(curr != NULL){
				delete curr;
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( e.what() );
	}
}
//=============================================================================
void TDiselPanelManager::Init( void *config, void *driver )
{
  vector<DguSettings> sett = *((vector<DguSettings> *)config);
  //Log::DEBUG("Init DiselPanelManager sett.size="+toString(sett.size()));

	  for(auto curr: sett)
	  {
			Ipanel *panel = NULL;
			IPort *port = NULL;

			unsigned long adr;
			adr=inet_addr(curr.portsettings.DeviceName.c_str());
			if (adr != INADDR_NONE)
			{//ip
				port = new SocketPort();
				SocketSettings ssett;
				ssett.IpAddress = curr.portsettings.DeviceName;
				ssett.IpPort = curr.portsettings.BaudRate;
				//ssett.BindPort			= curr.portsettings.BaudRate2;
				ssett.SocketRecvTimeout = curr.portsettings.RecvTimeout;
				ssett.SocketSendTimeout = curr.portsettings.SendTimeout;
				port->Init(&ssett);
			}
			else
			{//com
				port = new ComPort();
				port->Init(&curr.portsettings);
			}


			switch(curr.type)
			{
				case 2:
					panel = new Blc200Panel(27, curr.ename, curr.address);
					port->SetVirtual(false);
					break;
				case 3:
					panel = new Pw2Panel(27, curr.ename, curr.address, &curr.portsettings, driver);
					port->SetVirtual(true);
					break;
				case 4:
					panel = new Dse7xxxPanel(27, curr.ename, curr.address, &curr.portsettings, driver);
					port->SetVirtual(true);
					break;
				case 5:
					panel = new Telys2Panel(27, curr.ename, curr.address, &curr.portsettings, driver);
					port->SetVirtual(true);
					break;
				case 51:
					Log::DEBUG("Init TDiselPanelManager new vru VruTm251");
					panel = new VruTm251(curr.etype, curr.ename, curr.address, &curr.portsettings, driver);
					port->SetVirtual(true);
					break;
				case 52:
					Log::DEBUG("Init TDiselPanelManager new avr Ats022");
					panel = new Ats022(curr.etype, curr.ename, curr.address, &curr.portsettings, driver);
					port->SetVirtual(true);
					break;
				case 53:
					Log::DEBUG("Init TDiselPanelManager new power analizer CVM");
					panel = new CVM(curr.etype, curr.ename, curr.address, &curr.portsettings, driver);
					port->SetVirtual(true);
					break;
				case 54:
					Log::DEBUG("Init TDiselPanelManager new power analizer PM5xxx");
					panel = new PM5xxx(curr.etype, curr.ename, curr.address, &curr.portsettings, driver);
					port->SetVirtual(true);
					break;

			}
			if(panel!=NULL)
			{
				panel->settings 	= curr.portsettings;
				panel->Port 		= port;
				Port = port;
				panel->SetPeriod( Period );
				panel->BetweenTimeout = curr.betweentimeout;
				panel->NeedInitTimeout = curr.needinittimeout;

				Controllers.push_back(panel);
			}
			Period 				=  curr.period;
			Operiod				=  curr.period;
			Aperiod 			=  curr.aperiod;
	  }

	for(auto curr: Controllers)
	{
	    Log::DEBUG("TDiselPanelManager curr:");
	    Log::DEBUG("{");
	    Log::DEBUG("	Type="+toString((int)curr->type));
	    Log::DEBUG("	Addr="+toString((int)curr->address));
	    Log::DEBUG("	TDiselPanelManager curr->comm_settings:");
	    Log::DEBUG("	DeviceName="+toString(curr->settings.DeviceName));
	    Log::DEBUG("	BaudRate="+toString((int)curr->settings.BaudRate));
	    Log::DEBUG("	DataBits="+toString((int)curr->settings.DataBits));
	    Log::DEBUG("	StopBit="+toString((int)curr->settings.StopBit));
	    Log::DEBUG("	Parity="+toString((int)curr->settings.Parity));
	    Log::DEBUG("	RecvTimeout="+toString(curr->settings.RecvTimeout));
	    Log::DEBUG("	SendTimeout="+toString(curr->settings.SendTimeout));
	    Log::DEBUG("	BetweenTimeout="+toString(curr->BetweenTimeout));
	    Log::DEBUG("	NeedInitTimeout="+toString(curr->NeedInitTimeout));
	    Log::DEBUG("}");
	}
  //Log::DEBUG("Init DiselPanelManager Enable="+toString(Enable()));
}
//=============================================================================
bool TDiselPanelManager::Enable( void )
{
	return Controllers.size() > 0;
}
//=============================================================================
IPort *TDiselPanelManager::GetPort()
{
	  IPort*ret = NULL;
	  pthread_mutex_lock(&sych);
	  ret = Port;
	  pthread_mutex_unlock(&sych);
	  return ret;
}
//=============================================================================
Ipanel *TDiselPanelManager::GetSimple( Word index )
{
	Ipanel *conroller = NULL;
  	if(index < Controllers.size()){
  		conroller = Controllers[index];
  	}
  return conroller;
}
//=============================================================================
void TDiselPanelManager::CreateCMD( void)
{
	//Log::DEBUG("TDiselPanelManager::CreateCMD");
	int size = Controllers.size();
	if( size > 0)
	{
	  if( ControllerIndex >=  size){
	  	ControllerIndex = 0;
	  }
	  //Log::DEBUG( "[TDiselPanelManager] CreateCMD SubFase="+toString((int)SubFase)+" ControllerIndex="+toString(ControllerIndex));

	  Ipanel *Controller = GetSimple(ControllerIndex);
	  if(Controller != NULL)
	  {
	      if(SubFase == 0)
	      {
	    	  if( Controller->Port != Port )
	    	  {
	    		  if(Port != NULL)
	    			  Port->Close();
	    		  Port = Controller->Port;//change
	    		  sleep(1);
	    		  Log::DEBUG( "[TDiselPanelManager] CreateCMD changePort, SubFase="+toString((int)SubFase)+" ControllerIndex="+toString(ControllerIndex));
	    	  }
	      }
	     DataLen = Controller->CreateCMD(SubFase, Buffer);
	   }
	  //Log::DEBUG( "[TDiselPanelManager] CreateCMD DataLen="+toString((int)DataLen)+" SubFase="+toString((int)SubFase)+" ControllerIndex="+toString((int)ControllerIndex));
	}
    Fase    = SEND_CMD;
}
//=============================================================================
void TDiselPanelManager::SendCMD( void )
{
	Ipanel *Controller = GetSimple(ControllerIndex);
	//Log::DEBUG( "[TDiselPanelManager] SendCMD DataLen="+toString(DataLen)+" SubFase="+toString((int)SubFase)+" ControllerIndex="+toString(ControllerIndex));
	if(Controller != NULL)
	{
		sWord st = Controller->SendData(Port, Buffer, DataLen);
		if(  st > 0 )
		{//
			DataLen = st;
			////////////////
			/*
					  Byte bf[2] {0,0};
					  string sended = "";
					  for(int i = 0; i < DataLen; i++)
					  {
						  ByteToHex(bf, Buffer[i]);
						  sended += "0x"+string((char*)bf, 2)+" ";
					  }*/
					//Log::DEBUG( "[TDiselPanelManager] send ok: [" + sended +"] len="+toString(DataLen));
				    //Log::DEBUG( toString(MilSecCount.Value)+" [TDiselPanelManager] send ok: ["+ string( (char*)Buffer, DataLen ) + "] len="+toString(DataLen)+" subFase="+toString((int)SubFase));
			//////////////////////////

			TBuffer::ClrBUF(Buffer, DataLen);
	    	Fase    = RECV_CMD;
		}
		else if(st == 0)
		{
			DataLen = st; //!!!16.11.21 tk pri neotvete snmp parsilo zapros
			Fase    = EXEC_CMD;
			//Log::DEBUG("TDiselPanelManager::SendCMD DataLen="+toString(DataLen));
		}
		else
		{//!!!16.11.21 tk pri neotvete snmp parsilo zapros
			DataLen = 1;
			Log::DEBUG( "[TDiselPanelManager] goto EXEC_CMD st="+toString(st));
			Fase    = EXEC_CMD;
		}
	}
	else
	{
		  ControllerIndex = 0;
		  Fase    = CREATE_CMD;
		  SubFase = 0;
		  Log::DEBUG("TDiselPanelManager::SendCMD Controller != NULL");
	}
}
//=============================================================================
void TDiselPanelManager::RecvCMD( void )
{
	//Log::DEBUG("TDiselPanelManager::RecvCMD");
	static int 	_answerErrorCNT = 0;
	Ipanel *Controller = GetSimple(ControllerIndex);
	if( Controller != NULL)
	{
		Word  timeout = Controller->BetweenTimeout;
		sWord RecvLen = Controller->RecvData(Port, Buffer, sizeof(Buffer), SubFase);

		//Log::DEBUG("[TDiselPanelManager] RecvData len="+toString(RecvLen)+" subFase="+toString((int)SubFase)+" ControllerIndex="+toString((int)ControllerIndex));
		if(RecvLen > 0 )
		{
		    //////////////////////////
			/*
		    		 Byte bf[2] {0,0};
		    		  string recv = "";
		    		  for(int i = 0; i < RecvLen; i++)
		    		  {
		    			  ByteToHex(bf, Buffer[i]);
		    			  recv += "0x"+string((char*)bf, 2)+" ";
		    		  }
			 	 	 */
		    		  //Log::DEBUG(" [TDiselPanelManager] RecvData ok: len="+toString(RecvLen)+ " ControllerIndex=" + toString(ControllerIndex) + " subFase="+toString((int)SubFase));
		    		//Log::DEBUG( "[TDiselPanelManager] RecvData ok: [" + recv +"] len="+toString(RecvLen)+" subFase="+toString((int)SubFase));

		    		//Log::DEBUG( toString(MilSecCount.Value)+" [TDiselPanelManager1] RecvData ok: ["+ string( (char*)Buffer, RecvLen ) + "] len="+toString(RecvLen)+" subFase="+toString((int)SubFase));
		    //////////////////////
			  DataLen     = RecvLen;
			  Fase        = EXEC_CMD;
			  _answerErrorCNT = 0;
			  SetTimer( COMMON_TIMER, timeout);
		}
		else
		{
			  Log::ERROR("[TDiselPanelManager] READ ERROR Controller:"+ toString((int)ControllerIndex)+ " _answerErrorCNT="+toString(_answerErrorCNT));
			  if(_answerErrorCNT++ >= 10)
			  {
				  _answerErrorCNT       	= 0;
				  Controller->ErrCnt++;
				  if( Controller->IsEnable == true)
				  {
					  Controller->U.ClearValues();
		          	  Controller->I.ClearValues();
		          	  Controller->P.ClearValues();
				  	  Controller->Modify     	= true;
				  	  Controller->IsEnable 		= false;
				  	  DateTime = SystemTime.GetTime();
				  }
			      if( ++ControllerIndex >=  Controllers.size()){
			    	  ControllerIndex = 0;
			      }
			      SubFase     = 0;
			      timeout = 3000;
			  }
			  else
			  {
				  timeout 	= Controller->NeedInitTimeout;//timeout*2;//2000
				  /*
			      if(_answerErrorCNT == 5)
			      {
			    	SubFase     = 0;
			       	timeout 	= 3000;
			      }*/
			  }
			  Port->Close();
			  DataLen = 0;
			  Fase    = CREATE_CMD;
			  SetTimer( COMMON_TIMER, timeout);
			  Log::ERROR("[TDiselPanelManager] READ ERROR timeout="+ toString((int)timeout));
			  //Controller->NeedInit = true;
		  }
	}
	else
	{
		Log::DEBUG("TDiselPanelManager::RecvCMD Controller != NULL");
		  ControllerIndex = 0;
		  Fase    = CREATE_CMD;
		  SubFase = 0;
	}
}
//=============================================================================
void TDiselPanelManager::ExecCMD( void )
{
	static Byte 	_answerErrorCNT = 0;
	Ipanel *Controller = GetSimple(ControllerIndex);
	if( Controller != NULL)
	{
	    if( Controller->ParsingAnswer(Buffer, DataLen, SubFase) )
	    {
	    	//Log::DEBUG("TDiselPanelManager] ExecCMD subFase="+toString((int)SubFase)+" ControllerIndex="+toString(ControllerIndex)+" DataLen="+toString(DataLen) + " NeedInit="+toString((int)Controller->NeedInit));
	    	_answerErrorCNT = 0;
	    	  Word  timeout = Controller->BetweenTimeout;
	    	  SubFase = Controller->GetDataFromMessage(SubFase, Buffer, DataLen );
	    	  SetTimer( COMMON_TIMER, timeout);

	    	  //if(Controller->NeedInit)
	    	  //{
	    		//  Controller->NeedInit = false;
	    	  //}
	    	  //else
	    	  //{
				  switch(SubFase++)
				  {
				  case 98:
					 Log::DEBUG("TDiselPanelManager::ExecCMD SubFase=98, ControllerIndex="+ toString((int)ControllerIndex) + " FirstInitFlg="+toString((int)FirstInitFlg) + " IsEnable="+toString((int)Controller->IsEnable));
					 if( Controller->IsEnable == false)
					 {
						 if(!FirstInitFlg)
						 {
							 DateTime = SystemTime.GetTime();
							 Controller->Modify = true;
						 }
						 Controller->IsEnable = true;
						 Log::DEBUG("TDiselPanelManager::ExecCMD SubFase=98, Controller->Modify="+toString((int)Controller->Modify));
					  }
					  break;

				  case 99://last cmd
					  //Controller->Port->Close();
					  Log::DEBUG("TDiselPanelManager::ExecCMD SubFase=99, ControllerIndex="+toString((int)ControllerIndex));
					  Controller->AckCnt++;
					  if( ++ControllerIndex >=  Controllers.size()){
						  ControllerIndex = 0;
						  timeout = 10000;
						  if(Controllers.size() > 1)
							  timeout = 5000;
					  }
					  SetTimer( COMMON_TIMER, timeout);
					  SubFase = 0;
					  break;

					default:
						if(DataLen == 0)
						{
							SetTimer( COMMON_TIMER, 0);
							//Log::DEBUG("TDiselPanelManager::ExecCMD default, SubFase="+toString((int)SubFase));
						}
						else
						{
							Log::DEBUG("TDiselPanelManager] ExecCMD subFase="+toString((int)SubFase)+" ControllerIndex="+toString((int)ControllerIndex)+" DataLen="+toString(DataLen));
						}
						break;
				  };
				  //Log::DEBUG("TDiselPanelManager] ExecCMD subFase after switch="+toString((int)SubFase) );
	    	  //}
	    }
	    else
	    {
	    	Log::DEBUG( "[TDiselPanelManager] ParsingAnswer ERROR SubFase="+toString((int)SubFase));
	    	//SubFase = 0;
			if(_answerErrorCNT++ >= 10){
				_answerErrorCNT       = 0;
				Port->Close();
				SetTimer( COMMON_TIMER, 3000);
		        if( ++ControllerIndex >=  Controllers.size())//progon po ostalnim t.k. dalee
		        { ControllerIndex = 0; }
		        SubFase = 0;
			}
	    }
	    Controller->ManageData();
	}
	else
	{
		Log::DEBUG("TDiselPanelManager::ExecCMD Controller == NULL");
	    ControllerIndex = 0;
	    SubFase = 0;
	}
	//Log::DEBUG("TDiselPanelManager] ExecCMD subFase after all="+toString((int)SubFase) +" Fase="+toString((int)Fase));
	Fase    = CREATE_CMD;
}
//=============================================================================
void TDiselPanelManager::DoECMD( void *Par )
{
	SubFase = 0;
	Fase    = CREATE_CMD;
	Log::DEBUG("TDiselPanelManager::DoECMD");
}
//=============================================================================
void TDiselPanelManager::DetectStates( void  )
{
	if( !Enable()) return;

	  bool flag = false;
	  if( GetTimValue(PERIUD_TIMER) <= 0 )
	  {
	    	DWord tim = SystemTime.GetGlobalSeconds();
	    	if(FirstInitFlg == true)
	    	{
	    		Log::DEBUG("Disel_DetectStates FirstInitFlg="+toString(FirstInitFlg));
	    		FirstInitFlg = false;
	    		flag     = true;
	    		ModifyTime = ((DWord)( (DWord)(tim / Period) + 1)) * Period;
	    		if(ModifyTime > MAX_SECOND_IN_DAY) ModifyTime = Period;
	    	}
	    	else
	    	{
	    		long err = ModifyTime-tim;
	    		if( (ModifyTime <= tim) || (err > Period*2) )
	    		{
	    			flag     = true;
		    		ModifyTime = ((DWord)( (DWord)(tim / Period) + 1)) * Period;
		    		if(ModifyTime > MAX_SECOND_IN_DAY) ModifyTime = Period;
	    		}
	    	}
		  if( flag == true )
		  {
			  DateTime = SystemTime.GetTime();
			  Log::DEBUG("Disel_DetectStates Complite ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
			  for(auto curr: Controllers)
			  {
				  curr->Modify = true;//new
			  }
			  SetTimer( PERIUD_TIMER, 10000 );
		  }
		  else
		  {
			  SetTimer( PERIUD_TIMER, 500 );
			  bool resetperiod = false;
			  for(auto curr: Controllers)
			  {
				  curr->Modify = curr->DetectStates();//new
				  if(!resetperiod)
					  resetperiod = curr->NeedResetPeriod();
			  }
			  if(resetperiod){
				  Period = Aperiod;
			  }
			  else{
				  Period = Operiod;
			  }
		  }
	  }
}
//=============================================================================
void TDiselPanelManager::CreateMessageCMD( void *Par )
{
if( Par != NULL && Enable() )
  {
    if(ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0)
    {
      Ipanel *CurrController = NULL;//new
      int size = Controllers.size();
      int index;
      string framStr = "";
      for( index = 0; index < size; index++ )
      {
    	CurrController = GetSimple(index);
    	if(CurrController == NULL) continue;
        if( CurrController->Modify == true )
        {
        	Log::DEBUG( "[TDiselPanelManager] Disel_CreateMessageCMD CurrIndex="+toString(index));
        	  if(DateTime.Year == 0){
        		  DateTime = SystemTime.GetTime();}
              framStr += TBuffer::DateTimeToString( &DateTime )+">>";
              DateTime.Year  = 0;
              framStr += CurrController->GetStringValue();

              if(framStr.size() > 0){
            	 TFifo<string> *framFifo = (TFifo<string> *)Par;
                 framFifo->push( framStr );
              }
              framStr.clear();
              CurrController->Modify = false;
        }
      }
    }
  }
}
//=============================================================================
void TDiselPanelManager::ChangeState(Word controllerIndex, Word newState)
{
	Ipanel *Controller = GetSimple(controllerIndex);

	Log::DEBUG("TDiselPanelManager::ChangeState controllerIndex="+toString(controllerIndex)+
			" Controller="+toString((int)Controller)+" newState="+toString((int)newState));
	if(Controller != NULL){
		Word currState = Controller->State;
		//if(currState != newState){
			Log::DEBUG("TDiselPanelManager::ChangeState newState="+toString((Word)newState));
			Controller->ChangeState(newState);
		//}
	}
}
//=============================================================================
void TDiselPanelManager::ChangeParameters(Word controllerIndex, vector<string> parameters)
{
	Ipanel *Controller = GetSimple(controllerIndex);
	if(Controller != NULL){
		Controller->ChangeParameters(parameters);
	}
}
//=============================================================================


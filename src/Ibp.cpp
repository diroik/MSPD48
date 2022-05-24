/*
 * Ibp.cpp
 *
 *  Created on: Oct 20, 2014
 *      Author: user
 */

#include <Ibp.h>
//*****************************************************************************
//***
//*****************************************************************************
//=============================================================================
Iibp::Iibp(Byte Ename, Byte adr):IDevice(26, Ename, 0), Addr(adr),
	/*Et(26),En(Ename),Modify(false), IsEnable(false),*/ Alarms(20),alarmIndex(0),PointSteps(230, 1, 0), RectifiersInfo(32, 1),
	Fa_in(48, 1), Fb_in(48, 2), Fc_in(48, 3), Fa_out(48, 4), Fb_out(48, 5), Fc_out(48, 6)
{
	//Type = 0;
	Port = NULL;
	BetweenTimeout = 100;
	Ktran = 1.0;
	ErrCnt = 0;
	AckCnt = 0;
	//PortIsLAN = false;
}
//=============================================================================
Iibp::Iibp(Byte Etype, Byte Ename, Byte adr):Iibp(Ename, adr)
{
	Et = Etype;
}

//=============================================================================
Iibp::~Iibp()
{
	/*
	  if(Port !=NULL)
	  {
		  Port->Close();
		  delete Port;
	  }*/
}
//=============================================================================
string Iibp::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							  Uin.GetValueString()+
								UE.GetValueString();//+
	if(Port->PortIsLAN())
	{
		ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
		ReturnString += Port->ToString()+',';
	}
	for ( auto &x:  Rectifiers){
		ReturnString += x.second->GetValueString();
	}

	  /*
	  ReturnString += Alarms.GetValueString();
	  ReturnString += PointSteps.GetValueString();
	  */

	return ReturnString;
}
//=============================================================================
string Iibp::GetAdvStringValue(void)
{
	string ReturnString = "";
	/*
	for ( auto &x:  Rectifiers){
		ReturnString += x.second->GetValueString();
	}
	*/
	ReturnString += Alarms.GetValueString();
	ReturnString += PointSteps.GetValueString();
	return ReturnString;
}
//=============================================================================
string Iibp::GetStringCfgValue(void)
{
	 string ret = "";
	  ret += "{";
	  ret += "\"Et\":"+toString((int)Et)+",";
	  ret += "\"En\":"+toString((int)En)+",";
	  ret += "\"Address\":"+toString((int)Addr)+",";
	  ret += "\"Number\":\""+SiteInfo.SerialNo+"\""+",";
	  ret += "\"Value\":"+toString((int)UE.U.Value)+",";
	  ret += "\"IsEnable\":"+toString(IsEnable)+",";
	  ret += "\"AckCnt\":"+toString((int)AckCnt)+",";
	  ret += "\"ErrCnt\":"+toString((int)ErrCnt);
	  ret+="},";
	 return ret;
}
//=============================================================================
sWord Iibp::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;
	sWord bytes = 0;

	bytes = port->Recv( Buf, MaxLen );
	if(bytes >= 9 )
	{
		if(Buf[0] == 0x01 && Buf[6] == 0x02)
		{
			Word len = Buf[5];
			Bshort fc;
			fc.Data_b[0] = Buf[bytes-6];
			fc.Data_b[1] = Buf[bytes-5];
			if(bytes == (7+len+2))
			{
				RecvLen=bytes;
			}
		}
	}

	/*bytes = port->RecvTo( Buf, MaxLen, 0x02 );
	if(bytes >= 7 )
	{
		Word len = Buf[bytes-2];
		Bshort fc;
		fc.Data_b[0] = Buf[bytes-6];
		fc.Data_b[1] = Buf[bytes-5];
		bytes = port->Recv( Buf, len+2 );
		if(bytes >= 2)
		{
			RecvLen = bytes;
		}
	}*/


	return RecvLen;
}
//=============================================================================
bool Iibp::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;
	if(Len < 9)
		return false;
	Bshort MessCRC;
	MessCRC.Data_b[1] = BUF[Len-2];
	MessCRC.Data_b[0] = BUF[Len-1];
	Short CalcCRC = Crc16(BUF, Len-2);
	//Log::DEBUG("Len="+toString(Len)+" Iibp::ParsingAnswer CalcCRC="+toString((int)CalcCRC) + " MessCRC="+toString(MessCRC.Data_s) );
	return MessCRC.Data_s == CalcCRC;
}
//=============================================================================
void Iibp::InitGPIO( )
{
	/*
 if(portselect.size() > 0 && portstate.size() > 0)
 {
	 string sendStr = "(echo "+portselect+" > /sys/class/gpio/export) >& /dev/null";
	 sendToConsole(sendStr);

	 sendStr = "echo out > /sys/class/gpio/gpio"+portselect+"/direction";
	 sendToConsole(sendStr);

	 sendStr = "echo "+portstate+" > /sys/class/gpio/gpio"+portselect+"/value";
	 sendToConsole(sendStr);


 } */
}
//*****************************************************************************
//***
//*****************************************************************************
Fp1::Fp1(Byte Ename, Byte adr):Iibp(Ename, adr)
{
	Alarms.Pt.Pname = 11;//!!!!
}
//=============================================================================
Fp1::~Fp1()
{

}
//=============================================================================
Word Fp1::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Bshort       Tmp;
	 switch(CMD)
	 {
     case IBP_INIT:
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA2;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x02;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
       break;
       // 01 A5 80 02 01 00 02 52 C3
       // 01 A7 80 02 01 00 02 B0 C2
     case GET_STAT:
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA7;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x02;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;
    	 // 01 CB 80 02 01 00 02 DC CB
     case GET_SYS:
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xCB;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x02;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Fp1::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }

	 return DataLen;
}
//=============================================================================
Byte Fp1::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;
	Byte dataLen = BUF[5];
	Bshort Tmp;
	if(Len > dataLen)
	{
		BUF +=7;//replase pointer
		switch(subFase)
		{
			case IBP_INIT:
				SubFase = GET_STAT;
				break;

			case GET_STAT:
				if(dataLen >= 45)
				{
					Tmp.Data_b[1] = BUF[0];
					Tmp.Data_b[0] = BUF[1];
					CurrTime.Year 	= Tmp.Data_s;
					CurrTime.Month 	= BUF[2];
					CurrTime.Day 	= BUF[3];
					CurrTime.Hour	= BUF[4];
					CurrTime.Minute = BUF[5];
					Tmp.Data_b[1] = BUF[6];
					Tmp.Data_b[0] = BUF[7];
					UE.Iout.CalcMinAverMax(Tmp.Data_s/10.0*Ktran);

					Tmp.Data_b[1] = BUF[8];
					Tmp.Data_b[0] = BUF[9];
					UE.U.CalcMinAverMax(Tmp.Data_s/100.0);

					Tmp.Data_b[1] = BUF[10];
					Tmp.Data_b[0] = BUF[11];
					float sign = BUF[16] == 0 ? -1.0: 1.0;
					UE.Iakb.CalcMinAverMax(sign*Tmp.Data_s/10.0*Ktran);

					Tmp.Data_b[1] = BUF[12];
					Tmp.Data_b[0] = BUF[13];
					UE.Irect.CalcMinAverMax(Tmp.Data_s/10.0*Ktran);

					Tmp.Data_b[1] = BUF[14];
					Tmp.Data_b[0] = BUF[15];
					UE.Temp.CalcMinAverMax(Tmp.Data_s/100.0);

					//UE.State =  BUF[18] == 1 ? 2 : (BUF[18] == 2 ? 0 : ( BUF[18] == 0 ?   BUF[18]));
					UE.State =  BUF[18];
					switch(UE.State =  BUF[18])
					{
					case 0:
						UE.State = 3; //normal
						break;

					case 1:
						UE.State = 2;//test
						break;

					case 2:
						UE.State = 0;//auto
						break;

					case 3:
						UE.State = 1;//manual
						break;
					}


					Tmp.Data_b[1] = BUF[19];
					Tmp.Data_b[0] = BUF[20];
					Word alarm1 = Tmp.Data_s;
					Tmp.Data_b[1] = BUF[21];
					Tmp.Data_b[0] = BUF[22];
					Word alarm2 = Tmp.Data_s;
					for(int i = 0; i < Alarms.size(); i++)
					{
						bool revcAlarm = false;
						if(i < 16)
							revcAlarm = (bool)( alarm1 & (1 << i) );
						else{
							int ind = i - 16;
							revcAlarm = (bool)( alarm2 & (1 << ind) );
						}
						//Alarms[i]->Value = revcAlarm;
						Alarms[i]->SetValue(revcAlarm);
					}
				}
				SubFase = GET_SYS;
				break;

			case GET_SYS:
			{
				for(int i = 0; i < Len-7; i++){
					if(BUF[i] > 0xBB){
						BUF[i] = 0x5F;
					}
				}
				setlocale(LC_CTYPE, "rus");
			    //wstring str = wstring((char*)&BUF[0]);
				string Customer 	= isNullOrWhiteSpace( string((char*)&BUF[0]) ) ? "~": string((char*)&BUF[0]);
				string Location 	= isNullOrWhiteSpace( string((char*)&BUF[21]) ) ? "~": string((char*)&BUF[21]);
				string SerialNo 	= isNullOrWhiteSpace( string((char*)&BUF[42]) ) ? "0": string((char*)&BUF[42]);
				string CU_No 		= isNullOrWhiteSpace( string((char*)&BUF[63]) ) ? "0": string((char*)&BUF[63]);
				string SwVers 		= isNullOrWhiteSpace( string((char*)&BUF[84]) ) ? "~": string((char*)&BUF[84]);
				string BattType 	= isNullOrWhiteSpace( string((char*)&BUF[106]) ) ? "~": string((char*)&BUF[106]);

				SiteInfo.Customer = "ELTEK";
				SiteInfo.CU_No = "FP1";
				if(Port->PortIsLAN())
					SiteInfo.CU_No += " ETH";


				//SiteInfo.Customer 	= Customer;
				SiteInfo.SerialNo 	= trimAllNull(SerialNo);
				SiteInfo.SerialNo 	= replaceAll(SiteInfo.SerialNo, ",", " ");

				//SiteInfo.CU_No 		= CU_No;
				SiteInfo.Location 	= trimAllNull(Location);
				SiteInfo.Location 	= replaceAll(SiteInfo.Location, ",", " ");

				SiteInfo.SwVers		= trimAllNull(SwVers);
				SiteInfo.SwVers 	= replaceAll(SiteInfo.SwVers, ",", " ");

				SiteInfo.BattType 	= trimAllNull(BattType);
				SiteInfo.BattType 	= replaceAll(SiteInfo.BattType, ",", " ");

			}
				SubFase = IBP_SET_CMD;
				break;

		    case IBP_SET_CMD://
		    	//goto exe with SubFase==IBP_SET_CMD
				break;

			default:
				SubFase = IBP_EXIT;
				break;
		};
	}
    return SubFase;
}
//=============================================================================
string Fp1::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							  //Uin.GetValueString()+
								UE.GetValueString();
						         //"\r\n>,99,1,"+ WordToString((int)Type)+",";

		  return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
Fp2::Fp2(Byte Ename, Byte adr):Iibp(Ename, adr)
{

}
//=============================================================================
Fp2::~Fp2()
{

}
//=============================================================================
Word Fp2::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Bshort       Tmp;

	 switch(CMD)
	 {
     case IBP_INIT:
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA2;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = STX;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
       break;

     case GET_AC1://01 A4 80 02 01 08 02   00 04 01 FF 00 39 00 00   A3 3E
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;//function=read
         Buffer[DataLen++] = 0x04;//systemtype=acinput
         Buffer[DataLen++] = 0x01;//systemindex
         Buffer[DataLen++] = 0xFF;//subsystem
         Buffer[DataLen++] = 0x00;//subsystemindex
         Buffer[DataLen++] = 0x39;//dataobject=inputVoltage
         Buffer[DataLen++] = 0x00;//dataelement
         Buffer[DataLen++] = 0x00;//datasize
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
         break;

     case GET_AC2://01 A4 80 02 01 08 02   00 04 02 FF 00 39 00 00   90 3E
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x04;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x39;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
         break;

     case GET_AC3://01 A4 80 02 01 08 02   00 04 03 FF 00 39 00 00   41 3F
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x04;
         Buffer[DataLen++] = 0x03;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x39;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
         break;

     case GET_UE: //01 A4 80 02 01 08 02   00 02 FF FF FF 02 00 00   A4 0C
	         DataLen = 0;
	         Buffer[DataLen++] = SOH;
	         Buffer[DataLen++] = 0xA4;
	         Buffer[DataLen++] = 0x80;
	         Buffer[DataLen++] = 0x02;
	         Buffer[DataLen++] = 0x01;
	         Buffer[DataLen++] = 0x08;
	         Buffer[DataLen++] = STX;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x02;
	         Buffer[DataLen++] = 0xFF;
	         Buffer[DataLen++] = 0xFF;
	         Buffer[DataLen++] = 0xFF;
	         Buffer[DataLen++] = 0x02;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x00;
	         Tmp.Data_s = Crc16(Buffer, DataLen);
	         Buffer[DataLen++] = Tmp.Data_b[1];
	         Buffer[DataLen++] = Tmp.Data_b[0];
			break;

	case GET_AKB: //01 A4 80 02 01 08 02   00 02 FF FF FF 03 00 00   64 5D
	         DataLen = 0;
	         Buffer[DataLen++] = SOH;
	         Buffer[DataLen++] = 0xA4;
	         Buffer[DataLen++] = 0x80;
	         Buffer[DataLen++] = 0x02;
	         Buffer[DataLen++] = 0x01;
	         Buffer[DataLen++] = 0x08;
	         Buffer[DataLen++] = STX;
	         Buffer[DataLen++] = 0x00;//function=read
	         Buffer[DataLen++] = 0x02;//systemtype=battery
	         Buffer[DataLen++] = 0xFF;//systemindex=TOPLEVEL
	         Buffer[DataLen++] = 0xFF;//subsystem=TOPLEVEL
	         Buffer[DataLen++] = 0xFF;//subsystemindex=TOPLEVEL
	         Buffer[DataLen++] = 0x03;//dataobject=outputVoltage
	         Buffer[DataLen++] = 0x00;//dataelement=value
	         Buffer[DataLen++] = 0x00;//datasize
	         Tmp.Data_s = Crc16(Buffer, DataLen);
	         Buffer[DataLen++] = Tmp.Data_b[1];
	         Buffer[DataLen++] = Tmp.Data_b[0];
			break;

	case GET_TEMP://01 A4 80 02 01 08 02   00 02 FF FF FF 04 00 00   A5 EC
	         DataLen = 0;
	         Buffer[DataLen++] = SOH;
	         Buffer[DataLen++] = 0xA4;
	         Buffer[DataLen++] = 0x80;
	         Buffer[DataLen++] = 0x02;
	         Buffer[DataLen++] = 0x01;
	         Buffer[DataLen++] = 0x08;
	         Buffer[DataLen++] = STX;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x02;
	         Buffer[DataLen++] = 0xFF;
	         Buffer[DataLen++] = 0xFF;
	         Buffer[DataLen++] = 0xFF;
	         Buffer[DataLen++] = 0x04;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x00;
	         Tmp.Data_s = Crc16(Buffer, DataLen);
	         Buffer[DataLen++] = Tmp.Data_b[1];
	         Buffer[DataLen++] = Tmp.Data_b[0];
			break;

	case GET_LOAD://01 A4 80 02 01 08 02   00 03 FF 00 00 03 00 00   A4 69
	         DataLen = 0;
	         Buffer[DataLen++] = SOH;
	         Buffer[DataLen++] = 0xA4;
	         Buffer[DataLen++] = 0x80;
	         Buffer[DataLen++] = 0x02;
	         Buffer[DataLen++] = 0x01;
	         Buffer[DataLen++] = 0x08;
	         Buffer[DataLen++] = STX;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x03;
	         Buffer[DataLen++] = 0xFF;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x03;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x00;
	         Tmp.Data_s = Crc16(Buffer, DataLen);
	         Buffer[DataLen++] = Tmp.Data_b[1];
	         Buffer[DataLen++] = Tmp.Data_b[0];
			break;

	case GET_RECT://01 A4 80 02 01 08 02   00 05 FF 00 00 03 00 00   A4 0F
	         DataLen = 0;
	         Buffer[DataLen++] = SOH;
	         Buffer[DataLen++] = 0xA4;
	         Buffer[DataLen++] = 0x80;
	         Buffer[DataLen++] = 0x02;
	         Buffer[DataLen++] = 0x01;
	         Buffer[DataLen++] = 0x08;
	         Buffer[DataLen++] = STX;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x05;
	         Buffer[DataLen++] = 0xFF;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x03;
	         Buffer[DataLen++] = 0x00;
	         Buffer[DataLen++] = 0x00;
	         Tmp.Data_s = Crc16(Buffer, DataLen);
	         Buffer[DataLen++] = Tmp.Data_b[1];
	         Buffer[DataLen++] = Tmp.Data_b[0];
			break;

     case GET_STAT://01 A4 80 02 01 08 02  00 06 01 0E 01 2D 02 00   A4 0F
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x06;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x0E;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x2D;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;

     case GET_COMP://01 A4 80 02 01 08 02  00 01 FF 00 00 01 1B 00   54 E1
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x1B;
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;

     case GET_SITE://01 A4 80 02 01 08 02  00 01 FF 00 00 01 1C 00   64 E3
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x1C;
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;

     case GET_SERIALN://01 A4 80 02 01 08 02   00 01 FF 00 00 01 1D 00   F4 E2
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x1D;
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;

     case GET_AKB_TYPE://01 A4 80 02 01 08 02   00 02 FF FF FF 01 1A 00   C4 F7
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x1A;
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;

     case GET_ALARMS://01 A4 80 02 01 08 02   00 06 01 0E 01 2D 02 00  CRC
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x06;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x0E;
         Buffer[DataLen++] = 0x01+alarmIndex;//
         Buffer[DataLen++] = 0x2D;
         Buffer[DataLen++] = 0x02;//
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;

     case GET_SYS_MODE://01 A4 80 02 01 08 02   00 01 FF 00 00 01 18 00  CRC
         DataLen = 0;
         Buffer[DataLen++] = SOH;
         Buffer[DataLen++] = 0xA4;
         Buffer[DataLen++] = 0x80;
         Buffer[DataLen++] = 0x02;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x08;
         Buffer[DataLen++] = STX;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0xFF;
         Buffer[DataLen++] = 0x00;
         Buffer[DataLen++] = 0x00;//
         Buffer[DataLen++] = 0x01;
         Buffer[DataLen++] = 0x18;
         Buffer[DataLen++] = 0x00;
         Tmp.Data_s = Crc16(Buffer, DataLen);
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[0];
    	 break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Fp2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }

	 return DataLen;
}
//=============================================================================
Byte Fp2::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;
	Byte dataLen = BUF[5];
	sBshort Tmp;
	if(Len > dataLen)
	{
		BUF +=7;//replase pointer
		switch(subFase)
		{
			case IBP_INIT:
				SubFase = GET_AC1;
				break;

			case GET_AC1://00 04 01 FF 00 39 00 00
				if(BUF[1] == 0x04 && BUF[2] == 0x01 && BUF[3] == 0xFF && BUF[5] == 0x39){
					Tmp.Data_b[0] = BUF[8];
					Tmp.Data_b[1] = BUF[9];
					if(Tmp.Data_s == 32767)
					{
						Uin.Ua.CalcMinAverMax(0);
						Uin.Ua.ClrDivider();
						SubFase = GET_AC2;
					}
					else
					{
						if(Tmp.Data_s > 1000 || Tmp.Data_s < -10 ){
							Log::DEBUG("IBP_ERROR Tmp.Data_s=" + toString(Tmp.Data_s)+" Subfase="+toString((int)subFase));
							SubFase = IBP_ERROR;
						}
						else{
							//Log::DEBUG("Ua=" + toString(Uin.Ua.Value));
							Uin.Ua.CalcMinAverMax(Tmp.Data_s);
							SubFase = GET_AC2;
						}
					}
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_AC2://00 04 02 FF 00 39 00 00
				if(BUF[1] == 0x04 && BUF[2] == 0x02 && BUF[3] == 0xFF && BUF[5] == 0x39){
					Tmp.Data_b[0] = BUF[8];
					Tmp.Data_b[1] = BUF[9];
					if(Tmp.Data_s == 32767)
					{
						Uin.Ub.CalcMinAverMax(0);
						Uin.Ub.ClrDivider();
						SubFase = GET_AC3;
					}
					else
					{
						if(Tmp.Data_s > 1000 || Tmp.Data_s < -10 ){
							Log::DEBUG("IBP_ERROR Tmp.Data_s=" + toString(Tmp.Data_s)+" Subfase="+toString((int)subFase));
							SubFase = IBP_ERROR;
						}
						else{
							Uin.Ub.CalcMinAverMax(Tmp.Data_s);
							//Log::DEBUG("Ub=" + toString(Uin.Ub.Value));
							SubFase = GET_AC3;
						}
					}
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_AC3://00 04 03 FF 00 39 00 00
				if(BUF[1] == 0x04 && BUF[2] == 0x03 && BUF[3] == 0xFF && BUF[5] == 0x39){
					Tmp.Data_b[0] = BUF[8];
					Tmp.Data_b[1] = BUF[9];
					if(Tmp.Data_s == 32767)
					{
						Uin.Uc.CalcMinAverMax(0);
						Uin.Uc.ClrDivider();
						SubFase = GET_UE;
					}
					else
					{
						if(Tmp.Data_s > 1000 || Tmp.Data_s < -10 ){
							Log::DEBUG("IBP_ERROR Tmp.Data_s=" + toString(Tmp.Data_s)+" Subfase="+toString((int)subFase));
							SubFase = IBP_ERROR;
						}
						else{
							Uin.Uc.CalcMinAverMax(Tmp.Data_s);
							//Log::DEBUG("Uc=" + toString(Uin.Uc.Value));
							SubFase = GET_UE;
						}
					}
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_UE://00 02 FF FF FF 02 00 00
				if(BUF[1] == 0x02 && BUF[2] == 0xFF && BUF[3] == 0xFF && BUF[5] == 0x02)
				{//03 02 FF FF FF 02 00 02 EF14
					Tmp.Data_b[0] = BUF[8];
					Tmp.Data_b[1] = BUF[9];
					if(Tmp.Data_s/100.0 > 100 || Tmp.Data_s/100.0 < -10 ){
						Log::DEBUG("IBP_ERROR Tmp.Data_s=" + toString(Tmp.Data_s)+" Subfase="+toString((int)subFase));
						SubFase = IBP_ERROR;
					}
					else{
						UE.U.CalcMinAverMax(Tmp.Data_s/100.0);
						//Log::DEBUG("U=" + toString(UE.U.Value));
						SubFase = GET_AKB;
					}
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_AKB://00 02 FF FF FF 03 00 00
				if(BUF[1] == 0x02 && BUF[2] == 0xFF && BUF[3] == 0xFF && BUF[5] == 0x03)
				{
					Tmp.Data_b[0] = BUF[8];
					Tmp.Data_b[1] = BUF[9];

					signed short akbs = (signed short)Tmp.Data_s;
					if(akbs <= 32767 && akbs >= 23000)
					{//errors
						akbs = 0;
					}

					float akbf = akbs*Ktran;
					UE.Iakb.CalcMinAverMax(  akbf );

					//Log::DEBUG("Iakb=" + toString(UE.Iakb.Value));
					SubFase = GET_TEMP;
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_TEMP://00 02 FF FF FF 04 00 00
				if(BUF[1] == 0x02 && BUF[2] == 0xFF && BUF[3] == 0xFF && BUF[5] == 0x04){
					Tmp.Data_b[0] = BUF[8];
					Tmp.Data_b[1] = BUF[9];
					if(Tmp.Data_s == 32767)
					{
						UE.Temp.CalcMinAverMax(199);
						UE.Temp.ClrDivider();
						SubFase = GET_LOAD;
					}
					else
					{
						if(Tmp.Data_s > 200 || Tmp.Data_s < -100 ){
							Log::DEBUG("IBP_ERROR Tmp.Data_s=" + toString((short)Tmp.Data_s)+" Subfase="+toString((int)subFase));
							SubFase = IBP_ERROR;
						}
						else{
							UE.Temp.CalcMinAverMax(Tmp.Data_s);
							//Log::DEBUG("Temp=" + toString(UE.Temp.Value));
							SubFase = GET_LOAD;
						}
					}
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_LOAD://00 03 FF 00 00 03 00 00
				if(BUF[1] == 0x03 && BUF[2] == 0xFF && BUF[3] == 0x00 && BUF[5] == 0x03){
					Tmp.Data_b[0] = BUF[8];
					Tmp.Data_b[1] = BUF[9];
					UE.Iout.CalcMinAverMax(Tmp.Data_s*Ktran);
					//Log::DEBUG("Iout=" + toString(UE.Iout.Value));
					SubFase = GET_RECT;
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_RECT://00 05 FF 00 00 03 00 00
				if(BUF[1] == 0x05 && BUF[2] == 0xFF && BUF[3] == 0x00 && BUF[5] == 0x03){
					Tmp.Data_b[0] = BUF[8];
					Tmp.Data_b[1] = BUF[9];
					UE.Irect.CalcMinAverMax(Tmp.Data_s*Ktran);
					//Log::DEBUG("Irect=" + toString(UE.Irect.Value));
					SubFase = GET_COMP;
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_COMP://00 01 FF 00 00 01 1B 00
				if(BUF[1] == 0x01 && BUF[2] == 0xFF && BUF[3] == 0x00 && BUF[5] == 0x01 && BUF[6] == 0x1B){
					//SiteInfo.Customer = string((char*)&BUF[8]);
					SiteInfo.Customer = "ELTEK";
					SiteInfo.CU_No = "FP2 SMARTPACK";
					if(Ktran < 1.0)
						SiteInfo.CU_No += " S";

					if(Port->PortIsLAN())
						SiteInfo.CU_No += " ETH";
					//Log::DEBUG(SiteInfo.Customer);
					SubFase = GET_SITE;
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_SITE://00 01 FF 00 00 01 1C 00
				if(BUF[1] == 0x01 && BUF[2] == 0xFF && BUF[3] == 0x00 && BUF[5] == 0x01 && BUF[6] == 0x1C){
					string v = string((char*)&BUF[8]);
					SiteInfo.Location = isNullOrWhiteSpace(v) ? "~": trimAllNull(v);
					SiteInfo.Location 	= replaceAll(SiteInfo.Location, ",", " ");

					//Log::DEBUG(SiteInfo.Location);
					SubFase = GET_SERIALN;
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_SERIALN://00 01 FF 00 00 01 1D 00
				if(BUF[1] == 0x01 && BUF[2] == 0xFF && BUF[3] == 0x00 && BUF[5] == 0x01 && BUF[6] == 0x1D){
					string v =  string((char*)&BUF[8]);
					SiteInfo.SerialNo = isNullOrWhiteSpace(v) ? "0": trimAllNull(v);
					SiteInfo.SerialNo 	= replaceAll(SiteInfo.SerialNo, ",", " ");
					//Log::DEBUG(SiteInfo.SerialNo);
					SubFase = GET_AKB_TYPE;
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_AKB_TYPE://00 02 FF FF FF 01 1A 00
				if(BUF[1] == 0x02 && BUF[2] == 0xFF && BUF[3] == 0xFF && BUF[5] == 0x01 && BUF[6] == 0x1A){
					SiteInfo.BattType = isNullOrWhiteSpace( string((char*)&BUF[8]) ) ? "~" : trimAllNull(string((char*)&BUF[8]));
					SiteInfo.BattType 	= replaceAll(SiteInfo.BattType, ",", " ");
					//Log::DEBUG(SiteInfo.BattType);
					SubFase = GET_ALARMS;
				}
				else{
					SubFase = IBP_EXIT;
				}
				break;

			case GET_ALARMS://03 06 01 0E 01   2D 02 00
				if(		BUF[1] == 0x06 &&
						BUF[2] == 0x01 &&
						BUF[3] == 0x0E &&
						BUF[4]==(0x01+alarmIndex) && BUF[5] == 0x2D /**&& BUF[6] == 0x02*/)//OutputId=0 (value)
				{
					bool revcAlarm = (bool)((int)BUF[8]);
					Alarms[alarmIndex]->SetValue( revcAlarm );
					/*
					if(alarmIndex == 0)
					{
						Log::DEBUG("Ibp alarm index0 = "+toString(revcAlarm)+
								" stored="+toString(Alarms[alarmIndex]->Value)+
								" filter="+toString(Alarms[alarmIndex]->filter)+" cnt="+toString(Alarms[alarmIndex]->cnt) );
					}*/


					if(++alarmIndex >= 18){
						alarmIndex = 0;
						SubFase = GET_SYS_MODE;//exit
					}
					else{
						SubFase = GET_ALARMS;
					}
				}
				else{
					alarmIndex = 0;
					SubFase = IBP_EXIT;
				}
				break;

			case GET_SYS_MODE://00 01 FF 00 00 01 18 00
				if(BUF[1] == 0x01 && BUF[2] == 0xFF && BUF[3] == 0x00 && BUF[5] == 0x01 && BUF[6] == 0x18)
				{
					if(BUF[8] > 127){
						Log::DEBUG("IBP_ERROR BUF[8]=" + toString((int)BUF[8])+" Subfase="+toString((int)subFase));
						SubFase = IBP_ERROR;
					}
					else{
						UE.State = BUF[8];
						SubFase = GET_STAT;
					}
				}
				else
				{
					alarmIndex = 0;
					SubFase = IBP_EXIT;
				}
				break;

			case GET_STAT:
				SubFase = IBP_SET_CMD;
				break;

		    case IBP_SET_CMD://
		    	//goto exe with SubFase==IBP_SET_CMD
				break;

			default:
				SubFase = IBP_EXIT;
				break;
		};
	}
    return SubFase;
}
//*****************************************************************************
//***
//*****************************************************************************
Emerson1::Emerson1(Byte Ename, Byte adr):Iibp(Ename, adr)
{
	for(int i =0; i<4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}
}
//=============================================================================
Emerson1::~Emerson1()
{

}
//=============================================================================
Word Emerson1::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;

     ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT:
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x45;
         Buffer[DataLen++] = 0x31;
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x42;//
         Buffer[DataLen++] = 0x41;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x36;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
       break;

     case GET_AC1://4041
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_ALARMS://4044
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_ALARMS1://4244
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_UE://4241
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_RECT://4141
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x31;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

		case GET_RECT_ALARM://4144
	         DataLen = 0;
	         Buffer[DataLen++] = SOI;
	         Buffer[DataLen++] = 0x32;
	         Buffer[DataLen++] = 0x30;
	         Buffer[DataLen++] = Tmp2.Data_b[0];
	         Buffer[DataLen++] = Tmp2.Data_b[1];
	         Buffer[DataLen++] = 0x34;
	         Buffer[DataLen++] = 0x31;
	         Buffer[DataLen++] = 0x34;//
	         Buffer[DataLen++] = 0x34;//
	         Buffer[DataLen++] = 0x30;//
	         Buffer[DataLen++] = 0x30;//
	         Buffer[DataLen++] = 0x30;//
	         Buffer[DataLen++] = 0x30;//
	         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
	         Buffer[DataLen++] = Tmp.Data_b[0];
	         Buffer[DataLen++] = Tmp.Data_b[1];
	         Buffer[DataLen++] = Tmp.Data_b[2];
	         Buffer[DataLen++] = Tmp.Data_b[3];
	         Buffer[DataLen++] = EOI;
			break;

		case GET_RECT_BOOL://4143
	         DataLen = 0;
	         Buffer[DataLen++] = SOI;
	         Buffer[DataLen++] = 0x32;
	         Buffer[DataLen++] = 0x30;
	         Buffer[DataLen++] = Tmp2.Data_b[0];
	         Buffer[DataLen++] = Tmp2.Data_b[1];
	         Buffer[DataLen++] = 0x34;
	         Buffer[DataLen++] = 0x31;
	         Buffer[DataLen++] = 0x34;//
	         Buffer[DataLen++] = 0x33;//
	         Buffer[DataLen++] = 0x30;//
	         Buffer[DataLen++] = 0x30;//
	         Buffer[DataLen++] = 0x30;//
	         Buffer[DataLen++] = 0x30;//
	         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
	         Buffer[DataLen++] = Tmp.Data_b[0];
	         Buffer[DataLen++] = Tmp.Data_b[1];
	         Buffer[DataLen++] = Tmp.Data_b[2];
	         Buffer[DataLen++] = Tmp.Data_b[3];
	         Buffer[DataLen++] = EOI;
			break;

    case GET_AC2:
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x36;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

    case GET_AC3:
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x36;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

    case GET_STAT://E181
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x45;
         Buffer[DataLen++] = 0x31;
         Buffer[DataLen++] = 0x38;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

   //params
    case GET_SITE:
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x31;
         Buffer[DataLen++] = 0x35;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

    case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				Log::DEBUG("Emerson1::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
			}
   	 break;

     default:
       DataLen = 0;
       break;
	 }
	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte Emerson1::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;
	sBshort Tmp;
	BUF[Len] = 0;
	//BUF +=7;//replase pointer
	switch(subFase)
	{
		case IBP_INIT:
			SubFase = GET_AC1;
			break;

		case GET_AC1:
			if(Len > 48){
				Byte buf[9];
				buf[0] = BUF[17];
				buf[1] = BUF[18];
				buf[2] = BUF[19];
				buf[3] = BUF[20];
				buf[4] = BUF[21];
				buf[5] = BUF[22];
				buf[6] = BUF[23];
				buf[7] = BUF[24];
				buf[8] = 0;
				Uin.Ua.CalcMinAverMax(HexToFloat(buf));

				buf[0] = BUF[25];
				buf[1] = BUF[26];
				buf[2] = BUF[27];
				buf[3] = BUF[28];
				buf[4] = BUF[29];
				buf[5] = BUF[30];
				buf[6] = BUF[31];
				buf[7] = BUF[32];
				buf[8] = 0;
				Uin.Ub.CalcMinAverMax(HexToFloat(buf));

				buf[0] = BUF[33];
				buf[1] = BUF[34];
				buf[2] = BUF[35];
				buf[3] = BUF[36];
				buf[4] = BUF[37];
				buf[5] = BUF[38];
				buf[6] = BUF[39];
				buf[7] = BUF[40];
				buf[8] = 0;
				Uin.Uc.CalcMinAverMax(HexToFloat(buf));
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS:
			Log::DEBUG( "[IbpManager] ALARMS: RecvData ok: [" +toString(string((char*)BUF,Len)) +"] len="+toString(Len));
			if(Len > 31){
				Byte buf[3];
				for(int i= 0; i < 3; i++)
				{
					Word index = 17 + i*2;
					Word alIndex = i;
					buf[0] = BUF[index];
					buf[1] = BUF[index+1];
					buf[2] = 0;
					//Alarms[alIndex]->Value = (bool)HexToByte(buf);//input voltage A/B/C
					Alarms[alIndex]->SetValue((bool)HexToByte(buf));
				}
				//Alarm status of input voltage A/B/C (lover|higer|phase loss|fuse blow|switch off)

			}
			SubFase = GET_ALARMS1;
			break;

		case GET_ALARMS1:
			Log::DEBUG( "[IbpManager] ALARMS1: RecvData ok: [" +toString(string((char*)BUF,Len)) +"] len="+toString(Len));
			if(Len > 21){
				Byte buf[3];
				buf[0] = BUF[17];
				buf[1] = BUF[18];
				buf[2] = 0;

				int st = HexToByte(buf);
				//Alarms[3]->Value = st < 0 ? false: (bool)st;//dc voltage alarm status
				Alarms[3]->SetValue(st < 0 ? false: (bool)st);
				buf[0] = BUF[19];
				buf[1] = BUF[20];
				buf[2] = 0;

				st = HexToByte(buf);
				Word fuseln = st < 0 ? 0 : st;
				bool fuse_alarm = false;
				Word index = 21;
				for(int i= 0; i < fuseln; i++)
				{
					index = 21 + i*2;
					buf[0] = BUF[index];
					buf[1] = BUF[index+1];
					buf[2] = 0;
					st = HexToByte(buf);
					if(st >= 0)
						fuse_alarm = (bool)st;
				}
				//Alarms[4]->Value = fuse_alarm;//dc fuse_alarm status
				Alarms[4]->SetValue(fuse_alarm);
				index+=2;
				buf[0] = BUF[index++];
				buf[1] = BUF[index++];
				buf[2] = 0;
				st = HexToByte(buf);
				bool global_dc_alarm = false;
				if(st >=0)
				{
					Word l = Len-5;
					int plen = (l-index)/2;
					for(int i= 0; i < plen; i++){
						buf[0] = BUF[index++];
						buf[1] = BUF[index++];
						buf[2] = 0;
						st = HexToByte(buf);
					}
					if(st >= 0)
						global_dc_alarm = (bool)st;
				}
				//Alarms[5]->Value = global_dc_alarm;
				Alarms[5]->SetValue(global_dc_alarm);
			}
			SubFase = GET_UE;
			break;

		case GET_UE:
			//Log::DEBUG("GET_UE Len="+toString(Len));
			if(Len > 53)
			{
				Byte buf[9];
				buf[0] = BUF[17];
				buf[1] = BUF[18];
				buf[2] = BUF[19];
				buf[3] = BUF[20];
				buf[4] = BUF[21];
				buf[5] = BUF[22];
				buf[6] = BUF[23];
				buf[7] = BUF[24];
				buf[8] = 0;
				Log::DEBUG("UE.U="+toString(buf));
				UE.U.CalcMinAverMax(HexToFloat(buf));

				buf[0] = BUF[25];
				buf[1] = BUF[26];
				buf[2] = BUF[27];
				buf[3] = BUF[28];
				buf[4] = BUF[29];
				buf[5] = BUF[30];
				buf[6] = BUF[31];
				buf[7] = BUF[32];
				buf[8] = 0;
				Log::DEBUG("UE.Iout="+toString(buf));
				UE.Iout.CalcMinAverMax(HexToFloat(buf)*Ktran);//Irect

				buf[0] = BUF[35];
				buf[1] = BUF[36];
				buf[2] = BUF[37];
				buf[3] = BUF[38];
				buf[4] = BUF[39];
				buf[5] = BUF[40];
				buf[6] = BUF[41];
				buf[7] = BUF[42];
				buf[8] = 0;
				Log::DEBUG("UE.Iakb="+toString(buf));
				UE.Iakb.CalcMinAverMax(-1.0*HexToFloat(buf)*Ktran);
			}

			if(Len > 83)
			{
				Byte buf[9];
				buf[0] = BUF[71];
				buf[1] = BUF[72];
				buf[2] = BUF[73];
				buf[3] = BUF[74];
				buf[4] = BUF[75];
				buf[5] = BUF[76];
				buf[6] = BUF[77];
				buf[7] = BUF[78];
				buf[8] = 0;
				Log::DEBUG("UE.Temp="+toString(buf));
				UE.Temp.CalcMinAverMax(HexToFloat(buf));

			}
			SubFase = GET_RECT_ALARM;
			break;

		case GET_RECT_ALARM:
			Log::DEBUG( "[IbpManager] RECT_ALARM: RecvData ok: [" +toString(string((char*)BUF,Len)) +"] len="+toString(Len));
			if(Len > 17){
				Byte buf[3];
				Word index = 15;
				buf[0] = BUF[index++];
				buf[1] = BUF[index++];
				buf[2] = 0;
				int st = HexToByte(buf);
				Word lm = st < 0 ? 0 : st;
				if(lm > 10)
					lm = 10;
				Word l = Len-5;
				int mlen = (l-index)/2;
				bool global_rect_alarm = false;
				if(mlen > 0)
				{
					Word plen = mlen/lm;
					for(int i= 0; i < lm; i++)
					{
						if(index >= Len)
							break;
						map<int, TRectifier*>::iterator it = Rectifiers.find(i+1);
						if( it == Rectifiers.end())
							Rectifiers[i+1] = new TRectifier(31,i+1);
						bool rect_alarm = false;
						//Log::DEBUG( "[IbpManager] RECT_ALARM detect plen="+toString((Word)plen));
						bool rect_interface = false;
						for(int j= 0; j < plen; j++)
						{
							if(index > l-2){
								//Log::DEBUG( "[IbpManager] RECT_ALARM: break");
								break;
							}
							buf[0] = BUF[index++];
							buf[1] = BUF[index++];
							buf[2] = 0;
							st = HexToByte(buf);
							if(st >= 0 && j !=1 )//in byte whith index=1 storage data type
								rect_alarm = (bool)st;

							if(rect_alarm)
								global_rect_alarm = true;

							if(j == (plen-1))//interface status
							{
								rect_interface = st == 0xE2 ? true : false;
								if(rect_interface)
									Log::DEBUG( "[IbpManager] RECT_ALARM interface alarm detect, rect="+toString(i+1));
							}


							//Log::DEBUG( "[IbpManager] RECT_ALARM detect j="+
							//		toString((Word)j)+" index"+toString((Word)index)+" st="+toString((int)st));

						}
						Rectifiers[i+1]->Interface = rect_interface;
						Rectifiers[i+1]->Alarms = rect_alarm;
					}
				}
				//Alarms[6]->Value = global_rect_alarm;
				Alarms[6]->SetValue(global_rect_alarm);
				//Log::DEBUG( "[IbpManager] RECT_ALARM: lm="+toString(lm)+" l="+toString(l)+" mlen="+toString(mlen));
			}

			SubFase = GET_RECT;
			break;

		case GET_RECT:
			Log::DEBUG( "[IbpManager] RECT: RecvData ok: [" +toString(string((char*)BUF,Len)) +"] len="+toString(Len));
			if(Len > 23){
				Byte buf[9];
				Word index = 23;
				buf[0] = BUF[index++];
				buf[1] = BUF[index++];
				buf[2] = 0;
				int st = HexToByte(buf);
				Word lm = st < 0 ? 0 : st;
				if(lm > 10)
					lm = 10;
				Word l = Len-5;
				int mlen = (l-index)/2;
				float rectCurent = 0.0;
				//Log::DEBUG( "[IbpManager] RECT: lm="+toString(lm)+" l="+toString(l)+" mlen="+toString(mlen));
				if(mlen > 0)
				{
					if(index >= Len)
						break;
					Word plen = mlen/lm;
					for(int i= 0; i < lm; i++)
					{
						//Log::DEBUG( "[IbpManager] RECT: index ="+ toString(index)+" plen="+toString(plen));
						if(index > l-10){
							//Log::DEBUG( "[IbpManager] RECT: break index ="+ toString(index));
							break;
						}

						map<int, TRectifier*>::iterator it = Rectifiers.find(i+1);
						if( it == Rectifiers.end())
							Rectifiers[i+1] = new TRectifier(31,i+1);
					   Word st_index=index;
						buf[0] = BUF[index++];
						buf[1] = BUF[index++];
						buf[2] = BUF[index++];
						buf[3] = BUF[index++];
						buf[4] = BUF[index++];
						buf[5] = BUF[index++];
						buf[6] = BUF[index++];
						buf[7] = BUF[index++];
						buf[8] = 0;
						float rectVal = HexToFloat(buf)*Ktran;
						if(Rectifiers[i+1]->Interface == true)//alarm interface communication
							rectVal = 0;

						rectCurent += rectVal;
						Rectifiers[i+1]->I.CalcMinAverMax(rectVal);

						buf[0] = BUF[index++];
						buf[1] = BUF[index++];
						buf[2] = 0;
						int p = HexToByte(buf);
						if(p > 0){

						}
						index +=plen*2-(index-st_index);
					}
				}
				UE.Irect.CalcMinAverMax(rectCurent);//Iout
			}
			SubFase = GET_RECT_BOOL;
			break;

		case GET_RECT_BOOL:
			//Log::DEBUG( "[IbpManager] BOOL RecvData ok: [" +toString(string((char*)BUF,Len)) +"] len="+toString(Len));
			SubFase = GET_SITE;
			break;


		case GET_SITE:
			//Log::DEBUG( "[IbpManager] SITE RecvData ok: [" +toString(string((char*)BUF,Len)) +"] len="+toString(Len));
			SubFase = GET_STAT;
			break;

		case GET_STAT:
			if(Len > 14)
			{
				Byte buf[3];
				buf[0] = BUF[13];
				buf[1] = BUF[14];
				buf[2] = 0;

				UE.State = (HexToByte(buf) - 0xE0);
				SiteInfo.Customer = "Emerson";
				SiteInfo.CU_No = "M500D/M520S/PD48";
				if(Port->PortIsLAN())
					SiteInfo.CU_No += " ETH";
			}
			SubFase = GET_AC2;
			//Log::DEBUG("SubFase=GET_STAT");
			break;

		case GET_AC2:
			if(Len > 30){
				Byte buf[9];
				buf[0] = BUF[13];
				buf[1] = BUF[14];
				buf[2] = BUF[15];
				buf[3] = BUF[16];
				buf[4] = BUF[17];
				buf[5] = BUF[18];
				buf[6] = BUF[19];
				buf[7] = BUF[20];
				buf[8] = 0;
				PointSteps[0]->Value = HexToFloat(buf);//upper limit of AC voltage

				buf[0] = BUF[21];
				buf[1] = BUF[22];
				buf[2] = BUF[23];
				buf[3] = BUF[24];
				buf[4] = BUF[25];
				buf[5] = BUF[26];
				buf[6] = BUF[27];
				buf[7] = BUF[28];
				buf[8] = 0;
				PointSteps[1]->Value  = HexToFloat(buf);//lower limit of AC voltage
			}
			SubFase = GET_AC3;
			//Log::DEBUG("SubFase=GET_AC2");
			break;

		case GET_AC3:
			if(Len > 30){
				Byte buf[9];
				buf[0] = BUF[13];
				buf[1] = BUF[14];
				buf[2] = BUF[15];
				buf[3] = BUF[16];
				buf[4] = BUF[17];
				buf[5] = BUF[18];
				buf[6] = BUF[19];
				buf[7] = BUF[20];
				buf[8] = 0;
				PointSteps[2]->Value  = HexToFloat(buf);//upper limit of DC voltage

				buf[0] = BUF[21];
				buf[1] = BUF[22];
				buf[2] = BUF[23];
				buf[3] = BUF[24];
				buf[4] = BUF[25];
				buf[5] = BUF[26];
				buf[6] = BUF[27];
				buf[7] = BUF[28];
				buf[8] = 0;
				PointSteps[3]->Value  = HexToFloat(buf);//lower limit of DC voltage
			}
			SubFase = IBP_SET_CMD;
			//Log::DEBUG("SubFase=GET_AC3");
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord  Emerson1::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;
	//sWord bytes = 0;

	RecvLen = port->RecvTo( Buf, MaxLen, EOI );//port->Recv( Buf, MaxLen ); //
	return RecvLen;
}
//=============================================================================
bool Emerson1::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	Blong       Tmp;
	Byte Data_b[5];

	Tmp.Data_b[0] = BUF[Len-5];
	Tmp.Data_b[1] = BUF[Len-4];
	Tmp.Data_b[2] = BUF[Len-3];
	Tmp.Data_b[3] = BUF[Len-2];

	Short RecvCrc = HexToWord(Tmp.Data_b);
	Short CalcCrc  = CHKSUM(BUF, Len-5);


	Log::DEBUG("ParsingAnswer RecvCrc="+toString(RecvCrc) + " CalcCrc="+toString(CalcCrc)+ " Len="+toString(Len));

	return RecvCrc == CalcCrc;
}
//*****************************************************************************
//***
//*****************************************************************************
Huawei::Huawei(Byte Ename, Byte adr):Iibp(Ename, adr)
{
	for(int i =0; i<4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}

	ProtoVer.Data_b[0] = 0x32;
	ProtoVer.Data_b[1] = 0x30;
}
//=============================================================================
Huawei::~Huawei()
{

}
//=============================================================================
Word Huawei::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;

     ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://40 4F
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x46;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
       break;

     case GET_AC1: //40 41
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x46;//
         Buffer[DataLen++] = 0x46;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_ALARMS://40 44
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_ALARMS1://42 44
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_UE: //42 41
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x46;//
         Buffer[DataLen++] = 0x46;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;

         //Log::DEBUG( "GET_UE send: [" + toString(Buffer) +"] len="+toString(DataLen));
         break;

     case GET_RECT://41 41
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x31;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

    case GET_AC2://40 46
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x36;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

    case GET_AC3://42 46
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x32;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x36;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;


    case GET_STAT://41 43
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x31;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x33;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

   //params
    case GET_SITE://40 51
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x34;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x35;//
         Buffer[DataLen++] = 0x31;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

    case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
			}
   	 break;

     default:
       DataLen = 0;
       break;
	 }
	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte Huawei::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;
	sBshort Tmp;
	BUF[Len] = 0;
	//BUF +=7;//replase pointer
	switch(subFase)
	{
		case IBP_INIT:
			if(Len >= 18){
				ProtoVer.Data_b[0] = BUF[1];
				ProtoVer.Data_b[1] = BUF[2];
			}
			SubFase = GET_AC1;
			break;

		case GET_AC1://40 41
			//Log::DEBUG( "GET_AC1 recv: [" + toString(BUF) +"] len="+toString(Len));
			if(Len > 48){
				Byte buf[9];
				buf[0] = BUF[19];
				buf[1] = BUF[20];
				buf[2] = BUF[21];
				buf[3] = BUF[22];
				buf[4] = BUF[23];
				buf[5] = BUF[24];
				buf[6] = BUF[25];
				buf[7] = BUF[26];
				buf[8] = 0;
				Uin.Ua.CalcMinAverMax(HexToFloat(buf));

				buf[0] = BUF[27];
				buf[1] = BUF[28];
				buf[2] = BUF[29];
				buf[3] = BUF[30];
				buf[4] = BUF[31];
				buf[5] = BUF[32];
				buf[6] = BUF[33];
				buf[7] = BUF[34];
				buf[8] = 0;
				Uin.Ub.CalcMinAverMax(HexToFloat(buf));

				buf[0] = BUF[35];
				buf[1] = BUF[36];
				buf[2] = BUF[37];
				buf[3] = BUF[38];
				buf[4] = BUF[39];
				buf[5] = BUF[40];
				buf[6] = BUF[41];
				buf[7] = BUF[42];
				buf[8] = 0;
				Uin.Uc.CalcMinAverMax(HexToFloat(buf));
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS:
			if(Len > 31){
				Byte buf[3];
				for(int i= 0; i < 3; i++){
					Word index = 17 + i*2;
					Word alIndex = i;
					buf[0] = BUF[index];
					buf[1] = BUF[index+1];
					buf[2] = 0;
					//Alarms[alIndex]->Value = (bool)HexToByte(buf);//input voltage A/B/C
					Alarms[alIndex]->SetValue((bool)HexToByte(buf));
				}
			}
			SubFase = GET_ALARMS1;
			break;

		case GET_ALARMS1:
			//Log::DEBUG( "GET_ALARMS1 recv: [" + toString(BUF) +"] len="+toString(Len));
			if(Len > 21){
				Byte buf[3];
				buf[0] = BUF[19];
				buf[1] = BUF[20];
				buf[2] = 0;
				Word ln = HexToByte(buf);
				//Log::DEBUG( "GET_ALARMS1 ok: [" + toString(BUF) +"] len="+toString(Len));
				for(int i= 0; i < ln; i++){
					Word index = 21 + i*2;
					Word alIndex = i+3;
					if( (index+2) < Len && (i+3) < Alarms.size() ){
						buf[0] = BUF[index];
						buf[1] = BUF[index+1];
						buf[2] = 0;
						//Alarms[alIndex]->Value = (bool)HexToByte(buf);
						Alarms[alIndex]->SetValue((bool)HexToByte(buf));
					}
				}
			}
			SubFase = GET_UE;
			break;

		case GET_UE://42 41
			//Log::DEBUG("GET_UE Len="+toString(Len));
			//Log::DEBUG( "GET_UE recv: [" + toString(BUF) +"] len="+toString(Len));
			if(Len > 54){
				Byte buf[9];
				buf[0] = BUF[17];
				buf[1] = BUF[18];
				buf[2] = BUF[19];
				buf[3] = BUF[20];
				buf[4] = BUF[21];
				buf[5] = BUF[22];
				buf[6] = BUF[23];
				buf[7] = BUF[24];
				buf[8] = 0;
				//Log::DEBUG("UE.U="+toString(buf));
				UE.U.CalcMinAverMax(HexToFloat(buf));

				buf[0] = BUF[25];
				buf[1] = BUF[26];
				buf[2] = BUF[27];
				buf[3] = BUF[28];
				buf[4] = BUF[29];
				buf[5] = BUF[30];
				buf[6] = BUF[31];
				buf[7] = BUF[32];
				buf[8] = 0;
				//Log::DEBUG("UE.Iout="+toString(buf));
				//UE.Iout.CalcMinAverMax(HexToFloat(buf));//!!
				UE.Iout.CalcMinAverMax(HexToFloat(buf)*Ktran);

				buf[0] = BUF[35];
				buf[1] = BUF[36];
				buf[2] = BUF[37];
				buf[3] = BUF[38];
				buf[4] = BUF[39];
				buf[5] = BUF[40];
				buf[6] = BUF[41];
				buf[7] = BUF[42];
				buf[8] = 0;
				//Log::DEBUG("UE.Iakb="+toString(buf));
				UE.Iakb.CalcMinAverMax(1.0*HexToFloat(buf)*Ktran);
/*
				buf[0] = BUF[71];
				buf[1] = BUF[72];
				buf[2] = BUF[73];
				buf[3] = BUF[74];
				buf[4] = BUF[75];
				buf[5] = BUF[76];
				buf[6] = BUF[77];
				buf[7] = BUF[78];
				buf[8] = 0;
				Log::DEBUG("UE.Temp="+toString(buf));
				UE.Temp.CalcMinAverMax(HexToFloat(buf));*/

			}
			SubFase = GET_RECT;
			break;

		case GET_RECT://41 41
			//Log::DEBUG( "GET_RECT recv: [" + toString(BUF) +"] len="+toString(Len));

			if(Len > 23){
				Byte buf[9];
				buf[0] = BUF[23];
				buf[1] = BUF[24];
				buf[2] = 0;
				Word ln = HexToByte(buf);
				Log::DEBUG("ln="+toString(ln));
				if(ln > 10) ln = 10;
				float rectCurent = 0.0;
				for(int i= 0; i < ln; i++)
				{
					Word index = 25 + i*10;
					if( (index+8) < Len ){
						buf[0] = BUF[index];
						buf[1] = BUF[index+1];
						buf[2] = BUF[index+2];
						buf[3] = BUF[index+3];
						buf[4] = BUF[index+4];
						buf[5] = BUF[index+5];
						buf[6] = BUF[index+6];
						buf[7] = BUF[index+7];
						buf[8] = 0;

						string irect = toString(buf);
						if( !isNullOrWhiteSpace(irect)){
							rectCurent += HexToFloat(buf)*Ktran;
						}
					}
				}
				UE.Irect.CalcMinAverMax(rectCurent);
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE:///40 51
			//Log::DEBUG( "GET_SITE recv: [" + toString(BUF) +"] len="+toString(Len));
			if(Len > 40){
				Byte buf[50];
				for(int i=0; i < 10; i++){
					buf[i] = HexToByte(&BUF[13+(i*2)]);
				}
				string c = string( (const char*)buf, 10);
				for(int i=0; i < 20; i++){
					buf[i] = HexToByte(&BUF[37+(i*2)]);
				}
				string n = string( (const char*)buf, 20);

				//SiteInfo.Customer = trim( n );
				//SiteInfo.CU_No = trim( n ) + " " + trim( c );
				//Log::DEBUG( "CU_No: ["+SiteInfo.CU_No+"]" );

				buf[0] = BUF[33];
				buf[1] = BUF[34];
				buf[2] = 0;
				float sw = HexToByte(buf);

				buf[0] = BUF[35];
				buf[1] = BUF[36];
				buf[2] = 0;
				sw += HexToByte(buf)/100.0;

				SiteInfo.SwVers = toString(sw);
				SiteInfo.SwVers 	= replaceAll(SiteInfo.SwVers, ",", " ");

				SiteInfo.Customer = "HUAWEI";
				SiteInfo.CU_No = "CPMU01";
				if(Port->PortIsLAN())
					SiteInfo.CU_No += " ETH";
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT:
			//Log::DEBUG( "GET_STAT recv: [" + toString(BUF) +"] len="+toString(Len));
			if(Len > 14){
				Byte buf[3];
				buf[0] = BUF[15];
				buf[1] = BUF[16];
				buf[2] = 0;
				Word ln = HexToByte(buf);

				buf[0] = BUF[17];
				buf[1] = BUF[18];
				buf[2] = 0;

				UE.State =  !HexToByte(buf);
			}
			SubFase = GET_AC2;
			//Log::DEBUG("SubFase=GET_STAT");
			break;

		case GET_AC2://40 46
			if(Len > 30){
				Byte buf[9];
				buf[0] = BUF[13];
				buf[1] = BUF[14];
				buf[2] = BUF[15];
				buf[3] = BUF[16];
				buf[4] = BUF[17];
				buf[5] = BUF[18];
				buf[6] = BUF[19];
				buf[7] = BUF[20];
				buf[8] = 0;
				PointSteps[0]->Value = HexToFloat(buf);//upper limit of AC voltage

				buf[0] = BUF[21];
				buf[1] = BUF[22];
				buf[2] = BUF[23];
				buf[3] = BUF[24];
				buf[4] = BUF[25];
				buf[5] = BUF[26];
				buf[6] = BUF[27];
				buf[7] = BUF[28];
				buf[8] = 0;
				PointSteps[1]->Value  = HexToFloat(buf);//lower limit of AC voltage
			}
			SubFase = GET_AC3;
			//Log::DEBUG("SubFase=GET_AC2");
			break;

		case GET_AC3://42 46
			if(Len > 30){
				Byte buf[9];
				buf[0] = BUF[13];
				buf[1] = BUF[14];
				buf[2] = BUF[15];
				buf[3] = BUF[16];
				buf[4] = BUF[17];
				buf[5] = BUF[18];
				buf[6] = BUF[19];
				buf[7] = BUF[20];
				buf[8] = 0;
				PointSteps[2]->Value  = HexToFloat(buf);//upper limit of DC voltage

				buf[0] = BUF[21];
				buf[1] = BUF[22];
				buf[2] = BUF[23];
				buf[3] = BUF[24];
				buf[4] = BUF[25];
				buf[5] = BUF[26];
				buf[6] = BUF[27];
				buf[7] = BUF[28];
				buf[8] = 0;
				PointSteps[3]->Value  = HexToFloat(buf);//lower limit of DC voltage
			}
			SubFase = IBP_SET_CMD;
			//Log::DEBUG("SubFase=GET_AC3");
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord  Huawei::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;
	//sWord bytes = 0;

	RecvLen = port->RecvTo( Buf, MaxLen, EOI );//port->Recv( Buf, MaxLen ); //
	return RecvLen;
}
//=============================================================================
bool Huawei::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	Blong       Tmp;
	Byte Data_b[5];

	Tmp.Data_b[0] = BUF[Len-5];
	Tmp.Data_b[1] = BUF[Len-4];
	Tmp.Data_b[2] = BUF[Len-3];
	Tmp.Data_b[3] = BUF[Len-2];

	Short RecvCrc = HexToWord(Tmp.Data_b);
	Short CalcCrc  = CHKSUM(BUF, Len-5);
	//Log::DEBUG("ParsingAnswer RecvCrc="+toString(RecvCrc) + " CalcCrc="+toString(CalcCrc)+ " Len="+toString(Len));
	return RecvCrc == CalcCrc;
}
//*****************************************************************************
//***
//*****************************************************************************
Huawei2::Huawei2(Byte Ename, Byte adr):Iibp(Ename, adr), c_read("public"), c_write("private"), c_trap("trap")
{
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}

	/*
	for(int i = 0; i < 19; i++){//20-19=1 only global alarm
		Alarms.Alarms.pop_back();
	}*/
}
//=============================================================================
Huawei2::Huawei2(Byte Ename, Byte adr, string read, string write, string trap):Huawei2(Ename, adr)
{
	/*
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}
*/
    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("Huawei2::Huawei2 c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("Huawei2::Huawei2 c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("Huawei2::Huawei2 c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
Huawei2::~Huawei2()
{

}
//=============================================================================
Word Huawei2::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.1.1.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.5.2.1.1.4";
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.5.2.1.1.5";
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.5.2.1.1.6";
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_UE: //
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.6.1.3.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_LOAD: //
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.6.1.4.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT://
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.3.1.3.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AKB://
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.4.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.1.2.99.1.1";
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_STAT://
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.2.2.1.1.11.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_SITE://
    	 last_oid = "1.3.6.1.4.1.2011.6.164.1.2.2.1.1.5.1";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_POINT_STEPS://
    	 //Log::DEBUG("CreateCMD, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
    	 switch(PointSteps.CurrentIndex)
    	 {
			 case 0:
				 last_oid = "1.3.6.1.4.1.2011.6.164.1.5.1.5.0";
				 break;
			 case 1:
				 last_oid = "1.3.6.1.4.1.2011.6.164.1.5.1.6.0";
				 break;
			 case 2:
				 last_oid = "1.3.6.1.4.1.2011.6.164.1.6.1.7.0";
				 break;
			 case 3:
				 last_oid = "1.3.6.1.4.1.2011.6.164.1.6.1.8.0";
				 break;
    	 };
    	 //
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V1, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte Huawei2::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	Log::DEBUG("snmp_driver.GetData=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				if(val.size() > 1 && val.c_str()[0] == '$')
					val.erase(0, 1);

				Uin.Ua.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				if(val.size() > 1 && val.c_str()[0] == '$')
					val.erase(0, 1);
				Uin.Ub.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				if(val.size() > 1 && val.c_str()[0] == '$')
					val.erase(0, 1);
				Uin.Uc.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_UE;
			break;

		case GET_UE://
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_LOAD;
			break;

		case GET_LOAD://
			if(!isNullOrWhiteSpace(val)){
				UE.Iout.CalcMinAverMax( (atoi(val.c_str())/10.0)*Ktran );
			}
			SubFase = GET_RECT;
			break;

		case GET_RECT://
			if(!isNullOrWhiteSpace(val)){
				UE.Irect.CalcMinAverMax( (atoi(val.c_str())/10.0)*Ktran );
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://
			if(!isNullOrWhiteSpace(val)){
				//Log::DEBUG("Huawei2::GetDataFromMessage val=["+val+"] len="+toString(val.size()));
				UE.Iakb.CalcMinAverMax( (((short)atoi(val.c_str()))/10.0)*Ktran );
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://set only global alarm flag
			if(!isNullOrWhiteSpace(val)){
				Log::DEBUG("Huawei2::GetDataFromMessage Alarms index=["+val+"] len="+toString(val.size()));
				//Alarms[Alarms.size()-1]->Value = true;
				Alarms[Alarms.size()-1]->SetValue(true);
			}
			else{
				for(auto curr: Alarms.Alarms)
					curr->Value = false;
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				switch(st)
				{
					case 1:
						st = 1;
						break;
					case 2:
						st = 0;
						break;
					default:
						//st = 255;
						break;
				}
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			SiteInfo.Customer = "HUAWEI";
			SiteInfo.CU_No = "SMU02B/C";
			if(!isNullOrWhiteSpace(val)){
				size_t found = val.find("_");
				if( found != string::npos)
					SiteInfo.CU_No = val.substr(0, found);
			}
			if(Port->PortIsLAN())
				SiteInfo.CU_No += " ETH";
			SubFase = GET_POINT_STEPS;
			break;

	    case GET_POINT_STEPS://
			if(!isNullOrWhiteSpace(val)){
				float v = atof(val.c_str());
				if(PointSteps.CurrentIndex >= 2)
					v = v/10.0;
				PointSteps[PointSteps.CurrentIndex]->Value = v;
			}
			//Log::DEBUG("GetDataFromMessage, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
	    	if(++PointSteps.CurrentIndex < PointSteps.size()){
	    		SubFase = GET_POINT_STEPS;
	    	}
	    	else{
	    		PointSteps.CurrentIndex = 0;
	    		SubFase = IBP_SET_CMD;
	    	}
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord Huawei2::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool Huawei2::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool Huawei2::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool Huawei2::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0://apper AC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.5.0";
			 break;
		 case 1://lower AC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.6.0";
			 break;
		 case 2://apper DC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.7.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 3://lower DC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.8.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}
	return ret;
}
//*****************************************************************************
//***
//*****************************************************************************
HuaweiUPS::HuaweiUPS(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr),Uout(4),Iin(1), Iout(4)
{
	LoadPersent1 = LoadPersent2 = LoadPersent3 = 0.0;

	Log::DEBUG("HuaweiUPS::HuaweiUPS()");

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

}
//=============================================================================
HuaweiUPS::HuaweiUPS(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap):HuaweiUPS(Etype, Ename, adr)
{

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("HuaweiUPS::HuaweiUPS c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("HuaweiUPS::HuaweiUPS c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("HuaweiUPS::HuaweiUPS c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
HuaweiUPS::~HuaweiUPS()
{

}
//=============================================================================

string HuaweiUPS::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							  Uin.GetValueString()+
							  	  Uout.GetValueString()+
								  	Iin.GetValueString()+
								  	  Iout.GetValueString()+
							  	  	  	  UE.GetValueString()+
						         "\r\n>,27,1,"+ FloatToString(LoadPersent1)+","+ FloatToString(LoadPersent2)+","+ FloatToString(LoadPersent3)+",";
	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';

		  return ReturnString;
}
//=============================================================================
Word HuaweiUPS::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://name of ups system
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.1.1.2.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //V1in
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.3.100.1.1.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //V2in
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.3.100.1.2.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //V3in
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.3.100.1.3.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_UE: //battaryVoltage
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.6.100.1.1.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AKB: //battaryCurrent
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.6.100.1.2.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_TEMP: //battaryTemp
    	 last_oid = "1.3.6.1.2.1.33.1.2.7.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC1_OUT: //out voltage 1
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.1.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC2_OUT: //out voltage 2
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.2.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC3_OUT: //out voltage 3
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.3.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN1: //input current 1
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.3.100.1.5.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN2: //input current 2
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.3.100.1.6.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN3: //input current 3
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.3.100.1.7.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT1: //out current 1
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.4.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT2: //out current 2
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.5.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT3: //out current 3
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.6.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT1: //load persent 1
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.14.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT2: //load persent 2
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.15.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT3: //load persent 3
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.4.100.1.16.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://number of active alarms
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.12.2.0";//
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_STAT://work mode
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.2.101.1.9.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_SITE://device model
    	 last_oid = "1.3.6.1.2.1.33.1.1.2.0"; //"1.3.6.1.4.1.2011.6.174.1.2.100.1.2.1";//100.1.1.1
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_SERIALN://ESN
    	 last_oid = "1.3.6.1.4.1.2011.6.174.1.2.100.1.5.1";//ser number
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;


     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V2, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte HuaweiUPS::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("snmp_driver.GetData=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_UE;
			break;

		case GET_UE://
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://
			if(!isNullOrWhiteSpace(val)){
				UE.Iakb.CalcMinAverMax( (atoi(val.c_str())/10.0)*Ktran );
			}
			SubFase = GET_TEMP;
			break;

		case GET_TEMP://
			if(!isNullOrWhiteSpace(val)){
				float fv = atoi(val.c_str())/1.0;
				if(fv > 200) fv = 199.0;
				UE.Temp.CalcMinAverMax( fv );
			}
			SubFase = GET_AC1_OUT;
			break;

		case GET_AC1_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ua.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC2_OUT;
			break;

		case GET_AC2_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ub.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC3_OUT;
			break;

		case GET_AC3_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Uc.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN1;
			break;

		case GET_I_IN1://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ia.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN2;
			break;

		case GET_I_IN2://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ib.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN3;
			break;

		case GET_I_IN3://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ic.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT1;
			break;

		case GET_I_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ia.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT2;
			break;

		case GET_I_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ib.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT3;
			break;

		case GET_I_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ic.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_LOAD_OUT1;
			break;

		case GET_LOAD_OUT1://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent1 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_LOAD_OUT2;
			break;

		case GET_LOAD_OUT2://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent2 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_LOAD_OUT3;
			break;

		case GET_LOAD_OUT3://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent3 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://set only global alarm flag
			if(!isNullOrWhiteSpace(val))
			{
				//Alarms[Alarms.size()-1]->Value = atoi(val.c_str()) > 0;
				Alarms[Alarms.size()-1]->SetValue(atoi(val.c_str()) > 0);
			}
			else
			{
				for(auto curr: Alarms.Alarms)
				{
					curr->Value = false;
				}
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			SiteInfo.Customer = "HUAWEI";
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.CU_No = val;
				//if(Port->PortIsLAN()) SiteInfo.CU_No += " ETH";
			}

			SubFase = GET_SERIALN;
			break;

		case GET_SERIALN://

			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SerialNo = val;
			}
			SubFase = IBP_SET_CMD;
			break;


	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord HuaweiUPS::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool HuaweiUPS::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool HuaweiUPS::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool HuaweiUPS::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	/*
	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0://apper AC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.5.0";
			 break;
		 case 1://lower AC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.6.0";
			 break;
		 case 2://apper DC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.7.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 3://lower DC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.8.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}
	*/
	return ret;
}
//*****************************************************************************
//***
//*****************************************************************************
TsiBravoUPS::TsiBravoUPS(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr),Uout(4),Iin(1), Iout(4)
{
	//LoadPersent1 = LoadPersent2 = LoadPersent3 = 0.0;

	Log::DEBUG("TsiBravoUPS::TsiBravoUPS()");

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

	//20-17=3
	for(int i = 0; i < 17; i++){
		Alarms.Alarms.pop_back();
	}
	snmp_ver = SNMP_V1;
	//netsnmp::ClrRequestId();
}
//=============================================================================
TsiBravoUPS::TsiBravoUPS(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, SNMP_VER ver):TsiBravoUPS(Etype, Ename, adr)
{

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("TsiBravoUPS::TsiBravoUPS c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("TsiBravoUPS::TsiBravoUPS c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("TsiBravoUPS::TsiBravoUPS c_trap=["+c_trap+"] trap=["+trap+"]");

    snmp_ver = ver;
}
//=============================================================================
TsiBravoUPS::~TsiBravoUPS()
{

}
//=============================================================================

string TsiBravoUPS::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							  Uin.GetValueString()+
							  	  Uout.GetValueString()+
								  	Iin.GetValueString()+
								  	  Iout.GetValueString()+
							  	  	  	  UE.GetValueString();
						         //"\r\n>,27,1,"+ FloatToString(LoadPersent1)+","+ FloatToString(LoadPersent2)+","+ FloatToString(LoadPersent3)+",";
	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';

		  return ReturnString;
}
//=============================================================================
Word TsiBravoUPS::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://soft version
    	 last_oid = "1.3.6.1.4.1.12551.4.1.8.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //V1in
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.2.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //V2in
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.2.2";//
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //V3in
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.2.3";//
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_UE: //battaryVoltage
    	 last_oid = "1.3.6.1.4.1.12551.4.1.4.2.1.2.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AKB: //battaryCurrent
    	 last_oid = "1.3.6.1.4.1.12551.4.1.4.2.1.3.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_TEMP: //battaryTemp
    	 last_oid = "1.3.6.1.2.1.33.1.2.7.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC1_OUT: //out voltage 1
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.6.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC2_OUT: //out voltage 2
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.6.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC3_OUT: //out voltage 3
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.6.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_IN1: //input current 1
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.3.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_IN2: //input current 2
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.3.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_IN3: //input current 3
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.3.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT1: //out current 1
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.7.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT2: //out current 2
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.7.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT3: //out current 3
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.7.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT1: //out freq 1
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.8.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT2: //out freq 2
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.8.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT3: //out freq 3
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.8.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://number of active alarms
		 switch(Alarms.CurrentIndex)
		 {
				 case 0://major relay
					 last_oid = "1.3.6.1.4.1.12551.4.1.5.5";
					 break;
				 case 1://minor relay
					 last_oid = "1.3.6.1.4.1.12551.4.1.5.6";
					 break;
				 case 2://
					 last_oid = "1.3.6.1.4.1.12551.4.1.5.7";
					 break;
		 };
		 values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

    case GET_STAT://status
    	 last_oid = "1.3.6.1.2.1.33.1.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

    case GET_SITE://device model
    	 last_oid = "1.3.6.1.2.1.33.1.1.2.0";//"1.3.6.1.4.1.12551.4.1.8.1.0";//100.1.1.1
    	 values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

    case GET_SERIALN://ESN
    	 last_oid = "1.3.6.1.4.1.12551.4.1.8.2";//ser number
    	 values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;


     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("TsiBravoUPS::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, snmp_ver, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte TsiBravoUPS::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("snmp_driver.GetData=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_UE;
			break;

		case GET_UE://
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://
			if(!isNullOrWhiteSpace(val)){
				UE.Iakb.CalcMinAverMax( (atoi(val.c_str())/10.0)*Ktran );
			}
			SubFase = GET_TEMP;
			break;

		case GET_TEMP://
			if(!isNullOrWhiteSpace(val)){
				float fv = atoi(val.c_str())/1.0;
				if(fv > 200) fv = 199.0;
				UE.Temp.CalcMinAverMax( fv );
			}
			SubFase = GET_AC1_OUT;
			break;

		case GET_AC1_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ua.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC2_OUT;
			break;

		case GET_AC2_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ub.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC3_OUT;
			break;

		case GET_AC3_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Uc.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_IN1;
			break;

		case GET_I_IN1://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ia.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN2;
			break;

		case GET_I_IN2://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ib.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN3;
			break;

		case GET_I_IN3://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ic.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT1;
			break;

		case GET_I_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ia.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT2;
			break;

		case GET_I_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ib.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT3;
			break;

		case GET_I_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ic.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_LOAD_OUT1;
			break;

		case GET_LOAD_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Fa_out.F.CalcMinAverMax(atoi(val.c_str())/10.0);
				//LoadPersent1 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_LOAD_OUT2;
			break;

		case GET_LOAD_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Fb_out.F.CalcMinAverMax(atoi(val.c_str())/10.0);
				//LoadPersent2 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_LOAD_OUT3;
			break;

		case GET_LOAD_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Fc_out.F.CalcMinAverMax(atoi(val.c_str())/10.0);
				//LoadPersent3 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://
			if(!isNullOrWhiteSpace(val)){
				long v = atol(val.c_str());
				//Alarms[Alarms.CurrentIndex]->Value = Alarms.CurrentIndex != 2 ? (v > 0) : !(v > 0);
				Alarms[Alarms.CurrentIndex]->SetValue( Alarms.CurrentIndex != 2 ? (v > 0) : !(v > 0) );
			}
			//Log::DEBUG("GetDataFromMessage, Alarms.CurrentIndex="+toString(Alarms.CurrentIndex)+" val=["+val+"]" );
	    	if(++Alarms.CurrentIndex < Alarms.size()){
	    		SubFase = GET_ALARMS;
	    	}
	    	else{
	    		Alarms.CurrentIndex = 0;
	    		SubFase = GET_STAT;
	    	}
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			SiteInfo.Customer = "CET";
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.CU_No = val;
				//if(Port->PortIsLAN())SiteInfo.CU_No += " ETH";
			}

			SubFase = GET_SERIALN;
			break;

		case GET_SERIALN://

			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SerialNo = val;
			}
			SubFase = IBP_SET_CMD;
			break;


	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord TsiBravoUPS::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool TsiBravoUPS::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool TsiBravoUPS::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool TsiBravoUPS::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	/*
	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0://apper AC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.5.0";
			 break;
		 case 1://lower AC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.6.0";
			 break;
		 case 2://apper DC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.7.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 3://lower DC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.8.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}
	*/
	return ret;
}
//*****************************************************************************
//***
//*****************************************************************************
TsiBravoINV::TsiBravoINV(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr),Uout(4),Iin(1), Iout(4)
{
	//LoadPersent1 = LoadPersent2 = LoadPersent3 = 0.0;

	Log::DEBUG("TsiBravoINV::TsiBravoINV()");

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

	//20-17=3
	for(int i = 0; i < 17; i++){
		Alarms.Alarms.pop_back();
	}
	snmp_ver = SNMP_V1;
	//netsnmp::ClrRequestId();
}
//=============================================================================
TsiBravoINV::TsiBravoINV(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, SNMP_VER ver):TsiBravoINV(Etype, Ename, adr)
{

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("TsiBravoINV::TsiBravoINV c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("TsiBravoINV::TsiBravoINV c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("TsiBravoINV::TsiBravoINV c_trap=["+c_trap+"] trap=["+trap+"]");

    snmp_ver = ver;
}
//=============================================================================
TsiBravoINV::~TsiBravoINV()
{

}
//=============================================================================
string TsiBravoINV::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							  Uin.GetValueString()+
							  	  Uout.GetValueString()+
								  	Iin.GetValueString()+
								  	  Iout.GetValueString()+
							  	  	  	  UE.GetValueString();
						         //"\r\n>,27,1,"+ FloatToString(LoadPersent1)+","+ FloatToString(LoadPersent2)+","+ FloatToString(LoadPersent3)+",";
	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';

		  return ReturnString;
}
//=============================================================================
Word TsiBravoINV::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://soft version
    	 //netsnmp::ClrRequestId();
    	 last_oid = "1.3.6.1.4.1.12551.4.1.8.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //V1in
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.2.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //V2in
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.2.2";//
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //V3in
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.2.3";//
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_UE: //battaryVoltage
    	 last_oid = "1.3.6.1.4.1.12551.4.1.4.2.1.2.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AKB: //battaryCurrent
    	 last_oid = "1.3.6.1.4.1.12551.4.1.4.2.1.3.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_TEMP: //battaryTemp
    	 last_oid = "1.3.6.1.2.1.33.1.2.7.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC1_OUT: //out voltage 1
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.6.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC2_OUT: //out voltage 2
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.6.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_AC3_OUT: //out voltage 3
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.6.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_IN1: //input current 1
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.3.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_IN2: //input current 2
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.3.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_IN3: //input current 3
    	 last_oid = "1.3.6.1.4.1.12551.4.1.3.2.1.3.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT1: //out current 1
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.7.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT2: //out current 2
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.7.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT3: //out current 3
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.7.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT1: //out freq 1
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.8.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT2: //out freq 2
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.8.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT3: //out freq 3
    	 last_oid = "1.3.6.1.4.1.12551.4.1.2.2.1.8.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://number of active alarms
		 switch(Alarms.CurrentIndex)
		 {
				 case 0://major relay
					 last_oid = "1.3.6.1.4.1.12551.4.1.5.5";
					 break;
				 case 1://minor relay
					 last_oid = "1.3.6.1.4.1.12551.4.1.5.6";
					 break;
				 case 2://
					 last_oid = "1.3.6.1.4.1.12551.4.1.5.7";
					 break;
		 };
		 values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

    case GET_STAT://status
    	 last_oid = "1.3.6.1.2.1.33.1.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

    case GET_SITE://device model
    	 last_oid = "1.3.6.1.2.1.33.1.1.2.0";//"1.3.6.1.4.1.12551.4.1.8.1.0";//100.1.1.1
    	 values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;

    case GET_SERIALN://ESN
    	 last_oid = "1.3.6.1.4.1.12551.4.1.8.2";//ser number
    	 values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));
         break;


     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("TsiBravoUPS::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, snmp_ver, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte TsiBravoINV::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("snmp_driver.GetData=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			if(!isNullOrWhiteSpace(val)){
				Log::DEBUG("IBP_INIT snmp_driver.GetData=["+val+"] len="+toString(val.size()));
			}
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_UE;
			break;

		case GET_UE://
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://
			if(!isNullOrWhiteSpace(val)){
				UE.Iakb.CalcMinAverMax( (atoi(val.c_str())/10.0)*Ktran );
			}
			SubFase = GET_TEMP;
			break;

		case GET_TEMP://
			if(!isNullOrWhiteSpace(val)){
				float fv = atoi(val.c_str())/1.0;
				if(fv > 200) fv = 199.0;
				UE.Temp.CalcMinAverMax( fv );
			}
			SubFase = GET_AC1_OUT;
			break;

		case GET_AC1_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ua.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC2_OUT;
			break;

		case GET_AC2_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ub.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC3_OUT;
			break;

		case GET_AC3_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Uc.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_I_IN1;
			break;

		case GET_I_IN1://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ia.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN2;
			break;

		case GET_I_IN2://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ib.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN3;
			break;

		case GET_I_IN3://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ic.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT1;
			break;

		case GET_I_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ia.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_OUT2;
			break;

		case GET_I_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ib.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_OUT3;
			break;

		case GET_I_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ic.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_LOAD_OUT1;
			break;

		case GET_LOAD_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Fa_out.F.CalcMinAverMax(atoi(val.c_str())/10.0);
				//LoadPersent1 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_LOAD_OUT2;
			break;

		case GET_LOAD_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Fb_out.F.CalcMinAverMax(atoi(val.c_str())/10.0);
				//LoadPersent2 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_LOAD_OUT3;
			break;

		case GET_LOAD_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Fc_out.F.CalcMinAverMax(atoi(val.c_str())/10.0);
				//LoadPersent3 = atoi(val.c_str())/10.0;
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://
			if(!isNullOrWhiteSpace(val)){
				long v = atol(val.c_str());
				//Alarms[Alarms.CurrentIndex]->Value = Alarms.CurrentIndex != 2 ? (v > 0) : !(v > 0);
				Alarms[Alarms.CurrentIndex]->SetValue( Alarms.CurrentIndex != 2 ? (v > 0) : !(v > 0) );
			}
			//Log::DEBUG("GetDataFromMessage, Alarms.CurrentIndex="+toString(Alarms.CurrentIndex)+" val=["+val+"]" );
	    	if(++Alarms.CurrentIndex < Alarms.size()){
	    		SubFase = GET_ALARMS;
	    	}
	    	else{
	    		Alarms.CurrentIndex = 0;
	    		SubFase = GET_STAT;
	    	}
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			SiteInfo.Customer = "CET";
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.CU_No = val;
				//if(Port->PortIsLAN())SiteInfo.CU_No += " ETH";
			}

			SubFase = GET_SERIALN;
			break;

		case GET_SERIALN://

			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SerialNo = val;
			}
			SubFase = IBP_SET_CMD;
			break;


	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord TsiBravoINV::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool TsiBravoINV::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool TsiBravoINV::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool TsiBravoINV::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	/*
	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0://apper AC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.5.0";
			 break;
		 case 1://lower AC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.6.0";
			 break;
		 case 2://apper DC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.7.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 3://lower DC limit
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.8.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}
	*/
	return ret;
}
//*****************************************************************************
//***
//*****************************************************************************
UniversalUPS::UniversalUPS(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr),Uout(4),Iin(1), Iout(4)
{
	LoadPersent1 = LoadPersent2 = LoadPersent3 = 0.0;

	Log::DEBUG("UniversalUPS::UniversalUPS()");

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

}
//=============================================================================
UniversalUPS::UniversalUPS(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap):UniversalUPS(Etype, Ename, adr)
{

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("UniversalUPS::UniversalUPS c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("UniversalUPS::UniversalUPS c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("UniversalUPS::UniversalUPS c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
UniversalUPS::~UniversalUPS()
{

}
//=============================================================================
string UniversalUPS::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							  Uin.GetValueString()+
							  	  Uout.GetValueString()+
								  	Iin.GetValueString()+
								  	  Iout.GetValueString()+
							  	  	  	  UE.GetValueString()+
						         "\r\n>,27,1,"+ FloatToString(LoadPersent1)+","+ FloatToString(LoadPersent2)+","+ FloatToString(LoadPersent3)+",";
	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';

		  return ReturnString;
}
//=============================================================================
Word UniversalUPS::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://manufacturer of ups system
    	 last_oid = "1.3.6.1.2.1.33.1.1.1.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //V1in
    	 last_oid = "1.3.6.1.2.1.33.1.3.3.1.3.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //V2in
    	 last_oid = "1.3.6.1.2.1.33.1.3.3.1.3.2";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //V3in
    	 last_oid = "1.3.6.1.2.1.33.1.3.3.1.3.3";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_UE: //battaryVoltage
    	 last_oid = GetBatteryVoltage();//"1.3.6.1.2.1.33.1.2.5.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AKB: //battaryCurrent
    	 last_oid = GetBatteryCurrent();
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_TEMP: //battaryTemp
    	 last_oid = GetBatteryTemp();
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC1_OUT: //out voltage 1
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.2.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC2_OUT: //out voltage 2
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.2.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC3_OUT: //out voltage 3
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.2.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN1: //input current 1
    	 last_oid = "1.3.6.1.2.1.33.1.3.3.1.4.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN2: //input current 2
    	 last_oid = "1.3.6.1.2.1.33.1.3.3.1.4.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN3: //input current 3
    	 last_oid = "1.3.6.1.2.1.33.1.3.3.1.4.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT1: //out current 1
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.3.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT2: //out current 2
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.3.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT3: //out current 3
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.3.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT1: //load persent 1
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.5.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT2: //load persent 2
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.5.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT3: //load persent 3
    	 last_oid = "1.3.6.1.2.1.33.1.4.4.1.5.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC1_BYP: //bypass voltage 1
    	 last_oid = "1.3.6.1.2.1.33.1.5.3.1.2.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC2_BYP: //bypass voltage 2
    	 last_oid = "1.3.6.1.2.1.33.1.5.3.1.2.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC3_BYP: //bypass voltage 3
    	 last_oid = "1.3.6.1.2.1.33.1.5.3.1.2.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;


     case GET_ALARMS://number of active alarms
    	 last_oid = "1.3.6.1.2.1.33.1.6.1.0";//
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_STAT://battery status
    	 last_oid = "1.3.6.1.2.1.33.1.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_SITE://device model
    	 last_oid = "1.3.6.1.2.1.33.1.1.2.0";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_SERIALN://ESN
    	 last_oid = "1.3.6.1.6.3.1.1.6.1.0";//ser number
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;


     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V2, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte UniversalUPS::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("snmp_driver.GetData=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			SiteInfo.Customer = "NONAME";
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.Customer = val;
			}
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_UE;
			break;

		case GET_UE://
			//Log::DEBUG("UniversalUPS::GetDataFromMessage=["+val+"] last_oid="+last_oid);
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://
			//Log::DEBUG("UniversalUPS::GetDataFromMessage=["+val+"] last_oid="+last_oid);
			if(!isNullOrWhiteSpace(val)){
				UE.Iakb.CalcMinAverMax( (atoi(val.c_str())/10.0)*Ktran );
			}
			SubFase = GET_TEMP;
			break;

		case GET_TEMP://
			//Log::DEBUG("UniversalUPS::GetDataFromMessage=["+val+"] last_oid="+last_oid);
			if(!isNullOrWhiteSpace(val)){
				float fv = atoi(val.c_str())/1.0;
				if(fv > 200) fv = 199.0;
				UE.Temp.CalcMinAverMax( fv );
			}
			SubFase = GET_AC1_OUT;
			break;

		case GET_AC1_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ua.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC2_OUT;
			break;

		case GET_AC2_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ub.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC3_OUT;
			break;

		case GET_AC3_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Uc.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_I_IN1;
			break;

		case GET_I_IN1://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ia.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN2;
			break;

		case GET_I_IN2://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ib.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN3;
			break;

		case GET_I_IN3://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ic.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT1;
			break;

		case GET_I_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ia.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT2;
			break;

		case GET_I_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ib.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT3;
			break;

		case GET_I_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ic.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_LOAD_OUT1;
			break;

		case GET_LOAD_OUT1://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent1 = atoi(val.c_str())/1.0;
			}
			SubFase = GET_LOAD_OUT2;
			break;

		case GET_LOAD_OUT2://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent2 = atoi(val.c_str())/1.0;
			}
			SubFase = GET_LOAD_OUT3;
			break;

		case GET_LOAD_OUT3://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent3 = atoi(val.c_str())/1.0;
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://set only global alarm flag
			if(!isNullOrWhiteSpace(val))
			{
				//Alarms[Alarms.size()-1]->Value = atoi(val.c_str()) > 0;
				Alarms[Alarms.size()-1]->SetValue(atoi(val.c_str()) > 0);
			}
			else
			{
				for(auto curr: Alarms.Alarms)
				{
					curr->Value = false;
				}
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.CU_No = val;
			}
			SubFase = GET_SERIALN;
			break;

		case GET_SERIALN://
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SerialNo = val;
			}
			SubFase = GET_AC1_BYP;
			break;


		case GET_AC1_BYP://
			if(!isNullOrWhiteSpace(val)){
				Ubyp.Ua.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC2_BYP;
			break;

		case GET_AC2_BYP://
			if(!isNullOrWhiteSpace(val)){
				Ubyp.Ub.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC3_BYP;
			break;

		case GET_AC3_BYP://
			if(!isNullOrWhiteSpace(val)){
				Ubyp.Uc.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = IBP_SET_CMD;
			break;


	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord UniversalUPS::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool UniversalUPS::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool UniversalUPS::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool UniversalUPS::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	return ret;
}
//*****************************************************************************
//***
//*****************************************************************************
LiebertUPS::LiebertUPS(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap):UniversalUPS(Etype, Ename, adr)
{

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("LiebertUPS::LiebertUPS c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("LiebertUPS::LiebertUPS c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("LiebertUPS::LiebertUPS c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
LiebertUPS::~LiebertUPS()
{

}
//*****************************************************************************
//***
//*****************************************************************************
LiebertAPM::LiebertAPM(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr),Uout(4),Iin(1),Iout(4),LoadPersent(27, 1)
{
	Log::DEBUG("LiebertAPM::LiebertAPM()");

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";


    Alarms.clear();
/*
	for(int i = 0; i < 17; i++){
		Alarms.Alarms.pop_back();
	}*/

}
//=============================================================================
LiebertAPM::LiebertAPM(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap):LiebertAPM(Etype, Ename, adr)
{

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("LiebertAPM::LiebertAPM c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("LiebertAPM::LiebertAPM c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("LiebertAPM::LiebertAPM c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
LiebertAPM::~LiebertAPM()
{

}
//=============================================================================
string LiebertAPM::GetStringValue(void)
{
	//Log::DEBUG("LiebertAPM::GetStringValue");
	string ReturnString = WordToString(Et)+","+ WordToString(En)+","+
							  Uin.GetValueString()+
							  	  Uout.GetValueString()+
								  	Iin.GetValueString()+
								  	  Iout.GetValueString()+
							  	  	  	  UE.GetValueString()+
							  	  	  	  	  LoadPersent.GetValueString();
	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';
	//Log::DEBUG("LiebertAPM::GetStringValue ReturnString="+ReturnString);
	return ReturnString;
}
//=============================================================================
Word LiebertAPM::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://manufacturer model
    	 last_oid = "1.3.6.1.4.1.13400.2.20.1.1.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //V1in
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.1.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //V2in
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.2.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //V3in
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.3.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_UE: //positive battaryVoltage
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.45.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AKB: //positive battaryCurrent
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.46.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_NEG_UE: //negative battaryVoltage
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.47.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_NEG_AKB: //negative battaryCurrent
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.48.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_TEMP: //battaryTemp
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.51.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC1_OUT: //out voltage 1
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.16.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC2_OUT: //out voltage 2
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.17.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC3_OUT: //out voltage 3
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.18.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN1: //input current 1
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.7.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN2: //input current 2
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.8.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN3: //input current 3
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.9.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT1: //out current 1
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.19.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT2: //out current 2
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.20.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT3: //out current 3
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.21.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT1: //load persent 1
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.35.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT2: //load persent 2
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.36.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT3: //load persent 3
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.37.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC1_BYP: //bypass voltage 1
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.41.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC2_BYP: //bypass voltage 2
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.42.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC3_BYP: //bypass voltage 3
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.4.43.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;


     case GET_ALARMS://number of active alarms
    	 last_oid = "1.3.6.1.2.1.33.1.6.1.0";//
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_STAT://status
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_SITE://device model
    	 last_oid = "1.3.6.1.4.1.13400.2.20.1.2.0";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_UNIT_MODULES://numb of modules
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.3.1.0";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

    case GET_UNIT_CAPACITY://catacity
    	 last_oid = "1.3.6.1.4.1.13400.2.20.2.3.2.0";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V2, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte LiebertAPM::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("snmp_driver.GetData=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			SiteInfo.Customer = "NONAME";
			if(!isNullOrWhiteSpace(val)){

				SiteInfo.Customer = replaceAll(val, ",", " ");
			}
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_UE;
			break;

		case GET_UE://positive U akb
			//Log::DEBUG("LiebertAPM::GetDataFromMessage=["+val+"] last_oid="+last_oid);
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://positive Curr akb
			if(!isNullOrWhiteSpace(val)){
				UE.Iakb.CalcMinAverMax( (atoi(val.c_str())/100.0)*Ktran );
			}
			SubFase = GET_NEG_UE;
			break;

		case GET_NEG_UE://neg U akb
			if(!isNullOrWhiteSpace(val)){
				UE.Irect.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_NEG_AKB;
			break;

		case GET_NEG_AKB://neg Curr akb
			//Log::DEBUG("LiebertAPM::GetDataFromMessage=["+val+"] last_oid="+last_oid);
			if(!isNullOrWhiteSpace(val)){
				UE.Iout.CalcMinAverMax( (atoi(val.c_str())/100.0)*Ktran );
			}
			SubFase = GET_TEMP;
			break;


		case GET_TEMP://
			//Log::DEBUG("LiebertAPM::GetDataFromMessage=["+val+"] last_oid="+last_oid);
			if(!isNullOrWhiteSpace(val)){
				float fv = atoi(val.c_str())/100.0;
				Log::DEBUG("LiebertAPM::GET_TEMP=" + toString(fv));

				if(fv > 200) fv = 199.0;
				UE.Temp.CalcMinAverMax( fv );
			}
			SubFase = GET_AC1_OUT;
			break;

		case GET_AC1_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ua.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC2_OUT;
			break;

		case GET_AC2_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Ub.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC3_OUT;
			break;

		case GET_AC3_OUT://
			if(!isNullOrWhiteSpace(val)){
				Uout.Uc.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_IN1;
			break;

		case GET_I_IN1://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ia.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_IN2;
			break;

		case GET_I_IN2://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ib.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_IN3;
			break;

		case GET_I_IN3://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ic.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_OUT1;
			break;

		case GET_I_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ia.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_OUT2;
			break;

		case GET_I_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ib.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_OUT3;
			break;

		case GET_I_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ic.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_LOAD_OUT1;
			break;

		case GET_LOAD_OUT1://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent.Ia.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_LOAD_OUT2;
			break;

		case GET_LOAD_OUT2://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent.Ib.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_LOAD_OUT3;
			break;

		case GET_LOAD_OUT3://
			if(!isNullOrWhiteSpace(val)){
				LoadPersent.Ic.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_STAT;
			break;


		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.CU_No = val;
			}
			SubFase = GET_UNIT_MODULES;
			break;

		case GET_UNIT_MODULES://
			if(!isNullOrWhiteSpace(val)){
				//SiteInfo.ModulesNum = atoi(val.c_str())/100.0;
			}
			SubFase = GET_UNIT_CAPACITY;
			break;

		case GET_UNIT_CAPACITY://
			if(!isNullOrWhiteSpace(val)){
				//SiteInfo.ModulesCap = atoi(val.c_str())/100.0;
			}
			SubFase = GET_AC1_BYP;
			break;

		case GET_AC1_BYP://
			if(!isNullOrWhiteSpace(val)){
				Ubyp.Ua.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC2_BYP;
			break;

		case GET_AC2_BYP://
			if(!isNullOrWhiteSpace(val)){
				Ubyp.Ub.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC3_BYP;
			break;

		case GET_AC3_BYP://
			if(!isNullOrWhiteSpace(val)){
				Ubyp.Uc.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = IBP_SET_CMD;
			break;


	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord LiebertAPM::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool LiebertAPM::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool LiebertAPM::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool LiebertAPM::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	return ret;
}
//*****************************************************************************
//***
//*****************************************************************************
Emerson2::Emerson2(Byte Ename, Byte adr):Iibp(Ename, adr)
{
	/*
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}*/

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

}
//=============================================================================
Emerson2::Emerson2(Byte Ename, Byte adr, string read, string write, string trap):Iibp(Ename, adr)
{
	/*
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}*/

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

}
//=============================================================================
Emerson2::~Emerson2()
{

}
//=============================================================================
Word Emerson2::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://
    	 last_oid = "1.3.6.1.4.1.6302.2.1.1.4.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //
    	 last_oid = "1.3.6.1.4.1.6302.2.1.2.6.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //
    	 last_oid = "1.3.6.1.4.1.6302.2.1.2.6.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //
    	 last_oid = "1.3.6.1.4.1.6302.2.1.2.6.3.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_UE: //
    	 last_oid = "1.3.6.1.4.1.6302.2.1.2.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_LOAD: //
    	 last_oid = "1.3.6.1.4.1.6302.2.1.2.3.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AKB://
    	 last_oid = "1.3.6.1.4.1.6302.2.1.2.5.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT://
    	 last_oid = "1.3.6.1.4.1.6302.2.1.2.10.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://
    	 last_oid = "1.3.6.1.4.1.6302.2.1.4.1.5";
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_STAT://
    	 last_oid = "1.3.6.1.4.1.6302.2.1.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_SITE://
    	 last_oid = "1.3.6.1.4.1.6302.2.1.1.4.0";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V1, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }
	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte Emerson2::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("Emerson2::GetDataFromMessage=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atol(val.c_str())/1000.0);
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atol(val.c_str())/1000.0);
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atol(val.c_str())/1000.0);
			}
			SubFase = GET_UE;
			break;

		case GET_UE://
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atol(val.c_str())/1000.0);
			}
			SubFase = GET_LOAD;
			break;

		case GET_LOAD://
			if(!isNullOrWhiteSpace(val)){
				UE.Iout.CalcMinAverMax( (atol(val.c_str())/1000.0)*Ktran );//
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://
			if(!isNullOrWhiteSpace(val)){
				UE.Iakb.CalcMinAverMax( (atol(val.c_str())/1000.0)*Ktran );
			}
			SubFase = GET_RECT;
			break;

		case GET_RECT://
			if(!isNullOrWhiteSpace(val)){
				//UE.Irect.CalcMinAverMax(atoi(val.c_str())/10.0);
				UE.Irect.CalcMinAverMax(UE.Iakb.Value+UE.Iout.Value);

			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://
			if(!isNullOrWhiteSpace(val)){
				//Alarms[Alarms.size()-1]->Value = true;
				Alarms[Alarms.size()-1]->SetValue(true);
			}
			else{
				for(auto curr: Alarms.Alarms){
					curr->Value = false;
					//curr->SetValue(false);
				}
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				if(st >=4 && st <= 6){
					st=4;

				}else if(st >= 7){
					st-=2;
				}
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			SiteInfo.Customer = "EMERSON";
			SiteInfo.CU_No = "M800D";
			if(Port->PortIsLAN())
				SiteInfo.CU_No += " ETH";

			SubFase = IBP_SET_CMD;
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord Emerson2::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	Log::DEBUG("Emerson2::RecvData RecvLen="+toString(RecvLen));

	return RecvLen;
}
//=============================================================================
bool Emerson2::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool Emerson2::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool Emerson2::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0:
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.5.0";
			 break;
		 case 1:
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.6.0";
			 break;
		 case 2:
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.7.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 3:
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.8.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}
	return ret;
}
//*****************************************************************************
//***
//*****************************************************************************
EnatelSM32::EnatelSM32(Byte Ename, Byte adr):Iibp(Ename, adr)
{
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

}
//=============================================================================
EnatelSM32::EnatelSM32(Byte Ename, Byte adr, string read, string write, string trap):Iibp(Ename, adr)
{
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("EnatelSM32c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("EnatelSM32c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("EnatelSM32 c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
EnatelSM32::~EnatelSM32()
{

}
//=============================================================================
Word EnatelSM32::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;
     //ByteToHex(Tmp2.Data_b, Addr);
	 switch(CMD)
	 {
     case IBP_INIT://
    	 last_oid = "1.3.6.1.4.1.21940.2.5.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //
    	 last_oid = "1.3.6.1.4.1.21940.2.3.1.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //
    	 last_oid = "1.3.6.1.4.1.21940.2.3.1.2.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //
    	 last_oid = "1.3.6.1.4.1.21940.2.3.1.2.3.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_UE: //
    	 last_oid = "1.3.6.1.4.1.21940.2.4.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_LOAD: //
    	 last_oid = "1.3.6.1.4.1.21940.2.4.2.13.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT://
    	 last_oid = "1.3.6.1.4.1.21940.2.4.2.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AKB://
    	 last_oid = "1.3.6.1.4.1.21940.2.4.2.14.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://
    	 last_oid = "1.3.6.1.4.1.21940.2.5.5.1.1.9";
         values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_STAT://
    	 last_oid = "1.3.6.1.4.1.21940.2.5.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_SITE://
    	 last_oid = "1.3.6.1.4.1.21940.1.3.0";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_POINT_STEPS://
    	 //Log::DEBUG("EnatelSM32 CreateCMD, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
    	 switch(PointSteps.CurrentIndex)
    	 {
			 case 0:
				 last_oid = "1.3.6.1.4.1.21940.2.5.4.3.1.0";
				 break;
			 case 1:
				 last_oid = "1.3.6.1.4.1.21940.2.5.4.3.2.0";
				 break;
			 case 2:
				 last_oid = "1.3.6.1.4.1.21940.2.5.4.4.1.0";
				 break;
			 case 3:
				 last_oid = "1.3.6.1.4.1.21940.2.5.4.4.4.0";
				 break;
    	 };
    	 //
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("EnatelSM32::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V1, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }
	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte EnatelSM32::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;
	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("EnatelSM32::GetDataFromMessage=["+val+"] len="+toString(val.size())+" subFase="+toString((int)subFase));
	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atol(val.c_str())/100.0);
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atol(val.c_str())/100.0);
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atol(val.c_str())/100.0);
			}
			SubFase = GET_UE;
			break;

		case GET_UE://
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atol(val.c_str())/1000.0);
			}
			SubFase = GET_LOAD;
			break;

		case GET_LOAD://
			if(!isNullOrWhiteSpace(val)){
				UE.Iout.CalcMinAverMax( (atol(val.c_str())/1000.0)*Ktran );
			}
			SubFase = GET_RECT;
			break;

		case GET_RECT://
			if(!isNullOrWhiteSpace(val)){
				UE.Irect.CalcMinAverMax( (atol(val.c_str())/1000.0)*Ktran );
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://
			if(!isNullOrWhiteSpace(val)){
				signed long v = atoll(val.c_str());
				//Log::DEBUG("GetDataFromMessage, v="+toString(v));
				double ddv = v/1000.0;
				//Log::DEBUG("GetDataFromMessage, dv="+toString(ddv));
				UE.Iakb.CalcMinAverMax(ddv*Ktran);
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://
			Log::DEBUG("netsnmp::Buffer=["+string((char*)BUF, Len)+"]");
			if(!isNullOrWhiteSpace(val)){

				int alarmstatus = atoi(val.c_str());
				//Alarms[Alarms.size()-1]->Value = alarmstatus > 0;
				Alarms[Alarms.size()-1]->SetValue(alarmstatus > 0);
			}
			else{
				for(auto curr: Alarms.Alarms){
					curr->Value = false;
				}
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				switch(st){
					case 0:
						st = 0;
						break;
					case 1://akb test enable
						st = 2;
						break;
					default:
						st = 1;
						break;
				}
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			SiteInfo.Customer = "ENATEL";
			SiteInfo.CU_No = "SM32";
			if(Port->PortIsLAN())
				SiteInfo.CU_No += " ETH";

			SubFase = GET_POINT_STEPS;
			break;

	    case GET_POINT_STEPS://
			if(!isNullOrWhiteSpace(val)){
				long v = atol(val.c_str());
				PointSteps[PointSteps.CurrentIndex]->Value = v/1000.0;
			}
			//Log::DEBUG("GetDataFromMessage, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
	    	if(++PointSteps.CurrentIndex < PointSteps.size()){
	    		SubFase = GET_POINT_STEPS;
	    	}
	    	else{
	    		PointSteps.CurrentIndex = 0;
	    		SubFase = IBP_SET_CMD;
	    	}
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord EnatelSM32::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool EnatelSM32::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool EnatelSM32::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool EnatelSM32::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		data = toString( (long)(atof(data.c_str())*1000) );
		switch(i)
		{
		 case 0://apper AC limit
			 oid = "1.3.6.1.4.1.21940.2.5.4.3.1.0";
			 break;
		 case 1://lower AC limit
			 oid = "1.3.6.1.4.1.21940.2.5.4.3.2.0";
			 break;
		 case 2://apper DC limit
			 oid = "1.3.6.1.4.1.21940.2.5.4.4.1.0";
			 break;
		 case 3://lower DC limit
			 oid = "1.3.6.1.4.1.21940.2.5.4.4.4.0";
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("EnatelSM32::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}
	return ret;
}
//*****************************************************************************
//***
//*****************************************************************************
DeltaPsc3::DeltaPsc3(Byte Ename, Byte adr):Iibp(Ename, adr)
{

	for(int i = 0; i < 10; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}
	//20-17=3
	for(int i = 0; i < 17; i++){
		Alarms.Alarms.pop_back();
	}
    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

}
//=============================================================================
DeltaPsc3::DeltaPsc3(Byte Ename, Byte adr, string read, string write, string trap):Iibp(Ename, adr)
{

	for(int i = 0; i < 10; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}
	//20-17=3
	for(int i = 0; i < 17; i++){
		Alarms.Alarms.pop_back();
	}

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

}
//=============================================================================
DeltaPsc3::~DeltaPsc3()
{

}
//=============================================================================
Word DeltaPsc3::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
       break;


     case GET_AC1: //The configured number of rectifiers plugged into the system
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.4.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //Number of rectifiers that are not properly working
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.4.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //Number of rectifiers that are properly working.
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.4.3.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;


     case GET_UE: //
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.3.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_LOAD: //
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.3.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AKB://
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.3.3.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_TEMP://
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.3.4.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT://
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.3.7.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://
		 switch(Alarms.CurrentIndex)
		 {
				 case 0://Number of currently active urgent alarm sources
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.2.3.0";
					 break;
				 case 1://Number of currently active non urgent alarm sources
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.2.4.0";
					 break;
				 case 2://Number of currently active critical alarm sources
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.2.13.0";
					 break;
		 };
		 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_STAT://
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.3.5.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_POINT_STEPS://
		 //Log::DEBUG("EnatelSM32 CreateCMD, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
		 switch(PointSteps.CurrentIndex)
		 {
				 case 0:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.6.1.0";
					 break;
				 case 1:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.6.2.0";
					 break;
				 case 2:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.6.3.0";
					 break;
				 case 3:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.6.4.0";
					 break;
				 case 4:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.1.1.0";
					 break;
				 case 5:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.1.2.0";
					 break;
				 case 6:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.1.3.0";
					 break;
				 case 7:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.1.4.0";
					 break;
				 case 8:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.1.5.0";
					 break;
				 case 9:
					 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.5.1.6.0";
					 break;

		 };
		 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
        break;

    case GET_SITE://
    	 last_oid = "1.3.6.1.4.1.20246.2.3.1.1.1.2.1.1.0";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V1, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }
	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte DeltaPsc3::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("DeltaPsc3::GetDataFromMessage=["+val+"] len="+toString(val.size())+" subFase="+toString((int)subFase));

	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_AC1;
			break;

		case GET_AC1://The configured number of rectifiers plugged into the system
			if(!isNullOrWhiteSpace(val)){
				//Uin.Ua.CalcMinAverMax(atol(val.c_str())/1000.0);
				RectifiersInfo.All = atoi(val.c_str());
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://Number of rectifiers that are not properly working
			if(!isNullOrWhiteSpace(val)){
				//Uin.Ub.CalcMinAverMax(atol(val.c_str())/1000.0);
				RectifiersInfo.Normal = atoi(val.c_str());
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://Number of rectifiers that are properly working.
			if(!isNullOrWhiteSpace(val)){
				//Uin.Uc.CalcMinAverMax(atol(val.c_str())/1000.0);
				RectifiersInfo.Alarm = atoi(val.c_str());
			}
			SubFase = GET_UE;
			break;

		case GET_UE://
			if(!isNullOrWhiteSpace(val)){
				UE.U.CalcMinAverMax(atol(val.c_str())/100.0);
			}
			SubFase = GET_LOAD;
			break;

		case GET_LOAD://
			if(!isNullOrWhiteSpace(val)){
				UE.Iout.CalcMinAverMax( (atol(val.c_str())/10.0)*Ktran );
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://
			if(!isNullOrWhiteSpace(val)){
				UE.Iakb.CalcMinAverMax( (atol(val.c_str())/10.0)*Ktran );
			}
			SubFase = GET_TEMP;
			break;

		case GET_TEMP://
			if(!isNullOrWhiteSpace(val)){
				UE.Temp.CalcMinAverMax(atol(val.c_str())/10.0);
			}
			SubFase = GET_RECT;
			break;

		case GET_RECT://
			if(!isNullOrWhiteSpace(val)){
				//UE.Irect.CalcMinAverMax(atoi(val.c_str())/10.0);
				UE.Irect.CalcMinAverMax( (atol(val.c_str())/10.0)*Ktran );

			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://

			if(!isNullOrWhiteSpace(val)){
				long v = atol(val.c_str());
				//Alarms[Alarms.CurrentIndex]->Value = v > 0;
				Alarms[Alarms.CurrentIndex]->SetValue(v > 0);
			}
			//Log::DEBUG("GetDataFromMessage, Alarms.CurrentIndex="+toString(Alarms.CurrentIndex)+" val=["+val+"]" );
	    	if(++Alarms.CurrentIndex < Alarms.size()){
	    		SubFase = GET_ALARMS;
	    	}
	    	else{
	    		Alarms.CurrentIndex = 0;
	    		SubFase = GET_STAT;
	    	}
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val)){
				Word st = atol(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			SiteInfo.Customer = "DELTA";
			SiteInfo.CU_No = "PSC3";
			if(Port->PortIsLAN())
				SiteInfo.CU_No += " ETH";

			if(!isNullOrWhiteSpace(val)){
				SiteInfo.Location = trimAllNull(val);
				SiteInfo.Location 	= replaceAll(SiteInfo.Location, ",", " ");
			}
			SubFase = GET_POINT_STEPS;
			break;

	    case GET_POINT_STEPS://
			if(!isNullOrWhiteSpace(val)){
				long v = atol(val.c_str());
				PointSteps[PointSteps.CurrentIndex]->Value = v/100.0;
			}
			//Log::DEBUG("GetDataFromMessage, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
	    	if(++PointSteps.CurrentIndex < PointSteps.size()){
	    		SubFase = GET_POINT_STEPS;
	    	}
	    	else{
	    		PointSteps.CurrentIndex = 0;
	    		SubFase = IBP_SET_CMD;
	    	}
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord DeltaPsc3::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool DeltaPsc3::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool DeltaPsc3::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool DeltaPsc3::ChangeParameters( vector<string> parameters )
{
	bool ret = false;
	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0:
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.5.0";
			 break;
		 case 1:
			 oid = "1.3.6.1.4.1.2011.6.164.1.5.1.6.0";
			 break;
		 case 2:
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.7.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 3:
			 oid = "1.3.6.1.4.1.2011.6.164.1.6.1.8.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}
	return ret;
}
//=============================================================================
string DeltaPsc3::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							  //Uin.GetValueString()+
								UE.GetValueString();
						         //"\r\n>,99,1,"+ WordToString((int)Type)+",";
			if(Port->PortIsLAN())
			{
				ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
				ReturnString += Port->ToString()+',';
			}
		  return ReturnString;
}
//*****************************************************************************
//***
//*****************************************************************************
DeltaPsc1000::DeltaPsc1000(Byte Ename, Byte adr):Iibp(Ename, adr)
{

	for(int i =0; i<6; i++)
	{
		PointSteps.Params.push_back(new THistFloatParam());
	}
}
//=============================================================================
DeltaPsc1000::~DeltaPsc1000()
{

}
//=============================================================================
Word DeltaPsc1000::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Bshort       Tmp;
	 Bshort       Tmp2;

     //ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://009CNFIG
         DataLen = 0;
         Buffer[DataLen++] = STC;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x39;
         Buffer[DataLen++] = 0x43;
         Buffer[DataLen++] = 0x4E;//
         Buffer[DataLen++] = 0x46;//
         Buffer[DataLen++] = 0x49;//
         Buffer[DataLen++] = 0x47;//
         Tmp2.Data_s = Crc16ccitt0(Buffer, DataLen);
         WordToHex(Tmp.Data_b, Tmp2.Data_s);
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
       break;

     case GET_UE://009STATE
         DataLen = 0;
         Buffer[DataLen++] = STC;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x39;
         Buffer[DataLen++] = 0x53;
         Buffer[DataLen++] = 0x54;//
         Buffer[DataLen++] = 0x41;//
         Buffer[DataLen++] = 0x54;//
         Buffer[DataLen++] = 0x45;//
         Tmp2.Data_s = Crc16ccitt0(Buffer, DataLen);
         WordToHex(Tmp.Data_b, Tmp2.Data_s);
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         break;

     case GET_ALARMS://009ALARM
         DataLen = 0;
         Buffer[DataLen++] = STC;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x39;
         Buffer[DataLen++] = 0x41;
         Buffer[DataLen++] = 0x4C;//
         Buffer[DataLen++] = 0x41;//
         Buffer[DataLen++] = 0x52;//
         Buffer[DataLen++] = 0x4D;//
         Tmp2.Data_s = Crc16ccitt0(Buffer, DataLen);
         WordToHex(Tmp.Data_b, Tmp2.Data_s);
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         break;

     case GET_SYS://009SETUP
         DataLen = 0;
         Buffer[DataLen++] = STC;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x39;
         Buffer[DataLen++] = 0x53;
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x54;//
         Buffer[DataLen++] = 0x55;//
         Buffer[DataLen++] = 0x50;//
         Tmp2.Data_s = Crc16ccitt0(Buffer, DataLen);
         WordToHex(Tmp.Data_b, Tmp2.Data_s);
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         break;

     case GET_POINT_STEPS://
         DataLen = 0;
         Buffer[DataLen++] = STC;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x31;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x53;
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x54;//
         Buffer[DataLen++] = 0x55;//
         Buffer[DataLen++] = 0x50;//
         Buffer[DataLen++] = 0x4E;//
         Tmp2.Data_s = Crc16ccitt0(Buffer, DataLen);
         WordToHex(Tmp.Data_b, Tmp2.Data_s);
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         break;


    case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				Log::DEBUG("DeltaPsc1000::CreateCMD IBP_SET_CMD=["+mess+"]");
				DataLen = 0;
				for(int i = 0; i < mess.size(); i++)
				{
					Buffer[DataLen++] = mess.c_str()[i];
				}
			}
   	 break;

     default:
       DataLen = 0;
       break;
	 }
	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte DeltaPsc1000::GetDataFromMessage(Byte subFase, Byte *BUF, Word Len)
{
	Byte SubFase = subFase;
	sBshort Tmp;
	BUF[Len] = 0;
	switch(subFase)
	{
		case IBP_INIT:
			SiteInfo.Customer = "DELTA";
			SiteInfo.CU_No = "PSC1000";
			if(Len > 16)
			{
				string PSC = GetParamValue("PSC ", BUF, Len);
				string VER = GetParamValue("VER ", BUF, Len);
				string SITE = GetParamValue("CNFIGSIN ", BUF, Len);

				if(!isNullOrWhiteSpace(PSC)){
					SiteInfo.CU_No = "PSC"+trimAllNull(PSC);
					SiteInfo.CU_No 	= replaceAll(SiteInfo.CU_No, ",", " ");
				}
				if(!isNullOrWhiteSpace(VER)){
					SiteInfo.SwVers = trimAllNull(VER);
					SiteInfo.SwVers 	= replaceAll(SiteInfo.SwVers, ",", " ");
				}
				if(!isNullOrWhiteSpace(SITE)){
					SiteInfo.Location = trimAllNull(SITE);
					SiteInfo.Location 	= replaceAll(SiteInfo.Location, ",", " ");
				}
			}
			if(Port->PortIsLAN())
				SiteInfo.CU_No += " ETH";

			SubFase = GET_UE;
			break;

		case GET_UE:
			if(Len > 16)
			{
				string UL0 = GetParamValue("UL00 ", BUF, Len);
				string IEB = GetParamValue("IEB ", BUF, Len);
				string IEL = GetParamValue("IEL ", BUF, Len);
				string PEL = GetParamValue("PEL ", BUF, Len);
				string IER = GetParamValue("IER ", BUF, Len);
				string TMP0 = GetParamValue("TMP0 ", BUF, Len);
				string SC  = GetParamValue("SC ", BUF, Len);
				//Log::DEBUG("UL0=["+UL0+"]");
				//Log::DEBUG("IEB=["+IEB+"]");
				//Log::DEBUG("IEL=["+IEL+"]");
				//Log::DEBUG("PEL=["+PEL+"]");
				//Log::DEBUG("IER=["+IER+"]");
				//Log::DEBUG("TMP0=["+TMP0+"]");
				//Log::DEBUG("SC=["+SC+"]");

				if(!isNullOrWhiteSpace(UL0)){
					UE.U.CalcMinAverMax(atof(UL0.c_str()));
				}
				if(!isNullOrWhiteSpace(IEB)){
					UE.Iakb.CalcMinAverMax(atof(IEB.c_str())*Ktran);
				}
				if(!isNullOrWhiteSpace(IEL)){
					UE.Iout.CalcMinAverMax(atof(IEL.c_str())*Ktran);
				}
				if(!isNullOrWhiteSpace(PEL)){
					//UE..CalcMinAverMax(atof(PEL.c_str()));
				}
				if(!isNullOrWhiteSpace(IER)){
					UE.Irect.CalcMinAverMax(atof(IER.c_str())*Ktran);
				}
				if(!isNullOrWhiteSpace(TMP0)){
					UE.Temp.CalcMinAverMax(atof(TMP0.c_str()));
				}
				if(!isNullOrWhiteSpace(SC))
				{ // (0 = Float, 1 = TcFloat, 2..4 = Boost charge, 5 = Equalize, 6..7 = Battery test, 8 = Battery check,
					//9 = Discharge, 10 = External voltage request, 11 = Separate charging, 12 = Charge, 13 = TcCharge, 14 = Disconnected)
					UE.State = atoi(SC.c_str());
				}
				//
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://
			if(Len > 16)
			{
				string AL = GetParamValue("AL ", BUF, Len);
				Log::DEBUG("AL=["+AL+"]");
				if(!isNullOrWhiteSpace(AL))
				{
					for(int i=0; i < AL.size(); i++)
					{
						string vv = string(&AL.c_str()[i], 1);
						if(!isNullOrWhiteSpace(vv))
						{
							//Alarms[i]->Value = (bool)atoi(vv.c_str());
							Alarms[i]->SetValue( (bool)atoi(vv.c_str()) );
						}
					}

					vector<string> alarms = GetAlarmList(AL, ";SRC  ", BUF, Len);
					/*for(auto curr: alarms)
					{
						Log::DEBUG("ALarm=["+curr+"]");
					}*/
					Alarms.TextAlarms = alarms;
				}

			}
			SubFase = GET_SYS;
			break;

		case GET_SYS://
			if(Len > 16)
			{
				string Uno = GetParamValue("Uno ", BUF, Len);
				string Urm = GetParamValue("Urm ", BUF, Len);
				string Isl = GetParamValue("Isl ", BUF, Len);
				string BRT = GetParamValue("BRT ", BUF, Len);

				if(!isNullOrWhiteSpace(Uno)){
					PointSteps[0]->Value = atof(Uno.c_str());
				}
				if(!isNullOrWhiteSpace(Urm)){
					PointSteps[1]->Value = atof(Urm.c_str());
				}
				//Log::DEBUG("Uno=["+Uno+"]");
				//Log::DEBUG("Urm=["+Urm+"]");
				//Log::DEBUG("Isl=["+Isl+"]");
				//Log::DEBUG("BRT=["+BRT+"]");
			}
			SubFase = GET_POINT_STEPS;
			break;


		case GET_POINT_STEPS:
			if(Len > 16)
			{
				string Uah = GetParamValue("Uah ", BUF, Len);
				string Ual = GetParamValue("Ual ", BUF, Len);
				string Ush = GetParamValue("Ush ", BUF, Len);
				string Usl = GetParamValue("Usl ", BUF, Len);

				//Log::DEBUG("Uah"+Uah+"]");
				//Log::DEBUG("Ual=["+Ual+"]");
				//Log::DEBUG("Ush=["+Ush+"]");
				//Log::DEBUG("Usl=["+Usl+"]");

				if(!isNullOrWhiteSpace(Uah)){
					PointSteps[2]->Value = atof(Uah.c_str());
				}
				if(!isNullOrWhiteSpace(Ual)){
					PointSteps[3]->Value = atof(Ual.c_str());
				}
				if(!isNullOrWhiteSpace(Ush)){
					PointSteps[4]->Value = atof(Ush.c_str());
				}
				if(!isNullOrWhiteSpace(Usl)){
					PointSteps[5]->Value = atof(Usl.c_str());
				}

			}
			SubFase = IBP_SET_CMD;
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord  DeltaPsc1000::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;
	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool DeltaPsc1000::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{

	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 13)
		return false;

	int start = TBuffer::find_first_of(BUF, Len, STD);
	if(start < 0) return false;
	if(start + 3 >= Len) return false;
	string lenstr = string((char*)&BUF[start+1], 3);
	//Log::DEBUG("DeltaPsc1000::ParsingAnswer lenstr="+lenstr);

	Word len = atoi(lenstr.c_str());
	Word fullLen = (len+3+1);

	if(Len < fullLen)return false;

	string crcstr = string((char*)&BUF[start+1+3+len-4], 4);
	Short RecvCrc = HexToWord(crcstr);
	//Log::DEBUG("DeltaPsc1000::ParsingAnswer RecvCrc="+toString(RecvCrc));

	Short CalcCrc = Crc16ccitt0(&BUF[start], len);
	//Log::DEBUG("ParsingAnswer RecvCrc="+toString(RecvCrc) + " CalcCrc="+toString(CalcCrc)+ " Len="+toString(len));
	if(RecvCrc!=CalcCrc) return false;
	if(start > 0)
	{
		for(int i=0;i<fullLen;i++)
		{
			BUF[i] = BUF[i+start];
		}
		Len = fullLen;
	}
	//Log::DEBUG("DeltaPsc1000::ParsingAnswer mess=["+string((char*)BUF, Len)+"]");
	return true;
}
//=============================================================================
string  DeltaPsc1000::GetParamValue(string parname, Byte *BUF, Word Len )
{
	string ret = "";
	int st = TBuffer::find(BUF, Len, parname);
	if(st >= 0)
	{
		int et = TBuffer::find(&BUF[st+parname.length()], Len, ";");
		if(et > 0)
		{
			string val = string((char*)&BUF[st+parname.length()], et);
			bool err = false;
			for(int i=0;i<val.length();i++)
			{
				if(val.c_str()[i] < 0x20 || val.c_str()[i] > 0x39)
				{
					err = true;
					break;
				}
			}
			if(err)
				val = "";
			ret = val;
		}
	}
	return ret;
}
//=============================================================================
vector<string>  DeltaPsc1000::GetAlarmList(string start, string param, Byte *BUF, Word Len )
{
	vector<string> ret;

	int st = TBuffer::find(BUF, Len, start);
	if(st >=0)
	{
		Word index = st+start.length();
		string mess = string((char*)&BUF[index], Len-index);
		ret = TBuffer::Split(mess, param);

		for(int i=0; i < ret.size();i++)
		{
			string str = ret[i];
			int in = TBuffer::find_first_of((Byte*)str.c_str(), str.length(), ';');
			if(in < 0) continue;
			//Log::DEBUG("DeltaPsc1000::GetAlarmList in="+toString(in)+ " str="+str);
			ret[i] = str.substr(0, in);
		}
	}

	return ret;
}
//=============================================================================
string DeltaPsc1000::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							  //Uin.GetValueString()+
								UE.GetValueString();
						         //"\r\n>,99,1,"+ WordToString((int)Type)+",";
	return ReturnString;
}
//=============================================================================
bool DeltaPsc1000::ChangeParameters( vector<string> parameters )
{
	bool ret = false;

	CmdMessages.push_back("!015PSSWD224113FE18");

	if(parameters.size() >= 2)
	{

		if(!isNullOrWhiteSpace(parameters[0]) && !isNullOrWhiteSpace(parameters[1]) && parameters[0].size() == 4 && parameters[1].size() == 4)
		{
			string cmd = ":027SETUPUno "+parameters[0]+";Urm "+parameters[1]+";";
			Bshort       Tmp2;
			Bshort       Tmp;
	        Tmp2.Data_s = Crc16ccitt0((Byte*)cmd.c_str(), cmd.length());
	        WordToHex(Tmp.Data_b, Tmp2.Data_s);
	        cmd += string((char*)Tmp.Data_b, 4);

			Log::DEBUG("DeltaPsc1000::ChangeParameters cmd=["+cmd+"]");
			CmdMessages.push_back(cmd);
			ret = true;
		}
	}
	if(parameters.size() >= 6)
	{

		if(!isNullOrWhiteSpace(parameters[2]) && !isNullOrWhiteSpace(parameters[3]) && !isNullOrWhiteSpace(parameters[4]) && !isNullOrWhiteSpace(parameters[5]) &&
				parameters[2].size() == 4 && parameters[3].size() == 4 && parameters[4].size() == 4 && parameters[5].size() == 4)
		{
			string cmd = ":045SETUPUah "+parameters[2]+";Ual "+parameters[3]+";Ush "+parameters[4]+";Usl "+parameters[4]+";";
			Bshort       Tmp2;
			Bshort       Tmp;
	        Tmp2.Data_s = Crc16ccitt0((Byte*)cmd.c_str(), cmd.length());
	        WordToHex(Tmp.Data_b, Tmp2.Data_s);
	        cmd += string((char*)Tmp.Data_b, 4);

			Log::DEBUG("DeltaPsc1000::ChangeParameters cmd=["+cmd+"]");
			CmdMessages.push_back(cmd);
			ret = true;
		}
	}
	/*
	for(int i = 0; i < parameters.size(); i++)
	{
		string cmd = "";
		string data = parameters[i];
		switch(i)
		{
		 case 0:

			 break;
		 case 1:

			 break;
		 case 2:

			 break;
		 case 3:

			 break;
		};
		if(!isNullOrWhiteSpace(data))
		{
			Log::DEBUG("DeltaPsc1000::ChangeParameters cmd=["+cmd+"]");
			CmdMessages.push_back(cmd);
			ret = true;
		}
	}*/


	return ret;
}

//*****************************************************************************
//***
//*****************************************************************************
LiFeAkb::LiFeAkb(Byte Ename, Byte adr):Iibp(Ename, adr)
{
	for(int i =0; i < 2; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}

	ProtoVer.Data_b[0] = 0x31;
	ProtoVer.Data_b[1] = 0x31;
	Time = 0;
}
//=============================================================================
LiFeAkb::~LiFeAkb()
{

}
//=============================================================================
Word LiFeAkb::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;

     ByteToHex(Tmp2.Data_b, Addr);

	 switch(CMD)
	 {
     case IBP_INIT://D0 82 Obtain analog data
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x44;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x38;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x31;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
       break;

     case GET_AC1: //D0 82 Obtain analog data
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x44;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x38;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x31;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_ALARMS://D0 83
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x44;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x38;//
         Buffer[DataLen++] = 0x33;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x31;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

     case GET_ALARMS1://D0 42
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x44;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x31;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

    case GET_STAT://D0 44
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x44;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x31;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

   //params
    case GET_SITE://D0 84
         DataLen = 0;
         Buffer[DataLen++] = SOI;
         Buffer[DataLen++] = ProtoVer.Data_b[0];
         Buffer[DataLen++] = ProtoVer.Data_b[1];
         Buffer[DataLen++] = Tmp2.Data_b[0];
         Buffer[DataLen++] = Tmp2.Data_b[1];
         Buffer[DataLen++] = 0x44;
         Buffer[DataLen++] = 0x30;
         Buffer[DataLen++] = 0x38;//
         Buffer[DataLen++] = 0x34;//
         Buffer[DataLen++] = 0x45;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x32;//
         Buffer[DataLen++] = 0x30;//
         Buffer[DataLen++] = 0x31;//
         WordToHex(Tmp.Data_b, CHKSUM(Buffer, DataLen));
         Buffer[DataLen++] = Tmp.Data_b[0];
         Buffer[DataLen++] = Tmp.Data_b[1];
         Buffer[DataLen++] = Tmp.Data_b[2];
         Buffer[DataLen++] = Tmp.Data_b[3];
         Buffer[DataLen++] = EOI;
         break;

    case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
			}
   	 break;

     default:
       DataLen = 0;
       break;
	 }
	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte LiFeAkb::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;
	sBshort Tmp;
	BUF[Len] = 0;
	Byte buf[9];

	//BUF +=7;//replase pointer
	switch(subFase)
	{
		case IBP_INIT:
			if(Len >= 18)
			{
				ProtoVer.Data_b[0] = BUF[1];
				ProtoVer.Data_b[1] = BUF[2];
			}
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(Len >= 53)
			{
				buf[0] = BUF[13];
				buf[1] = BUF[14];
				buf[2] = BUF[15];
				buf[3] = BUF[16];
				buf[4] = 0;
				Data.Ucmax.CalcMinAverMax(HexToWord(buf)/1000.0);

				buf[0] = BUF[17];
				buf[1] = BUF[18];
				buf[2] = BUF[19];
				buf[3] = BUF[20];
				buf[4] = 0;
				Data.Ucmin.CalcMinAverMax(HexToWord(buf)/1000.0);

				buf[0] = BUF[21];
				buf[1] = BUF[22];
				buf[2] = 0;
				Data.Tmin.CalcMinAverMax(HexToByte(buf)/1.0);

				buf[0] = BUF[23];
				buf[1] = BUF[24];
				buf[2] = 0;
				Data.Tmax.CalcMinAverMax(HexToByte(buf)/1.0);

				buf[0] = BUF[25];
				buf[1] = BUF[26];
				buf[2] = BUF[27];
				buf[3] = BUF[28];
				buf[4] = 0;
				Data.Utot.CalcMinAverMax(HexToWord(buf)/100.0);

				buf[0] = BUF[29];
				buf[1] = BUF[30];
				buf[2] = BUF[31];
				buf[3] = BUF[32];
				buf[4] = 0;
				Data.Ich.CalcMinAverMax(HexToWord(buf)/100.0);

				buf[0] = BUF[33];
				buf[1] = BUF[34];
				buf[2] = BUF[35];
				buf[3] = BUF[36];
				buf[4] = 0;
				Data.Idch.CalcMinAverMax(HexToWord(buf)/100.0);

				buf[0] = BUF[37];
				buf[1] = BUF[38];
				buf[2] = 0;
				Data.Soc.CalcMinAverMax(HexToByte(buf)/1.0);

				buf[0] = BUF[39];
				buf[1] = BUF[40];
				buf[2] = BUF[41];
				buf[3] = BUF[42];
				buf[4] = 0;
				PointSteps[0]->Value = HexToWord(buf)/100.0;

				buf[0] = BUF[43];
				buf[1] = BUF[44];
				buf[2] = 0;
				Time = HexToWord(buf)/10.0;

				buf[0] = BUF[45];
				buf[1] = BUF[46];
				buf[2] = BUF[47];
				buf[3] = BUF[48];
				buf[4] = 0;
				PointSteps[1]->Value = HexToWord(buf)/100.0;

				buf[0] = BUF[49];
				buf[1] = BUF[50];
				buf[2] = 0;
				Data.Soh.CalcMinAverMax(HexToByte(buf)/1.0);

				buf[0] = BUF[51];
				buf[1] = BUF[52];
				buf[2] = 0;
				SiteInfo.SerialNo = toString((int)HexToByte(buf));
				SiteInfo.SerialNo 	= replaceAll(SiteInfo.SerialNo, ",", " ");

				SiteInfo.Customer = "COSLIGHT";
				SiteInfo.BattType = "GYFP4875";
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS:
			if(Len > 31)
			{

			}
			SubFase = GET_ALARMS1;
			break;

		case GET_ALARMS1:
			if(Len > 31)
			{

			}
			SubFase = GET_STAT;
			break;

		case GET_STAT:
			if(Len > 14)
			{

			}
			SubFase = GET_SITE;
			//Log::DEBUG("SubFase=GET_STAT");
			break;

		case GET_SITE:///40 51
			if(Len > 40)
			{

			}
			SubFase = IBP_SET_CMD;
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord  LiFeAkb::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;
	//sWord bytes = 0;

	RecvLen = port->RecvTo( Buf, MaxLen, EOI );//port->Recv( Buf, MaxLen ); //
	return RecvLen;
}
//=============================================================================
bool LiFeAkb::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	Blong       Tmp;
	Byte Data_b[5];

	Tmp.Data_b[0] = BUF[Len-5];
	Tmp.Data_b[1] = BUF[Len-4];
	Tmp.Data_b[2] = BUF[Len-3];
	Tmp.Data_b[3] = BUF[Len-2];

	Short RecvCrc = HexToWord(Tmp.Data_b);
	Short CalcCrc  = CHKSUM(BUF, Len-5);
	//Log::DEBUG("ParsingAnswer RecvCrc="+toString(RecvCrc) + " CalcCrc="+toString(CalcCrc)+ " Len="+toString(Len));
	return RecvCrc == CalcCrc;
}
//=============================================================================
string LiFeAkb::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
							Data.GetValueString()+
							"\r\n>,30,1,"+FloatToString(Time, 0, 2)+',';


		  return ReturnString;
}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//***
//*****************************************************************************
CondLennox::CondLennox(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr)
{
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}

    c_read  = "public";
    c_write = "carel";
    c_trap  = "trap";

}
//=============================================================================
CondLennox::CondLennox(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, Byte type):Iibp(Etype, Ename, adr)
{
	for(int i = 0; i < 6; i++){//6
		PointSteps.Params.push_back(new THistFloatParam());
	}
	//first 20 in Iibp::Iibp(
	for(int i = 0; i < 21; i++){//21
		Alarms.Alarms.push_back(new THistBoolParam());
	}


	if(type == 102)
	{
		PointSteps.Params.push_back(new THistFloatParam());//7
		/*
		for(int i = 41; i < 50; i++){
			Alarms.Alarms.push_back(new THistBoolParam());
		}*/
		Log::DEBUG("CondLennox type="+toString((int)type));
	}


	//start Alarms.index = 28
	Log::DEBUG("CondLennox Alarms.size="+toString(Alarms.size()));

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "carel": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("CondLennox c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("CondLennox c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("CondLennox c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
CondLennox::~CondLennox()
{

}
//=============================================================================
Word CondLennox::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

	 switch(CMD)
	 {
     case IBP_INIT://
    	 last_oid = "1.3.6.1.4.1.9839.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //Humidity
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //Room temp
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.4.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //supply temp (air outlet temp)
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.5.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_UE: //outside temp
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.6.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_STAT://status
    	 last_oid = "1.3.6.1.4.1.9839.2.1.3.12.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

   case GET_SITE://unit type
	     last_oid = "1.3.6.1.4.1.9839.2.1.3.11.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://
    	 //last_oid = "1.3.6.1.4.1.2011.6.164.1.1.2.99.1.1";
         //values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V1, c_read, new Oid(last_oid));
    	 last_oid = "1.3.6.1.4.1.9839.2.1.1."+toString(Alarms.CurrentIndex+28)+".0";
    	 //Log::DEBUG("alarms oid="+last_oid);
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_POINT_STEPS://
    	 //Log::DEBUG("CreateCMD, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
    	 switch(PointSteps.CurrentIndex)
    	 {
			 case 0://temp set point
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.10.0";
				 break;
			 case 1://min temp set point limit
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.11.0";
				 break;
			 case 2://max temp set point limit
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.12.0";
				 break;
			 case 3://humid set point
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.13.0";
				 break;
			 case 4://min humid set point limit
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.14.0";
				 break;
			 case 5://max humid set point limit
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.15.0";
				 break;
			 case 6://water recovery
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.9.0";// for XM
				 break;
    	 };
    	 //
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V1, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte CondLennox::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("CondLennox::GetDataFromMessage=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_AC1;
			break;

		case GET_AC1://Humidity
			if(!isNullOrWhiteSpace(val)){
				Humidity.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://Room temp
			if(!isNullOrWhiteSpace(val)){
				RoomTemp.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://supply temp (air outlet temp)
			if(!isNullOrWhiteSpace(val)){
				RoomTempOut.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_UE;
			break;

		case GET_UE://outside temp
			if(!isNullOrWhiteSpace(val)){
				ExtTemp.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val))
			{
				Word st = atoi(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			if(!isNullOrWhiteSpace(val))
			{
				SiteInfo.Customer = "LENNOX";
				SiteInfo.CU_No = "UNKN";
				Word st = atoi(val.c_str());

				switch(st)
				{
				case 0:
					SiteInfo.CU_No = "DX";
					break;
				case 1:
					SiteInfo.CU_No = "CW";
					break;
				case 2:
					SiteInfo.CU_No = "DUAL COOLING";
					break;
				};

			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://
			if(!isNullOrWhiteSpace(val))
			{
				//Alarms[Alarms.CurrentIndex]->Value = atoi(val.c_str());
				Alarms[Alarms.CurrentIndex]->SetValue(atoi(val.c_str()));
			}
	    	if(++Alarms.CurrentIndex < Alarms.size())
	    	{
	    		SubFase = GET_ALARMS;
	    	}
	    	else
	    	{
	    		Alarms.CurrentIndex = 0;
	    		SubFase = GET_POINT_STEPS;
	    	}
			break;


	    case GET_POINT_STEPS://
			if(!isNullOrWhiteSpace(val))
			{
				float v = atof(val.c_str())/10.0;
				PointSteps[PointSteps.CurrentIndex]->Value = v;
			}
	    	if(++PointSteps.CurrentIndex < PointSteps.size())
	    	{
	    		SubFase = GET_POINT_STEPS;
	    	}
	    	else
	    	{
	    		PointSteps.CurrentIndex = 0;
	    		SubFase = IBP_SET_CMD;
	    	}
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord CondLennox::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool CondLennox::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool CondLennox::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool CondLennox::ChangeParameters( vector<string> parameters )
{
	bool ret = false;

	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0://temp set point
			 oid = "1.3.6.1.4.1.9839.2.1.2.10.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 1://min temp set point limit
			 oid = "1.3.6.1.4.1.9839.2.1.2.11.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 2://max temp set point limit
			 oid = "1.3.6.1.4.1.9839.2.1.2.12.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 3://humid set point
			 oid = "1.3.6.1.4.1.9839.2.1.2.13.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 4://min humid set point limit
			 oid = "1.3.6.1.4.1.9839.2.1.2.14.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 5://max humid set point limit
			 oid = "1.3.6.1.4.1.9839.2.1.2.15.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}

	return ret;
}
//=============================================================================
string CondLennox::GetStringValue(void)
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

	ReturnString += "\r\n>,"+WordToString(76)+','+WordToString(1)+',';
	ReturnString += FloatToString(AirPressure.Aver, 0, 2)+','+FloatToString(AirPressure.Min, 0, 2)+','+ FloatToString(AirPressure.Max, 0, 2)+',';
	AirPressure.ClrDivider();


	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';

	/*
	ReturnString +=Alarms.GetValueString();

	ReturnString += "\r\n>,201,1,"+ WordToString(UE.State)+',';


	ReturnString += PointSteps.GetValueString();


	string snumb = "000000";
	if(!IsEnable && SiteInfo.SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";

    ReturnString += WordToString(IsEnable)+',';
    */

	return ReturnString;
}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//***
//*****************************************************************************
CondLiebert::CondLiebert(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr)
{
	for(int i = 0; i < 4; i++){
		PointSteps.Params.push_back(new THistFloatParam());
	}

    c_read  = "public";
    c_write = "carel";
    c_trap  = "trap";

}
//=============================================================================
CondLiebert::CondLiebert(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, Byte type):Iibp(Etype, Ename, adr)
{
	for(int i = 0; i < 6; i++){//6
		PointSteps.Params.push_back(new THistFloatParam());
	}
	//first 20 in Iibp::Iibp(
	for(int i = 0; i < 17; i++){//
		Alarms.Alarms.push_back(new THistBoolParam());
	}

	Log::DEBUG("CondLiebert Alarms.size="+toString(Alarms.size()));

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "carel": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("CondLiebert c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("CondLiebert c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("CondLiebert c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
CondLiebert::~CondLiebert()
{

}
//=============================================================================
Word CondLiebert::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

	 switch(CMD)
	 {
     case IBP_INIT://
    	 last_oid = "1.3.6.1.4.1.476.1.42.2.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //Supply Humidity
    	 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5027";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_SYS: //Return Humidity
    	 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5028";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //AirTempSupply
    	 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5002";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //AirTempReturn
    	 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.4291";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_UE: //fan speed
    	 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5077";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_STAT://system status
    	 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.4123";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

   case GET_SITE://unit type
	     last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.4240";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

   case GET_SERIALN://ser n
	     last_oid = "1.3.6.1.4.1.476.1.42.2.1.4.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://
    	 switch(Alarms.CurrentIndex)
    	 {
			 case 0://Customer input 1
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4270";
				 break;
			 case 1://Customer input 2
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4271";
				 break;
			 case 2://Customer input 3
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4272";
				 break;
			 case 3://Customer input 4
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4273";
				 break;
			 case 4://smoke detected
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4720";
				 break;
			 case 5://water under floor
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4723";
				 break;
			 case 6://service required
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4726";
				 break;

			 case 7://loss of power
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4714";
				 break;
			 case 8://Ext over temp
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5104";
				 break;
			 case 9://ext loss of flow
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5105";
				 break;
			 case 10://ext condenser pump hi water
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5106";
				 break;
			 case 11://ext fire detect
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5108";
				 break;
			 case 12://unit on
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5109";
				 break;
			 case 13://unit off
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5110";
				 break;
			 case 14://unit standby
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5111";
				 break;
			 case 15://unit part shut down
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5112";
				 break;
			 case 16://unit shut down
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5113";
				 break;
			 case 17://water leakage
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5114";
				 break;
			 case 18://BMS timeout
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5115";
				 break;
			 case 19://maintenance due
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5116";
				 break;
			 case 20://maintenance completed
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5117";
				 break;
			 case 21://clogged air filter
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5118";
				 break;
			 case 22://ram battery issue
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5119";
				 break;
			 case 23://low memory
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5119";//not find
				 break;
			 case 24://master unit comm lost
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5120";
				 break;
			 case 25://hi power shutdown
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5121";
				 break;
			 case 26://supply fluid temp sensor
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.4651";
				 break;
			 case 27://unspec general event
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5588";
				 break;
			 case 28://sec unspec general event
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.6232";
				 break;
			 case 29://sec comm lost
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.6233";
				 break;
			 case 30://static pressue sensor
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5629";
				 break;
			 case 31://static pressue lost
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5910";
				 break;
			 case 32://power source A
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.6236";
				 break;
			 case 33://power source B
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.6237";
				 break;
			 case 34://fluid flow sensor
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5912";
				 break;
			 case 35://fluid temp sensor
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5911";
				 break;
			 case 36://unit code missing
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.100.5418";
				 break;
    	 };
    	 //Log::DEBUG("alarms oid="+last_oid);
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_POINT_STEPS://
    	 //Log::DEBUG("CreateCMD, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
    	 switch(PointSteps.CurrentIndex)
    	 {
			 case 0://temp set point
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5008";
				 break;
			 case 1://hi supply air temp threshold
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5014";
				 break;
			 case 2://lo supply air temp threshold
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5018";
				 break;
			 case 3://hi return air temp threshold
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5022";
				 break;
			 case 4://humid set point
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5029";
				 break;
			 case 5://hi return humid threshold
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5033";
				 break;
			 case 6://hi return humid threshold
				 last_oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5035";
				 break;
    	 };
    	 //
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("CondLiebert::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V1, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte CondLiebert::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("CondLiebert::GetData=["+val+"] last_oid="+toString(last_oid)+ " subFase="+toString((int)subFase));

	switch(subFase)
	{
		case IBP_INIT://
			SiteInfo.Customer = "NONAME";
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.Customer = val;
			}
			SubFase = GET_AC1;
			break;

		case GET_AC1://Supply Humidity
			if(!isNullOrWhiteSpace(val)){
				HumiditySupply.CalcMinAverMax( atof(val.c_str()) );
			}
			SubFase = GET_SYS;
			break;


	     case GET_SYS: //Return Humidity
			 if(!isNullOrWhiteSpace(val)){
				 HumidityReturn.CalcMinAverMax( atof(val.c_str()) );
			 }
			 SubFase = GET_AC2;
	         break;

		case GET_AC2://AirTempSupply
			if(!isNullOrWhiteSpace(val)){
				AirTempSupply.CalcMinAverMax( atof(val.c_str()) );//AirTempReturn
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://supply temp (air outlet temp)
			if(!isNullOrWhiteSpace(val)){
				AirTempReturn.CalcMinAverMax( atof(val.c_str()) );//
			}
			SubFase = GET_UE;
			break;

		case GET_UE://fan speed
			if(!isNullOrWhiteSpace(val)){
				FanSpeed.CalcMinAverMax( atof(val.c_str()) );//
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT://system status
			if(!isNullOrWhiteSpace(val))
			{
				SiteInfo.BattType = val;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			SiteInfo.CU_No = "UNKN";
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.CU_No = val;
			}
			SubFase = GET_SERIALN;
			break;

		case GET_SERIALN://
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SerialNo = val;
			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://
			if(!isNullOrWhiteSpace(val))
			{
				//Alarms[Alarms.CurrentIndex]->Value = val.find("nactive") == string::npos;
				Alarms[Alarms.CurrentIndex]->SetValue(val.find("nactive") == string::npos);
			}
	    	if(++Alarms.CurrentIndex < Alarms.size())
	    	{
	    		SubFase = GET_ALARMS;
	    	}
	    	else
	    	{
	    		Alarms.CurrentIndex = 0;
	    		SubFase = GET_POINT_STEPS;
	    	}
			break;


	    case GET_POINT_STEPS://
			if(!isNullOrWhiteSpace(val))
			{
				float v = atof(val.c_str());
				PointSteps[PointSteps.CurrentIndex]->Value = v;
			}
	    	if(++PointSteps.CurrentIndex < PointSteps.size())
	    	{
	    		SubFase = GET_POINT_STEPS;
	    	}
	    	else
	    	{
	    		PointSteps.CurrentIndex = 0;
	    		SubFase = IBP_SET_CMD;
	    	}
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord CondLiebert::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool CondLiebert::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool CondLiebert::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool CondLiebert::ChangeParameters( vector<string> parameters )
{
	bool ret = false;

	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0://temp set point
			 oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5008";
			 data = toString( atof(data.c_str()) );
			 break;
		 case 1://min temp set point limit
			 oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5014";
			 data = toString( atoi(data.c_str()) );
			 break;
		 case 2://max temp set point limit
			 oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5018";
			 data = toString( atoi(data.c_str()) );
			 break;
		 case 3://humid set point
			 oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5029";
			 data = toString( atoi(data.c_str()) );
			 break;
		 case 4://min humid set point limit
			 oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5033";
			 data = toString( atoi(data.c_str()) );
			 break;
		 case 5://max humid set point limit
			 oid = "1.3.6.1.4.1.476.1.42.3.9.20.1.20.1.2.1.5035";
			 data = toString( atoi(data.c_str()) );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			Log::DEBUG("CondLiebert::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}

	return ret;
}
//=============================================================================
string CondLiebert::GetStringValue(void)
{

	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';
	ReturnString += FloatToString(AirTempSupply.Aver, 0, 2)+','+FloatToString(AirTempSupply.Min, 0, 2)+','+FloatToString(AirTempSupply.Max, 0, 2)+',';
	AirTempSupply.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';
	ReturnString += FloatToString(AirTempReturn.Aver, 0, 2)+','+FloatToString(AirTempReturn.Min, 0, 2)+','+FloatToString(AirTempReturn.Max, 0, 2)+',';
	AirTempReturn.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(71)+','+WordToString(1)+',';
	ReturnString += FloatToString(HumiditySupply.Aver, 0, 2)+','+FloatToString(HumiditySupply.Min, 0, 2)+','+ FloatToString(HumiditySupply.Max, 0, 2)+',';
	HumiditySupply.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(71)+','+WordToString(2)+',';
	ReturnString += FloatToString(HumidityReturn.Aver, 0, 2)+','+FloatToString(HumidityReturn.Min, 0, 2)+','+ FloatToString(HumidityReturn.Max, 0, 2)+',';
	HumidityReturn.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(87)+','+WordToString(1)+',';
	ReturnString += FloatToString(FanSpeed.Aver, 0, 2)+','+FloatToString(FanSpeed.Min, 0, 2)+','+ FloatToString(FanSpeed.Max, 0, 2)+',';
	FanSpeed.ClrDivider();


	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';

	/*
	ReturnString +=Alarms.GetValueString();

	ReturnString += "\r\n>,201,1,"+ WordToString(UE.State)+',';


	ReturnString += PointSteps.GetValueString();


	string snumb = "000000";
	if(!IsEnable && SiteInfo.SerialNo == "0")
		snumb = "0";
    ReturnString += "\r\n>,49,1,"+snumb+",";

    ReturnString += WordToString(IsEnable)+',';
    */

	return ReturnString;
}
//*****************************************************************************
//*****************************************************************************
//***
//*****************************************************************************
//=============================================================================
XmFast::XmFast(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, Byte type):Iibp(Etype, Ename, adr)
{
	for(int i = 0; i < 3; i++){//6
		PointSteps.Params.push_back(new THistFloatParam());
	}
	//first 20 in Iibp::Iibp(
	for(int i = 0; i < 30; i++){//30
		Alarms.Alarms.push_back(new THistBoolParam());
	}


	if(type == 102)
	{
		Log::DEBUG("XmFast type="+toString((int)type));
	}


	//start Alarms.index = 28
	Log::DEBUG("XmFast Alarms.size="+toString(Alarms.size()));

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "carel": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("XmFast c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("XmFast c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("XmFast c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
XmFast::~XmFast()
{

}
//=============================================================================
Word XmFast::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

	 switch(CMD)
	 {
     case IBP_INIT://
    	 last_oid = "1.3.6.1.4.1.9839.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //input temp
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC2: //output temp
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.2.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AC3: //freecool temp
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.3.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_TEMP: //temp of nagnetania (1 compressor)
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.4.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_AKB: //outside temp
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.5.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_UE: //discharge pressure
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.6.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT: //analog 7
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.7.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT_ALARM: //analog 8
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.8.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT_BOOL: //analog 9
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.9.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_LOAD: //analog 10
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.10.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_STAT://status
    	 last_oid = "1.3.6.1.4.1.9839.2.1.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

   case GET_SITE://unit type
	     last_oid = "1.3.6.1.4.1.9839.2.1.3.11.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_ALARMS://
    	 //last_oid = "1.3.6.1.4.1.2011.6.164.1.1.2.99.1.1";
         //values = snmp_driver.GetPDU(SNMP_GET_NEXT_req, SNMP_V1, c_read, new Oid(last_oid));
    	 last_oid = "1.3.6.1.4.1.9839.2.1.1."+toString(Alarms.CurrentIndex+45)+".0";
    	 //Log::DEBUG("alarms oid="+last_oid);
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_POINT_STEPS://
    	 //Log::DEBUG("CreateCMD, PointSteps.CurrentIndex="+toString(PointSteps.CurrentIndex));
    	 switch(PointSteps.CurrentIndex)
    	 {
			 case 0://temp set point
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.11.0";
				 break;
			 case 1://condensation set point
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.13.0";
				 break;
			 case 2://range of regulation temp
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.14.0";
				 break;
    	 };
    	 //
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V1, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte XmFast::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("snmp_driver.GetData=["+val+"] len="+toString(val.size()));

	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_AC1;
			break;

		case GET_AC1://input temp
			if(!isNullOrWhiteSpace(val)){
				RoomTemp.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_AC2;
			break;

		case GET_AC2://output temp
			if(!isNullOrWhiteSpace(val)){
				RoomTempOut.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_AC3;
			break;

		case GET_AC3://freecool temp
			if(!isNullOrWhiteSpace(val)){
				FreeTemp.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_TEMP;
			break;

		case GET_TEMP:////temp of nagnetania (1 compressor)
			if(!isNullOrWhiteSpace(val)){
				DischTempComp1.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_AKB;
			break;

		case GET_AKB://outside temp
			if(!isNullOrWhiteSpace(val)){
				ExtTemp.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_UE;
			break;

		case GET_UE://discharge pressure
			if(!isNullOrWhiteSpace(val)){
				DischPressureComp1.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_RECT;
			break;

		case GET_RECT://analog 7
			if(!isNullOrWhiteSpace(val)){
				AnalogValue7.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_RECT_ALARM;
			break;

		case GET_RECT_ALARM://analog 8
			if(!isNullOrWhiteSpace(val)){
				AnalogValue10.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_RECT_BOOL;
			break;

		case GET_RECT_BOOL://analog 9
			if(!isNullOrWhiteSpace(val)){
				AnalogValue9.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_LOAD;
			break;

		case GET_LOAD://analog 10
			if(!isNullOrWhiteSpace(val)){
				AnalogValue10.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_STAT;
			break;

		case GET_STAT://
			if(!isNullOrWhiteSpace(val))
			{
				Word st = atoi(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_SITE;
			break;

		case GET_SITE://
			if(!isNullOrWhiteSpace(val))
			{
				SiteInfo.Customer = "FAST";
				SiteInfo.CU_No = "UNKN";
				Word st = atoi(val.c_str());

				switch(st)
				{
				case 0:
					SiteInfo.CU_No = "DX";
					break;
				case 1:
					SiteInfo.CU_No = "CW";
					break;
				case 2:
					SiteInfo.CU_No = "DUAL COOLING";
					break;
				};

			}
			SubFase = GET_ALARMS;
			break;

		case GET_ALARMS://
			if(!isNullOrWhiteSpace(val))
			{
				//Alarms[Alarms.CurrentIndex]->Value = atoi(val.c_str());
				Alarms[Alarms.CurrentIndex]->SetValue( atoi(val.c_str()) );
			}
	    	if(++Alarms.CurrentIndex < Alarms.size())
	    	{
	    		SubFase = GET_ALARMS;
	    	}
	    	else
	    	{
	    		Alarms.CurrentIndex = 0;
	    		SubFase = GET_POINT_STEPS;
	    	}
			break;


	    case GET_POINT_STEPS://
			if(!isNullOrWhiteSpace(val))
			{
				float v = atof(val.c_str())/10.0;
				PointSteps[PointSteps.CurrentIndex]->Value = v;
			}
	    	if(++PointSteps.CurrentIndex < PointSteps.size())
	    	{
	    		SubFase = GET_POINT_STEPS;
	    	}
	    	else
	    	{
	    		PointSteps.CurrentIndex = 0;
	    		SubFase = IBP_SET_CMD;
	    	}
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord XmFast::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool XmFast::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool XmFast::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool XmFast::ChangeParameters( vector<string> parameters )
{
	bool ret = false;

	for(int i = 0; i < parameters.size(); i++){
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0://temp set point
			 oid = "1.3.6.1.4.1.9839.2.1.2.11.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 1://min temp set point limit
			 oid = "1.3.6.1.4.1.9839.2.1.2.13.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 2://max temp set point limit
			 oid = "1.3.6.1.4.1.9839.2.1.2.14.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}

	return ret;
}
//=============================================================================
string XmFast::GetStringValue(void)
{

	string ReturnString = WordToString(Et)+","+WordToString(En)+",";
	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(1)+',';//input temp
	ReturnString += FloatToString(RoomTemp.Aver, 0, 2)+','+FloatToString(RoomTemp.Min, 0, 2)+','+FloatToString(RoomTemp.Max, 0, 2)+',';
	RoomTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(2)+',';//outside temp
	ReturnString += FloatToString(ExtTemp.Aver, 0, 2)+','+FloatToString(ExtTemp.Min, 0, 2)+','+FloatToString(ExtTemp.Max, 0, 2)+',';
	ExtTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(3)+',';//output temp
	ReturnString += FloatToString(RoomTempOut.Aver, 0, 2)+','+FloatToString(RoomTempOut.Min, 0, 2)+','+FloatToString(RoomTempOut.Max, 0, 2)+',';
	RoomTempOut.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(4)+',';//freecool temp
	ReturnString += FloatToString(FreeTemp.Aver, 0, 2)+','+FloatToString(FreeTemp.Min, 0, 2)+','+ FloatToString(FreeTemp.Max, 0, 2)+',';
	FreeTemp.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(11)+',';//temp of nagnetania (1 compressor)
	ReturnString += FloatToString(DischTempComp1.Aver, 0, 2)+','+FloatToString(DischTempComp1.Min, 0, 2)+','+ FloatToString(DischTempComp1.Max, 0, 2)+',';
	DischTempComp1.ClrDivider();

	//ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(12)+',';//temp of nagnetania (2 compressor)
	//ReturnString += FloatToString(DischTempComp2.Aver, 0, 2)+','+FloatToString(DischTempComp2.Min, 0, 2)+','+ FloatToString(DischTempComp2.Max, 0, 2)+',';
	//DischTempComp2.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(13)+',';
	ReturnString += FloatToString(AnalogValue7.Aver, 0, 2)+','+FloatToString(AnalogValue7.Min, 0, 2)+','+ FloatToString(AnalogValue7.Max, 0, 2)+',';
	AnalogValue7.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(14)+',';
	ReturnString += FloatToString(AnalogValue8.Aver, 0, 2)+','+FloatToString(AnalogValue8.Min, 0, 2)+','+ FloatToString(AnalogValue8.Max, 0, 2)+',';
	AnalogValue8.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(15)+',';
	ReturnString += FloatToString(AnalogValue9.Aver, 0, 2)+','+FloatToString(AnalogValue9.Min, 0, 2)+','+ FloatToString(AnalogValue9.Max, 0, 2)+',';
	AnalogValue9.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(16)+',';
	ReturnString += FloatToString(AnalogValue10.Aver, 0, 2)+','+FloatToString(AnalogValue10.Min, 0, 2)+','+ FloatToString(AnalogValue10.Max, 0, 2)+',';
	AnalogValue10.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(76)+','+WordToString(11)+',';
	ReturnString += FloatToString(DischPressureComp1.Aver, 0, 2)+','+FloatToString(DischPressureComp1.Min, 0, 2)+','+ FloatToString(DischPressureComp1.Max, 0, 2)+',';
	DischPressureComp1.ClrDivider();

	//ReturnString += "\r\n>,"+WordToString(76)+','+WordToString(12)+',';
	//ReturnString += FloatToString(DischPressureComp2.Aver, 0, 2)+','+FloatToString(DischPressureComp2.Min, 0, 2)+','+ FloatToString(DischPressureComp2.Max, 0, 2)+',';
	//DischPressureComp2.ClrDivider();

	ReturnString += "\r\n>,"+WordToString(201)+','+WordToString(1)+',';
	ReturnString += toString((int)UE.State)+',';

	//ReturnString += "\r\n>,"+WordToString(201)+','+WordToString(2)+',';
	//ReturnString += toString((int)UE2.State)+',';

	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';
	return ReturnString;
}

//*****************************************************************************
//***
//*****************************************************************************
//=============================================================================
XmFastComp2::XmFastComp2(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap, Byte type):Iibp(Etype, Ename, adr)
{
	for(int i = 0; i < 3; i++){//6
		PointSteps.Params.push_back(new THistFloatParam());
	}
	PointSteps.Pt.Pname = 2;
	SiteInfo.Pt.Pname 	= 2;

	Alarms.Alarms.clear();


    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "carel": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("XmFast c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("XmFast c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("XmFast c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
XmFastComp2::~XmFastComp2()
{

}
//=============================================================================
Word XmFastComp2::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

	 switch(CMD)
	 {
     case IBP_INIT://
    	 last_oid = "1.3.6.1.4.1.9839.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
       break;

     case GET_TEMP: //temp of nagnetania (2 compressor)
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.4.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;


     case GET_UE: //discharge pressure
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.6.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT: //analog 7
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.7.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT_ALARM: //analog 8
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.8.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_RECT_BOOL: //analog 9
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.9.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_LOAD: //analog 10
    	 last_oid = "1.3.6.1.4.1.9839.2.1.2.10.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

    case GET_STAT://status
    	 last_oid = "1.3.6.1.4.1.9839.2.1.1.1.0";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case GET_POINT_STEPS://
    	 switch(PointSteps.CurrentIndex)
    	 {
			 case 0://temp set point
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.11.0";
				 break;
			 case 1://condensation set point
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.13.0";
				 break;
			 case 2://range of regulation temp
				 last_oid = "1.3.6.1.4.1.9839.2.1.2.14.0";
				 break;
    	 };
    	 //
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V1, c_read, new Oid(last_oid));
         break;

     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("Huawei2::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V1, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte XmFastComp2::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("XmFastComp2 snmp_driver.GetData=["+val+"] len="+toString(val.size()) +" SubFase="+toString((int)subFase));

	switch(subFase)
	{
		case IBP_INIT://
			SubFase = GET_TEMP;
			break;

		case GET_TEMP:////temp of nagnetania (2 compressor)
			if(!isNullOrWhiteSpace(val)){
				DischTempComp2.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_UE;
			break;


		case GET_UE://discharge pressure
			if(!isNullOrWhiteSpace(val)){
				DischPressureComp2.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_STAT;
			break;

			/*
		case GET_RECT://analog 7
			if(!isNullOrWhiteSpace(val)){
				AnalogValue7.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_RECT_ALARM;
			break;

		case GET_RECT_ALARM://analog 8
			if(!isNullOrWhiteSpace(val)){
				AnalogValue10.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_RECT_BOOL;
			break;

		case GET_RECT_BOOL://analog 9
			if(!isNullOrWhiteSpace(val)){
				AnalogValue9.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_LOAD;
			break;

		case GET_LOAD://analog 10
			if(!isNullOrWhiteSpace(val)){
				AnalogValue10.CalcMinAverMax( ((short)atoi(val.c_str()))/10.0 );
			}
			SubFase = GET_STAT;
			break;
			*/


		case GET_STAT://
			if(!isNullOrWhiteSpace(val))
			{
				Word st = atoi(val.c_str());
				UE.State =  st;
			}
			SubFase = GET_POINT_STEPS;
			break;

	    case GET_POINT_STEPS://
			if(!isNullOrWhiteSpace(val))
			{
				float v = atof(val.c_str())/10.0;
				PointSteps[PointSteps.CurrentIndex]->Value = v;
			}
	    	if(++PointSteps.CurrentIndex < PointSteps.size())
	    	{
	    		SubFase = GET_POINT_STEPS;
	    	}
	    	else
	    	{
	    		PointSteps.CurrentIndex = 0;
	    		SubFase = IBP_SET_CMD;
	    	}
			break;

	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord XmFastComp2::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool XmFastComp2::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool XmFastComp2::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool XmFastComp2::ChangeParameters( vector<string> parameters )
{
	bool ret = false;

	for(int i = 3; i < parameters.size(); i++)
	{
		string oid  = "";
		string data = parameters[i];
		string type = "66";//0x42
		switch(i)
		{
		 case 0://temp set point
			 oid = "1.3.6.1.4.1.9839.2.1.2.11.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 1://min temp set point limit
			 oid = "1.3.6.1.4.1.9839.2.1.2.13.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		 case 2://max temp set point limit
			 oid = "1.3.6.1.4.1.9839.2.1.2.14.0";
			 data = toString( atoi(data.c_str())*10 );
			 break;
		};
		if(!isNullOrWhiteSpace(oid) && !isNullOrWhiteSpace(data)){
			//Log::DEBUG("Huawei2::ChangeParameters oid="+oid+ "data="+data);
			CmdMessages.push_back(oid+" "+data+" "+type);
			ret = true;
		}
	}

	return ret;
}
//=============================================================================
string XmFastComp2::GetStringValue(void)
{
	int en = En > 10 ? (En - 10) : En;

	string ReturnString = WordToString(Et)+","+WordToString(en)+",";

	ReturnString += "\r\n>,"+WordToString(9)+','+WordToString(12)+',';//temp of nagnetania (2 compressor)
	ReturnString += FloatToString(DischTempComp2.Aver, 0, 2)+','+FloatToString(DischTempComp2.Min, 0, 2)+','+ FloatToString(DischTempComp2.Max, 0, 2)+',';
	DischTempComp2.ClrDivider();


	ReturnString += "\r\n>,"+WordToString(76)+','+WordToString(12)+',';
	ReturnString += FloatToString(DischPressureComp2.Aver, 0, 2)+','+FloatToString(DischPressureComp2.Min, 0, 2)+','+ FloatToString(DischPressureComp2.Max, 0, 2)+',';
	DischPressureComp2.ClrDivider();


	ReturnString += "\r\n>,"+WordToString(201)+','+WordToString(2)+',';
	ReturnString += toString((int)UE.State)+',';

	return ReturnString;

}
//*****************************************************************************
//***
//*****************************************************************************
VertivPDU::VertivPDU(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr), E(40,1), Iin(1), Iout(4)
{
	//LoadPersent1 = LoadPersent2 = LoadPersent3 = 0.0;

	Log::DEBUG("VertivPDU::VertivPDU()");

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

	//20-20=0
	for(int i = 0; i < 20; i++){
		Alarms.Alarms.pop_back();
	}

}
//=============================================================================
VertivPDU::VertivPDU(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap):VertivPDU(Etype, Ename, adr)
{

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("TsiBravoUPS::TsiBravoUPS c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("TsiBravoUPS::TsiBravoUPS c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("TsiBravoUPS::TsiBravoUPS c_trap=["+c_trap+"] trap=["+trap+"]");
}
//=============================================================================
VertivPDU::~VertivPDU()
{

}
//=============================================================================

string VertivPDU::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
								  E.GetValueString() +
							  	  Uin.GetValueString()+
							  	  Iin.GetValueString()+
							  	  Iout.GetValueString()+
							  	  //UE.GetValueString()+
							  	  P.GetValueString()+
							  	  S.GetValueString()+
							  	  KM.GetValueString();
							  	  //SiteInfo.GetValueString() + WordToString(IsEnable)+',';//mast be before SiteInfo !!

	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';

	return ReturnString;
}
//=============================================================================
Word VertivPDU::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

	 switch(CMD)
	 {
     case IBP_INIT://product version
    	 last_oid = "1.3.6.1.4.1.21239.5.2.1.2.0";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //V1in
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.4.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AC2: //V2in
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.4.2";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AC3: //V3in
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.4.3";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN1: //input current 1
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.8.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_I_IN2: //input current 2
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.8.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_I_IN3: //input current 3
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.8.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT1: //breker current 1
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.3.1.4.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_I_OUT2: //breker current 2
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.3.1.4.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_I_OUT3: //breker current 3
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.3.1.4.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC1_OUT: //real power 1
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.12.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AC2_OUT: //real power 2
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.12.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AC3_OUT: //real power 3
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.12.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AKB_TYPE: //real power summ
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.1.1.9.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT1: //full power 1
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.13.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_LOAD_OUT2: //full power 2
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.13.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_LOAD_OUT3: //full power 3
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.13.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
    case GET_STAT://full power summ
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.1.1.10.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;


    case GET_RECT: //power factor 1
   	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.14.1";
        values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
        break;
    case GET_RECT_ALARM: //power factor 2
   	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.14.2";
        values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
        break;
    case GET_RECT_BOOL: //power factor 3
   	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.14.3";
        values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
        break;
   case GET_LOAD://power factor summ
   	 last_oid = "1.3.6.1.4.1.21239.5.2.3.1.1.11.1";
        values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
        break;


   case GET_ALARMS: //energy 1
  	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.15.1";
       values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;
   case GET_ALARMS1: //energy 2
  	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.15.2";
       values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;
   case GET_TEMP: //energy 3
  	 last_oid = "1.3.6.1.4.1.21239.5.2.3.2.1.15.3";
       values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;
  case GET_COMP://energy summ
  	 last_oid = "1.3.6.1.4.1.21239.5.2.3.1.1.12.1";
       values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;

    case GET_SITE://device model
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.1.1.3.1";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
    case GET_SERIALN://ESN
    	 last_oid = "1.3.6.1.4.1.21239.5.2.3.1.1.2.1";//ser number
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;


     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("TsiBravoUPS::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V2, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }
	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte VertivPDU::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("VertivPDU::GetData=["+val+"] last_oid="+last_oid + " subFase="+toString((IBPSUBFASE)subFase));

	switch(subFase)
	{
		case IBP_INIT://
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SwVers = val;
			}
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC2;
			break;
		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC3;
			break;
		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_IN1;
			break;

		case GET_I_IN1://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ia.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_IN2;
			break;
		case GET_I_IN2://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ib.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_IN3;
			break;
		case GET_I_IN3://
			if(!isNullOrWhiteSpace(val)){
				Iin.Ic.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_OUT1;
			break;

		case GET_I_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ia.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_OUT2;
			break;
		case GET_I_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ib.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_I_OUT3;
			break;
		case GET_I_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ic.CalcMinAverMax(atoi(val.c_str())/100.0);
			}
			SubFase = GET_AC1_OUT;
			break;
/////////////////////////
	     case GET_AC1_OUT: //real power 1
	    	 if(!isNullOrWhiteSpace(val)){
	    		 P.Pa.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_AC2_OUT;
	         break;
	     case GET_AC2_OUT: //real power 2
	    	 if(!isNullOrWhiteSpace(val)){
	    		 P.Pb.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_AC3_OUT;
	         break;
	     case GET_AC3_OUT: //real power 3
	    	 if(!isNullOrWhiteSpace(val)){
	    		 P.Pc.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_AKB_TYPE;
	         break;
	     case GET_AKB_TYPE: //real power summ
	    	 if(!isNullOrWhiteSpace(val)){
	    		 P.P.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_LOAD_OUT1;
	         break;

	     case GET_LOAD_OUT1: //full power 1
	    	 if(!isNullOrWhiteSpace(val)){
	    		 S.Sa.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_LOAD_OUT2;
	         break;
	     case GET_LOAD_OUT2: //full power 2
	    	 if(!isNullOrWhiteSpace(val)){
	    		 S.Sb.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_LOAD_OUT3;
	         break;
	     case GET_LOAD_OUT3: //full power 3
	    	 if(!isNullOrWhiteSpace(val)){
	    		 S.Sc.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_STAT;
	         break;
	    case GET_STAT://full power summ
	    	 if(!isNullOrWhiteSpace(val)){
	    		 S.S.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_RECT;
	         break;


	    case GET_RECT: //power factor 1
	    	 if(!isNullOrWhiteSpace(val)){
	    		 KM.KMa.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_RECT_ALARM;
	        break;
	    case GET_RECT_ALARM: //power factor 2
	    	 if(!isNullOrWhiteSpace(val)){
	    		 KM.KMb.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_RECT_BOOL;
	        break;
	    case GET_RECT_BOOL: //power factor 3
	    	 if(!isNullOrWhiteSpace(val)){
	    		 KM.KMc.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_LOAD;
	        break;
	   case GET_LOAD://power factor summ
	    	 if(!isNullOrWhiteSpace(val)){
	    		 KM.KM.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_ALARMS;
	        break;


	   case GET_ALARMS: //energy 1
	    	 if(!isNullOrWhiteSpace(val)){
	    		 E.Efa = (atoi(val.c_str())/1.000);
	    	 }
	    	 SubFase = GET_ALARMS1;
	       break;
	   case GET_ALARMS1: //energy 2
	    	 if(!isNullOrWhiteSpace(val)){
	    		 E.Efb = (atoi(val.c_str())/1.000);
	    	 }
	    	 SubFase = GET_TEMP;
	       break;
	   case GET_TEMP: //energy 3
	    	 if(!isNullOrWhiteSpace(val)){
	    		 E.Efc = (atoi(val.c_str())/1.000);
	    	 }
	    	 SubFase = GET_COMP;
	       break;
	  case GET_COMP://energy summ
	    	 if(!isNullOrWhiteSpace(val)){
	    		 E.Ea = (atoi(val.c_str())/1.000);
	    	 }
	    	 SubFase = GET_SITE;
	       break;

///////////////////////////
		case GET_SITE://
			SiteInfo.Customer = "Vertiv";
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.CU_No = val;
			}

			SubFase = GET_SERIALN;
			break;
		case GET_SERIALN://
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SerialNo = val;
			}
			SubFase = IBP_SET_CMD;
			break;


	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord VertivPDU::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool VertivPDU::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool VertivPDU::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool VertivPDU::ChangeParameters( vector<string> parameters )
{
	return true;
}
//*****************************************************************************
//***
//*****************************************************************************
ApcPDU::ApcPDU(Byte Etype, Byte Ename, Byte adr):Iibp(Etype, Ename, adr), E(40,1), Iout(4)
{
	Log::DEBUG("ApcPDU::ApcPDU()");

    c_read  = "public";
    c_write = "private";
    c_trap  = "trap";

    Fases = 0;
    Outlets = 0;
    MaxCurrent = 0;

	//20-20=0
	for(int i = 0; i < 20; i++){
		Alarms.Alarms.pop_back();
	}

}
//=============================================================================
ApcPDU::ApcPDU(Byte Etype, Byte Ename, Byte adr, string read, string write, string trap):ApcPDU(Etype, Ename, adr)
{

    c_read  = isNullOrWhiteSpace(read) ? "public" : read;
    c_write = isNullOrWhiteSpace(write) ? "private": write;
    c_trap  = isNullOrWhiteSpace(trap) ? "trap" : trap;

    Log::DEBUG("ApcPDU::TsiBravoUPS c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("ApcPDU::TsiBravoUPS c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("ApcPDU::TsiBravoUPS c_trap=["+c_trap+"] trap=["+trap+"]");

}
//=============================================================================
ApcPDU::~ApcPDU()
{

}
//=============================================================================

string ApcPDU::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+
								  //E.GetValueString() +
									">,40,1,"+ FloatToString(E.Ea, 0, 3)+','+
							  	  Uin.GetValueString()+
							  	  Iout.GetValueString()+
							  	  P.GetValueString()+
							  	  S.GetValueString()+
							  	  KM.GetValueString();
							  	  //SiteInfo.GetValueString() + WordToString(IsEnable)+',';//mast be before SiteInfo !!

	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';

	return ReturnString;
}
//=============================================================================
Word ApcPDU::CreateCMD(Byte CMD, Byte *Buffer)
{
	 Word DataLen = 0;
	 Blong       Tmp;
     Bshort		 Tmp2;
     vector<Byte> values;

	 switch(CMD)
	 {
     case IBP_INIT://product version
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.2.1.6.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;

     case GET_AC1: //V1in
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.6.1";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AC2: //V2in
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.6.2";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AC3: //V3in
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.6.3";//
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_IN1: //number of fases
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.4.2.1.7.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_I_IN2: //number of outlets
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.4.2.1.4.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_I_IN3: //max current
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.4.2.1.9.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_I_OUT1: //breker current 1
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.5.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_I_OUT2: //breker current 2
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.5.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_I_OUT3: //breker current 3
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.5.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_AC1_OUT: //real power 1
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.7.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AC2_OUT: //real power 2
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.7.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AC3_OUT: //real power 3
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.7.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_AKB_TYPE: //real power summ
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.4.3.1.5.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;

     case GET_LOAD_OUT1: //full power 1
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.8.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_LOAD_OUT2: //full power 2
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.8.2";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
     case GET_LOAD_OUT3: //full power 3
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.8.3";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
    case GET_STAT://full power summ
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.4.3.1.16.1";
         values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;


    case GET_RECT: //power factor 1
   	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.9.1";
        values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
        break;
    case GET_RECT_ALARM: //power factor 2
   	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.9.2";
        values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
        break;
    case GET_RECT_BOOL: //power factor 3
   	 last_oid = "1.3.6.1.4.1.318.1.1.26.6.3.1.9.3";
        values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
        break;
   case GET_LOAD://power factor summ
   	 last_oid = "1.3.6.1.4.1.318.1.1.26.4.3.1.17.1";
        values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
        break;

  case GET_COMP://energy summ
  	 last_oid = "1.3.6.1.4.1.318.1.1.26.4.3.1.9.1";
       values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
       break;

    case GET_SITE://device model
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.2.1.8.1";
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;
    case GET_SERIALN://ESN
    	 last_oid = "1.3.6.1.4.1.318.1.1.26.2.1.9.1";//ser number
    	 values = snmp_driver.GetPDU(SNMP_GET_req, SNMP_V2, c_read, new Oid(last_oid));
         break;


     case IBP_SET_CMD:
			if(CmdMessages.size() > 0)
			{
				string mess = CmdMessages.front();
				CmdMessages.pop_front();
				vector<string>  cmd = TBuffer::Split(mess, " ");
				//Log::DEBUG("ApcPDU::CreateCMD=["+mess+"] split size="+toString(cmd.size()));
				if(cmd.size() >= 3)
				{
					last_oid 		= cmd[0];
					string data 	= cmd[1];
					int type 		= atoi(cmd[2].c_str());
					values = snmp_driver.GetPDU(SNMP_SET_req, SNMP_V2, c_write, new Oid(last_oid),data, type);
				}
			}
    	 break;

     default:
       DataLen = 0;
       break;
	 }

	 for(auto curr: values){
		 Buffer[DataLen] = values[DataLen++];
	 }


	 Buffer[DataLen] = 0;
	 return DataLen;
}
//=============================================================================
Byte ApcPDU::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte SubFase = subFase;//IBP_EXIT;
	sBshort Tmp;
	BUF[Len] = 0;

	string val = snmp_driver.GetData(BUF, Len, last_oid);
	//Log::DEBUG("ApcPDU::GetData=["+val+"] last_oid="+last_oid + " subFase="+toString((IBPSUBFASE)subFase));

	switch(subFase)
	{
		case IBP_INIT://
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SwVers = val;
			}
			SubFase = GET_AC1;
			break;

		case GET_AC1://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ua.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC2;
			break;
		case GET_AC2://
			if(!isNullOrWhiteSpace(val)){
				Uin.Ub.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_AC3;
			break;
		case GET_AC3://
			if(!isNullOrWhiteSpace(val)){
				Uin.Uc.CalcMinAverMax(atoi(val.c_str())/1.0);
			}
			SubFase = GET_I_IN1;
			break;

		case GET_I_IN1://
			if(!isNullOrWhiteSpace(val)){
				Fases = atoi(val.c_str());
			}
			SubFase = GET_I_IN2;
			break;
		case GET_I_IN2://
			if(!isNullOrWhiteSpace(val)){
				Outlets = atoi(val.c_str());
			}
			SubFase = GET_I_IN3;
			break;
		case GET_I_IN3://
			if(!isNullOrWhiteSpace(val)){
				MaxCurrent = atoi(val.c_str());
			}
			SubFase = GET_I_OUT1;
			break;

		case GET_I_OUT1://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ia.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT2;
			break;
		case GET_I_OUT2://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ib.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_I_OUT3;
			break;
		case GET_I_OUT3://
			if(!isNullOrWhiteSpace(val)){
				Iout.Ic.CalcMinAverMax(atoi(val.c_str())/10.0);
			}
			SubFase = GET_AC1_OUT;
			break;
/////////////////////////
	     case GET_AC1_OUT: //real power 1
	    	 if(!isNullOrWhiteSpace(val)){
	    		 P.Pa.CalcMinAverMax(atoi(val.c_str())/100.0);
	    	 }
	    	 SubFase = GET_AC2_OUT;
	         break;
	     case GET_AC2_OUT: //real power 2
	    	 if(!isNullOrWhiteSpace(val)){
	    		 P.Pb.CalcMinAverMax(atoi(val.c_str())/100.0);
	    	 }
	    	 SubFase = GET_AC3_OUT;
	         break;
	     case GET_AC3_OUT: //real power 3
	    	 if(!isNullOrWhiteSpace(val)){
	    		 P.Pc.CalcMinAverMax(atoi(val.c_str())/100.0);
	    	 }
	    	 SubFase = GET_AKB_TYPE;
	         break;
	     case GET_AKB_TYPE: //real power summ
	    	 if(!isNullOrWhiteSpace(val)){
	    		 P.P.CalcMinAverMax(atoi(val.c_str())/100.0);
	    	 }
	    	 SubFase = GET_LOAD_OUT1;
	         break;

	     case GET_LOAD_OUT1: //full power 1
	    	 if(!isNullOrWhiteSpace(val)){
	    		 S.Sa.CalcMinAverMax(atoi(val.c_str())/100.0);
	    	 }
	    	 SubFase = GET_LOAD_OUT2;
	         break;
	     case GET_LOAD_OUT2: //full power 2
	    	 if(!isNullOrWhiteSpace(val)){
	    		 S.Sb.CalcMinAverMax(atoi(val.c_str())/100.0);
	    	 }
	    	 SubFase = GET_LOAD_OUT3;
	         break;
	     case GET_LOAD_OUT3: //full power 3
	    	 if(!isNullOrWhiteSpace(val)){
	    		 S.Sc.CalcMinAverMax(atoi(val.c_str())/100.0);
	    	 }
	    	 SubFase = GET_STAT;
	         break;
	    case GET_STAT://full power summ
	    	 if(!isNullOrWhiteSpace(val)){
	    		 S.S.CalcMinAverMax(atoi(val.c_str())/100.0);
	    	 }
	    	 SubFase = GET_RECT;
	         break;


	    case GET_RECT: //power factor 1
	    	 if(!isNullOrWhiteSpace(val)){
	    		 KM.KMa.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_RECT_ALARM;
	        break;
	    case GET_RECT_ALARM: //power factor 2
	    	 if(!isNullOrWhiteSpace(val)){
	    		 KM.KMb.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_RECT_BOOL;
	        break;
	    case GET_RECT_BOOL: //power factor 3
	    	 if(!isNullOrWhiteSpace(val)){
	    		 KM.KMc.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_LOAD;
	        break;
	   case GET_LOAD://power factor summ
	    	 if(!isNullOrWhiteSpace(val)){
	    		 KM.KM.CalcMinAverMax(atoi(val.c_str())/1.0);
	    	 }
	    	 SubFase = GET_COMP;
	        break;

		  case GET_COMP://energy summ
		    	 if(!isNullOrWhiteSpace(val)){
		    		 E.Ea = (atoi(val.c_str())/1.000);
		    	 }
		    	 SubFase = GET_SITE;
		       break;

			case GET_SITE://
				SiteInfo.Customer = "APC";
				if(!isNullOrWhiteSpace(val)){
					SiteInfo.CU_No = val;
				}
				SubFase = GET_SERIALN;
				break;

		case GET_SERIALN://
			if(!isNullOrWhiteSpace(val)){
				SiteInfo.SerialNo = val;
			}
			SubFase = IBP_SET_CMD;
			break;


	    case IBP_SET_CMD://
	    	//goto exe with SubFase==IBP_SET_CMD
			break;

		default:
			SubFase = IBP_EXIT;
			break;
	};

    return SubFase;
}
//=============================================================================
sWord ApcPDU::RecvData(IPort* port, Byte *Buf, Word MaxLen)
{
	sWord RecvLen = 0;

	RecvLen = port->Recv( Buf, MaxLen );
	return RecvLen;
}
//=============================================================================
bool ApcPDU::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	if(Len == 0 || subFase == IBP_EXIT)
		return true;

	if(Len < 9)
		return false;
	return snmp_driver.ParsingPDU(BUF, Len);
}
//=============================================================================
bool ApcPDU::ChangeState( Byte newState )
{

	return true;
}
//=============================================================================
bool ApcPDU::ChangeParameters( vector<string> parameters )
{
	return true;
}
//*****************************************************************************
//***UniversalSnmpDevice
//*****************************************************************************
UniversalSnmpDevice::UniversalSnmpDevice(Byte Etype, Byte Ename, Byte type):IDevice(Etype, Ename, type)
{
	Log::DEBUG("UniversalSnmpDevice()");

    c_read  	= "public";
    c_write 	= "private";
    c_trap  	= "trap";
    snmp_ver 	= SNMP_V1;
}
//=============================================================================
UniversalSnmpDevice::UniversalSnmpDevice(Byte Etype, Byte Ename, Byte type, SNMP_VER ver, string read, string write, string trap):UniversalSnmpDevice(Etype, Ename, type)
{

    c_read  	= isNullOrWhiteSpace(read) ? "public" : read;
    c_write 	= isNullOrWhiteSpace(write) ? "private": write;
    c_trap  	= isNullOrWhiteSpace(trap) ? "trap" : trap;
    snmp_ver 	= ver;

    Log::DEBUG("UniversalSnmpDevice c_read=["+c_read+"] read=["+read+"]");
    Log::DEBUG("UniversalSnmpDevice c_write=["+c_write+"] write=["+write+"]");
    Log::DEBUG("UniversalSnmpDevice c_trap=["+c_trap+"] trap=["+trap+"]");

}
//=============================================================================
UniversalSnmpDevice::~UniversalSnmpDevice()
{

}
//=============================================================================
Word UniversalSnmpDevice::CreateCMD(Byte CMD, Byte *Buffer)
{
	Word DataLen = 0;
    vector<Byte> values;

    if(CMD >= 100)
    {


    }
    else
    {
    	switch(CMD)
    	{
        	case IBP_INIT://product version
        		last_oid = "1.3.6.1.2.1.1.3.0";//
        		values = snmp_driver.GetPDU(SNMP_GET_req, snmp_ver, c_read, new Oid(last_oid));//SysUpTime
        		break;

        	case IBP_SET_CMD:
        		if(CmdMessages.size() > 0)
        		{
        			string mess = CmdMessages.front();
        			CmdMessages.pop_front();
        			vector<string>  cmd = TBuffer::Split(mess, " ");
        			if(cmd.size() >= 3)
        			{
        				last_oid 		= cmd[0];
        				string data 	= cmd[1];
        				int type 		= atoi(cmd[2].c_str());
        				values = snmp_driver.GetPDU(SNMP_SET_req, snmp_ver, c_write, new Oid(last_oid),data, type);
        			}
        		}
        		break;

        	default:
        		DataLen = 0;
        		break;
    	};
    }
	for(auto curr: values){
		Buffer[DataLen] = values[DataLen++];
	}
	Buffer[DataLen] = 0;
	return DataLen;
}
//=============================================================================
Byte UniversalSnmpDevice::GetDataFromMessage(Byte subFase,Byte *BUF, Word Len)
{
	Byte ret = 0;
	return ret;
}
//=============================================================================
sWord UniversalSnmpDevice::RecvData( IPort* Port, Byte *Buf, Word MaxLen )
{
	sWord ret = 0;
	return ret;
}
//=============================================================================
bool UniversalSnmpDevice::ParsingAnswer( Byte *BUF, Word &Len, Byte subFase)
{
	bool ret = false;
	return ret;
}
//=============================================================================
bool UniversalSnmpDevice::ChangeState(Byte newState)
{
	  return true;
}
//=============================================================================
bool UniversalSnmpDevice::ChangeParameters(vector<string> parameters)
{
	  return true;
}
//=============================================================================
string UniversalSnmpDevice::GetStringValue(void)
{
	string ReturnString = WordToString(Et)+","+
							WordToString(En)+","+


	ReturnString += "\r\n>,"+WordToString(0)+','+WordToString(1)+',';
	ReturnString += Port->ToString()+',';
	return ReturnString;
}
//=============================================================================
string UniversalSnmpDevice::GetAdvStringValue(void)
{
	string ReturnString = "";
	return ReturnString;
}
//=============================================================================
string UniversalSnmpDevice::GetStringCfgValue(void)
{
	string ReturnString = "";
	return ReturnString;
}
//=============================================================================
bool UniversalSnmpDevice::DetectStates(void)
{
	bool ret = false;
	return ret;
}
//=============================================================================
void UniversalSnmpDevice::ClearValues(void)
{

}
//=============================================================================
void UniversalSnmpDevice::InitGPIO(void)
{

}
//=============================================================================
vector<TSimpleDevice*> UniversalSnmpDevice::analizeCommands(vector<string> cmds)
{
	vector<TSimpleDevice*> ret;
	for(auto dev: cmds)
	{
		vector<string> devstr = TBuffer::Split(dev, ":" );
		if(devstr.size() >= 2)
		{
			vector<string> parvect = TBuffer::Split(devstr[0], "=" );//0 oid
			if(parvect.size() >= 2)
			{
				string oid = parvect[1];
				parvect = TBuffer::Split(devstr[1], "=");//1 len
				if(parvect.size() >= 2)
				{
					int len = atoi(parvect[1].c_str());
					if(devstr.size() > 2)
					{
						parvect = TBuffer::Split(devstr[2], "=");//2 type
						if(parvect.size() >= 2)
						{
							TSimpleDevice *sdev = NULL;
							Word Pt = 9;
							Word Pn = 1;
							string min;
							string max;
							string def;
							string mult;
							string type = parvect[1];
							sdev = new TSimpleDevice(Et, En);
							sdev->Oid = oid;
							//Log::DEBUG("devstr.size()="+toString(devstr.size()));
							for(int i=3; i < devstr.size(); i++)
							{
								parvect = TBuffer::Split(devstr[i], "=");// 3... et en pt pn max min def mult maxerr minerr
								if(parvect.size() >= 2)
								{
									if(parvect[0] == "et")
									{
										if( !isNullOrWhiteSpace(parvect[1]) )
											sdev->Et.Etype = atoi(parvect[1].c_str());
									}
									else if(parvect[0] == "en")
									{
										if( !isNullOrWhiteSpace(parvect[1]) )
											sdev->Et.Ename = atoi(parvect[1].c_str());
									}
									else if(parvect[0] == "pt")
									{
										if( !isNullOrWhiteSpace(parvect[1]) )
											Pt = atoi(parvect[1].c_str());;
									}
									else if(parvect[0] == "pn")
									{
										if( !isNullOrWhiteSpace(parvect[1]) )
											Pn = atoi(parvect[1].c_str());
									}
									else if(parvect[0] == "min")
									{
										min = parvect[1];
									}
									else if(parvect[0] == "max")
									{
										max = parvect[1];
									}
									else if(parvect[0] == "def")
									{
										def = parvect[1];
									}
									else if(parvect[0] == "mult")
									{
										mult = parvect[1];
									}
								}
							}
							//Log::DEBUG("befor min="+min+" befor max="+max);
							max = TBuffer::clear_start_end(max, '[', ']');
							min = TBuffer::clear_start_end(min, '[', ']');
							def = TBuffer::clear_start_end(def, '[', ']');
							mult = TBuffer::clear_start_end(mult, '[', ']');

							vector<string> maxvect = TBuffer::Split(max, ",");
							vector<string> minvect = TBuffer::Split(min, ",");
							vector<string> defvect = TBuffer::Split(def, ",");
							vector<string> multvect = TBuffer::Split(mult, ",");
							//Log::DEBUG( "min="+min+" size="+toString(minvect.size()) +" max="+max+" size="+toString(maxvect.size()) );
							for(int i=0;i< len; i++)
							{
								//Log::DEBUG("type="+type+" i="+ toString(i));
								IDatchik *dat = NULL;
								if(type == "analog")
								{
									float mi = -1;
									float ma = 1000;
									float def = 0.0;
									float mult = 1.0;
									if(i < minvect.size()){
										if(!isNullOrWhiteSpace(minvect[i]))
											mi = atof(minvect[i].c_str());
									}
									if(i < maxvect.size()){
										if(!isNullOrWhiteSpace(maxvect[i]))
											ma = atof(maxvect[i].c_str());
									}
									if(i < multvect.size()){
										if(!isNullOrWhiteSpace(multvect[i]))
											mult = atof(multvect[i].c_str());
									}
									if(i < defvect.size()){
										if(!isNullOrWhiteSpace(defvect[i]))
										{
											def = atof(defvect[i].c_str());
											//Log::DEBUG("def="+toString(def));
										}

									}
									if(mult == 0)
										mult = 1.0;
									dat = new TAnalogDatchik(Pt, Pn++, ma, mi, mult, def);
								}
								else if(type == "discrete")
								{
									//Log::DEBUG("discrete Pt="+toString(Pt)+" Pn="+ toString(Pn));
									dat = new TDiscreteDatchik(Pt, Pn++, false);
								}
								else
								{//constant
									float mult = 1;
									if(i < multvect.size())
									{
										if(!isNullOrWhiteSpace(multvect[i]))
											mult = atof(multvect[i].c_str());
									}
									if(mult == 0)
										mult = 1.0;
									dat = new TConstantDatchik(Pt, Pn++, mult);
								}
								if(dat!= NULL)
								{
									dat->Index 		= i;
									sdev->DatchikList.push_back( dat );
								}
							}
							if(sdev->DatchikList.size() == 0){
								delete sdev;
								sdev = NULL;
							}
							//sdev->cmd = GetCmdList(addr, func, reg, len);
							ret.push_back(sdev);
						}
					}
				}

			}
		}
	}

	return ret;
}



//*****************************************************************************
//***
//*****************************************************************************
TIbpManager::TIbpManager( void ):TFastTimer(3, &MilSecCount),
                                                                Fase(CREATE_CMD),
                                                                NewFase(99),
                                                                SubFase(IBP_INIT),
                                                                ControllerIndex(0),
                                                                Period(600),
                                                                DataLen(0),
                                                                fd(NULL),
                                                                Port(NULL)
{

  SetTimer(PERIUD_TIMER, 60000);
  SetTimer(ADDITIONAL_TIMER, 0);
  FirstInitFlg = true;
  ModifyTime = Period;
}
//=============================================================================
TIbpManager::~TIbpManager( void )
{
	try
	{
	  for(auto curr: Controllers)
	  {
		 delete curr;
	  }
	  Controllers.clear();
	}
	catch(exception &e)
	{
		Log::ERROR( e.what() );
	}
}
//=============================================================================
//=============================================================================
void TIbpManager::Init( /*IPort *CurrPort, */ void *config, void *f )
{

	vector<IbpSettings> sett 	= *(vector<IbpSettings> *)config;
	fd            			= (TFifo<string> *)f;
	  for(auto curr: sett)
	  {
		  Period =  curr.period;
		  Iibp *currController = NULL;//		new Fp1(name++, curr.address);
		  Log::DEBUG("TIbpManager::Init type="+toString((int)curr.type));
		  switch(curr.type)
		  {
		  case 1:
		  case 21:
			  currController = new Fp1(curr.ename, curr.address);
			  break;
		  case 2:
		  case 22:
			  currController = new Fp2(curr.ename, curr.address);
			  break;
		  case 3:
			  currController = new Emerson1(curr.ename, curr.address);
			  break;
		  case 23:
			  currController = new Emerson2(curr.ename, curr.address,
						  curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;
		  case 4:
			  currController = new Huawei(curr.ename, curr.address);
			  break;
		  case 24:
			  currController
				  = new Huawei2(curr.ename, curr.address,
						  curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;
		  case 25:
			  currController
				  = new EnatelSM32(curr.ename, curr.address,
						  curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;

		  case 6:
			  currController = new DeltaPsc1000(curr.ename, curr.address);
			  break;
		  case 26:
			  currController
				  = new DeltaPsc3(curr.ename, curr.address,
						  curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;


		  case 15:
			  currController = new LiFeAkb(curr.ename, curr.address);
			  break;


		  case 41:
			  currController = new UniversalUPS(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;
		  case 42:
			  currController = new LiebertUPS(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;
		  case 43:
			  currController = new LiebertAPM(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;

		  case 44:
			  currController = new HuaweiUPS(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;
		  case 45:
			  currController = new TsiBravoUPS(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap, SNMP_V2);
			  break;
		  case 46:
			  currController = new TsiBravoINV(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap, SNMP_V1);
			  break;



		  case 101://cond
			  currController = new CondLennox(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap, curr.type);
			  break;

		  case 102://XM
			  currController = new XmFast(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap, curr.type);
			  break;
		  case 103://XM2
			  currController = new XmFastComp2(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap, curr.type);
			  break;
		  case 110://cond
			  currController = new CondLiebert(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap, curr.type);
			  break;

		  case 201:
			  currController = new VertivPDU(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;

		  case 202:
			  currController = new ApcPDU(curr.etype, curr.ename, curr.address, curr.snmpsettings.read, curr.snmpsettings.write, curr.snmpsettings.trap);
			  break;
		  }

		  if(currController != NULL)
		  {
			  unsigned long adr;
			  adr=inet_addr(curr.portsettings.DeviceName.c_str());
			  if (adr != INADDR_NONE)
			  {//ip
				  currController->Port = new SocketPortUdp();
				  SocketSettings sett;
				  sett.IpAddress 				= curr.portsettings.DeviceName;
				  sett.IpPort 				= curr.portsettings.BaudRate;
				  sett.BindPort				= curr.portsettings.BaudRate2;
				  sett.SocketRecvTimeout 		= curr.portsettings.RecvTimeout;
				  sett.SocketSendTimeout 		= curr.portsettings.SendTimeout;
				  currController->Port->Init(&sett);

				  if(curr.type < 20)
					  curr.type+=20;
				//currController->PortIsLAN = true;
			  }
			  else
			  {//com
				  currController->Port = new ComPort();
					currController->Port->Init(&curr.portsettings);
			  }
			  //currController->SetPeriod( curr.period );
			  currController->settings 	= curr.portsettings;
			  currController->Type 				= curr.type;
			  currController->BetweenTimeout	= curr.betweentimeout;
			  currController->Ktran				= curr.ktran;
			  Port 								= currController->Port;
			  Controllers.push_back( currController );

	    	  int controllerPort = (int)currController->Port;
	    	  int managerPort = (int)Port;
	    	  //Log::DEBUG( "[IbpManager] Init currController="+ toString((int)currController) +" controllerPort="+toString(controllerPort) + " managerPort="+toString(managerPort));
		  }
	  }
		for(auto cr: Controllers)
		{

			Iibp *curr = (Iibp*)cr;
		    Log::DEBUG("TIbpManager curr:");
		    Log::DEBUG("{");
		    Log::DEBUG("	Type="+toString((int)curr->Type));
		    Log::DEBUG("	Addr="+toString((int)curr->Addr));
		    Log::DEBUG("	TIbpManager curr->comm_settings:");
		    Log::DEBUG("	DeviceName="+toString(curr->settings.DeviceName));
		    Log::DEBUG("	BaudRate="+toString((int)curr->settings.BaudRate));
		    Log::DEBUG("	DataBits="+toString((int)curr->settings.DataBits));
		    Log::DEBUG("	StopBit="+toString((int)curr->settings.StopBit));
		    Log::DEBUG("	Parity="+toString((int)curr->settings.Parity));
		    Log::DEBUG("	RecvTimeout="+toString(curr->settings.RecvTimeout));
		    Log::DEBUG("	Ktran="+toString(curr->Ktran) );
		    Log::DEBUG("	BetweenTimeout="+toString(curr->BetweenTimeout));
		    Log::DEBUG("}");
		}
}
//=============================================================================
IDevice* TIbpManager::GetSimple( Word index )
{
	IDevice *conroller = NULL;
  	  if(index < Controllers.size())
  	  {
  		conroller = Controllers[index];
  	  }
  return conroller;
}
//=============================================================================
IDevice* TIbpManager::GetControllerByEname( Byte ename )
{
	IDevice *conroller = NULL;

	for(auto curr: Controllers)
	{
  	  if(curr->En == ename)
  	  {
  		conroller = curr;
  		break;
  	  }
	}
  return conroller;
}
//=============================================================================
void TIbpManager::CreateCMD( void)
{
  int size = Controllers.size();
  if( size > 0){
    if( ControllerIndex >=  size){
    	ControllerIndex = (size - 1);
    }
    IDevice *CurrController = GetSimple(ControllerIndex);//new
    if(CurrController != NULL)
    {
      if(SubFase == IBP_INIT)
      {
    	  int controllerPort = (int)CurrController->Port;
    	  int managerPort = (int)Port;
    	  Log::DEBUG( "[IbpManager] CreateCMD CurrController="+ toString((int)CurrController) +" controllerPort="+toString(controllerPort) + " managerPort="+toString(managerPort));
    	  if( Port != CurrController->Port)
    	  {
    		  if(Port != NULL)
    			  Port->Close();
    		  Port = CurrController->Port;//change
    		  sleep(1);
    	  }
        CurrController->InitGPIO();
      }
      DataLen = CurrController->CreateCMD(SubFase, Buffer);
    }
    Fase    = SEND_CMD;
    //Log::DEBUG( "[IbpManager] CreateCMD");
  }
}
//=============================================================================
void TIbpManager::SendCMD( void )
{
  Log::DEBUG( "[IbpManager] IBP_SendCMD DataLen="+toString(DataLen));

  sWord st = Port->Send(Buffer, DataLen);
  if( st > 0 )
  {//
	  //Log::DEBUG( "[IbpManager] Send ok: [" + toString(Buffer) +"]"+" SubFase="+toString((int)SubFase));
	  DataLen = st;
/////////////////////
	  Byte bf[2] {0,0};
	  string sended = "";
	  for(int i = 0; i < DataLen; i++)
	  {
		  ByteToHex(bf, Buffer[i]);
		  sended += "0x"+string((char*)bf, 2)+" ";
	  }
	Log::DEBUG( "[TIbpManager] send ok: [" + sended +"] len="+toString(DataLen)+" SubFase="+toString((int)SubFase));

////////////////
    TBuffer::ClrBUF(Buffer, DataLen);
    Fase    = RECV_CMD;
  }
  else if(st == 0)
  {
	  DataLen = st; //!!!16.11.21 tk pri neotvete snmp parsilo zapros
	  Log::DEBUG( "[IbpManager] goto EXEC_CMD st="+toString(st));
	  Fase    = EXEC_CMD;
  }
  else
  {//!!!16.11.21 tk pri neotvete snmp parsilo zapros
	  DataLen = 1;
	  Log::DEBUG( "[IbpManager] goto EXEC_CMD st="+toString(st));
	  Fase    = EXEC_CMD;
  }
}
//=============================================================================
void TIbpManager::RecvCMD( void )
{
  static int 	_answerErrorCNT = 0;
  IDevice* 	CurrController 	= GetSimple(ControllerIndex);
  if(CurrController == NULL)
  {
	  ControllerIndex = 0;
	  Fase    = CREATE_CMD;
	  SubFase = IBP_INIT;
	  return;
  }
  Word  timeout = CurrController->BetweenTimeout;
  sWord RecvLen = CurrController->RecvData(Port, Buffer, sizeof(Buffer));
  if(RecvLen > 0 )
  {
	 //Log::DEBUG( "[IbpManager] RecvData ok: [" + toString(Buffer) +"] len="+toString(RecvLen)+" SubFase="+toString((int)SubFase));
    //Log::DEBUG("OK");
    //////////////////////////
   		  Byte bf[2] {0,0};
    		  string recv = "";
    		  for(int i = 0; i < RecvLen; i++)
    		  {
    			  ByteToHex(bf, Buffer[i]);
    			  recv += "0x"+string((char*)bf, 2)+" ";
    		  }
    Log::DEBUG( "[IbpManager] RecvData ok: [" + recv +"] len="+toString(RecvLen)+" SubFase="+toString((int)SubFase));

    //////////////////////
  	DataLen     = RecvLen;
    Fase        = EXEC_CMD;
  	_answerErrorCNT = 0;
    SetTimer( COMMON_TIMER, timeout);//500//1500
  }
  else
  {
	  if(_answerErrorCNT++ >= 10)
	  {
	        _answerErrorCNT       = 0;
	        CurrController->ErrCnt++;
	        if( CurrController->IsEnable == true && SubFase == IBP_INIT )//
	        {//
	        	if(CurrController->Type < 40)
	        	{
	  	          CurrController->ClearValues();
	        	}
	        	CurrController->Modify     = true;
	        	CurrController->IsEnable   = false;
	        	DateTime = SystemTime.GetTime();
	        }
	        Port->Close();
	        DataLen         = 0;
	        if( ++ControllerIndex >=  Controllers.size())//progon po ostalnim t.k. dalee
	        	ControllerIndex = 0;

	        SubFase     = IBP_INIT;
	        timeout = 30000;
	        Log::DEBUG( "[IbpManager]::IBP_RecvCMD _answerErrorCNT>= 10, wait 30s");
	  }
      else
      {
	      if(_answerErrorCNT == 5)
	      {
	    	  SubFase     = IBP_INIT;
	       	  Port->Close();
	       	  timeout = 10000;
	       	  Log::DEBUG( "[IbpManager]::IBP_RecvCMD _answerErrorCNT == 5, wait 10s");
	      }
      }
	  DataLen = 0;
	  Fase    = CREATE_CMD;
	  SetTimer( COMMON_TIMER, timeout);
  }
}
//=============================================================================
void TIbpManager::ExecCMD( void )
{
  static Byte 	_answerErrorCNT = 0;
  int size = Controllers.size();
  //Log::DEBUG("IBP_ExecCMD Controllers.size()="+toString((int)Controllers.size()));
  if( size > 0)
  {
	Log::DEBUG("IBP_ExecCMD ControllerIndex="+toString((int)ControllerIndex)+" SubFase="+toString((int)SubFase) + " DataLen="+toString(DataLen));
    if( ControllerIndex >=  size){ ControllerIndex = (size - 1); }
    IDevice *CurrController = GetSimple(ControllerIndex);//new
    Word  timeout = CurrController->BetweenTimeout;
    if( CurrController->ParsingAnswer(Buffer, DataLen, SubFase) )
    {
    	 _answerErrorCNT = 0;
    	  SubFase = CurrController->GetDataFromMessage(SubFase, Buffer, DataLen);
    	  //Log::DEBUG("IBP_ExecCMD SubFase="+toString((int)SubFase));
	      switch(SubFase)
	      {
	      default:
	      		break;

	      case IBP_SET_CMD:
		    	if(CurrController->CmdMessages.size() == 0)
		    		SubFase = IBP_EXIT;
	    	    //Log::DEBUG("IBP_SET_CMD cmd_size="+toString(CurrController->CmdMessages.size()));
	      		//SubFase = IBP_EXIT;
	      		break;

	        case IBP_EXIT:
	          if( CurrController->IsEnable == false)
	          {//
	        	 if(!FirstInitFlg)
	        	 {
	        		 DateTime = SystemTime.GetTime();
	        		 CurrController->Modify = true;
	        	 }
	        	 CurrController->IsEnable  = true;
	        	 Log::DEBUG("IBP_ExecCMD IBP_EXIT FASE resume IsEnable, FirstInitFlg="+toString(FirstInitFlg)+ ", Modify="+toString(CurrController->Modify));
	          }
	          CurrController->AckCnt++;
	          //Log::DEBUG("IBP_ExecCMD IBP_EXIT FASE ControllerIndex="+toString(ControllerIndex));

	          if( ++ControllerIndex >=  size){//normal inc
	            ControllerIndex = 0;
	            timeout = 5000;
				//if(Controllers.size() > 1)
				//	timeout = 10000;
	          }
	          SetTimer( COMMON_TIMER, timeout);
	          SubFase = IBP_INIT;
	          Port->Close();
	          //Log::DEBUG("IBP_EXIT FASE");
	          break;

	        case IBP_ERROR:
	        	CurrController->ErrCnt++;
		        if( ++ControllerIndex >=  size){//normal inc
		            ControllerIndex = 0;
		        }
		        SetTimer( COMMON_TIMER, 10000);
		        SubFase = IBP_INIT;
		        Port->Close();
		        Log::DEBUG("IBP_ERROR FASE");
	        	break;

	      }
    }
    else
    {
    	Log::DEBUG("[IbpManager]::IBP_ExecCMD ParsingAnswer=ERROR. SubFase="+toString((int)SubFase)+" _answerErrorCNT="+toString((int)_answerErrorCNT));
		Port->Close();
		timeout = 1000;
    	if(_answerErrorCNT++ >= 10)
		{
			_answerErrorCNT       = 0;
	        if( CurrController->IsEnable == true && SubFase == IBP_INIT )//
	        {//
	        	  if(CurrController->Type < 40)
	        	  {
	  	            CurrController->ClearValues();
	        	  }
		          CurrController->Modify     = true;
		          CurrController->IsEnable   = false;
		          DateTime = SystemTime.GetTime();
	        }
			Port->Close();
			timeout = 30000;
	        if( ++ControllerIndex >=  size)//progon po ostalnim t.k. dalee
	        	ControllerIndex = 0;
	        SubFase = IBP_INIT;
	        Log::DEBUG("[IbpManager]::IBP_ExecCMD ParsingAnswer=ERROR _answerErrorCNT>= 10, wait 30s");
		}
    	else
    	{
  	      if(_answerErrorCNT == 5)
  	      {
  	    	  SubFase     = IBP_INIT;
  	       	  Port->Close();
  	       	  timeout = 10000;
  	       	  Log::DEBUG( "[IbpManager]::IBP_ExecCMD ParsingAnswer _answerErrorCNT == 5, wait 10s");
  	      }
    	}
		SetTimer( COMMON_TIMER, timeout);
    }
  }
  Fase    = CREATE_CMD;
}
//=============================================================================
void TIbpManager::DoECMD( void *Par )
{
	SubFase = IBP_INIT;
	Fase    = CREATE_CMD;
}
//=============================================================================
void TIbpManager::DetectStates( void  )
{
	if( !Enable()) return;

	  bool flag = false;
	  if( GetTimValue(PERIUD_TIMER) <= 0 )
	  {
	    	DWord tim = SystemTime.GetGlobalSeconds();
	    	if(FirstInitFlg == true)
	    	{
	    		Log::DEBUG("IBP_DetectStates FirstInitFlg="+toString(FirstInitFlg));
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
	    			//Log::DEBUG("IBP_DetectStates err="+toString(err)+" ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
	    			flag     = true;
		    		ModifyTime = ((DWord)( (DWord)(tim / Period) + 1)) * Period;
		    		if(ModifyTime > MAX_SECOND_IN_DAY)
		    			ModifyTime = Period;
	    		}
	    	}
		  if( flag == true )
		  {
			  DateTime = SystemTime.GetTime();
			  Log::DEBUG("IBP_DetectStates Complite ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
			  for(auto curr: Controllers)
			  {
				  curr->Modify = true;//new
			  }
			  SetTimer( PERIUD_TIMER, 10000 );
		  }
		  else
		  {
			  SetTimer( PERIUD_TIMER, 500 );
			  for(auto curr: Controllers)
			  {
				  curr->Modify = curr->DetectStates();//new
			  }
		  }
	  }
}
//=============================================================================
void TIbpManager::CreateMessageCMD( void *Par )
{
	if( !Enable())
		return;

	if( Par != NULL )
	{
		if(ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0)
		{
			IDevice *CurrController = NULL;//new
			int size = Controllers.size();
			int index;
			string framStr = "";
			for( index = 0; index < size; index++ )
			{
				CurrController = GetSimple(index);
				if(CurrController == NULL) continue;
				if( CurrController->Modify == true )
				{
					Log::DEBUG( "[TIbpManager] IBP_CreateMessageCMD CurrCounterIndex="+toString(index));
					if(DateTime.Year == 0){
						DateTime = SystemTime.GetTime();}
					framStr += TBuffer::DateTimeToString( &DateTime )+">>";
					DateTime.Year  = 0;
					framStr += CurrController->GetStringValue();
					framStr += CurrController->GetAdvStringValue();
				  /*//go to GetAdvStringValue:
				  for ( auto &x:  CurrController->Rectifiers){
					  framStr += x.second->GetValueString();
				  }
				  framStr += CurrController->Alarms.GetValueString();
				  framStr += CurrController->PointSteps.GetValueString();*/

					framStr += CurrController->SiteInfo.GetValueString();
					framStr += WordToString(CurrController->IsEnable)+',';//mast be after SiteInfo !!

					framStr += "\r\n>,99,1,"+WordToString(CurrController->Type)+',';
					if(framStr.size() > 0)
					{
						TFifo<string> *framFifo = (TFifo<string> *)Par;
						framFifo->push( framStr );
					}
					framStr.clear();
					CurrController->Modify = false;
				}
			}//for
		}
	}
}
//=============================================================================
bool TIbpManager::Enable( void )
{
	return Controllers.size() > 0;
}
//=============================================================================
Byte TIbpManager::GetFase(void)
{
	if(NewFase != 99 && Fase == CREATE_CMD)
	{
		Fase = NewFase;
		NewFase = 99;
	}
	return Fase;
}
//=============================================================================
void TIbpManager::SetFase(Byte nfase)
{
	NewFase = nfase;
}
//=============================================================================
/*sub_cmd - index
 * cmdtype:
 *  0 - start test
 *  1- end test
 *  2 */
bool TIbpManager::SetNewIcmd(vector<IbpCmdParameter> cmdList)
{
	//Log::DEBUG("TEnergyCounterManager::SetNewIcmd size ="+ toString(cmdList.size()));

	bool ret = true;
	/*
	TRtcTime dt = SystemTime.GetTime();
	for(auto curr: cmdList)
	{
		TIbpCMD *newCmd = new TIbpCMD();
		newCmd->DateTime 		= dt;
		newCmd->ControllerInd   = curr.index;
		newCmd->Pn				= curr.pn;
		newCmd->SubSystemIndex 	= curr.sub_sys_index;
		newCmd->DataObject 		= curr.data_object;
		newCmd->DataElement 	= curr.data_element;
		newCmd->Data			= curr.Data;
		CMDMessages.push_back(newCmd);
		SetFase(DO_ECMD);
		//Log::DEBUG("TEnergyCounterManager cmd =1, ECMDMessages.size="+ toString(ECMDMessages.size()) +" Fase=" + toString((int)Fase));
	}*/

	return ret;
}
//=============================================================================
void TIbpManager::ChangeState(Word controllerIndex, Word newState)
{
	IDevice *Controller = GetSimple(controllerIndex);

	Log::DEBUG("TIbpManager::ChangeState controllerIndex="+toString(controllerIndex)+" Controller="+toString((int)Controller)+" newState="+toString((int)newState));
	if(Controller != NULL)
	{
		//Word currState = Controller->UE.State;
		//if(currState != newState){
			//Log::DEBUG("TIbpManager::ChangeState newState="+toString((Word)newState));
			Controller->ChangeState(newState);
		//}
	}
}
//=============================================================================
void TIbpManager::ChangeParameters(Word controllerIndex, vector<string> parameters)
{
	IDevice *Controller = GetSimple(controllerIndex);

	Log::DEBUG("TIbpManager::ChangeParameters controllerIndex="+toString(controllerIndex)+
			" Controller="+toString((int)Controller)+" parameters.size="+toString(parameters.size()));
	if(Controller != NULL){
		Controller->ChangeParameters(parameters);
	}
}




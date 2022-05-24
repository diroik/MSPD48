/*
 * InnerRs485module.cpp
 *
 *  Created on: Mar 15, 2017
 *      Author: user
 */

#include <InnerRs485module.h>

//*****************************************************************************
//***
//*****************************************************************************
//=============================================================================
Imodul::Imodul(Byte et, Byte en):Et(et), En(en), type(0)
{
	modify 		= false;
	Port 		= NULL;
	BetweenTimeout = 100;
	ErrCnt 		= 0;
	AckCnt 		= 0;
	MaxErrors 	= 5;
    ResetPort 	= 0;
    SelValue 	= 0;
    SelTimeout 	= 0;
    SelPorts 	= "";
    hasalarm 	= false;
    CurrentCmd 	= NULL;
    period 		= 1800;
    modifyTime 	= period;
}
//=============================================================================
Imodul::~Imodul()
{

}
//=============================================================================
void Imodul::SetPeriod(Word Period)
{
	period = Period;
}
//=============================================================================
Word Imodul::GetPeriod(void)
{
	return period;
}
//=============================================================================
vector<string> Imodul::GetStringValue(void)
{
	vector<string> Return;
	if(Devices.size() > 0)
	{
		string ReturnStr = "";
		string Header = WordToString(Et)+','+ WordToString(En)+',';

		TMamStruct aver;
		TMamStruct min;
		TMamStruct max;

		//aver.CalcMinAverMax(199.0);
		//min.CalcMinAverMax(199.0);
		//max.CalcMinAverMax(199.0);

		Word cnt = 0;
		Word err = 0;

		for(auto curr: Devices)
		{
			TMamStruct curm = curr->GetMinMax();
			if(curm.Divider > 0)
			{
				aver.CalcMinAverMax(curm.Aver);
				min.CalcMinAverMax(curm.Min);
				max.CalcMinAverMax(curm.Max);
			}

			cnt += curr->GetCnt();
			err += curr->GetErr();

			if(curr->GetModify())
			{
				string devstr = curr->GetValueString();
				string newdevstr = replaceAll(devstr, Header, "");

				bool cmp = StrCmp(devstr, newdevstr);
				//Log::DEBUG("Imodul::GetStringValue Devices devstr = [" + devstr + "] newdevstr = [" + newdevstr + "]");
				//Log::DEBUG("Imodul::GetStringValue Devices devstr = [" + devstr + "]");
				if( cmp == true)
				{
					//Log::DEBUG("Imodul::GetStringValue Return.push_back( devstr )=["+devstr+"]");
					if(!isNullOrWhiteSpace(devstr))
						Return.push_back( devstr );
				}
				else
				{
					//Log::DEBUG("Imodul::GetStringValue newdevstr != devstr ["+newdevstr+"]");
					ReturnStr += newdevstr;
				}
			}
		}

		Log::DEBUG("Imodul::GetStringValue ConfigDevices.size="+toString(ConfigDevices.size()));
		if(ConfigDevices.size() > 0)
		{//not write original, write config only
			ReturnStr = "";
			Return.clear();

			//Header = WordToString(Et)+','+ WordToString(En)+',';
			for(auto curr: ConfigDevices)
			{
				if(curr->GetModify())
				{
					string devstr = curr->GetValueString();
					//string newdevstr = replaceAll(devstr, Header, "");
					Return.push_back( devstr );
					//Log::DEBUG("Imodul::GetStringValue Return.push_back( devstr )=["+devstr+"]");
					//bool cmp = StrCmp(devstr, newdevstr);
					//Log::DEBUG("Imodul::GetStringValue ConfigDevices devstr = [" + devstr + "] newdevstr = [" + newdevstr + "]");
					//Log::DEBUG("Imodul::GetStringValue ConfigDevices devstr = [" + devstr + "]\r\n newdevstr=["+newdevstr+"]");
					/*
					if( cmp == true)
					{
						Log::DEBUG("Imodul::GetStringValue Return.push_back( devstr )=["+devstr+"]");
						if(!isNullOrWhiteSpace(devstr))
							Return.push_back( devstr );
					}
					else
					{
						Log::DEBUG("Imodul::GetStringValue newdevstr != devstr ["+newdevstr+"]");
						ReturnStr += newdevstr;
					}*/
				}
			}
		}

		if(!isNullOrWhiteSpace(ReturnStr))
		{
			int t = type;
			string ipstr = "";
			if(Port->PortIsLAN() == true)
			{
				ipstr = "\r\n>,"+WordToString(0)+','+ WordToString(1)+',';
				ipstr += Port->ToString()+',';
				t += 20;
			}

			string allstr = "";
			if(aver.Divider > 0 && min.Divider > 0 && max.Divider > 0)
			{
				allstr = "\r\n>,"+WordToString(99)+','+ WordToString(1)+',';
				allstr += FloatToString(aver.Aver, 0, 2)+','+ FloatToString(min.Min, 0, 2)+','+FloatToString(max.Max, 0, 2)+',';
				allstr += WordToString(cnt)+','+WordToString(err)+',';
			}
			allstr += "\r\n>,99,2,"+WordToString(t)+',';

			ReturnStr = Header+ipstr+ReturnStr+allstr;
			Return.push_back( ReturnStr );
		}

	}
	//string ReturnString = WordToString(Et)+","+WordToString(En)+",";
    //ReturnString += "\r\n>,49,1,"+WordToString(IsEnable)+",";
    //ReturnString += "\r\n>,99,1,"+WordToString(type)+',';
    //Return.push_back(ReturnString);
	return Return;
}
//=============================================================================
string Imodul::GetStringCfgValue(void)
{
	string Return;

	if( ConfigDevices.size() == 0)
	{
		for(auto curr: Devices){
			Return+=curr->GetConfigString();
		}
		Return += "\r\n";
	}
	for(auto curr: ConfigDevices)
	{
		Return+=curr->GetConfigString();
	}


	 return Return;
}
//=============================================================================
void  Imodul::InitGPIO()
{
	//Log::DEBUG("Imodul::InitGPIO");
	vector<string> ports = Split(SelPorts, " ");
	if(ports.size() > 0)
	{
		Word currvalue=0;
		for(int i = 0; i < ports.size(); i++)
		{
			string sendStr = "(echo "+toString(ports[i])+" > /sys/class/gpio/export) >& /dev/null";
			string valStr = "cat /sys/class/gpio/gpio"+ports[i]+"/value";
			Word val = atoi(sendToConsole(valStr).c_str());
			currvalue |= (val << i);
		}

		if(currvalue != SelValue)
		{
			Log::DEBUG("SelCurrGPIOvalue="+toString(SelValue));
			for(int i = 0; i < ports.size(); i++)
			{
				 string sendStr = "(echo "+toString(ports[i])+" > /sys/class/gpio/export) >& /dev/null";
				 sendToConsole(sendStr);
				 sendStr = "echo out > /sys/class/gpio/gpio"+toString(ports[i])+"/direction";
				 sendToConsole(sendStr);

				 Word portstate = ((SelValue >> i) & 1);
				 //Log::DEBUG("portstate="+toString(portstate));
				 sendStr = "echo "+toString(portstate)+" > /sys/class/gpio/gpio"+ports[i]+"/value";
				 sendToConsole(sendStr);
			}
			//sleep(1);
			if(ResetPort > 0)
			{
				 string sendStr = "(echo "+toString(ResetPort)+" > /sys/class/gpio/export) >& /dev/null";
				 sendToConsole(sendStr);
				 sendStr = "echo out > /sys/class/gpio/gpio"+toString(ResetPort)+"/direction";
				 sendToConsole(sendStr);

				 sendStr = "echo 1 > /sys/class/gpio/gpio"+toString(ResetPort)+"/value";//
				 sendToConsole(sendStr);
				 usleep(100000);
				 sendStr = "echo 0 > /sys/class/gpio/gpio"+toString(ResetPort)+"/value";//
				 sendToConsole(sendStr);
			}
			if(SelTimeout > 0)
				sleep(SelTimeout);
		}
	}
}
//=============================================================================
sWord  Imodul::SendData(IPort* port, Byte *Buf, Word Len)
{
	return port->Send(Buf, Len);
}

//=============================================================================
sWord Imodul::RecvData(IPort* port, Byte *Buf, Word MaxLen, Byte subfase)
{
	sWord RecvLen = 0;
	return RecvLen;
}
//=============================================================================
bool Imodul::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Len < 5) return false;
	//Byte MessCRC = BUF[Len-1];
	//Byte CalcCRC = TBuffer::Crc8( BUF, Len-1);

	//return MessCRC == CalcCRC;
	return true;
}
//=============================================================================
bool Imodul::ChangeParameter( Word reg, Word fun, string val )
{
	return true;
}
//=============================================================================
bool Imodul::DetectStates( void )
{
	bool ret = false;
	DWord tim = SystemTime.GetGlobalSeconds();
	long err = modifyTime - tim;

	if( (modifyTime <= tim) || (err > period*2) ){
		ret     	= true;
		modifyTime 	= ((DWord)( (DWord)(tim / period) + 1)) * period;
		if(modifyTime > MAX_SECOND_IN_DAY)
			modifyTime = period;
	}
	return ret;
}
//=============================================================================
string Imodul::GetDatchikValue(Word et, Word en, Word pt, Word pn)
{
	string ret = "";
	for(auto dev: Devices)
	{
		if(dev->Et.Etype == et && dev->Et.Ename)
		{
			DWord tmp = 0;
			for(auto d : dev->DatchikList)
			{
				if(d->Pt.Ptype == pt)
				{
					//Log::DEBUG("Imodul::GetDatchikValue pt="+toString(pt)+" type="+toString((int)d->Type));
					if(d->Type == 2)//discrete
					{
						DWord tl = atol(d->GetTinyValueString().c_str()) << d->Index;
						//Log::DEBUG("Imodul::GetDatchikValue tl="+toString(tl));
						tmp |= tl;
					}
					else
					{
						if(d->Pt.Pname == pn){
							ret = d->GetTinyValueString();
							break;
						}
					}
				}
			}
			ret = toString(tmp);
			break;
		}
	}
	return ret;
}
//=============================================================================
//=============================================================================
void Imodul::InitDevices( vector<string> cmds)
{
	Devices = analizeCommands(cmds);
}
//=============================================================================
void Imodul::InitConfigs( vector<string> cfgs)
{
	ConfigDevices = analizeCommands(cfgs);
}
//=============================================================================
vector<TSimpleDevice*> Imodul::analizeCommands(vector<string> cmds)
{
	vector<TSimpleDevice*> ret;
	for(auto dev: cmds)
	{
		vector<string> devstr = TBuffer::Split(dev, ":" );
		if(devstr.size() >= 4)
		{
			string addrstr	=	devstr[0];
			string funcstr	=	devstr[1];
			string regstr	=	devstr[2];
			string lenrstr	=	devstr[3];

			vector<string> parvect = TBuffer::Split(addrstr, "=" );
			if(parvect.size() >= 2)
			{
				int addr = atoi(parvect[1].c_str());
				parvect = TBuffer::Split(funcstr, "=");
				if(parvect.size() >= 2)
				{
					int func = atoi(parvect[1].c_str());
					parvect = TBuffer::Split(regstr, "=");
					if(parvect.size() >= 2)
					{
						int reg = atoi(parvect[1].c_str());
						parvect = TBuffer::Split(lenrstr, "=");
						if(parvect.size() >= 2)
						{
							int len = atoi(parvect[1].c_str());
							//AddToCmdList(addr, func, reg, len );
							if(devstr.size() > 4)
							{
								parvect = TBuffer::Split(devstr[4], "=");
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
									sdev->Address = addr;
									//Log::DEBUG("devstr.size()="+toString(devstr.size()));
									for(int i=5;i<devstr.size();i++)
									{
										parvect = TBuffer::Split(devstr[i], "=");
										if(parvect.size() >= 2)
										{
											if(parvect[0] == "et"){
												if( !isNullOrWhiteSpace(parvect[1]) )
													Et = atoi(parvect[1].c_str());
												//Log::DEBUG("Et="+parvect[1]);
											}
											else if(parvect[0] == "en"){
												if( !isNullOrWhiteSpace(parvect[1]) )
													En = atoi(parvect[1].c_str());
												//Log::DEBUG("En="+parvect[1]);
											}
											else if(parvect[0] == "pt"){
												if( !isNullOrWhiteSpace(parvect[1]) )
													Pt = atoi(parvect[1].c_str());
												//Log::DEBUG("Pt="+parvect[1]);
											}
											else if(parvect[0] == "pn"){
												if( !isNullOrWhiteSpace(parvect[1]) )
													Pn = atoi(parvect[1].c_str());
												//Log::DEBUG("Pn="+parvect[1]);
											}
											else if(parvect[0] == "min"){
												min = parvect[1];
												//Log::DEBUG("min="+parvect[1]);
											}
											else if(parvect[0] == "max"){
												max = parvect[1];
												//Log::DEBUG("max="+parvect[1]);
											}
											else if(parvect[0] == "def"){
												def = parvect[1];
												//Log::DEBUG("def="+parvect[1]);
											}
											else if(parvect[0] == "mult"){
												mult = parvect[1];
												//Log::DEBUG("mult="+parvect[1]);
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
										//Log::DEBUG("ModbusAsciiMkdd::ModbusAsciiMkdd type="+type+" i="+ toString(i));
										IDatchik *dat = NULL;
										if(type == "analog")
										{
											float mi = -1;
											float ma = 1000;
											float def = 0.0;
											float mult = 0.1;
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
											//Log::DEBUG("ModbusAsciiMkdd::ModbusAsciiMkdd discrete Pt="+toString(Pt)+" Pn="+ toString(Pn));
											dat = new TDiscreteDatchik(Pt, Pn++, false);
										}
										else
										{//constant
											//Log::DEBUG("ModbusAsciiMkdd::ModbusAsciiMkdd discrete Pt="+toString(Pt)+" Pn="+ toString(Pn));
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
											dat->Register 	= reg;
											dat->Index 		= i;
											sdev->DatchikList.push_back( dat );
										}
									}
									sdev->Et.Etype = Et;
									sdev->Et.Ename = En;

									if(sdev->DatchikList.size() == 0){
										delete sdev;
										sdev = NULL;
									}
									sdev->cmd = GetCmdList(addr, func, reg, len);
									ret.push_back(sdev);
								}
							}
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
//=============================================================================
ModbusAscii::ModbusAscii(Byte et, Byte en, vector<string> cmds, vector<string> cfg):Imodul(et, en)
{
	type = 1;
	//int in=0;
	InitDevices(cmds);
	InitConfigs(cfg);


	Log::DEBUG("ModbusAscii Devices.size="+toString(Devices.size()) + " ConfigDevices.size="+ toString(ConfigDevices.size()));
}
//=============================================================================
ModbusAscii::~ModbusAscii()
{

}
//=============================================================================
Word ModbusAscii::CreateCMD(Byte CMD, Byte *Buffer)
{
	Word DataLen = 0;
	Byte bf[] {0,0,0,0};

	if(CurrentCmd != NULL)
	{
		IModbusRegisterList *cmd = CurrentCmd;
		ushort *data = (ushort*)cmd->GetData();
		//Log::DEBUG("ModbusAscii::CreateCMD CurrentCmd addr="+toString(cmd->addr)+" reg="+toString(cmd->reg)+" fun="+toString(cmd->fun) + " data="+toString(*data));

		Buffer[DataLen++] = ':';
		ByteToHex(bf, cmd->addr);
		Buffer[DataLen++] = bf[0];
		Buffer[DataLen++] = bf[1];

		ByteToHex(bf, cmd->fun);
		Buffer[DataLen++] = bf[0];
		Buffer[DataLen++] = bf[1];

		WordToHex(bf, cmd->reg);
		Buffer[DataLen++] = bf[0];
		Buffer[DataLen++] = bf[1];
		Buffer[DataLen++] = bf[2];
		Buffer[DataLen++] = bf[3];

		WordToHex(bf, *data);
		Buffer[DataLen++] = bf[0];
		Buffer[DataLen++] = bf[1];
		Buffer[DataLen++] = bf[2];
		Buffer[DataLen++] = bf[3];

		ByteToHex(bf, CheckSum(&Buffer[1], DataLen-1));
		Buffer[DataLen++] = bf[0];
		Buffer[DataLen++] = bf[1];

		Buffer[DataLen++] = 0x0D;
		Buffer[DataLen++] = 0x0A;

		delete CurrentCmd;
		CurrentCmd = NULL;
	}
	else
	{
		//Log::DEBUG("ModbusAscii::CreateCMD size="+toString(Devices.size()) + " CMD="+toString((int)CMD));
		if(CMD >= 0 && CMD < Devices.size())
		{
			IModbusRegisterList *cmd = Devices[CMD]->cmd;
			if(cmd != NULL)
			{
				Buffer[DataLen++] = ':';
				ByteToHex(bf, cmd->addr);
				Buffer[DataLen++] = bf[0];
				Buffer[DataLen++] = bf[1];

				ByteToHex(bf, cmd->fun);
				Buffer[DataLen++] = bf[0];
				Buffer[DataLen++] = bf[1];

				WordToHex(bf, cmd->reg);
				Buffer[DataLen++] = bf[0];
				Buffer[DataLen++] = bf[1];
				Buffer[DataLen++] = bf[2];
				Buffer[DataLen++] = bf[3];

				WordToHex(bf, cmd->len);
				Buffer[DataLen++] = bf[0];
				Buffer[DataLen++] = bf[1];
				Buffer[DataLen++] = bf[2];
				Buffer[DataLen++] = bf[3];

				ByteToHex(bf, CheckSum(&Buffer[1], DataLen-1));
				//Log::DEBUG("CheckSum="+string((char*)bf,2));
				Buffer[DataLen++] = bf[0];
				Buffer[DataLen++] = bf[1];

				//Buffer[DataLen++] = '}';
				Buffer[DataLen++] = 0x0D;
				Buffer[DataLen++] = 0x0A;
			}
		}
	}
	//Log::DEBUG("ModbusAscii::CreateCMD size="+toString(Devices.size()) + " DataLen="+toString(DataLen));

	return DataLen;
}
//=============================================================================
Byte ModbusAscii::GetDataFromMessage(Byte CMD,Byte *Buffer, Word Len)
{
	string value = GetDataFromMessage(Buffer, Len);
	if(! isNullOrWhiteSpace(value) )
	{
		Log::DEBUG("GetDataFromMessage CMD="+toString((int)CMD)+" mess="+string((char*)Buffer,Len));
		if(CMD >= 0 && CMD < Devices.size() && Len >= 8)
		{
			int len = atoi(value.substr(4,2).c_str())/2;
			Log::DEBUG("len="+toString(len)+ "value="+value+" Len="+toString(Len));
			TSimpleDevice* currDev = Devices[CMD];
			IModbusRegisterList *cmd = currDev->cmd;
			if(cmd != NULL)
			{
				bool alarm = false;
				for(auto currDatchik: currDev->DatchikList)
				{
					Log::DEBUG( "Address="+ toString(cmd->addr) +"Register="+toString(currDatchik->Register)+" Index="+toString(currDatchik->Index));
					if(currDatchik->Register == cmd->reg)
					{
						int ind = 7+currDatchik->Index*4;
						if(cmd->fun == 1 || cmd->fun == 2)
							ind = 7;
						Log::DEBUG( "ind="+toString(ind)+" fun="+toString((int)cmd->fun) );
						if((ind+4) <= Len)
						{
							string strVal = string((char*)&Buffer[ind],4);
							Word val = HexToWord( strVal );//zavisimaya oshibka: val=0F00 ravno 15 (0F), a ne 3840 (0F00)!! v rele i discretax sdelel korrectirovku
							Log::DEBUG( "ModbusAsciiMkdd::GetDataFromMessage val="+toString(val) );
							currDatchik->CalculateValue((Word*)&val); // attention!! need cast to (Word*)!!!!!
							currDatchik->StateValue = currDatchik->DetectPorogs((Word*)&val);
							if(currDatchik->StateValue != currDatchik->HStateValue)
							{
								currDatchik->HStateValue = currDatchik->StateValue;
								currDatchik->Modify = true;
								if( currDatchik->Type == 3 )
									Log::DEBUG("ModbusAscii::DetectPorog val="+toString(val));
								alarm = true;
							}

							for(auto currCfg: ConfigDevices)
							{
								IModbusRegisterList *ccmd = currCfg->cmd;
								if( ccmd != NULL && ccmd->addr == cmd->addr && ccmd->fun == cmd->fun && ccmd->reg >= cmd->reg && ccmd->reg <= (cmd->reg + cmd->len))
								{
									if(ccmd->fun == 1 || ccmd->fun == 2)//discrete
									{
										Word cval = val >> (ccmd->reg - cmd->reg);
										Log::DEBUG( "ModbusAsciiMkdd::GetDataFromMessage val="+toString(val) + " ccmd->reg=" +toString(ccmd->reg) + " cmd->reg="+toString(cmd->reg) + " cval=" + toString(cval) );
										for(auto cDatchik: currCfg->DatchikList)
										{
											cDatchik->CalculateValue((Word*)&cval); // attention!! need cast to (Word*)!!!!!
											cDatchik->StateValue = cDatchik->DetectPorogs((Word*)&cval);
											if(cDatchik->StateValue != cDatchik->HStateValue)
											{
												cDatchik->HStateValue = cDatchik->StateValue;
												cDatchik->Modify = true;
											}
										}
										currCfg->IsEnable = true;
									}
									else if(ccmd->fun == 3)
									{
										int dreg = currDatchik->Register + currDatchik->Index;
										for(auto cDatchik: currCfg->DatchikList)
										{
											int creg = cDatchik->Register + cDatchik->Index;
											if(dreg == creg)
											{
												Log::DEBUG( "ModbusAsciiMkdd::GetDataFromMessage val="+toString(val)+" dreg=" +toString(dreg)+" creg="+toString(creg));
												cDatchik->CalculateValue((Word*)&val); // attention!! need cast to (Word*)!!!!!
												cDatchik->StateValue = cDatchik->DetectPorogs((Word*)&val);
												if(cDatchik->StateValue != cDatchik->HStateValue)
												{
													cDatchik->HStateValue = cDatchik->StateValue;
													cDatchik->Modify = true;
												}
												currCfg->IsEnable = true;
											}
										}
									}
								}
							}
						}
					}
				}
				hasalarm = alarm;
			}
		}
	}
	return CMD;
}
//=============================================================================
void ModbusAscii::ManageData(void)
{

}
//=============================================================================
sWord ModbusAscii::RecvData(IPort* port, Byte *Buf, Word MaxLen, Byte subfase)
{
	sWord RecvLen = 0;

	RecvLen = port->RecvTo( Buf, MaxLen, 0x0A);

	return RecvLen;
}
//=============================================================================
bool ModbusAscii::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Devices.size() > 0 && subFase < (Devices.size()-1))//
	{
		Log::DEBUG("ModbusAscii::ParsingAnswer Len="+toString(Len) + " subFase="+toString((int)subFase));
		if(Len > 10)
		{
			if(BUF[Len-2]==0x0D && BUF[Len-1]==0x0A)
			{
				int s = TBuffer::find_first_of(BUF, Len, ':');
				if(s >=0 && (Len-s) > 10)
				{
					Byte lrc = HexToByte(&BUF[Len-4]);
					Byte calcLrc = CheckSum(&BUF[s+1], Len-s-5);
					Log::DEBUG("ModbusAscii::ParsingAnswer lrc="+toString( (int)lrc) +" calcLrc="+toString((int)calcLrc) + "s="+toString(s)+" Len="+toString(Len)  );
					if(lrc == calcLrc)
					{
						return true;
					}
				}
			}
			return false;

		}
		else
		{
			return false;
		}
	}
	return true;
}
//=============================================================================
bool ModbusAscii::ChangeParameter( Word reg, Word fun, string val )
{
	return true;
}
//=============================================================================
//*****************************************************************************
//***
//*****************************************************************************
//=============================================================================
ModbusRtu::ModbusRtu(Byte et, Byte en, vector<string> cmds, vector<string> cfg):Imodul(et, en)
{
	type = 0;
	InitDevices(cmds);
	InitConfigs(cfg);

	Log::DEBUG("ModbusRtu Devices.size="+toString(Devices.size()) + " ConfigDevices.size="+ toString(ConfigDevices.size()));
}
//=============================================================================
ModbusRtu::~ModbusRtu()
{

}
//=============================================================================
Word ModbusRtu::CreateCMD(Byte CMD, Byte *Buffer)
{
	Word DataLen = 0;
	Bshort tmp;
	//Log::DEBUG("ModbusRtu::CreateCMD CMD="+toString((int)CMD)+" Devices.size()="+toString(Devices.size()));

	if(CurrentCmd != NULL)
	{
		IModbusRegisterList *cmd = CurrentCmd;
		Buffer[DataLen++] = cmd->addr;
		Buffer[DataLen++] = cmd->fun;

		tmp.Data_s = cmd->reg;
		Buffer[DataLen++] = tmp.Data_b[1];
		Buffer[DataLen++] = tmp.Data_b[0];

		tmp.Data_s = cmd->len;
		Buffer[DataLen++] = tmp.Data_b[1];
		Buffer[DataLen++] = tmp.Data_b[0];

		tmp.Data_s = CheckSum(Buffer, DataLen);
		Buffer[DataLen++] = tmp.Data_b[0];
		Buffer[DataLen++] = tmp.Data_b[1];

		delete CurrentCmd;
		CurrentCmd = NULL;
	}
	else
	{
		if(CMD >= 0 && CMD < Devices.size())
		{
			IModbusRegisterList *cmd = Devices[CMD]->cmd;
			if(cmd != NULL)
			{
				//Log::DEBUG("ModbusRtu::CreateCMD cmd="+cmd->GetCfgString());
				Buffer[DataLen++] = cmd->addr;
				Buffer[DataLen++] = cmd->fun;

				tmp.Data_s = cmd->reg;
				Buffer[DataLen++] = tmp.Data_b[1];
				Buffer[DataLen++] = tmp.Data_b[0];

				tmp.Data_s = cmd->len;
				Buffer[DataLen++] = tmp.Data_b[1];
				Buffer[DataLen++] = tmp.Data_b[0];

				tmp.Data_s = CheckSum(Buffer, DataLen);
				Buffer[DataLen++] = tmp.Data_b[0];
				Buffer[DataLen++] = tmp.Data_b[1];
			}
		}
	}
	return DataLen;
}
//=============================================================================
Byte ModbusRtu::GetDataFromMessage(Byte CMD,Byte *Buffer, Word Len)
{
	//Log::DEBUG("ModbusRtu::GetDataFromMessage Len="+toString(Len));

	if(CMD >= 0 && CMD < Devices.size() && Len > 5)
	{
		int addr = Buffer[0];
		int len = Buffer[2]/2;
		TSimpleDevice* currDev = Devices[CMD];
		IModbusRegisterList *cmd = currDev->cmd;
		if(cmd != NULL && cmd->addr == addr)
		{
			for(auto currDatchik: currDev->DatchikList)
			{
				//Log::DEBUG("ModbusRtu::GetDataFromMessage currDatchik>Register="+toString(currDatchik->Register));
				if(currDatchik->Register == cmd->reg)
				{
					int ind = 3+currDatchik->Index*2;
					if(cmd->fun == 2)
						ind = 3;
					if((ind+2) <= (Len-2))
					{
						Bshort tmp;
						tmp.Data_b[1] = Buffer[ind];
						tmp.Data_b[0] = Buffer[ind+1];
						Word val = tmp.Data_s;

						Log::DEBUG("ModbusRtu::GetDataFromMessage value="+toString(val));

						currDatchik->CalculateValue((Word*)&val); // attention!! need cast to (Word*)!!!!!
						currDatchik->StateValue = currDatchik->DetectPorogs((Word*)&val);
						if(currDatchik->StateValue != currDatchik->HStateValue)
						{
							currDatchik->HStateValue = currDatchik->StateValue;
							currDatchik->Modify = true;
							if( currDatchik->Type == 3 )
								Log::DEBUG("ModbusRtu::DetectPorog val="+toString(val));
						}
					}
				}
			}
		}
	}

	return CMD;
}
//=============================================================================
void ModbusRtu::ManageData(void)
{

}
//=============================================================================
sWord ModbusRtu::RecvData(IPort* port, Byte *Buf, Word MaxLen, Byte subfase)
{
	sWord RecvLen = 0;
	RecvLen = port->Recv( Buf, MaxLen);
	return RecvLen;
}
//=============================================================================
bool ModbusRtu::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Devices.size() > 0 && subFase < (Devices.size()-1))//
	{
		Log::DEBUG("ModbusRtu::ParsingAnswer Len="+toString(Len) + " subFase="+toString((int)subFase));
		if(Len > 5)
		{
			Bshort tmp;
			tmp.Data_b[0] = BUF[Len-2];
			tmp.Data_b[1] = BUF[Len-1];

			Word calcLrc = CheckSum(BUF, Len-2);
			Log::DEBUG("ModbusAscii::ParsingAnswer ModbusRtu::ParsingAnswer lrc="+toString( (int)tmp.Data_s) +" calcLrc="+toString((int)calcLrc) +" Len="+toString(Len)  );
			if(tmp.Data_s == calcLrc)
			{
				return true;
			}
			return false;
		}
		else
		{
			return false;
		}
	}
	return true;
}

//*****************************************************************************
//***
//*****************************************************************************
//=============================================================================
ModbusTcp::ModbusTcp(Byte et, Byte en, vector<string> cmds, vector<string> cfg):Imodul(et, en)
{
	type = 2;
	InitDevices(cmds);
	InitConfigs(cfg);

	TransactionId.Data_s = 0;
	ProtocolId.Data_s = 0;

	Log::DEBUG("ModbusTcp Devices.size="+toString(Devices.size()) + " ConfigDevices.size="+ toString(ConfigDevices.size()));
}
//=============================================================================
ModbusTcp::~ModbusTcp()
{

}
//=============================================================================
Word ModbusTcp::CreateCMD(Byte CMD, Byte *Buffer)
{
	Word DataLen = 0;
	Bshort tmp;
	Log::DEBUG("ModbusTcp::CreateCMD CMD="+toString((int)CMD)+" Devices.size()="+toString(Devices.size()));

	if(CurrentCmd != NULL)
	{
		IModbusRegisterList *cmd = CurrentCmd;
		Buffer[6 + DataLen++] = cmd->addr;
		Buffer[6 + DataLen++] = cmd->fun;

		tmp.Data_s = cmd->reg;
		Buffer[6 + DataLen++] = tmp.Data_b[1];
		Buffer[6 + DataLen++] = tmp.Data_b[0];

		tmp.Data_s = cmd->len;
		Buffer[6 + DataLen++] = tmp.Data_b[1];
		Buffer[6 + DataLen++] = tmp.Data_b[0];

		Len.Data_s = DataLen;
		++TransactionId.Data_s;
		Buffer[0] = TransactionId.Data_b[1];
		Buffer[1] = TransactionId.Data_b[0];

		Buffer[2] = ProtocolId.Data_b[1];
		Buffer[3] = ProtocolId.Data_b[0];

		Buffer[4] = Len.Data_b[1];
		Buffer[5] = Len.Data_b[0];

		DataLen += 6;

		delete CurrentCmd;
		CurrentCmd = NULL;
	}
	else
	{
		if(CMD >= 0 && CMD < Devices.size())
		{
			IModbusRegisterList *cmd = Devices[CMD]->cmd;
			if(cmd != NULL)
			{
				Log::DEBUG("ModbusTcp::CreateCMD cmd(GetCfgString)="+cmd->GetCfgString());
				//Log::INFO( "[IModbusRegisterList] object is: [" +ret + "]");
				Buffer[6 + DataLen++] = cmd->addr;
				Buffer[6 + DataLen++] = cmd->fun;

				tmp.Data_s = cmd->reg;
				Buffer[6 + DataLen++] = tmp.Data_b[1];
				Buffer[6 + DataLen++] = tmp.Data_b[0];

				tmp.Data_s = cmd->len;
				Buffer[6 + DataLen++] = tmp.Data_b[1];
				Buffer[6 + DataLen++] = tmp.Data_b[0];

				Len.Data_s = DataLen;
				++TransactionId.Data_s;
				Buffer[0] = TransactionId.Data_b[1];
				Buffer[1] = TransactionId.Data_b[0];

				Buffer[2] = ProtocolId.Data_b[1];
				Buffer[3] = ProtocolId.Data_b[0];

				Buffer[4] = Len.Data_b[1];
				Buffer[5] = Len.Data_b[0];

				DataLen += 6;
			}
		}
	}
	return DataLen;
}
//=============================================================================
Byte ModbusTcp::GetDataFromMessage(Byte CMD,Byte *Buffer, Word Len)
{
	//Log::DEBUG("ModbusTcp::GetDataFromMessage Len="+toString(Len));

	if(CMD >= 0 && CMD < Devices.size() && Len > 5)
	{
		int addr = Buffer[0 + 6];
		int len = Buffer[2 + 6]/2;
		TSimpleDevice* currDev = Devices[CMD];
		IModbusRegisterList *cmd = currDev->cmd;

		Log::DEBUG("ModbusTcp::GetDataFromMessage addr="+toString(addr) + " len=" + toString(len));
		if(cmd != NULL && cmd->addr == addr)
		{
			for(auto currDatchik: currDev->DatchikList)
			{
				//Log::DEBUG("ModbusTcp::GetDataFromMessage currDatchik>Register="+toString(currDatchik->Register) + " cmd->reg="+toString(cmd->reg));
				if(currDatchik->Register == cmd->reg)
				{
					int ind = 6+3+currDatchik->Index*2;//cmd->fun == 3
					if(cmd->fun == 2)
						ind = 6+3;
					else if(cmd->fun == 1)
					{
						ind = 6 + 3 + ((Word)(currDatchik->Index/16)) * 2;
					}

					if((ind+1) < Len)
					{
						Bshort tmp;
						tmp.Data_b[1] = Buffer[ind];
						tmp.Data_b[0] = Buffer[ind+1];

						if(cmd->fun == 1){
							tmp.Data_b[0] = Buffer[ind];
							tmp.Data_b[1] = Buffer[ind+1];
						}

						Word val = tmp.Data_s;
						currDatchik->CalculateValue((Word*)&val); // attention!! need cast to (Word*)!!!!!
						/*
						Log::DEBUG("ModbusTcp::GetDataFromMessage currDatchik->Index=" +toString(currDatchik->Index)+
								" ind="+toString(ind)+
								" val=" + toString(val)+
								" value=["+currDatchik->GetValueString()+"]");*/

						currDatchik->StateValue = currDatchik->DetectPorogs((Word*)&val);
						if(currDatchik->StateValue != currDatchik->HStateValue)
						{
							currDatchik->HStateValue = currDatchik->StateValue;
							currDatchik->Modify = true;
							if( currDatchik->Type == 3 )
								Log::DEBUG("ModbusTcp::DetectPorog val="+toString(val));
						}
					}
				}
			}
		}
	}
	return CMD;
}
//=============================================================================
void ModbusTcp::ManageData(void)
{

}
//=============================================================================
sWord ModbusTcp::RecvData(IPort* port, Byte *Buf, Word MaxLen, Byte subfase)
{
	sWord RecvLen = 0;
	RecvLen = port->Recv( Buf, MaxLen);
	return RecvLen;
}
//=============================================================================
bool ModbusTcp::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
{
	if(Devices.size() > 0 && subFase < (Devices.size()-1))//
	{
		Log::DEBUG("ModbusTcp::ParsingAnswer Len="+toString(Len) + " subFase="+toString((int)subFase));
		if(Len >= 11)
		{
			Bshort tmp;
			Short needId = TransactionId.Data_s;// - 1;

			tmp.Data_b[1] = BUF[0];
			tmp.Data_b[0] = BUF[1];

			Log::DEBUG("ModbusTcp::ParsingAnswer needId="+toString(needId) + " recvId="+toString(tmp.Data_s));

			return needId == tmp.Data_s;
		}
		else
		{
			return false;
		}
	}
	return true;
}

///*****************************************************************************
//***
//*****************************************************************************
TInnerRs485Manager::TInnerRs485Manager( void ):TFastTimer(3,&MilSecCount),
												//Fase(CREATE_CMD),
												Fase(RECV_CMD),
												SubFase(0),
												Period(600),
												Aperiod(60),
												DataLen(0),
												//fd(NULL),
												Port(NULL)
{
		ControllerIndex = 0;
		SetTimer(PERIUD_TIMER, 60000);
		SetTimer(ADDITIONAL_TIMER, 5000);
		FirstInitFlg = true;
		ModifyTime = Period;
}
//=============================================================================
TInnerRs485Manager::~TInnerRs485Manager()
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
void TInnerRs485Manager::Init( void *config, void *driver )
{
  vector<ModuleSettings> sett = *((vector<ModuleSettings> *)config);
  Log::DEBUG("Init TInnerRs485Manager sett.size="+toString(sett.size()));

  for(auto curr: sett)
  {
	 Imodul *modul = NULL;
	 Log::DEBUG("Init TInnerRs485Manager etype="+toString((int)curr.etype)+" ename="+toString(curr.ename));
	 switch(curr.type)
	 {
			case 0:
				modul = new ModbusRtu(curr.etype, curr.ename, curr.commands, curr.configs);
				break;
			case 1:
				modul = new ModbusAscii(curr.etype, curr.ename, curr.commands, curr.configs);
				break;
			case 2:
				modul = new ModbusTcp(curr.etype, curr.ename, curr.commands, curr.configs);
				break;
	 };
	 if(modul != NULL)
	 {
		 if(modul->Port == NULL)
		 {
			unsigned long adr;
			adr = inet_addr( curr.portsettings.DeviceName.c_str() );
			if (adr != INADDR_NONE)
			{//ip
				modul->Port = new SocketPort();
				SocketSettings ssett;
				ssett.IpAddress = curr.portsettings.DeviceName;
				ssett.IpPort = curr.portsettings.BaudRate;
				//ssett.BindPort			= curr.portsettings.BaudRate2;
				ssett.SocketRecvTimeout = curr.portsettings.RecvTimeout;
				ssett.SocketSendTimeout = curr.portsettings.SendTimeout;
				modul->Port->Init(&ssett);
			}
			else
			{//com
				modul->Port = new ComPort();
				modul->Port->Init(&curr.portsettings);
			}
			//modul->settings 	= curr.portsettings;
		 }

				/*//old
		 if(modul->Port == NULL)
		 {
			 modul->Port = new ComPort();
			 modul->Port->Init(&curr.portsettings);
			 modul->settings 	= curr.portsettings;
		 }*/

		 Port = modul->Port;
		 Period  = curr.period;
		 Aperiod = curr.aperiod;

		 modul->SetPeriod( curr.period );
		 modul->BetweenTimeout 	= curr.betweentimeout;
		 modul->MaxErrors 		= curr.maxerrors;
		 modul->ResetPort		= curr.resetport;
		 modul->SelValue		= curr.selvalue;
		 modul->SelTimeout		= curr.seltimeout;
		 modul->SelPorts		= curr.selports;

   	  //int controllerPort = (int)modul->Port;
   	  //int managerPort = (int)Port;
   	  //Log::DEBUG( "[TInnerRs485Manager] Init Devices.size=" + toString(modul->Devices.size()) + " currController="+ toString((int)modul) +" controllerPort="+toString(controllerPort) + " managerPort="+toString(managerPort));


		 Controllers.push_back(modul);

		 Log::DEBUG("\r\nInit TInnerRs485Manager.Controller:\r\n type="+toString((int)curr.type)+" et="+
				 toString((int)modul->Et)+" en="+toString((int)modul->En) + " commands="+modul->GetStringCfgValue());

	  }

  }
  Log::DEBUG("Init TInnerRs485Manager Controllers.size="+toString(Controllers.size())+" Period="+toString(Period)+" Aperiod="+toString(Aperiod));
}
//=============================================================================
bool TInnerRs485Manager::Enable( void )
{
	return Controllers.size() > 0;
}
//=============================================================================
IPort *TInnerRs485Manager::GetPort()
{
	  IPort*ret = NULL;
	  pthread_mutex_lock(&sych);
	  ret = Port;
	  pthread_mutex_unlock(&sych);
	  return ret;
}
//=============================================================================
Imodul *TInnerRs485Manager::GetSimple( Word index )
{
	Imodul *conroller = NULL;
  	if(index < Controllers.size())
  	{
  		conroller = Controllers[index];
  	}
  return conroller;
}
//=============================================================================
void TInnerRs485Manager::CreateCMD( void)
{
	int size = Controllers.size();
	if( size > 0)
	{
	  if( ControllerIndex >=  size)
	  {
	  	ControllerIndex = 0;
	  }
	  Imodul *Controller = GetSimple(ControllerIndex);

	  //Log::DEBUG("TInnerRs485Manager::CreateCMD size="+toString(size) + " ControllerIndex="+toString(ControllerIndex)+ " Controller="+toString((int)Controller));
	  if(Controller != NULL)
	  {
	      if(SubFase == 0)
	      {
	    	  //int controllerPort = (int)Controller->Port;
	    	  //int managerPort = (int)Port;
	    	  //Log::DEBUG( "[IbpManager] CreateCMD CurrController="+ toString((int)Controller) +" controllerPort="+toString(controllerPort) + " managerPort="+toString(managerPort));

	    	  if( Controller->Port != Port )
	    	  {
	    		  if(Port != NULL)
	    			  Port->Close();
	    		  Port = Controller->Port;//change
	    		  //sleep(1);
	    	  }
	    	  Controller->InitGPIO();
	      }
	     DataLen = Controller->CreateCMD(SubFase, Buffer);
	   }
	  //Log::DEBUG( "[TInnerRs485Manager] CreateCMD DataLen="+toString(DataLen)+" SubFase="+toString((int)SubFase)+"ControllerIndex="+toString(ControllerIndex));
	}
    Fase    = SEND_CMD;
}
//=============================================================================
void TInnerRs485Manager::SendCMD( void )
{
	Imodul *Controller = GetSimple(ControllerIndex);
	//Log::DEBUG( "[TInnerRs485Manager] SendCMD DataLen="+toString(DataLen)+" SubFase="+toString((int)SubFase)+" ControllerIndex="+toString(ControllerIndex));
	if(Controller != NULL)
	{

		if( Controller->SendData(Port, Buffer, DataLen) > 0 )
		{//
			//string sended = string( (char*)Buffer, DataLen );
			//sended = replaceAll(sended, "\r\n", "");
			//Log::DEBUG( toString(MilSecCount.Value)+" [TInnerRs485Manager] send ok: ["+ sended + "] len="+toString(DataLen)+" subFase="+toString((int)SubFase));
			/////////////////////
				  Byte bf[2] {0,0};
				  string sended = "";
				  for(int i = 0; i < DataLen; i++)
				  {
					  ByteToHex(bf, Buffer[i]);
					  sended += "0x"+string((char*)bf, 2)+" ";
				  }
				Log::DEBUG( "[TInnerRs485Manager] send ok: [" + sended +"] len="+toString(DataLen)+" SubFase="+toString((int)SubFase));
			//////////////////////////

			TBuffer::ClrBUF(Buffer, DataLen);
	    	Fase    = RECV_CMD;
		}
		else
		{
			Fase    = EXEC_CMD;
		}
	}
	else
	{
		  ControllerIndex = 0;
		  Fase    = CREATE_CMD;
		  SubFase = 0;
	}
}
//=============================================================================
void TInnerRs485Manager::RecvCMD( void )
{
	static int 	_answerErrorCNT = 0;
	Imodul *Controller = GetSimple(ControllerIndex);
	if( Controller != NULL)
	{
		Word  timeout = Controller->BetweenTimeout;
		sWord RecvLen = Controller->RecvData(Port, Buffer, sizeof(Buffer), SubFase);
		//Log::DEBUG( toString(MilSecCount.Value)+" [TInnerRs485Manager] RecvData len="+toString(RecvLen)+" subFase="+toString((int)SubFase));
		if(RecvLen > 0 )
		{

			//string rcvstr = string( (char*)Buffer, RecvLen );
			//rcvstr = replaceAll(rcvstr, "\r\n", "");
			//Log::DEBUG( toString(MilSecCount.Value)+" [TInnerRs485Manager] RecvData ok: ["+ rcvstr+ "] len="+toString(RecvLen)+" subFase="+toString((int)SubFase));

			//////////////////////////////////////////////
	   		  Byte bf[2] {0,0};
	    		  string recv = "";
	    		  for(int i = 0; i < RecvLen; i++)
	    		  {
	    			  ByteToHex(bf, Buffer[i]);
	    			  recv += "0x"+string((char*)bf, 2)+" ";
	    		  }
	    		  Log::DEBUG( "[TInnerRs485Manager] RecvData ok: [" + recv +"] len="+toString(RecvLen)+" SubFase="+toString((int)SubFase));

			//////////////////////
			  DataLen     = RecvLen;
			  Fase        = EXEC_CMD;
			  _answerErrorCNT = 0;
			  SetTimer( COMMON_TIMER, timeout);
		}
		else
		{
			Log::ERROR("[TInnerRs485Manager] READ ERROR _answerErrorCNT="+toString(_answerErrorCNT));
			if(_answerErrorCNT++ >= Controller->MaxErrors)
			{
				  _answerErrorCNT       	= 0;
				  Controller->ErrCnt++;

				  //Log::ERROR("[TInnerRs485Manager] READ ERROR Controller: En="+ toString((int)Controller->En)+" SubFase="+toString((int)SubFase));
				  if( Controller->GetIsEnable(SubFase) == true)
				  {
					  //Log::ERROR("[TInnerRs485Manager] READ ERROR SetModify(true) IsEnable(SubFase, false) SubFase="+toString((int)SubFase));
				  	  DateTime = SystemTime.GetTime();
				  	  Controller->SetModify(SubFase, true);
				  	  Controller->SetIsEnable( SubFase, false);

				  	  if(Controller->CurrentCmd != NULL)
				  	  {
				  			int addr = Controller->CurrentCmd->addr;
				  			for(auto currCfg: Controller->ConfigDevices)
				  			{
				  				if(currCfg->cmd != NULL && currCfg->cmd->addr == addr)
				  				{
				  					currCfg->IsEnable = false;
				  				}
				  			}
				  	  }

				  }
				  SubFase++;
			  }

			  DataLen = 0;
			  Fase    = CREATE_CMD;
			  SetTimer( COMMON_TIMER, timeout);
		  }
	}
	else
	{
		  ControllerIndex = 0;
		  Fase    = CREATE_CMD;
		  SubFase = 0;
	}
}
//=============================================================================
void TInnerRs485Manager::ExecCMD( void )
{
	static Byte 	_answerErrorCNT = 0;
	Imodul *Controller = GetSimple(ControllerIndex);
	if( Controller != NULL)
	{
		//Log::DEBUG("TInnerRs485Manager::ExecCMD SubFase="+toString((int)SubFase)+", ControllerIndex="+toString((int)ControllerIndex));
	    if( Controller->ParsingAnswer(Buffer, DataLen, SubFase) )
	    {
	    	_answerErrorCNT = 0;
	    	Controller->GetDataFromMessage(SubFase, Buffer, DataLen );
	    	//Log::DEBUG("TInnerRs485Manager::ExecCMD SubFase="+toString((int)SubFase));
	    	if( Controller->GetIsEnable(SubFase) == false)
	    	{
	    		if(!FirstInitFlg)
	    		{
	    			DateTime = SystemTime.GetTime();
	    			Controller->SetModify(SubFase, true);
	    		}
	    		Controller->SetIsEnable(SubFase, true);
	    		//Log::DEBUG("TInnerRs485Manager::ExecCMD IsEnable(SubFase, true)");
	    	}
	    	Controller->AckCnt++;

	    	int lastCmd = Controller->GetLastCmd();
		    if(SubFase >= lastCmd)//add >= was ==
		    {//last cmd
		    	if( ++ControllerIndex >=  Controllers.size())
			    {
			      Port->Close();//add
			      ControllerIndex = 0;
			      SetTimer( COMMON_TIMER, Controller->BetweenTimeout*2);
			    }
			    else
			    {
			      SetTimer( COMMON_TIMER, Controller->BetweenTimeout);
			    }
			    SubFase = 0;
		    }
		    else
		    {
		    	SubFase++;
		    }
		    Controller->ManageData();
	    }
	    else
	    {
	    	Log::DEBUG( "[TInnerRs485Manager] ParsingAnswer ERROR");
			if(_answerErrorCNT++ >= 10)
			{
				_answerErrorCNT       = 0;
				Controller->ErrCnt++;
				if( Controller->GetIsEnable(SubFase) == true)
				{
				  	  DateTime = SystemTime.GetTime();
				  	  Controller->SetModify(SubFase, true);
				  	  Controller->SetIsEnable( SubFase, false);
				}

				Port->Close();
				SetTimer( COMMON_TIMER, 10000);
		        if( ++ControllerIndex >=  Controllers.size())//progon po ostalnim t.k. dalee
		        	ControllerIndex = 0;
		        SubFase = 0;
			}
	    }
	}
	else
	{
		  ControllerIndex = 0;
		  SubFase = 0;
	}
	Fase    = CREATE_CMD;
}
//=============================================================================
void TInnerRs485Manager::DetectStates( void  )
{
	if(! Enable()) return;

	  bool flag = false;
	  if( GetTimValue(PERIUD_TIMER) <= 0 )
	  {
		    DWord tim = SystemTime.GetGlobalSeconds();
		    Word period = Period;
	    	if(FirstInitFlg == true)
	    	{
	    		FirstInitFlg = false;
	    		flag     = true;
	    		ModifyTime = ((DWord)( (DWord)(tim / period) + 1)) * period;
	    		if(ModifyTime > MAX_SECOND_IN_DAY) ModifyTime = period;
	    	}
	    	else
	    	{
	    		long err = ModifyTime-tim;
	    		if( (ModifyTime <= tim) || (err > period*2) )
	    		{
	    			flag     = true;
		    		ModifyTime = ((DWord)( (DWord)(tim / period) + 1)) * period;
		    		if(ModifyTime > MAX_SECOND_IN_DAY) ModifyTime = period;
	    		}
	    	}
	    	if( flag == true )
	    	{
			  //Log::DEBUG("TInnerRs485Manager::DetectStates SetModify(true) Period="+toString(period) + " ModifyTime="+toString(ModifyTime)+" tim="+toString(tim) );
			  DateTime = SystemTime.GetTime();
			  for(auto curr: Controllers){
				  curr->SetModify(true);
			  }
			  SetTimer( PERIUD_TIMER, 30000 );
	    	}
	    	else
	    	{
	    		SetTimer( PERIUD_TIMER, 500 );
				for(auto curr: Controllers){
					curr->SetModify( curr->DetectStates() );//new
				}
	    	}
	  }
}
//=============================================================================
void TInnerRs485Manager::CreateMessageCMD( void *Par )
{
if( Par != NULL && Enable() )
  {
    if(ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0)
    {
      Imodul *CurrController = NULL;//new
      int size = Controllers.size();
      int index;
      string framStr = "";
      for( index = 0; index < size; index++ )
      {
    	CurrController = GetSimple(index);
    	if(CurrController == NULL) continue;
        if( CurrController->GetModify() )
        {
        	  if(DateTime.Year == 0)
        		  DateTime = SystemTime.GetTime();
              string DateStr = TBuffer::DateTimeToString( &DateTime )+">>";
              DateTime.Year  = 0;

              for( auto framStr: CurrController->GetStringValue() )
              {
				  if(framStr.size() > 0){
					 TFifo<string> *framFifo = (TFifo<string> *)Par;
					 framFifo->push( DateStr+framStr );
				  }
              }
              CurrController->SetModify(false);
        }
      }
    }
  }
}
//=============================================================================
void TInnerRs485Manager::ChangeParameters(Word et, Word en, vector<tuple<int, int,string>> parameters)
{
	int size = Controllers.size();
	if( size > 0)
	{
	   for(int index = 0; index < size; index++)
	   {
		   Log::DEBUG("TInnerRs485Manager::ChangeParameters index="+toString(index));
		   Imodul *Controller = GetSimple(index);
		   if(Controller != NULL)
		   {
			   if(Controller->Et == et && Controller->En == en)
			   {
				   ushort releval = atoi(Controller->GetDatchikValue(et, en, 201, 1).c_str()) >> 8 ;
				   Log::DEBUG("TInnerRs485Manager::ChangeParameters et="+toString(et)
						   + " en="+toString(en)  + " index="+toString(index) +" parameters.size="+toString(parameters.size())+ " cur releval="+toString(releval));

				   bool releflg = false;
				   for(auto p : parameters)
				   {
					   int pt = std::get<1>(p);
					   int pn = std::get<0>(p);
					   ushort val = atoi(std::get<2>(p).c_str());

					   if(pt == 201 && pn > 8)
					   {
						   releflg = true;
						   Log::DEBUG("TInnerRs485Manager::ChangeParameters pt="+toString(pt) + " pn" + toString(pn) + " val="+toString(val));
						   if(val == 0)
							   releval &=  ~( 1 << (pn-9) );
						   else
							   releval |=  1 << (pn-9);
					   }
				   }
				   if(releflg)
				   {
					   Log::DEBUG("TInnerRs485Manager::ChangeParameters new releval="+toString(releval));
					   TSimpleDevice *dev = Controller->GetDevice(et, en);
					   if(dev != NULL)
					   {
						   Word reg = 0x4004;
						   Word addr = dev->Address;

						   Controller->CurrentCmd = new ModbusWriteSingleRegister(addr, reg, releval, NULL );
						   Fase    = CREATE_CMD;

						   SubFase = 0;//Controller->GetLastCmd() + 1;
					   }
				   }
				   break;
			   }
		   }
	   }
	}
}
//=============================================================================
void TInnerRs485Manager::ChangeController(void)
{
	   int size = Controllers.size();
	   if( size > 0)
	   {
		  if( ++ControllerIndex >=  Controllers.size())
		  {
			  ControllerIndex = 0;
		  }
		  Imodul *CurrController = GetSimple(ControllerIndex);//new
		  if(CurrController != NULL)
		  {
		   	  //int controllerPort = (int)CurrController->Port;
		   	  //int managerPort = (int)Port;
		   	  //Log::DEBUG( "[TInnerRs485Manager] Init currController="+ toString((int)CurrController) +" controllerPort="+toString(controllerPort) + " managerPort="+toString(managerPort));
				  if( CurrController->Port != Port )
				  {
					  if(Port != NULL)
					  {
						Port->Close();
					  }
					  Port = CurrController->Port;//change
				  }
		  }
	   }
	  	//SetTimer( COMMON_TIMER, 1000);
	  	SubFase = 0;
	  	Fase    = CREATE_CMD;
}
//=============================================================================

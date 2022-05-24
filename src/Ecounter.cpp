/*
 * Ecounter.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: user
 */

#include <Ecounter.h>

	//*****************************************************************************
	//*** ICounter class
	//*****************************************************************************
	ICounter::ICounter(Byte Ename, Byte adr):Addr(adr),
											Et(41),
											En(Ename),
											Modify(false),
											IsEnable(false),
											E(40,1),E1(40,2),E2(40,3),E3(40,4),E4(40,5),E5(40,6)
	{
		Index = 0;
		Type = 0;
		isQ = false;

		BetweenTimeout = 100;
		cntPort = NULL;
		ErrCnt = 0;
		AckCnt = 0;
		Poverka = "";
		plc 		= false;
		cmd 		= "";

		KtranI = 0;
		KtranU = 0;
		LastTime = "";
		timezone = 0;
		index = 0;
		ProfilCounter = 0;
		NeedFirstFlg = true;
		hexPassword = false;
	}
	ICounter::~ICounter()
	{
	}
	//------------------------------------------------------------------------------
	string ICounter::GetStringValue(void)
	{
		string ReturnString = WordToString(Et)+","+
								WordToString(En)+","+
								  E.GetValueString()+
									U.GetValueString()+
									  I.GetValueString()+
										P.GetValueString()+
										  Q.GetValueString()+
											S.GetValueString()+
											  KM.GetValueString()+
											  	  F.GetValueString();

		//Poverka

			  return ReturnString;
	}
	//------------------------------------------------------------------------------
	string ICounter::GetStringCfgValue(void)
	{
		 string ReturnString = "";
		 return ReturnString;
	}
	//=============================================================================
	bool ICounter::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
	{
	  if(subFase == SET_PLC){
		  return true;
	  }

	  if(Len == 0) {
		  return true;
	  }

	  if(subFase == GET_E5 ){
		  //Log::DEBUG("[Counter] ParsingAnswer Len="+toString(Len)+" subFase=GET_E5");
		  return true;
	  }

	  if(subFase == GET_EISP ){
		  //Log::DEBUG("[Counter] ParsingAnswer Len="+toString(Len)+" subFase=GET_EISP");
		  return true;
	  }

	  /*
	  if(Len == 4 && BUF[1] == 0x01)
	  {
		  Log::DEBUG("Len="+toString(Len)+" Cmd Not Supported");
		  return true;
	  }*/

	  if(Len < 3)
	  {
		return false;
	  }
	  else
	  {
		Bshort MessCRC;

		MessCRC.Data_b[0] = BUF[Len-2];
		MessCRC.Data_b[1] = BUF[Len-1];
		Short CalcCRC = Crc16(BUF, Len-2);
		//Log::DEBUG("[Counter] Len="+toString(Len)+" ParsingAnswer CalcCRC="+toString((int)CalcCRC) + " MessCRC="+toString(MessCRC.Data_s) );

		return MessCRC.Data_s == CalcCRC;
	  }
	}
	//*****************************************************************************
	//*** TMercury230Counter class
	//*****************************************************************************
	TMercury230Counter::TMercury230Counter(Byte Ename, string adr):ICounter(Ename, 0 )//, IsEnable(false)
	{
	  int a = atoi(adr.c_str())%1000;
	  Log::DEBUG("TMercury230Counter::TMercury230Counter a%1000="+toString(a));
	  if(a > 240)
		  a = a%100;
	  Log::DEBUG("TMercury230Counter::TMercury230Counter a%100="+toString(a));

	  Addr = a;
	  OrigAddr = a;

	  Modify = false;
	  Type = 0;
	  isQ = false;
	}
	//=============================================================================
	TMercury230Counter::~TMercury230Counter( )
	{
	}
	//=============================================================================
	Word TMercury230Counter::CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *ecmd)
	{
	 Word DataLen = 0;
	 Bshort       Tmp;

	    switch(CMD)
	    {
	      case SET_PLC: // test connection
	        if(Index == 0)
	        {
	        	switch(plc)
	        	{
					case 1://nzif
				          Buffer[DataLen++] = 0xFC;
				          Buffer[DataLen++] = 0x00;
				          Buffer[DataLen++] = 0x00;
				          Buffer[DataLen++] = 0x00;
				          Buffer[DataLen++] = 0x00;
				          Buffer[DataLen++] = 0x03;
				          Buffer[DataLen++] = 0x2E;
				          Buffer[DataLen++] = 0x0B;
				          Buffer[DataLen++] = 0x02;
				          Buffer[DataLen++] = 0x42;
				          Buffer[DataLen++] = 0x3A;
				          Buffer[DataLen++] = 0x35;
				          Buffer[DataLen++] = 0xC7;
				          Tmp.Data_s = Crc16(Buffer, DataLen);
				          Buffer[DataLen++] = Tmp.Data_b[0];
				          Buffer[DataLen++] = Tmp.Data_b[1];
						break;
					case 2://icbcom1f
						break;

					case 3://icbcom3f
			        	if(cmd.size() > 0 && cmd.size() % 2 == 0)
			        	{
			        		for(int ind = 0; ind < cmd.size()/2; ind++)
			        		{
			        			Buffer[DataLen++] = HexToByte( (char*)&cmd.c_str()[ind*2] );
			        		}
			    	        //Tmp.Data_s = Crc16(Buffer, DataLen);
			    	        //Buffer[DataLen++] = Tmp.Data_b[0];
			    	        //Buffer[DataLen++] = Tmp.Data_b[1];
			        	}
						break;
					case 4://rm
			        	if(cmd.size() > 0)
			        	{
			        		for(int ind = 0; ind < cmd.size(); ind++){
			        			Buffer[DataLen++] = cmd.c_str()[ind];
			        		}
			        	}
			        	break;
	        	};
	        }
	        break;

	      case OPEN_CHANNAL:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 0x01;

	    	if(ecmd!=NULL && ecmd->Password.size() > 0)
	    	{
	    		Buffer[2] = 0x02;
	    		for(auto currB: ecmd->Password)
	    			Buffer[DataLen++] = currB;
	    		ecmd->Password.clear();
	    	}
	    	else{
		        for(int i = 0; i < 6;i++){
		        	Buffer[DataLen++] = 0x01 + (hexPassword ? 0x30: 0);
		        }

	    	}

	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_EISP:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x26;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_ETYPE:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x12;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_E://
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        for(int i = 0; i < 2;i++)
	        { Buffer[DataLen++] = 0x00;}
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_ER:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        for(int i = 0; i < 2;i++)
	        { Buffer[DataLen++] = 0x00;}
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_E1:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        Buffer[DataLen++] = 0x00;
	        Buffer[DataLen++] = 0x01;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_E2:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        Buffer[DataLen++] = 0x00;
	        Buffer[DataLen++] = 0x02;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_E3:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        Buffer[DataLen++] = 0x00;
	        Buffer[DataLen++] = 0x03;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_E4:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        Buffer[DataLen++] = 0x00;
	        Buffer[DataLen++] = 0x04;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_E5:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        Buffer[DataLen++] = 0x00;
	        Buffer[DataLen++] = 0x05;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_Ef:
	        if(isQ)
	        {
				Buffer[DataLen++] = Addr;
				Buffer[DataLen++] = 0x05;
				Buffer[DataLen++] = 0x60;
				Buffer[DataLen++] = 0x00;
				Tmp.Data_s = Crc16(Buffer, DataLen);
				Buffer[DataLen++] = Tmp.Data_b[0];
				Buffer[DataLen++] = Tmp.Data_b[1];
	        }
	        break;
	      case GET_E1f:
	        if(isQ)
	        {
				Buffer[DataLen++] = Addr;
				Buffer[DataLen++] = 0x05;
				Buffer[DataLen++] = 0x60;
				Buffer[DataLen++] = 0x01;
				Tmp.Data_s = Crc16(Buffer, DataLen);
				Buffer[DataLen++] = Tmp.Data_b[0];
				Buffer[DataLen++] = Tmp.Data_b[1];
	        }
	        break;
	      case GET_E2f:
	        if(isQ)
	        {
				Buffer[DataLen++] = Addr;
				Buffer[DataLen++] = 0x05;
				Buffer[DataLen++] = 0x60;
				Buffer[DataLen++] = 0x02;
				Tmp.Data_s = Crc16(Buffer, DataLen);
				Buffer[DataLen++] = Tmp.Data_b[0];
				Buffer[DataLen++] = Tmp.Data_b[1];
	        }
	        break;
	      case GET_E3f:
	        if(isQ)
	        {
				Buffer[DataLen++] = Addr;
				Buffer[DataLen++] = 0x05;
				Buffer[DataLen++] = 0x60;
				Buffer[DataLen++] = 0x03;
				Tmp.Data_s = Crc16(Buffer, DataLen);
				Buffer[DataLen++] = Tmp.Data_b[0];
				Buffer[DataLen++] = Tmp.Data_b[1];
	        }
	        break;
	      case GET_E4f:
	        if(isQ)
	        {
				Buffer[DataLen++] = Addr;
				Buffer[DataLen++] = 0x05;
				Buffer[DataLen++] = 0x60;
				Buffer[DataLen++] = 0x04;
				Tmp.Data_s = Crc16(Buffer, DataLen);
				Buffer[DataLen++] = Tmp.Data_b[0];
				Buffer[DataLen++] = Tmp.Data_b[1];
	        }
	        break;
	      case GET_U:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x16;
	        Buffer[DataLen++] = 0x11;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_I:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x16;
	        Buffer[DataLen++] = 0x21;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_P:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x16;
	        Buffer[DataLen++] = 0x00;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_Q:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x16;
	        Buffer[DataLen++] = 0x04;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_S:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x16;
	        Buffer[DataLen++] = 0x08;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_KM:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x16;
	        Buffer[DataLen++] = 0x30;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_F:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x16;
	        Buffer[DataLen++] = 0x40;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_KTRAN:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x02;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_NETADDR:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x05;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_SERIAL:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x00;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case CLOSE_CHANNAL:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x02;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case SET_ECMD:
	    	  if(ecmd!=NULL && ecmd->Data.size() > 1)
	    	  {
	    		  for(auto currB: ecmd->Data)
	    		  {
	    			  Buffer[DataLen++] = currB;
	    		  }
	    		  Tmp.Data_s = Crc16(Buffer, DataLen);
	    		  Buffer[DataLen++] = Tmp.Data_b[0];
	    		  Buffer[DataLen++] = Tmp.Data_b[1];
	    	  }
	    	  break;

	      default:
	        DataLen = 0;
	        break;
	    };

	    if(DataLen > 0)
	    {
			if(CMD != SET_PLC && plc == 3)
			{
				Byte* tmpBUF = new Byte[DataLen+5];
				Word dataInd = 0;
				tmpBUF[dataInd++] = 0x0A;
				tmpBUF[dataInd++] = 0x61;
				tmpBUF[dataInd++] = DataLen;

				for(int i=0; i < DataLen; i++){
					tmpBUF[dataInd++] = Buffer[i];
				}
		        Tmp.Data_s = Crc16(tmpBUF, dataInd);
		        tmpBUF[dataInd++] = Tmp.Data_b[0];
		        tmpBUF[dataInd++] = Tmp.Data_b[1];
				for(int i=0; i < dataInd; i++){
					Buffer[i] = tmpBUF[i];
				}
				DataLen = dataInd;
				delete [] tmpBUF;
			}
	    }
	    return DataLen;
	}
	//=============================================================================
	Word TMercury230Counter::GetRecvMessageLen(Byte CMD)
	{
	  Word Len = 0;
	  switch(CMD)
	  {
	    case SET_PLC:
		  if(Index == 0)
		  {
	        	switch(plc)
	        	{
					case 1://nzif
						Len = 8;
						break;
					case 2://icbcom1f
						break;
					case 3://icbcom3f
						Len = 100;
						break;
					case 4://rm
						//Len = 100;
			        	break;
	        	};
		  }
	      break;

	    case OPEN_CHANNAL:
	      Len = 4;break;

	    case GET_EISP:
	      Len = 5;break;

	    case GET_ETYPE:
	      Len = 9;break;

	    case GET_E:
	    case GET_ER:
	    case GET_E1:
	    case GET_E2:
	    case GET_E3:
	    case GET_E4:
	    case GET_E5:
	      Len = 19;break;

	    case GET_Ef:
	    case GET_E1f:
	    case GET_E2f:
	    case GET_E3f:
	    case GET_E4f:
	        if(isQ)
	        	Len = 15;
	        else
	        	Len = 0;
	      break;
	    case GET_U:
	    case GET_I:
	      Len = 12;break;

	    case GET_P:
	    case GET_Q:
	    case GET_S:
	    case GET_KM:
	      Len = 15;break;

	    case GET_F:
	      Len = 6;break;

	    case GET_SERIAL:
	      Len = 10;break;

	    case GET_KTRAN:
	      Len = 7;break;

	    case GET_NETADDR:
	      Len = 5;break;

	    case CLOSE_CHANNAL:
	      Len = 4;break;

	    case SET_ECMD:
	    	Len = 100;
	    	break;

	    default:
	      break;
	  };

	  //if(CMD == SET_PLC) Len = 0;
	  //else  Len = 100;///////////

		 if(CMD != SET_PLC && plc == 3 && Len > 0)
		 {
			 Len += 5;
		 }

	  return Len;
	}
	//=============================================================================
	sWord   TMercury230Counter::RecvData(IPort* port, Byte *Buf, sWord DataLen, Word MaxLen, Byte subfase, sWord &refLen)
	{
	  //Log::DEBUG("RecvData DataLen"+toString((int)DataLen)+" subfase="+toString((int)subfase));
	  return port->Recv( Buf, DataLen );//
	}
	//=============================================================================
	string TMercury230Counter::GetStringValue(void)
	{
		//string ReturnString = WordToString(Et)+","+WordToString(En)+","+ E.GetValueString();
		string ReturnString = WordToString(Et)+","+WordToString(En)+",";
		if(IsEnable)
		{
			ReturnString += E.GetValueString();
			ReturnString += E1.GetValueString();
			ReturnString += E2.GetValueString();
			ReturnString += E3.GetValueString();
			ReturnString += E4.GetValueString();
			ReturnString += E5.GetValueString();
		}

		ReturnString += CouterType.GetValueString();

		if(NeedFirstFlg == true){
			ReturnString += U.GetValueString();
			ReturnString += I.GetValueString();
			ReturnString += P.GetValueString();
			ReturnString += Q.GetValueString();
			ReturnString += S.GetValueString();
			ReturnString += KM.GetValueString();
			ReturnString += F.GetValueString();
		}
		ReturnString += SerialN.GetValueString()+WordToString(IsEnable);

		if(KtranI  > 0)
		{
			ReturnString +=",\r\n>,231,16,"+WordToString(KtranI)+',';
			if(KtranU  > 0){
				ReturnString += WordToString(KtranU)+',';
			}
		}
		ReturnString +="\r\n>,99,1,"+WordToString(Type)+',';
		return ReturnString;
	}
	//=============================================================================
	bool TMercury230Counter::SetNewEcmd(TRtcTime &dt, EcounterCmdParameter curr)
	{
		bool ret = false;
		Log::DEBUG("addr="+ toString((int)curr.addr)+" cmd=" + toString((int)curr.cmd)+" sub_cmd=" + toString((int)curr.sub_cmd)+" parameters.size=" + toString(curr.parameters.size()));
		Byte cmd = curr.cmd;
		switch(cmd)
		{
			case 1:
			{//this year
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0x10);
				newCmd->Data.push_back(curr.sub_cmd);//t1,t2,t3....
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
				//Log::DEBUG("TEnergyCounterManager cmd =1, ECMDMessages.size="+ toString(ECMDMessages.size()) +" Fase=" + toString((int)Fase));
			}
				break;
			case 2:
			{//prev year
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0x20);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;
			case 3:
			{//mon
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				Byte c = 0x30 + curr.parameters[0];
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(c);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;
			case 4:
			{//this day
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0x40);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;
			case 5:
			{//prev day
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0x50);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;

			case 12:
			{//this day start
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0xC0);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
			}
				break;
			case 13:
			{//prev day start
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0xD0);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
			}
				break;


			case 40:
			{//get time
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 11;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(cmd/10);//04
				newCmd->Data.push_back(curr.sub_cmd);//00
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;

			default:
				break;
		};
		return ret;
	}
	//=============================================================================
	bool TMercury230Counter::GetDateTime(void)
	{
		Log::DEBUG("TMercury230Counter::GetDateTime");
		TRtcTime dt = SystemTime.GetTime();
		TEnergyCMD *newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->DateTime = dt;
		newCmd->Type = 40;
		newCmd->RetLen = 11;
		newCmd->Data.push_back(Addr);//addr
		newCmd->Data.push_back(40/10);//04
		newCmd->Data.push_back(0);//00
		ECMDMessages.push_back(newCmd);

		return true;
	}
	//=============================================================================
	bool TMercury230Counter::CorrectDateTime(string time)
	{//ss:mm:hh
		bool ret = false;
		vector<string> data = Split(time, ":");
		if(data.size() == 3)
		{
			TRtcTime dt = SystemTime.GetTime();
			TEnergyCMD *newCmd = new TEnergyCMD();
			newCmd->Etype = Et;
			newCmd->Ename = En;
			newCmd->DateTime = dt;
			newCmd->Type = 0x0D;//set or correct
			newCmd->RetLen = 4;
			newCmd->Data.push_back(Addr);//addr
			newCmd->Data.push_back(0x03);//cmd=write
			newCmd->Data.push_back(0x0D);//sub_cmd=correct time
			for(auto curr: data)
			{
				newCmd->Data.push_back( (Byte)HexToByte((char*)curr.c_str()) );
			}
			ECMDMessages.push_back(newCmd);
			ret = true;
		}
		return ret;
	}
	//=============================================================================
	bool TMercury230Counter::SetDateTime(string datetime, string password)
	{//ss:mm:hh
		bool ret = false;
		vector<string> dt = Split(datetime, " ");
		Log::DEBUG( "SetDateTime datetime=["+datetime+"]");
		if(dt.size() == 2)
		{
			string date = dt[0];
			string time = dt[1];

			vector<string> data = Split(time, ":");
			vector<string> data1 = Split(date, ".");
			if(data.size() == 3 && data1.size() == 4)
			{
				TRtcTime dt = SystemTime.GetTime();
				for(int i = 0; i < 1; i++)
				{
					TEnergyCMD *newCmd = new TEnergyCMD();
					newCmd->Etype = Et;
					newCmd->Ename = En;
					newCmd->DateTime = dt;
					newCmd->Type = 0x0D;
					newCmd->RetLen = 4;
					newCmd->Data.push_back(Addr);//addr
					newCmd->Data.push_back(0x03);//cmd=write
					newCmd->Data.push_back(0x0C);//sub_cmd=set time
					for(auto curr: data)
					{
						newCmd->Data.push_back( (Byte)HexToByte((char*)curr.c_str()) );
					}
					for(auto curr: data1)
					{
						newCmd->Data.push_back( (Byte)HexToByte((char*)curr.c_str()) );
					}
					newCmd->Data.push_back(0x01); // winter
					if(password.size() == 6)
					{
						newCmd->Password.clear();
						Log::DEBUG( "SetDateTime password=["+password+"]");

						for(int i=0; i < password.size(); i++)
							newCmd->Password.push_back( password.c_str()[i] - 0x30 );
					}
					ECMDMessages.push_back(newCmd);
				}

				ret = true;
			}
		}
		return ret;
	}
	//=============================================================================
	bool TMercury230Counter::GetLastProfile(void)
	{
		Log::DEBUG("TMercury230Counter::GetLastProfile");
		TRtcTime dt = SystemTime.GetTime();
		TEnergyCMD *newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->DateTime = dt;
		newCmd->Type = 0x13;
		newCmd->RetLen = 12;
		newCmd->Data.push_back(Addr);//addr
		newCmd->Data.push_back(0x08);//
		newCmd->Data.push_back(0x13);//
		ECMDMessages.push_back(newCmd);
		return true;
	}
	//=============================================================================
	bool TMercury230Counter::GetProfileByAddr(int ind, unsigned int address, string needdate, bool withopen, bool withclose)
	{
		Bshort TMP;
		//Log::DEBUG("TMercury230Counter::GetProfileByAddr Address ="+toString((unsigned int)address) + " withopen="+toString(withopen)+ " withclose="+toString(withclose));
		TRtcTime dt = SystemTime.GetTime();
		TEnergyCMD *newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->Index = ind;
		newCmd->DateTime = dt;
		newCmd->Type = 0x63;
		newCmd->RetLen = 33;
		newCmd->Data.push_back(Addr);//addr
		newCmd->Data.push_back(0x06);//


		//newCmd->Data.push_back(address > 0xFFFF ? 0x83 : 0x03);//

		newCmd->Data.push_back(  (address > 0xFFFF && address <= 0x1FFFF) ? 0x83 : 0x03);//
		TMP.Data_s = address > 0xFFFF ? (address - 0x10000) :  address;



		newCmd->Data.push_back(TMP.Data_b[1]);//
		newCmd->Data.push_back(TMP.Data_b[0]);//

		newCmd->Data.push_back(0x1E);//30 min

		newCmd->Tmp[0] = CouterType.Const;
		newCmd->Tmp[1] = CouterType.UTC;
		newCmd->TmpStr[0] = needdate;

		newCmd->WithOpenChanal = withopen;
		newCmd->WithCloseChanal = withclose;//!!

		ECMDMessages.push_back(newCmd);
		return true;
	}
	//=============================================================================
	bool TMercury230Counter::GetStartOfLastDayEnergy(void)
	{
		Log::DEBUG("TMercury230Counter::GetStartOfLastDayEnergy");
		TRtcTime dt = SystemTime.GetTime();
		TEnergyCMD *newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->DateTime = dt;
		newCmd->Type = 0xD0;
		newCmd->RetLen = 19;
		newCmd->Data.push_back(Addr);//addr
		newCmd->Data.push_back(0x05);//
		newCmd->Data.push_back(0xD0);//
		newCmd->Data.push_back(0x00);// tarif summ
		ECMDMessages.push_back(newCmd);
		return true;
	}
	//=============================================================================
	void TMercury230Counter::GetEisp(Byte *Buffer, Word len)
	{
		if(len == 5)
		{
			if(Buffer[1] == 0x08 && Buffer[2] == 0x26){
				CouterType.Model = "M233";
				return;
			}
			Bshort TMP;
			TMP.Data_b[1] = Buffer[1];
			TMP.Data_b[0] = Buffer[2];

			//Log::DEBUG("GetEisp="+toString(TMP.Data_s));

			if(TMP.Data_s == 0x7EF5 || TMP.Data_s == 0x657A){
				CouterType.Model = "M234";
			}
			else if(TMP.Data_s == 0x5E41 || TMP.Data_s == 0x4E51){
				CouterType.Model = "M236";
			}
			else if(TMP.Data_s == 0x27E1)
			{
				CouterType.Model = "M230";
			}
			else if(TMP.Data_s == 0xA27F || TMP.Data_s == 0x57AC){
				CouterType.Model = "M231";
			}
			else
			{
				CouterType.Model = "M230";
			}
		}
	}
	//=============================================================================
	void TMercury230Counter::GetType(Byte *Buffer, Word len)
	{
		if(len > 5)
		{
		  Byte type = Buffer[3] & 0x0F;
		  string Tmp = isNullOrWhiteSpace(CouterType.Model) ? "M230" : CouterType.Model;

		  //Tmp += "ART";
		  Tmp += "A";
		  if( ((Buffer[3] >> 4) & 1) == 0)
			  Tmp+= "R";
		  if(Buffer[3] & (1 << 6))
			  Tmp+= "T";

		  Word cont = Buffer[2] & 0x0F;
		  switch(cont)
		  {
		  case 0:
			  CouterType.Const = 5000;
			  break;
		  case 1:
			  CouterType.Const = 25000;
			  break;
		  case 2:
			  CouterType.Const = 1250;
			  break;
		  case 3:
			  CouterType.Const = 500;
			  break;
		  case 4:
			  CouterType.Const = 1000;
			  break;
		  case 5:
			  CouterType.Const = 250;
			  break;

		  }

		  if(type > 0)
			  type--;
		  Tmp += WordToString(type, 2)+"";
		  if(Buffer[2] & (1 << 5)){
			  Tmp+= "P";}
		  if(Buffer[5] & (1 << 1)){
			  Tmp+= "Q";
			  isQ = true;
		  }
		  else{
			  isQ = false;
		  }
		  Byte inter = (Buffer[4] >> 2) & 0x03;
		  switch(inter)
		  {
		  case 0:
			  Tmp+= "C";
			  break;
		  case 1:
			  Tmp+= "R";
			  break;
		  };
		  if(Buffer[5] & (1 << 2)){
			  Tmp+= "S";}
		  if(Buffer[4] & (1 << 4)){
			  Tmp+= "I";}
		  if(Buffer[4] & (1 << 5)){
			  Tmp+= "G";}
		  if(Buffer[4] & (1 << 6)){
			  Tmp+= "L";}
		  if(Buffer[4] & 1 ){
			  Tmp+= "N";}
		  CouterType.Type = Tmp;
		}
	}
	//=============================================================================
	void TMercury230Counter::GetEa(Byte *Buffer, Word len)
	{
	  if(len > 4)
	  {
		  Blong Tmp;
		  Tmp.Data_b[0] = Buffer[3];
	      Tmp.Data_b[1] = Buffer[4];
	      Tmp.Data_b[2] = Buffer[1];
	      Tmp.Data_b[3] = Buffer[2];
	      float val = Tmp.Data_l/1000.0;
	      if(val < 3000000.0)
	    	  E.Ea = val;
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetEa_minus(Byte *Buffer, Word len)
	{
	  if(len > 8)
	  {
		  Blong Tmp;
	          Tmp.Data_b[0] = Buffer[7];
	          Tmp.Data_b[1] = Buffer[8];
	          Tmp.Data_b[2] = Buffer[5];
	          Tmp.Data_b[3] = Buffer[6];
	          if(	Tmp.Data_b[0] == 0xFF &&
	        		Tmp.Data_b[1] == 0xFF &&
	        		Tmp.Data_b[2] == 0xFF &&
	        		Tmp.Data_b[3] == 0xFF)
	          {	  Tmp.Data_l = 0; }
	          //E.Ea_minus = Tmp.Data_l/1000.0;

		      float val = Tmp.Data_l/1000.0;
		      if(val < 3000000.0)
		    	  E.Ea_minus = val;
	  }
	}
	//=============================================================================
	float TMercury230Counter::GetEt(int tindex, Byte etype, Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  switch(etype)
	  {
	  case 0:
		  if(len > 4)
		  {
			  Tmp.Data_b[0] = Buffer[3];
		      Tmp.Data_b[1] = Buffer[4];
		      Tmp.Data_b[2] = Buffer[1];
		      Tmp.Data_b[3] = Buffer[2];
		  }
		  break;

	  case 1:
		  if(len > 12)
		  {
		       Tmp.Data_b[0]     = Buffer[11];
		       Tmp.Data_b[1]     = Buffer[12];
		       Tmp.Data_b[2]     = Buffer[9];
		       Tmp.Data_b[3]     = Buffer[10];
		  }
		  break;
	  };
	  return Tmp.Data_l/1000.0;
	}
	//=============================================================================
	float TMercury230Counter::GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len)
	{
	  Blong Tmp;

	  switch(etype)
	  {
	  case 0:
		  if(len > 8)
		  {
		          Tmp.Data_b[0] = Buffer[7];
		          Tmp.Data_b[1] = Buffer[8];
		          Tmp.Data_b[2] = Buffer[5];
		          Tmp.Data_b[3] = Buffer[6];
		          if(	Tmp.Data_b[0] == 0xFF &&
		        		Tmp.Data_b[1] == 0xFF &&
		        		Tmp.Data_b[2] == 0xFF &&
		        		Tmp.Data_b[3] == 0xFF)
		          {	  Tmp.Data_l = 0; }

		  }
		  break;

	  case 1:
		  if(len > 16)
		  {
		          Tmp.Data_b[0]     = Buffer[15];
		          Tmp.Data_b[1]     = Buffer[16];
		          Tmp.Data_b[2]     = Buffer[13];
		          Tmp.Data_b[3]     = Buffer[14];
		          if(	Tmp.Data_b[0] == 0xFF &&
		        		Tmp.Data_b[1] == 0xFF &&
		        		Tmp.Data_b[2] == 0xFF &&
		        		Tmp.Data_b[3] == 0xFF)
		          {	  Tmp.Data_l = 0; }
		  }
		  break;
	  };
	  return Tmp.Data_l/1000.0;
	}

	//=============================================================================
	void TMercury230Counter::GetEr(Byte *Buffer, Word len)
	{
	  if(len > 12){
		  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[11];
	          Tmp.Data_b[1]     = Buffer[12];
	          Tmp.Data_b[2]     = Buffer[9];
	          Tmp.Data_b[3]     = Buffer[10];
	          //E.Er = Tmp.Data_l/1000.0;

		      float val = Tmp.Data_l/1000.0;
		      if(val < 3000000.0)
		    	  E.Er = val;
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetEr_minus(Byte *Buffer, Word len)
	{
	  if(len > 16)
	  {
		  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[15];
	          Tmp.Data_b[1]     = Buffer[16];
	          Tmp.Data_b[2]     = Buffer[13];
	          Tmp.Data_b[3]     = Buffer[14];
	          if(	Tmp.Data_b[0] == 0xFF &&
	        		Tmp.Data_b[1] == 0xFF &&
	        		Tmp.Data_b[2] == 0xFF &&
	        		Tmp.Data_b[3] == 0xFF)
	          {	  Tmp.Data_l = 0; }
	          //E.Er_minus=Tmp.Data_l/1000.0;

		      float val = Tmp.Data_l/1000.0;
		      if(val < 3000000.0)
		    	  E.Er_minus = val;
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetUa(Byte *Buffer, Word len)
	{
	  if(len > 4){
		  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[2];
	          Tmp.Data_b[1]     = Buffer[3];
	          Tmp.Data_b[2]     = Buffer[1];
	          Tmp.Data_b[3]     = 0;
	          U.Ua.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetUb(Byte *Buffer, Word len)
	{

	  if(len > 6){
		  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[4];
	          Tmp.Data_b[3]     = 0;
	          U.Ub.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetUc(Byte *Buffer, Word len)
	{

	  if(len > 9){
		  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[8];
	          Tmp.Data_b[1]     = Buffer[9];
	          Tmp.Data_b[2]     = Buffer[7];
	          Tmp.Data_b[3]     = 0;
	          U.Uc.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetIa(Byte *Buffer, Word len)
	{
	  	  if(len > 4){
	  		  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[2];
	          Tmp.Data_b[1]     = Buffer[3];
	          Tmp.Data_b[2]     = Buffer[1];
	          Tmp.Data_b[3]     = 0;
	          I.Ia.CalcMinAverMax(Tmp.Data_l/1000.0);
	  	  }
	}
	//=============================================================================
	void TMercury230Counter::GetIb(Byte *Buffer, Word len)
	{

	  if(len > 6){
		  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[4];
	          Tmp.Data_b[3]     = 0;
	          I.Ib.CalcMinAverMax(Tmp.Data_l/1000.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetIc(Byte *Buffer, Word len)
	{
	  if(len > 9){
		  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[8];
	          Tmp.Data_b[1]     = Buffer[9];
	          Tmp.Data_b[2]     = Buffer[7];
	          Tmp.Data_b[3]     = 0;
	          I.Ic.CalcMinAverMax(Tmp.Data_l/1000.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetP(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 4){
	          Tmp.Data_b[0]     = Buffer[2];
	          Tmp.Data_b[1]     = Buffer[3];
	          Tmp.Data_b[2]     = Buffer[1]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          P.P.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetPa(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 6){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[4]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          P.Pa.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetPb(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 9){
	          Tmp.Data_b[0]     = Buffer[8];
	          Tmp.Data_b[1]     = Buffer[9];
	          Tmp.Data_b[2]     = Buffer[7]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          P.Pb.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetPc(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[11];
	          Tmp.Data_b[1]     = Buffer[12];
	          Tmp.Data_b[2]     = Buffer[10]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          P.Pc.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetQ(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 4){
	          Tmp.Data_b[0]     = Buffer[2];
	          Tmp.Data_b[1]     = Buffer[3];
	          Tmp.Data_b[2]     = Buffer[1]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          Q.Q.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetQa(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 6){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[4]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          Q.Qa.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetQb(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 9){
	          Tmp.Data_b[0]     = Buffer[8];
	          Tmp.Data_b[1]     = Buffer[9];
	          Tmp.Data_b[2]     = Buffer[7]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          Q.Qb.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetQc(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[11];
	          Tmp.Data_b[1]     = Buffer[12];
	          Tmp.Data_b[2]     = Buffer[10]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          Q.Qc.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetS(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 4){
	          Tmp.Data_b[0]     = Buffer[2];
	          Tmp.Data_b[1]     = Buffer[3];
	          Tmp.Data_b[2]     = Buffer[1]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          S.S.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetSa(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 6){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[4]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          S.Sa.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetSb(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 9){
	          Tmp.Data_b[0]     = Buffer[8];
	          Tmp.Data_b[1]     = Buffer[9];
	          Tmp.Data_b[2]     = Buffer[7]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          S.Sb.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	void TMercury230Counter::GetSc(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[11];
	          Tmp.Data_b[1]     = Buffer[12];
	          Tmp.Data_b[2]     = Buffer[10]&0x3F;
	          Tmp.Data_b[3]     = 0;
	          S.Sc.CalcMinAverMax(Tmp.Data_l/100.0);
	  }
	}
	//=============================================================================
	float TMercury230Counter::GetKM(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[2];
	          Tmp.Data_b[1]     = Buffer[3];
	          Tmp.Data_b[2]     = Buffer[1]&0x3F;
	          Tmp.Data_b[3]     = 0;
	 return Tmp.Data_l/1000.0;
	}
	//=============================================================================
	float TMercury230Counter::GetKMa(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[4]&0x3F;
	          Tmp.Data_b[3]     = 0;
	 return Tmp.Data_l/1000.0;
	}
	//=============================================================================
	float TMercury230Counter::GetKMb(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[8];
	          Tmp.Data_b[1]     = Buffer[9];
	          Tmp.Data_b[2]     = Buffer[7]&0x3F;
	          Tmp.Data_b[3]     = 0;
	 return Tmp.Data_l/1000.0;
	}
	//=============================================================================
	float TMercury230Counter::GetKMc(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[11];
	          Tmp.Data_b[1]     = Buffer[12];
	          Tmp.Data_b[2]     = Buffer[10]&0x3F;
	          Tmp.Data_b[3]     = 0;
	 return Tmp.Data_l/1000.0;
	}
	//=============================================================================
	float TMercury230Counter::GetF(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	          Tmp.Data_b[0]     = Buffer[2];
	          Tmp.Data_b[1]     = Buffer[3];
	          Tmp.Data_b[2]     = Buffer[1];
	          Tmp.Data_b[3]     = 0;
	 return Tmp.Data_l/100.0;
	}
	//=============================================================================
	Word TMercury230Counter::GetKtranI(Byte *Buffer, Word len)
	{
		if(len >= 7)
		{
		  Bshort Tmp;
				  Tmp.Data_b[1]     = Buffer[3];
				  Tmp.Data_b[0]     = Buffer[4];
		  return Tmp.Data_s;
		}
		else
			return 0;
	}
	//=============================================================================
	Word TMercury230Counter::GetKtranU(Byte *Buffer, Word len)
	{
		if(len >= 7)
		{
		  Bshort Tmp;
				  Tmp.Data_b[1]     = Buffer[1];
				  Tmp.Data_b[0]     = Buffer[2];
		  return Tmp.Data_s;
		}
		else
			return 0;
	}

	//=============================================================================
	Word TMercury230Counter::GetNetAddr(Byte *Buffer, Word len)
	{
		if(len >= 5)
		{
			Bshort Tmp;
			Tmp.Data_b[1]     = Buffer[1];
			Tmp.Data_b[0]     = Buffer[2];

			if(Addr == 0 &&  Tmp.Data_s > 0)
			{
				Addr = Tmp.Data_s;
				Log::DEBUG("TMercury230Counter newAddr=="+toString(Tmp.Data_s));
			}
			return Tmp.Data_s;
		}
		else
			return 0;
	}
	//=============================================================================
	string TMercury230Counter::GetSerial(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 4)
	  {
	          Tmp.Data_l     = Buffer[4];
	          Tmp.Data_l    += Buffer[3]*100l;
	          Tmp.Data_l    += Buffer[2]*10000l;
	          Tmp.Data_l    += Buffer[1]*1000000l;
	  }
	  //Log::DEBUG("TMercury230Counter::GetSerial="+toString(Tmp.Data_l));

/*
	  if(Addr == 0)
	  {
		  int a = Tmp.Data_l%1000;
		  //Log::DEBUG("TMercury230Counter::TMercury230Counter a%1000="+toString(a));
		  if(a > 240)
			  a = a%100;
		  //Log::DEBUG("TMercury230Counter::TMercury230Counter a%100="+toString(a));
		  if(a == 0)
			  a=1;
		  Addr = a;
	  }
*/

	 return LongToString(Tmp.Data_l, 8);
	}
	//=============================================================================
	string TMercury230Counter::GetEcmd(TEnergyCMD* ecmd, Byte *Buffer, Word len, TReturnValue *r)
	{
		string framStr = "";
		if(len > 0)
		{
			//Log::DEBUG("GetEcmd ecmd->Type="+toString((int)ecmd->Type));
			if(ecmd->Type <= 5)
			{
				TEnergy Energy(140, ecmd->Data[3]);
				Energy.Ea 		= GetEt(0, 0, Buffer, len);
				Energy.Ea_minus = GetEt_minus(0, 0, Buffer, len);
				Energy.Er 		= GetEt(0, 1, Buffer, len);
				Energy.Er_minus = GetEt_minus(0, 1, Buffer, len);
				Log::DEBUG("Ea="+toString(Energy.Ea)+" Ea_minus="+toString(Energy.Ea_minus) + " Er="+toString(Energy.Er) + " Er_minus="+toString(Energy.Er_minus));

				if(ecmd->Index == 0)
				{
					framStr += WordToString(ecmd->Etype)+',';
					framStr += WordToString( ecmd->Ename )+',';
				}
				framStr += Energy.GetValueString() +  WordToString((int)ecmd->Data[2]) + ',' + WordToString( ecmd->Type )+',' ;
			}
			else if (ecmd->Type == 0xD0)//get startOfLastDay energy
			{
				TEnergy Energy(141, ecmd->Data[3]);
				Energy.Ea 		= GetEt(0, 0, Buffer, len);
				Energy.Ea_minus = GetEt_minus(0, 0, Buffer, len);
				Energy.Er 		= GetEt(0, 1, Buffer, len);
				Energy.Er_minus = GetEt_minus(0, 1, Buffer, len);
				string Ea 		= toString(Energy.Ea);
				string Ea_minus = toString(Energy.Ea_minus);
				string Er 		= toString(Energy.Er);
				string Er_minus = toString(Energy.Er_minus);

				Log::DEBUG("Ea="+Ea+" Ea_minus="+Ea_minus+" Er="+Er+ " Er_minus="+Er_minus);

				framStr += WordToString(ecmd->Etype)+',';
				framStr += WordToString( ecmd->Ename )+',';
				framStr += Energy.GetValueString()+WordToString( ecmd->Type )+',';

				string tt = WordToString( ecmd->Type );

				if(r!=NULL)
				{
					r->Data.push_back(tt);
					r->Data.push_back(Ea);
					r->Data.push_back(Ea_minus);
					r->Data.push_back(Er);
					r->Data.push_back(Er_minus);

					r->IsComplite(true);
				}
			}
			else if (ecmd->Type == 40)//get time
			{
				  string sec = ByteToHex(Buffer[1]);
				  string min = ByteToHex(Buffer[2]);
				  string hou = ByteToHex(Buffer[3]);
				  string nda = ByteToHex(Buffer[4]);
				  string day = ByteToHex(Buffer[5]);
				  string mon = ByteToHex(Buffer[6]);
				  string yea = ByteToHex(Buffer[7]);
				  string typ = ByteToHex(Buffer[8]);

				  int y = atoi(yea.c_str())+2000;
				  int t = atoi(typ.c_str());

				  string ret = day+"."+mon+"."+toString(y)+" "+hou+":"+min+":"+sec;
				  string timetype = toString(t);

				  string tt = WordToString( ecmd->Type );

				  framStr += WordToString(ecmd->Etype)+',';
				  framStr += WordToString( ecmd->Ename )+',';
				  framStr +=">,51,1,"+tt+','+ret+","+timetype+',';//timetype=zima|leto

				  if(r!=NULL)
				  {
					  r->Data.push_back(tt);
					  r->Data.push_back(ret);
					  r->Data.push_back(timetype);

					  r->IsComplite(true);
				  }
			}
			else if (ecmd->Type == 0x0D)//correct time
			{
				string tt = WordToString( ecmd->Type );
				string st = WordToString( Buffer[1]);

				framStr += WordToString(ecmd->Etype)+',';
				framStr += WordToString( ecmd->Ename )+',';
				framStr +=">,51,2,"+tt+","+st+',';

				if(r!=NULL)
				{
					r->Data.push_back(tt);
					r->Data.push_back(st);
					r->IsComplite(true);
				}
			}
			else if (ecmd->Type == 0x13)//get last profil
			{
				Bshort TMP;
				  TMP.Data_b[1] 	= Buffer[1];
				  TMP.Data_b[0] 	= Buffer[2];

				  string addr 		= toString(TMP.Data_s);
				  string state 		= toString((int)Buffer[3]);

				  string hou = ByteToHex(Buffer[4]);
				  string min = ByteToHex(Buffer[5]);
				  string day = ByteToHex(Buffer[6]);
				  string mon = ByteToHex(Buffer[7]);
				  string yea = ByteToHex(Buffer[8]);
				  int y = atoi(yea.c_str())+2000;
				  string ret = day+"."+mon+"."+toString(y)+" "+hou+":"+min+":"+"00";
				  string tt = WordToString( ecmd->Type );

				  framStr += WordToString(ecmd->Etype)+',';
				  framStr += WordToString( ecmd->Ename )+',';
				  framStr +=">,51,4,"+tt+','+ret+","+addr+','+state;

				  if(r!=NULL)
				  {
					  r->Data.push_back(tt);
					  r->Data.push_back(ret);
					  r->Data.push_back(addr);
					  r->Data.push_back(state);
					  r->IsComplite(true);
				  }
			}
			else if (ecmd->Type == 0x63)//get profil by address
			{
				Bshort TMP;
				//Log::DEBUG("GetEcmd ecmd->Tmp[0]="+toString(ecmd->Tmp[0]));

				//string header = WordToString(ecmd->Etype)+',' + WordToString( ecmd->Ename )+',';

				Byte *BUF = &Buffer[1];
				string lastdate = "";
				for(int i =0; i < 2; i++)
				{
					BUF = BUF + i*15;

					int ste = BUF[0];
					string hou = ByteToHex(BUF[1]);
					string min = ByteToHex(BUF[2]);
					string day = ByteToHex(BUF[3]);
					string mon = ByteToHex(BUF[4]);
					string yer = toString(atoi( ByteToHex(BUF[5]).c_str() ) + 2000);
					string per = ByteToHex(BUF[6]);

					int houi = atoi(hou.c_str());
					int mini = atoi(min.c_str());

					string date = (houi == 0 && mini == 0) ? lastdate :  day+"."+mon+"."+yer;

					lastdate = date;

					int index = houi*2;
					if( mini != 0) index++;
					if(houi == 0 && mini == 0) index = 48;
					Log::DEBUG("GetEcmd hou="+hou +" min="+min+" mini="+toString(mini)+" index="+toString(index));

					TEnergy Energy(149, index);
					TMP.Data_b[0] = BUF[7];
					TMP.Data_b[1] = BUF[8];
					Energy.Ea 			= 	TMP.Data_s == 0xFFFF ? 0.0 : TMP.Data_s*1.0 *(60.0/30.0)/(2.0*ecmd->Tmp[0]);

					TMP.Data_b[0] = BUF[9];
					TMP.Data_b[1] = BUF[10];
					Energy.Ea_minus 	= 	TMP.Data_s == 0xFFFF ? 0.0 : TMP.Data_s*1.0 *(60.0/30.0)/(2.0*ecmd->Tmp[0]);

					TMP.Data_b[0] = BUF[11];
					TMP.Data_b[1] = BUF[12];
					Energy.Er 			= 	TMP.Data_s == 0xFFFF ? 0.0 : TMP.Data_s*1.0 *(60.0/30.0)/(2.0*ecmd->Tmp[0]);

					TMP.Data_b[0] = BUF[13];
					TMP.Data_b[1] = BUF[14];
					Energy.Er_minus 	= 	TMP.Data_s == 0xFFFF ? 0.0 : TMP.Data_s*1.0 *(60.0/30.0)/(2.0*ecmd->Tmp[0]);

					string Ea 		= toString(Energy.Ea);
					string Ea_minus = toString(Energy.Ea_minus);
					string Er 		= toString(Energy.Er);
					string Er_minus = toString(Energy.Er_minus);

					Log::DEBUG("GetEcmd Ea=" + Ea + " Ea_minus=" + Ea_minus + " Er=" + Er + "Er_minus=" + Er_minus);

					//string ret = day+"."+mon+"."+yer+" "+hou+":"+min;

					string needdate = ecmd->TmpStr[0];
					Log::DEBUG("PROFIL: currdate=" + date + " needdate="+needdate);
					if(!isNullOrWhiteSpace(needdate) && needdate == date)
					{
						string addstr = Energy.GetValueString() + toString(ste) +","+ date +","+ toString(ecmd->Tmp[1]) + "," + WordToString( ecmd->Type )+',';
						framStr += addstr;
						Log::DEBUG("PROFIL: addstr=" + addstr);
					}
				}
				//ring tt = WordToString( ecmd->Type );
				if(r!=NULL)
				{
					//r->Data.push_back(tt);
					//r->Data.push_back(Ea);
					//r->Data.push_back(Ea_minus);
					//r->Data.push_back(Er);
					//r->Data.push_back(Er_minus);
					//Log::DEBUG("PROFIL: framStr="+framStr);
					//r->Data.push_back(framStr);
					//framStr = "";
					r->IsComplite(true);
				}
			}
		}
		//Log::DEBUG("GetEcmd ecmd->Tmp="+toString(ecmd->Tmp));
		return framStr;
	}
	//*****************************************************************************
	//*** TMercury200Counter class
	//*****************************************************************************
	TMercury200Counter::TMercury200Counter(Byte Ename, string adr):ICounter(Ename, 0 )//, IsEnable(false)
	{
	  //DWord a = atol(adr.c_str())%1000000;
	  DWord a = atol(adr.c_str());//%1000000;
	  Log::DEBUG("TMercury200Counter::TMercury200Counter a="+toString(a));
	  Address = a;

	  Modify = false;
	  Type = 1;
	  isQ = false;

	  //U.Pt.Pname = 2;
	  U.Params.clear();
	  U.Params.push_back(&U.Ua);
	  U.Size = U.Params.size()*3;

	  //I.Pt.Pname = 2;
	  I.Params.clear();
	  I.Params.push_back(&I.Ia);
	  I.Size = I.Params.size()*3;

	  P.Params.clear();
	  P.Params.push_back(&P.P);
	  P.Params.push_back(&P.Pa);
	  P.Size = P.Params.size()*3;

	  Emass.push_back(&E1);
	  Emass.push_back(&E2);
	  Emass.push_back(&E3);
	  Emass.push_back(&E4);
	}
	//=============================================================================
	TMercury200Counter::~TMercury200Counter( )
	{
	}
	//=============================================================================
	Word TMercury200Counter::CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *ecmd)
	{
		Word DataLen = 0;
		Bshort       Tmp;
		Blong        Tmp1;

		Tmp1.Data_l = Address;

     	switch(CMD)
		{
		  case SET_PLC: // test connection
		        if(Index == 0)
		        {
		        	switch(plc)
		        	{
						case 1://nzif
					          Buffer[DataLen++] = 0xFC;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x03;
					          Buffer[DataLen++] = 0x2E;
					          Buffer[DataLen++] = 0x0B;
					          Buffer[DataLen++] = 0x02;
					          Buffer[DataLen++] = 0x42;
					          Buffer[DataLen++] = 0x3A;
					          Buffer[DataLen++] = 0x35;
					          Buffer[DataLen++] = 0xC7;
					          Tmp.Data_s = Crc16(Buffer, DataLen);
					          Buffer[DataLen++] = Tmp.Data_b[0];
					          Buffer[DataLen++] = Tmp.Data_b[1];
							break;
						case 2://icbcom1f
							break;

						case 3://icbcom3f
				        	if(cmd.size() > 0 && cmd.size() % 2 == 0)
				        	{
				        		for(int ind = 0; ind < cmd.size()/2; ind++)
				        		{
				        			Buffer[DataLen++] = HexToByte( (char*)&cmd.c_str()[ind*2] );
				        		}
				    	        //Tmp.Data_s = Crc16(Buffer, DataLen);
				    	        //Buffer[DataLen++] = Tmp.Data_b[0];
				    	        //Buffer[DataLen++] = Tmp.Data_b[1];
				        	}
							break;
						case 4://rm
				        	if(cmd.size() > 0)
				        	{
				        		for(int ind = 0; ind < cmd.size(); ind++){
				        			Buffer[DataLen++] = cmd.c_str()[ind];
				        		}
				        	}
				        	break;
		        	};
		        }
			break;

		  case OPEN_CHANNAL:
			Buffer[DataLen++] = Tmp1.Data_b[3];
			Buffer[DataLen++] = Tmp1.Data_b[2];
			Buffer[DataLen++] = Tmp1.Data_b[1];
			Buffer[DataLen++] = Tmp1.Data_b[0];
			Buffer[DataLen++] = 0x2F;//cmd = test subserial
			Tmp.Data_s = Crc16(Buffer, DataLen);
			Buffer[DataLen++] = Tmp.Data_b[0];
			Buffer[DataLen++] = Tmp.Data_b[1];
			break;

		  case GET_E:
			Buffer[DataLen++] = Tmp1.Data_b[3];
			Buffer[DataLen++] = Tmp1.Data_b[2];
			Buffer[DataLen++] = Tmp1.Data_b[1];
			Buffer[DataLen++] = Tmp1.Data_b[0];
			Buffer[DataLen++] = 0x27;//cmd = energy
			Tmp.Data_s = Crc16(Buffer, DataLen);
			Buffer[DataLen++] = Tmp.Data_b[0];
			Buffer[DataLen++] = Tmp.Data_b[1];
			break;

		  case GET_Ua:
			Buffer[DataLen++] = Tmp1.Data_b[3];
			Buffer[DataLen++] = Tmp1.Data_b[2];
			Buffer[DataLen++] = Tmp1.Data_b[1];
			Buffer[DataLen++] = Tmp1.Data_b[0];
			Buffer[DataLen++] = 0x63;//cmd = u,i,f,cos
			Tmp.Data_s = Crc16(Buffer, DataLen);
			Buffer[DataLen++] = Tmp.Data_b[0];
			Buffer[DataLen++] = Tmp.Data_b[1];
			break;

		  case GET_SERIAL:
			Buffer[DataLen++] = Tmp1.Data_b[3];
			Buffer[DataLen++] = Tmp1.Data_b[2];
			Buffer[DataLen++] = Tmp1.Data_b[1];
			Buffer[DataLen++] = Tmp1.Data_b[0];
			Buffer[DataLen++] = 0x2F;//cmd = test subserial
			Tmp.Data_s = Crc16(Buffer, DataLen);
			Buffer[DataLen++] = Tmp.Data_b[0];
			Buffer[DataLen++] = Tmp.Data_b[1];
			break;

		  case CLOSE_CHANNAL:
			Buffer[DataLen++] = Tmp1.Data_b[3];
			Buffer[DataLen++] = Tmp1.Data_b[2];
			Buffer[DataLen++] = Tmp1.Data_b[1];
			Buffer[DataLen++] = Tmp1.Data_b[0];
			Buffer[DataLen++] = 0x2F;//cmd = test subserial
			Tmp.Data_s = Crc16(Buffer, DataLen);
			Buffer[DataLen++] = Tmp.Data_b[0];
			Buffer[DataLen++] = Tmp.Data_b[1];
			break;

		  default:
			DataLen = 0;
			break;
		};

	    if(DataLen > 0)
	    {
			if(CMD != SET_PLC && plc == 3)
			{
				Byte* tmpBUF = new Byte[DataLen+5];
				Word dataInd = 0;
				tmpBUF[dataInd++] = 0x0A;
				tmpBUF[dataInd++] = 0x61;
				tmpBUF[dataInd++] = DataLen;

				for(int i=0; i < DataLen; i++){
					tmpBUF[dataInd++] = Buffer[i];
				}
		        Tmp.Data_s = Crc16(tmpBUF, dataInd);
		        tmpBUF[dataInd++] = Tmp.Data_b[0];
		        tmpBUF[dataInd++] = Tmp.Data_b[1];
				for(int i=0; i < dataInd; i++){
					Buffer[i] = tmpBUF[i];
				}
				DataLen = dataInd;
				delete [] tmpBUF;
			}
	    }
     	return DataLen;
	}
	//=============================================================================
	Word TMercury200Counter::GetRecvMessageLen(Byte CMD)
	{
	  Word Len = 0;
	  switch(CMD)
	  {
	    case SET_PLC:
			  if(Index == 0)
			  {
		        	switch(plc)
		        	{
						case 1://nzif
							Len = 8;
							break;
						case 2://icbcom1f
							break;
						case 3://icbcom3f
							Len = 100;
							break;
						case 4://rm
							//Len = 100;
				        	break;
		        	};
			  }
	      break;

	    case OPEN_CHANNAL:
	      Len = 11;break;

	    case GET_E:
	      Len = 23;break;

	    case GET_Ua:
	      Len = 14;break;

	    case GET_SERIAL:
	      Len = 11;break;

	    case CLOSE_CHANNAL:
	      Len = 11;break;

	    default:
	      break;
	  };

		 if(CMD != SET_PLC && plc == 3 && Len > 0)
		 {
			 Len += 5;
		 }
	  return Len;
	}
	//=============================================================================
	sWord TMercury200Counter::RecvData(IPort* port, Byte *Buf, sWord DataLen, Word MaxLen, Byte subfase,
			sWord &refLen)
	{
	  //Log::DEBUG("RecvData DataLen"+toString((int)DataLen)+" subfase="+toString((int)subfase));
	  return port->Recv( Buf, DataLen );//
	}
	//=============================================================================
	string TMercury200Counter::GetStringValue(void)
	{
		string ReturnString = WordToString(Et)+","+ WordToString(En)+","+ E.GetSimpleValueString() + CouterType.GetValueString();
		if(NeedFirstFlg == true)
		{
			ReturnString += U.GetValueString();
			ReturnString += I.GetValueString();
			ReturnString += P.GetValueString();
		}
		ReturnString += SerialN.GetValueString()+WordToString(IsEnable)+',';
		ReturnString += "\r\n>,99,1,"+WordToString(Type)+',';
		return ReturnString;
	}
	//=============================================================================
	void TMercury200Counter::GetType(Byte *Buffer, Word len)
	{
		CouterType.Type = "M200";
	}
	//=============================================================================
	void TMercury200Counter::GetUa(Byte *Buffer, Word len)
	{
		Byte bf[2] {0,0};
		string tmpVal = "";
		if(len >= 12){
			ByteToHex(bf, Buffer[5]);
			tmpVal+= string((char*)bf, 2);
			ByteToHex(bf, Buffer[6]);
			tmpVal+= string((char*)bf, 2);
			if(!isNullOrWhiteSpace(tmpVal)){
				U.Ua.CalcMinAverMax( (float)(atoi(tmpVal.c_str())/10.0) );
			};

			tmpVal = "";
			ByteToHex(bf, Buffer[7]);
			tmpVal+= string((char*)bf, 2);
			ByteToHex(bf, Buffer[8]);
			tmpVal+= string((char*)bf, 2);
			if(!isNullOrWhiteSpace(tmpVal)){
				I.Ia.CalcMinAverMax( (float)(atoi(tmpVal.c_str())/100.0) );
			};
			P.Pa.CalcMinAverMax( I.Ia.Value*U.Ua.Value );
		}
	}
	//=============================================================================
	void TMercury200Counter::GetP(Byte *Buffer, Word len)
	{

		P.P.CalcMinAverMax(P.Pa.Value+P.Pb.Value+P.Pc.Value);

	}
	//=============================================================================
	void TMercury200Counter::GetEa(Byte *Buffer, Word len)
	{
		Byte bf[2] {0,0};
		string tmpVal = "";
		int index = 5;
		float eAll = 0;
		for(int i=0; i < Emass.size(); i++)//4 tarif
		{
			tmpVal = "";
			for(int j=0; j < 4; j++)//4 bytes per tarif
			{
				if(index >= len)
					break;
				ByteToHex(bf, Buffer[index++]);
				tmpVal+= string((char*)bf, 2);
			}
			if(!isNullOrWhiteSpace(tmpVal)){
				Emass[i]->Ea = (float)(atol(tmpVal.c_str())/100.0);
				eAll += Emass[i]->Ea;
			}
		}
		E.Ea = eAll;
	}
	//=============================================================================
	void TMercury200Counter::GetEa_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}
	//=============================================================================
	void TMercury200Counter::GetEr(Byte *Buffer, Word len)
	{
	}
	//=============================================================================
	void TMercury200Counter::GetEr_minus(Byte *Buffer, Word len)
	{
	}
	//=============================================================================
	float TMercury200Counter::GetEt(int tindex, Byte etype, Byte *Buffer, Word len)
	{
		float ret = 0.0;
		return ret;
	}
	//=============================================================================
	float TMercury200Counter::GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len)
	{
		float ret = 0.0;
		return ret;
	}
	//=============================================================================
	string TMercury200Counter::GetSerial(Byte *Buffer, Word len)
	{
		string ret = "00000000";
		Blong TMP;
		//Log::DEBUG("TMercury200Counter::GetSerial len = "+toString(len));
		if(len >= 9){
			TMP.Data_b[3] = Buffer[5];
			TMP.Data_b[2] = Buffer[6];
			TMP.Data_b[1] = Buffer[7];
			TMP.Data_b[0] = Buffer[8];
			ret = LongToString(TMP.Data_l, 8);
		}
		return ret;
	}
	//*****************************************************************************
	//*** TPschCounter class
	//*****************************************************************************
	TPschCounter::TPschCounter(Byte Ename, string adr):ICounter(Ename, 0)
	{
		  int a = atoi(adr.c_str());
		  Addr = a;
		  OrigAddr = a;

		  Modify = false;
		  Type = 6;
		  //isQ = false;
	}
	//=============================================================================
	TPschCounter::~TPschCounter( )
	{
	}
	//=============================================================================
	Word TPschCounter::CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *ecmd)
	{
	 Word DataLen = 0;
	 Bshort       Tmp;

	     switch(CMD)
	    {

	      case SET_PLC: // test connection
		        if(Index == 0)
		        {
		        	switch(plc)
		        	{
						case 1://nzif
					          Buffer[DataLen++] = 0xFC;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x03;
					          Buffer[DataLen++] = 0x2E;
					          Buffer[DataLen++] = 0x0B;
					          Buffer[DataLen++] = 0x02;
					          Buffer[DataLen++] = 0x42;
					          Buffer[DataLen++] = 0x3A;
					          Buffer[DataLen++] = 0x35;
					          Buffer[DataLen++] = 0xC7;
					          Tmp.Data_s = Crc16(Buffer, DataLen);
					          Buffer[DataLen++] = Tmp.Data_b[0];
					          Buffer[DataLen++] = Tmp.Data_b[1];
							break;
						case 2://icbcom1f
							break;

						case 3://icbcom3f
				        	if(cmd.size() > 0 && cmd.size() % 2 == 0)
				        	{
				        		for(int ind = 0; ind < cmd.size()/2; ind++)
				        		{
				        			Buffer[DataLen++] = HexToByte( (char*)&cmd.c_str()[ind*2] );
				        		}
				    	        //Tmp.Data_s = Crc16(Buffer, DataLen);
				    	        //Buffer[DataLen++] = Tmp.Data_b[0];
				    	        //Buffer[DataLen++] = Tmp.Data_b[1];
				        	}
							break;
						case 4://rm
				        	if(cmd.size() > 0)
				        	{
				        		for(int ind = 0; ind < cmd.size(); ind++){
				        			Buffer[DataLen++] = cmd.c_str()[ind];
				        		}
				        	}
				        	break;
		        	};
		        }
	        break;
	      case OPEN_CHANNAL:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x01;
	        for(int i = 0; i < 6;i++)
	        { Buffer[DataLen++] = 0x30;}
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_ETYPE:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x12;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_E:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        for(int i = 0; i < 2;i++)
	        { Buffer[DataLen++] = 0x00;}
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;


	      case GET_ER:
	        Buffer[DataLen++] = Addr;
	        Buffer[DataLen++] = 0x05;
	        for(int i = 0; i < 2;i++)
	        { Buffer[DataLen++] = 0x00;}
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;


	      case GET_U:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x1B;
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = 0x10;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_I:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x1B;
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = 0x20;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_P:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x1B;
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = 0x00;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_Q:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x1B;
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = 0x04;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_S:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x1B;
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = 0x08;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_KM:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x1B;
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = 0x30;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case GET_F:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x1B;
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = 0x41;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_KTRAN:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x02;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_NETADDR:
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x05;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case GET_SERIAL:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x08;
	        Buffer[DataLen++] = 0x00;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;
	      case CLOSE_CHANNAL:
	        DataLen = 0;
	        Buffer[DataLen++] = Addr;//new
	        Buffer[DataLen++] = 0x02;
	        Tmp.Data_s = Crc16(Buffer, DataLen);
	        Buffer[DataLen++] = Tmp.Data_b[0];
	        Buffer[DataLen++] = Tmp.Data_b[1];
	        break;

	      case SET_ECMD:
	    	  if(ecmd!=NULL && ecmd->Data.size() > 1)
	    	  {
	    		  for(auto currB: ecmd->Data)
	    		  {
	    			  Buffer[DataLen++] = currB;
	    		  }
	    		  Tmp.Data_s = Crc16(Buffer, DataLen);
	    		  Buffer[DataLen++] = Tmp.Data_b[0];
	    		  Buffer[DataLen++] = Tmp.Data_b[1];
	    	  }
	    	  break;

	      default:
	        DataLen = 0;
	        break;
	    };

		    if(DataLen > 0)
		    {
				if(CMD != SET_PLC && plc == 3)
				{
					Byte* tmpBUF = new Byte[DataLen+5];
					Word dataInd = 0;
					tmpBUF[dataInd++] = 0x0A;
					tmpBUF[dataInd++] = 0x61;
					tmpBUF[dataInd++] = DataLen;

					for(int i=0; i < DataLen; i++){
						tmpBUF[dataInd++] = Buffer[i];
					}
			        Tmp.Data_s = Crc16(tmpBUF, dataInd);
			        tmpBUF[dataInd++] = Tmp.Data_b[0];
			        tmpBUF[dataInd++] = Tmp.Data_b[1];
					for(int i=0; i < dataInd; i++){
						Buffer[i] = tmpBUF[i];
					}
					DataLen = dataInd;
					delete [] tmpBUF;
				}
		    }
	     return DataLen;
	}
	//=============================================================================
	Word TPschCounter::GetRecvMessageLen(Byte CMD)
	{
	  Word Len = 0;
	  switch(CMD)
	  {
	    case SET_PLC:
		  if(Index == 0)
		  {
	        	switch(plc)
	        	{
					case 1://nzif
						Len = 8;
						break;
					case 2://icbcom1f
						break;
					case 3://icbcom3f
						Len = 100;
						break;
					case 4://rm
						//Len = 100;
			        	break;
	        	};
		  }
	      break;

	    case OPEN_CHANNAL:
	      Len = 4;break;

	    case GET_ETYPE:
	      Len = 6;break;

	    case GET_E:
	    case GET_ER:
	    case GET_E1:
	    case GET_E2:
	    case GET_E3:
	    case GET_E4:
	    case GET_E5:
	      Len = 19;
	      break;

	    case GET_U:
	    case GET_I:
	      Len = 19;
	      break;

	    case GET_P:
	    case GET_Q:
	    case GET_S:
	    case GET_KM:
	    case GET_F:
	      Len = 19;
	      break;

	    case GET_SERIAL:
	      Len = 10;
	      break;

	    case GET_KTRAN:
	      Len = 13;
	      break;

	    case GET_NETADDR:
	      Len = 5;
	      break;

	    case CLOSE_CHANNAL:
	      Len = 4;
	      break;

	    case SET_ECMD:
	    	Len = 100;
	    	break;

	    default:
	      break;
	  };

		 if(CMD != SET_PLC && plc == 3 && Len > 0)
		 {
			 Len += 5;
		 }
	  return Len;
	}
	//=============================================================================
	sWord   TPschCounter::RecvData(IPort* port, Byte *Buf, sWord DataLen, Word MaxLen, Byte subfase, sWord &refLen)
	{
	  //Log::DEBUG("RecvData DataLen"+toString((int)DataLen)+" subfase="+toString((int)subfase));
	  return port->Recv( Buf, DataLen );//
	}
	//=============================================================================
	string TPschCounter::GetStringValue(void)
	{
		string ReturnString = WordToString(Et)+","+ WordToString(En)+","+ E.GetValueString();
								  //E1.GetValueString()+
								  //E2.GetValueString()+
								  //E3.GetValueString()+
								  //E4.GetValueString()+
								  //E5.GetValueString()+
		ReturnString += CouterType.GetValueString();
		if(NeedFirstFlg == true)
		{
			ReturnString += U.GetValueString();
			ReturnString += I.GetValueString();
			ReturnString += P.GetValueString();
			ReturnString += Q.GetValueString();
			ReturnString += S.GetValueString();
			ReturnString += KM.GetValueString();
			ReturnString += F.GetValueString();
		}
		ReturnString += SerialN.GetValueString()+WordToString(IsEnable)+',';
		ReturnString += "\r\n>,99,1,"+WordToString(Type)+',';
			  return ReturnString;
	}
	//=============================================================================
	bool TPschCounter::SetNewEcmd(TRtcTime &dt, EcounterCmdParameter curr)
	{
		bool ret = false;
		Log::DEBUG("addr="+ toString((int)curr.addr)+" cmd=" + toString((int)curr.cmd)+" sub_cmd=" + toString((int)curr.sub_cmd)+" parameters.size=" + toString(curr.parameters.size()));
		Byte cmd = curr.cmd;
		switch(cmd)
		{
			case 1:
			{//this year
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0x10);
				newCmd->Data.push_back(curr.sub_cmd);//t1,t2,t3....
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
				//Log::DEBUG("TEnergyCounterManager cmd =1, ECMDMessages.size="+ toString(ECMDMessages.size()) +" Fase=" + toString((int)Fase));
			}
				break;
			case 2:
			{//prev year
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0x20);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;
			case 3:
			{//mon
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				Byte c = 0x30 + curr.parameters[0];
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(c);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;
			case 4:
			{//this day
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0x40);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;
			case 5:
			{//prev day
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Index = curr.index;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x05);
				newCmd->Data.push_back(0x50);
				newCmd->Data.push_back(curr.sub_cmd);
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;

			case 12:
			{//this day start
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x0A);
				newCmd->Data.push_back(0x84);
				newCmd->Data.push_back(0x00);
				newCmd->Data.push_back(curr.sub_cmd);
				newCmd->Data.push_back(0x0F);
				newCmd->Data.push_back(0x00);
				ECMDMessages.push_back(newCmd);
				ret = true;
			}
				break;
			case 13:
			{//prev day start
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 19;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(0x0A);
				newCmd->Data.push_back(0x85);
				newCmd->Data.push_back(0x00);
				newCmd->Data.push_back(curr.sub_cmd);
				newCmd->Data.push_back(0x0F);
				newCmd->Data.push_back(0x00);
				ECMDMessages.push_back(newCmd);
				ret = true;
			}
				break;


			case 40:
			{//get time
				TEnergyCMD *newCmd = new TEnergyCMD();
				newCmd->Etype = curr.Et;
				newCmd->Ename = curr.En;
				newCmd->DateTime = dt;
				newCmd->Type = cmd;
				newCmd->RetLen = 11;
				newCmd->Data.push_back(curr.addr);//addr
				newCmd->Data.push_back(cmd/10);//04
				newCmd->Data.push_back(curr.sub_cmd);//00
				ECMDMessages.push_back(newCmd);
				ret = true;
				//SetFase(DO_ECMD);
			}
				break;

			default:
				break;
		};
		return ret;
	}
	//=============================================================================
	bool TPschCounter::GetDateTime(void)
	{
		Log::DEBUG("TPschCounter::GetDateTime");
		TRtcTime dt = SystemTime.GetTime();
		TEnergyCMD *newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->DateTime = dt;
		newCmd->Type = 40;
		newCmd->RetLen = 11;
		newCmd->Data.push_back(Addr);//addr
		newCmd->Data.push_back(40/10);//04
		newCmd->Data.push_back(0);//00
		ECMDMessages.push_back(newCmd);

		return true;
	}
	//=============================================================================
	bool TPschCounter::CorrectDateTime(string time)
	{//ss:mm:hh
		bool ret = false;
		vector<string> data = Split(time, ":");
		if(data.size() == 3)
		{
			TRtcTime dt = SystemTime.GetTime();
			TEnergyCMD *newCmd = new TEnergyCMD();
			newCmd->Etype = Et;
			newCmd->Ename = En;
			newCmd->DateTime = dt;
			newCmd->Type = 0x0D;//set or correct
			newCmd->RetLen = 4;
			newCmd->Data.push_back(Addr);//addr
			newCmd->Data.push_back(0x03);//cmd=write
			newCmd->Data.push_back(0x0D);//sub_cmd=correct time
			for(auto curr: data)
			{
				newCmd->Data.push_back( (Byte)HexToByte((char*)curr.c_str()) );
			}
			ECMDMessages.push_back(newCmd);
			Log::DEBUG( "TPschCounter::CorrectDateTime time=["+time+"]");
			ret = true;
		}
		return ret;
	}
	//=============================================================================
	//=============================================================================
	bool TPschCounter::SetDateTime(string datetime, string password)
	{//ss:mm:hh
		bool ret = false;
		vector<string> dt = Split(datetime, " ");
		//Log::DEBUG( "TPschCounter::SetDateTime datetime=["+datetime+"]");
		if(dt.size() == 2)
		{
			string date = dt[0];
			string time = dt[1];
			vector<string> data = Split(time, ":");
			vector<string> data1 = Split(date, ".");
			if(data.size() == 3 && data1.size() == 4)
			{
				TRtcTime dt = SystemTime.GetTime();
				for(int i = 0; i < 3; i++)
				{
					TEnergyCMD *newCmd = new TEnergyCMD();
					newCmd->Etype = Et;
					newCmd->Ename = En;
					newCmd->DateTime = dt;
					newCmd->Type = 0x0D;//set or correct
					newCmd->RetLen = 4;
					newCmd->Data.push_back(Addr);//addr
					newCmd->Data.push_back(0x03);//cmd=write
					newCmd->Data.push_back(0x0C);//sub_cmd=set time
					for(auto curr: data)
					{
						newCmd->Data.push_back( (Byte)HexToByte((char*)curr.c_str()) );
					}
					for(auto curr: data1)
					{
						newCmd->Data.push_back( (Byte)HexToByte((char*)curr.c_str()) );
					}
					newCmd->Data.push_back(0x01); // winter
					if(password.size() == 6)
					{
						newCmd->Password.clear();
						Log::DEBUG( "TPschCounter::SetDateTime password=["+password+"]");

						for(int i=0; i < password.size(); i++)
							newCmd->Password.push_back( password.c_str()[i] - 0x30 );
					}
					ECMDMessages.push_back(newCmd);
					Log::DEBUG( "TPschCounter::SetDateTime datetime=["+datetime+"]");
				}
				ret = true;
			}
		}
		return ret;
	}
	//=============================================================================
	//=============================================================================
	bool TPschCounter::GetLastProfile(void)
	{
		Log::DEBUG("TPschCounter::GetLastProfile");
		TRtcTime dt = SystemTime.GetTime();
		TEnergyCMD *newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->DateTime = dt;
		newCmd->Type = 0x13;
		newCmd->RetLen = 10;
		newCmd->Data.push_back(Addr);//addr
		newCmd->Data.push_back(0x08);//
		newCmd->Data.push_back(0x04);//
		newCmd->Data.push_back(0x00);//

		ECMDMessages.push_back(newCmd);
		return true;
	}
	//=============================================================================
	string TPschCounter::GetEcmd(TEnergyCMD* ecmd, Byte *Buffer, Word len, TReturnValue *r)
	{
		string framStr = "";
		if(len > 0)
		{
			//Log::DEBUG("GetEcmd ecmd->Type="+toString((int)ecmd->Type));
			if(ecmd->Type <= 5)
			{
				TEnergy Energy(140, ecmd->Data[3]);
				Energy.Ea 		= GetEt(0, 0, Buffer, len);
				Energy.Ea_minus = GetEt_minus(0, 0, Buffer, len);
				Energy.Er 		= GetEt(0, 1, Buffer, len);
				Energy.Er_minus = GetEt_minus(0, 1, Buffer, len);
				Log::DEBUG("Ea="+toString(Energy.Ea)+" Ea_minus="+toString(Energy.Ea_minus) + " Er="+toString(Energy.Er) + " Er_minus="+toString(Energy.Er_minus));

				if(ecmd->Index == 0)
				{
					framStr += WordToString(ecmd->Etype)+',';
					framStr += WordToString( ecmd->Ename )+',';
				}
				framStr += Energy.GetValueString() +  WordToString((int)ecmd->Data[2]) + ',' + WordToString( ecmd->Type )+',' ;
			}
			else if (ecmd->Type == 0xD0)//get startOfLastDay energy
			{
				TEnergy Energy(141, ecmd->Data[3]);
				Energy.Ea 		= GetEt(0, 0, Buffer, len);
				Energy.Ea_minus = GetEt_minus(0, 0, Buffer, len);
				Energy.Er 		= GetEt(0, 1, Buffer, len);
				Energy.Er_minus = GetEt_minus(0, 1, Buffer, len);
				string Ea 		= toString(Energy.Ea);
				string Ea_minus = toString(Energy.Ea_minus);
				string Er 		= toString(Energy.Er);
				string Er_minus = toString(Energy.Er_minus);

				Log::DEBUG("Ea="+Ea+" Ea_minus="+Ea_minus+" Er="+Er+ " Er_minus="+Er_minus);

				framStr += WordToString(ecmd->Etype)+',';
				framStr += WordToString( ecmd->Ename )+',';
				framStr += Energy.GetValueString()+WordToString( ecmd->Type )+',';

				string tt = WordToString( ecmd->Type );

				if(r!=NULL)
				{
					r->Data.push_back(tt);
					r->Data.push_back(Ea);
					r->Data.push_back(Ea_minus);
					r->Data.push_back(Er);
					r->Data.push_back(Er_minus);

					r->IsComplite(true);
				}
			}
			else if (ecmd->Type == 40)//get time
			{
				  string sec = ByteToHex(Buffer[1]);
				  string min = ByteToHex(Buffer[2]);
				  string hou = ByteToHex(Buffer[3]);
				  string nda = ByteToHex(Buffer[4]);
				  string day = ByteToHex(Buffer[5]);
				  string mon = ByteToHex(Buffer[6]);
				  string yea = ByteToHex(Buffer[7]);
				  string typ = ByteToHex(Buffer[8]);

				  int y = atoi(yea.c_str())+2000;
				  int t = atoi(typ.c_str());

				  string ret = day+"."+mon+"."+toString(y)+" "+hou+":"+min+":"+sec;
				  string timetype = toString(t);

				  string tt = WordToString( ecmd->Type );

				  framStr += WordToString(ecmd->Etype)+',';
				  framStr += WordToString( ecmd->Ename )+',';
				  framStr +=">,51,1,"+tt+','+ret+","+timetype+',';//timetype=zima|leto

				  if(r!=NULL)
				  {
					  r->Data.push_back(tt);
					  r->Data.push_back(ret);
					  r->Data.push_back(timetype);

					  r->IsComplite(true);
				  }
			}
			else if (ecmd->Type == 0x0D)//correct or set time
			{
				string tt = WordToString( ecmd->Type );
				string st = WordToString( Buffer[1]);

				framStr += WordToString(ecmd->Etype)+',';
				framStr += WordToString( ecmd->Ename )+',';
				framStr +=">,51,2,"+tt+","+st+',';

				if(r!=NULL)
				{
					r->Data.push_back(tt);
					r->Data.push_back(st);
					r->IsComplite(true);
				}
			}
			else if (ecmd->Type == 0x13)//get last profil
			{
				Bshort TMP;
				  TMP.Data_b[1] 	= Buffer[6];
				  TMP.Data_b[0] 	= Buffer[7];

				  string addr 		= toString(TMP.Data_s);
				  string state 		= toString((int)(Buffer[1] >> 7));

				  string min = ByteToHex(Buffer[1] && 0x7F);
				  string hou = ByteToHex(Buffer[2]);

				  string day = ByteToHex(Buffer[3]);
				  string mon = ByteToHex(Buffer[4]);
				  string yea = ByteToHex(Buffer[5]);

				  int y = atoi(yea.c_str())+2000;
				  string ret = day+"."+mon+"."+toString(y)+" "+hou+":"+min+":"+"00";
				  string tt = WordToString( ecmd->Type );

				  framStr += WordToString(ecmd->Etype)+',';
				  framStr += WordToString( ecmd->Ename )+',';
				  framStr +=">,51,4,"+tt+','+ret+","+addr+','+state;

				  if(r!=NULL)
				  {
					  r->Data.push_back(tt);
					  r->Data.push_back(ret);
					  r->Data.push_back(addr);
					  r->Data.push_back(state);
					  r->IsComplite(true);
				  }
			}
			else if (ecmd->Type == 0x63)//get profil by address
			{
				Bshort TMP;
				Byte *BUF = &Buffer[1];
				string lastdate = "";
				for(int i =0; i < 2; i++)
				{
					BUF = BUF + i*15;

					int ste = BUF[0];
					string hou = ByteToHex(BUF[1]);
					string min = ByteToHex(BUF[2]);
					string day = ByteToHex(BUF[3]);
					string mon = ByteToHex(BUF[4]);
					string yer = toString(atoi( ByteToHex(BUF[5]).c_str() ) + 2000);
					string per = ByteToHex(BUF[6]);

					int houi = atoi(hou.c_str());
					int mini = atoi(min.c_str());

					string date = (houi == 0 && mini == 0) ? lastdate :  day+"."+mon+"."+yer;

					lastdate = date;

					int index = houi*2;
					if( mini != 0) index++;
					if(houi == 0 && mini == 0) index = 48;
					Log::DEBUG("GetEcmd hou="+hou +" min="+min+" mini="+toString(mini)+" index="+toString(index));

					TEnergy Energy(149, index);
					TMP.Data_b[0] = BUF[7];
					TMP.Data_b[1] = BUF[8];
					Energy.Ea 			= 	TMP.Data_s == 0xFFFF ? 0.0 : TMP.Data_s*1.0 *(60.0/30.0)/(2.0*ecmd->Tmp[0]);

					TMP.Data_b[0] = BUF[9];
					TMP.Data_b[1] = BUF[10];
					Energy.Ea_minus 	= 	TMP.Data_s == 0xFFFF ? 0.0 : TMP.Data_s*1.0 *(60.0/30.0)/(2.0*ecmd->Tmp[0]);

					TMP.Data_b[0] = BUF[11];
					TMP.Data_b[1] = BUF[12];
					Energy.Er 			= 	TMP.Data_s == 0xFFFF ? 0.0 : TMP.Data_s*1.0 *(60.0/30.0)/(2.0*ecmd->Tmp[0]);

					TMP.Data_b[0] = BUF[13];
					TMP.Data_b[1] = BUF[14];
					Energy.Er_minus 	= 	TMP.Data_s == 0xFFFF ? 0.0 : TMP.Data_s*1.0 *(60.0/30.0)/(2.0*ecmd->Tmp[0]);

					string Ea 		= toString(Energy.Ea);
					string Ea_minus = toString(Energy.Ea_minus);
					string Er 		= toString(Energy.Er);
					string Er_minus = toString(Energy.Er_minus);

					Log::DEBUG("GetEcmd Ea=" + Ea + " Ea_minus=" + Ea_minus + " Er=" + Er + "Er_minus=" + Er_minus);

					//string ret = day+"."+mon+"."+yer+" "+hou+":"+min;

					string needdate = ecmd->TmpStr[0];
					Log::DEBUG("PROFIL: currdate=" + date + " needdate="+needdate);
					if(!isNullOrWhiteSpace(needdate) && needdate == date)
					{
						string addstr = Energy.GetValueString() + toString(ste) +","+ date +","+ toString(ecmd->Tmp[1]) + "," + WordToString( ecmd->Type )+',';
						framStr += addstr;
						Log::DEBUG("PROFIL: addstr=" + addstr);
					}
				}
				//ring tt = WordToString( ecmd->Type );
				if(r!=NULL)
				{
					//r->Data.push_back(tt);
					//r->Data.push_back(Ea);
					//r->Data.push_back(Ea_minus);
					//r->Data.push_back(Er);
					//r->Data.push_back(Er_minus);
					//Log::DEBUG("PROFIL: framStr="+framStr);
					//r->Data.push_back(framStr);
					//framStr = "";
					r->IsComplite(true);
				}
			}
		}
		//Log::DEBUG("GetEcmd ecmd->Tmp="+toString(ecmd->Tmp));
		return framStr;
	}


	void TPschCounter::GetType(Byte *Buffer, Word len)
	{
		if(len > 4)
		{//> 3
		  Byte type = Buffer[3] >> 4;
		  string Tmp = "";
		  Log::DEBUG("Counter type="+toString((int)type));
		  isQ = true;
		  switch(type)
		  {
		  case 0:
			  Tmp = "SET-4TM.01(02)";
			  break;
		  case 1:
			  Tmp = "SET-4TM.03";
			  break;
		  case 2:
			  Tmp = "SEB-1TM.01(02)";
			  break;
		  case 3:
			  Tmp = "PSCH-4TM.05";
			  break;
		  case 4:
			  Tmp = "PSCH-3TM.05";
			  break;
		  case 5:
			  Tmp = "SEO-1.15";
			  break;
		  case 11:
			  Tmp = "PSCH-4TM.05MK";
			  break;
		  default:
			  isQ = false;
			  break;
		  }
		  CouterType.Type = Tmp;

		  Word cont = Buffer[2] & 0x0F;
		  switch(cont)
		  {
		  case 0:
			  CouterType.Const = 5000;
			  break;
		  case 1:
			  CouterType.Const = 25000;
			  break;
		  case 2:
			  CouterType.Const = 1250;
			  break;
		  case 3:
			  CouterType.Const = 6250;
			  break;
		  case 4:
			  CouterType.Const = 500;
			  break;
		  case 5:
			  CouterType.Const = 250;
			  break;
		  };
		  Log::DEBUG("Counter cont="+toString((int)cont));
		}
	}
/*
	void TMercury230Counter::GetType(Byte *Buffer, Word len)
	{
		if(len > 5)
		{
		  Byte type = Buffer[3] & 0x0F;
		  string Tmp = isNullOrWhiteSpace(CouterType.Model) ? "M230" : CouterType.Model;

		  //Tmp += "ART";
		  Tmp += "A";
		  if( ((Buffer[3] >> 4) & 1) == 0)
			  Tmp+= "R";
		  if(Buffer[3] & (1 << 6))
			  Tmp+= "T";

		  Word cont = Buffer[2] & 0x0F;
		  switch(cont)
		  {
		  case 0:
			  CouterType.Const = 5000;
			  break;
		  case 1:
			  CouterType.Const = 25000;
			  break;
		  case 2:
			  CouterType.Const = 1250;
			  break;
		  case 3:
			  CouterType.Const = 500;
			  break;
		  case 4:
			  CouterType.Const = 1000;
			  break;
		  case 5:
			  CouterType.Const = 250;
			  break;

		  }

		  if(type > 0)
			  type--;
		  Tmp += WordToString(type, 2)+"";
		  if(Buffer[2] & (1 << 5)){
			  Tmp+= "P";}
		  if(Buffer[5] & (1 << 1)){
			  Tmp+= "Q";
			  isQ = true;
		  }
		  else{
			  isQ = false;
		  }
		  Byte inter = (Buffer[4] >> 2) & 0x03;
		  switch(inter)
		  {
		  case 0:
			  Tmp+= "C";
			  break;
		  case 1:
			  Tmp+= "R";
			  break;
		  };
		  if(Buffer[5] & (1 << 2)){
			  Tmp+= "S";}
		  if(Buffer[4] & (1 << 4)){
			  Tmp+= "I";}
		  if(Buffer[4] & (1 << 5)){
			  Tmp+= "G";}
		  if(Buffer[4] & (1 << 6)){
			  Tmp+= "L";}
		  if(Buffer[4] & 1 ){
			  Tmp+= "N";}
		  CouterType.Type = Tmp;
		}
	}
*/
	//=============================================================================
	void TPschCounter::GetEa(Byte *Buffer, Word len)
	{
		  Blong Tmp;
		  if(len > 4)
		  {
				  Tmp.Data_b[0] = Buffer[4];
				  Tmp.Data_b[1] = Buffer[3];
				  Tmp.Data_b[2] = Buffer[2];
				  Tmp.Data_b[3] = Buffer[1];
				  float f = (float)(PSCH_CONSTANT*2.0);
				  if(CouterType.Const > 0)
					  f = CouterType.Const*2.0;

				  E.Ea = Tmp.Data_l/f;
		  }
	}
	//=============================================================================
	void TPschCounter::GetEa_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}
	//=============================================================================
	void TPschCounter::GetEr(Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[12];
	          Tmp.Data_b[1]     = Buffer[11];
	          Tmp.Data_b[2]     = Buffer[10];
	          Tmp.Data_b[3]     = Buffer[9];

	          float f = (float)(PSCH_CONSTANT*2.0);
	          if(CouterType.Const > 0)
	        	  f = CouterType.Const*2.0;

	          E.Er = Tmp.Data_l/f;
	  }
	}
	//=============================================================================
	void TPschCounter::GetEr_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}

	//=============================================================================
	float TPschCounter::GetEt(int tindex, Byte etype, Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  float f = (float)(PSCH_CONSTANT*2.0);
	  if(CouterType.Const > 0)
		  f = CouterType.Const*2.0;

	  switch(etype)
	  {
	  case 0:
		  if(len > 4)
		  {
			  Tmp.Data_b[0] = Buffer[4];
		      Tmp.Data_b[1] = Buffer[3];
		      Tmp.Data_b[2] = Buffer[2];
		      Tmp.Data_b[3] = Buffer[1];
		  }
		  break;

	  case 1:
		  if(len > 12)
		  {
		       Tmp.Data_b[0]     = Buffer[12];//
		       Tmp.Data_b[1]     = Buffer[11];//
		       Tmp.Data_b[2]     = Buffer[10];//
		       Tmp.Data_b[3]     = Buffer[9];//
		  }
		  break;
	  };
	  return Tmp.Data_l/f;
	}
	//=============================================================================
	float TPschCounter::GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len)
	{
	  Blong Tmp;
	  float f = (float)(PSCH_CONSTANT*2.0);
	  if(CouterType.Const > 0)
		  f = CouterType.Const*2.0;

	  switch(etype)
	  {
	  case 0:
		  if(len > 8)
		  {
		          Tmp.Data_b[0] = Buffer[8];
		          Tmp.Data_b[1] = Buffer[7];
		          Tmp.Data_b[2] = Buffer[6];
		          Tmp.Data_b[3] = Buffer[5];
		          if(	Tmp.Data_b[0] == 0xFF &&
		        		Tmp.Data_b[1] == 0xFF &&
		        		Tmp.Data_b[2] == 0xFF &&
		        		Tmp.Data_b[3] == 0xFF)
		          {	  Tmp.Data_l = 0; }

		  }
		  break;

	  case 1:
		  if(len > 16)
		  {
		          Tmp.Data_b[0]     = Buffer[16];
		          Tmp.Data_b[1]     = Buffer[15];
		          Tmp.Data_b[2]     = Buffer[14];
		          Tmp.Data_b[3]     = Buffer[13];
		          if(	Tmp.Data_b[0] == 0xFF &&
		        		Tmp.Data_b[1] == 0xFF &&
		        		Tmp.Data_b[2] == 0xFF &&
		        		Tmp.Data_b[3] == 0xFF)
		          {	  Tmp.Data_l = 0; }
		  }
		  break;
	  };
	  return Tmp.Data_l/f;
	}


	//=============================================================================
	void TPschCounter::GetUa(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 8){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[7];
	          Tmp.Data_b[3]     = Buffer[8];
	          U.Ua.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetUb(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[9];
	          Tmp.Data_b[1]     = Buffer[10];
	          Tmp.Data_b[2]     = Buffer[11];
	          Tmp.Data_b[3]     = Buffer[12];
	          U.Ub.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetUc(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 16){
	          Tmp.Data_b[0]     = Buffer[13];
	          Tmp.Data_b[1]     = Buffer[14];
	          Tmp.Data_b[2]     = Buffer[15];
	          Tmp.Data_b[3]     = Buffer[16];
	          U.Uc.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetIa(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 8){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[7];
	          Tmp.Data_b[3]     = Buffer[8];
	          I.Ia.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetIb(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[9];
	          Tmp.Data_b[1]     = Buffer[10];
	          Tmp.Data_b[2]     = Buffer[11];
	          Tmp.Data_b[3]     = Buffer[12];
	          I.Ib.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetIc(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 16){
	          Tmp.Data_b[0]     = Buffer[13];
	          Tmp.Data_b[1]     = Buffer[14];
	          Tmp.Data_b[2]     = Buffer[15];
	          Tmp.Data_b[3]     = Buffer[16];
	          I.Ic.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetP(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 4){
	          Tmp.Data_b[0]     = Buffer[1];
	          Tmp.Data_b[1]     = Buffer[2];
	          Tmp.Data_b[2]     = Buffer[3];
	          Tmp.Data_b[3]     = Buffer[4];
	          if(Tmp.Data_f < 0.0)  Tmp.Data_f *= -1.0;

	          P.P.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetPa(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 8){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[7];
	          Tmp.Data_b[3]     = Buffer[8];
	          if(Tmp.Data_f < 0.0)  Tmp.Data_f *= -1.0;

	          P.Pa.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetPb(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[9];
	          Tmp.Data_b[1]     = Buffer[10];
	          Tmp.Data_b[2]     = Buffer[11];
	          Tmp.Data_b[3]     = Buffer[12];
	          if(Tmp.Data_f < 0.0)  Tmp.Data_f *= -1.0;

	          P.Pb.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetPc(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 16){
	          Tmp.Data_b[0]     = Buffer[13];
	          Tmp.Data_b[1]     = Buffer[14];
	          Tmp.Data_b[2]     = Buffer[15];
	          Tmp.Data_b[3]     = Buffer[16];
	          if(Tmp.Data_f < 0.0)  Tmp.Data_f *= -1.0;

	          P.Pc.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetQ(Byte *Buffer, Word len)
	{
	  Ufloat Tmp;
	  if(len > 4){
	          Tmp.Data_b[0]     = Buffer[1];
	          Tmp.Data_b[1]     = Buffer[2];
	          Tmp.Data_b[2]     = Buffer[3];
	          Tmp.Data_b[3]     = Buffer[4];
	          if(Tmp.Data_f < 0.0)  Tmp.Data_f *= -1.0;

	          Q.Q.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetQa(Byte *Buffer, Word len)
	{
	  Ufloat Tmp;
	  if(len > 8){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[7];
	          Tmp.Data_b[3]     = Buffer[8];
	          if(Tmp.Data_f < 0.0)  Tmp.Data_f *= -1.0;

	          Q.Qa.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetQb(Byte *Buffer, Word len)
	{
	  Ufloat Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[9];
	          Tmp.Data_b[1]     = Buffer[10];
	          Tmp.Data_b[2]     = Buffer[11];
	          Tmp.Data_b[3]     = Buffer[12];
	          if(Tmp.Data_f < 0.0)  Tmp.Data_f *= -1.0;

	          Q.Qb.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetQc(Byte *Buffer, Word len)
	{
	  Ufloat Tmp;
	  if(len > 16){
	          Tmp.Data_b[0]     = Buffer[13];
	          Tmp.Data_b[1]     = Buffer[14];
	          Tmp.Data_b[2]     = Buffer[15];
	          Tmp.Data_b[3]     = Buffer[16];
	          if(Tmp.Data_f < 0.0)  Tmp.Data_f *= -1.0;

	          Q.Qc.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetS(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 4){
	          Tmp.Data_b[0]     = Buffer[1];
	          Tmp.Data_b[1]     = Buffer[2];
	          Tmp.Data_b[2]     = Buffer[3];
	          Tmp.Data_b[3]     = Buffer[4];
	          S.S.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetSa(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 8){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[7];
	          Tmp.Data_b[3]     = Buffer[8];
	          S.Sa.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetSb(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[9];
	          Tmp.Data_b[1]     = Buffer[10];
	          Tmp.Data_b[2]     = Buffer[11];
	          Tmp.Data_b[3]     = Buffer[12];
	          S.Sb.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	void TPschCounter::GetSc(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 16){
	          Tmp.Data_b[0]     = Buffer[13];
	          Tmp.Data_b[1]     = Buffer[14];
	          Tmp.Data_b[2]     = Buffer[15];
	          Tmp.Data_b[3]     = Buffer[16];
	          S.Sc.CalcMinAverMax(Tmp.Data_f);
	  }
	}
	//=============================================================================
	float TPschCounter::GetKM(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 4){
	          Tmp.Data_b[0]     = Buffer[1];
	          Tmp.Data_b[1]     = Buffer[2];
	          Tmp.Data_b[2]     = Buffer[3];
	          Tmp.Data_b[3]     = Buffer[4];
	  }
	 return Tmp.Data_f;
	}
	//=============================================================================
	float TPschCounter::GetKMa(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 8){
	          Tmp.Data_b[0]     = Buffer[5];
	          Tmp.Data_b[1]     = Buffer[6];
	          Tmp.Data_b[2]     = Buffer[7];
	          Tmp.Data_b[3]     = Buffer[8];
	  }
	 return Tmp.Data_f;
	}
	//=============================================================================
	float TPschCounter::GetKMb(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 12){
	          Tmp.Data_b[0]     = Buffer[9];
	          Tmp.Data_b[1]     = Buffer[10];
	          Tmp.Data_b[2]     = Buffer[11];
	          Tmp.Data_b[3]     = Buffer[12];
	  }
	 return Tmp.Data_f;
	}
	//=============================================================================
	float TPschCounter::GetKMc(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 16){
	          Tmp.Data_b[0]     = Buffer[13];
	          Tmp.Data_b[1]     = Buffer[14];
	          Tmp.Data_b[2]     = Buffer[15];
	          Tmp.Data_b[3]     = Buffer[16];
	  }
	 return Tmp.Data_f;
	}
	//=============================================================================
	float TPschCounter::GetF(Byte *Buffer, Word len)
	{
	  Bfloat Tmp;
	  if(len > 4){
	          Tmp.Data_b[0]     = Buffer[1];
	          Tmp.Data_b[1]     = Buffer[2];
	          Tmp.Data_b[2]     = Buffer[3];
	          Tmp.Data_b[3]     = Buffer[4];
	  }
	 return Tmp.Data_f;
	}
	//=============================================================================
	string TPschCounter::GetSerial(Byte *Buffer, Word len)
	{
		string ret = "00000000";
		  Blong Tmp;
		  if(len > 4){
				  Tmp.Data_b[0]     = Buffer[4];
				  Tmp.Data_b[1]     = Buffer[3];
				  Tmp.Data_b[2]     = Buffer[2];
				  Tmp.Data_b[3]     = Buffer[1];
				  ret = LongToString(Tmp.Data_l);
		  }
		  return ret;
	}
	//=============================================================================
	Word TPschCounter::GetKtranI(Byte *Buffer, Word len)
	{
		if(len >= 7)
		{
		  Bshort Tmp;
				  Tmp.Data_b[1]     = Buffer[3];
				  Tmp.Data_b[0]     = Buffer[4];
		  return Tmp.Data_s;
		}
		else
			return 0;
	}
	//=============================================================================
	Word TPschCounter::GetKtranU(Byte *Buffer, Word len)
	{
		if(len >= 7)
		{
		  Bshort Tmp;
				  Tmp.Data_b[1]     = Buffer[1];
				  Tmp.Data_b[0]     = Buffer[2];
		  return Tmp.Data_s;
		}
		else
			return 0;
	}

	//=============================================================================
	Word TPschCounter::GetNetAddr(Byte *Buffer, Word len)
	{
		if(len >= 5)
		{
			Bshort Tmp;
			Tmp.Data_b[1]     = Buffer[1];
			Tmp.Data_b[0]     = Buffer[2];

			if(Addr == 0 &&  Tmp.Data_s > 0)
			{
				Addr = Tmp.Data_s;
				Log::DEBUG("TPschCounter newAddr=="+toString(Tmp.Data_s));
			}
			return Tmp.Data_s;
		}
		else
			return 0;
	}
	//*****************************************************************************
	//*** TEnergomeraCE class
	//*****************************************************************************
	TEnergomeraCE::TEnergomeraCE(Byte Ename, string adr):ICounter(Ename, atoi(adr.c_str())), Address(adr)
	{
	  Modify = false;
	  Type = 2;
	  LastCmd = "";
	  LastCmdPointer = NULL;
	}
	//=============================================================================
	TEnergomeraCE::~TEnergomeraCE( )
	{
	}
	//=============================================================================
	Word TEnergomeraCE::CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *ecmd)
	{
	 Word DataLen = 0;
	 Bshort Tmp;
	 LastCmd = "";

	    switch(CMD)
	    {
		  case SET_PLC: // test connection
		        if(Index == 0)
		        {
		        	switch(plc)
		        	{
						case 1://nzif
					          Buffer[DataLen++] = 0xFC;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x03;
					          Buffer[DataLen++] = 0x2E;
					          Buffer[DataLen++] = 0x0B;
					          Buffer[DataLen++] = 0x02;
					          Buffer[DataLen++] = 0x42;
					          Buffer[DataLen++] = 0x3A;
					          Buffer[DataLen++] = 0x35;
					          Buffer[DataLen++] = 0xC7;
					          Tmp.Data_s = Crc16(Buffer, DataLen);
					          Buffer[DataLen++] = Tmp.Data_b[0];
					          Buffer[DataLen++] = Tmp.Data_b[1];
							break;
						case 2://icbcom1f
							break;

						case 3://icbcom3f
				        	if(cmd.size() > 0 && cmd.size() % 2 == 0)
				        	{
				        		for(int ind = 0; ind < cmd.size()/2; ind++)
				        		{
				        			Buffer[DataLen++] = HexToByte( (char*)&cmd.c_str()[ind*2] );
				        		}
				    	        //Tmp.Data_s = Crc16(Buffer, DataLen);
				    	        //Buffer[DataLen++] = Tmp.Data_b[0];
				    	        //Buffer[DataLen++] = Tmp.Data_b[1];
				        	}
							break;
						case 4://rm
				        	if(cmd.size() > 0)
				        	{
				        		for(int ind = 0; ind < cmd.size(); ind++){
				        			Buffer[DataLen++] = cmd.c_str()[ind];
				        		}
				        	}
				        	break;
		        	};
		        }
			break;

	      case OPEN_CHANNAL:
	        DataLen = 0;
	        Buffer[DataLen++] = '/';
	        Buffer[DataLen++] = '?';
	        if(Address.size() > 0 && atoi(Address.c_str()) > 0){
	          for(int ind = 0; ind < Address.size(); ind++){
	            Buffer[DataLen++] = Address.c_str()[ind];
	          }
	        }
	        Buffer[DataLen++] = '!';
	        Buffer[DataLen++] = 0x0D;
	        Buffer[DataLen++] = 0x0A;
	        break;

	      case AUTORIZATE:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x06;
	        Buffer[DataLen++] = 0x30;
	        Buffer[DataLen++] = 0x35;
	        Buffer[DataLen++] = 0x31;
	        Buffer[DataLen++] = 0x0D;
	        Buffer[DataLen++] = 0x0A;
	        break;

	      case GET_EISP: //GET_ETYPE:
	    	DataLen = 0;
	    	if(ecmd == NULL || ecmd->Password.size()== 0)
	    		break;
			Buffer[DataLen++] = 0x01;
			Buffer[DataLen++] = 'P';
			Buffer[DataLen++] = '1';
			Buffer[DataLen++] = 0x02;
			Buffer[DataLen++] = '(';

			if(ecmd!=NULL && ecmd->Password.size() > 0){
				for(auto currB: ecmd->Password)
		    		Buffer[DataLen++] = currB;
				ecmd->Password.clear();
		    }
		    else{
		    	for(int i = 0; i < 6;i++)
		    		Buffer[DataLen++] = 0x37;
		    }
			Buffer[DataLen++] = ')';
			Buffer[DataLen++] = 0x03;
			Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_ETYPE://GET_ETYPE
		    DataLen = 0;
		    Buffer[DataLen++] = 0x01;
		    Buffer[DataLen++] = 'R';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = 0x02;

		    LastCmd = "IDENT";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

		    Buffer[DataLen++] = '(';
		    Buffer[DataLen++] = ')';
		    Buffer[DataLen++] = 0x03;
		    Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x4D;
		    break;

	      case GET_E:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "ET0PE";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//'7';
	        break;

	      case GET_U:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "VOLTA";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x5F;
	        break;

	      case GET_I:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "CURRE";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x5A;
	        break;

	      case GET_P:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "POWPP";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x6F;
	        break;

	      case GET_KM:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "COS_f";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x03;
	        break;

	      case GET_F:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "FREQU";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x5C;
	        break;

	      case GET_SERIAL:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "SNUMB";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x5E;
	        break;

	      case CLOSE_CHANNAL:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'B';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x75;
	        break;

	      case SET_ECMD:
	    	  if(ecmd!=NULL && ecmd->Data.size() > 1)
	    	  {
	    		  for(auto currB: ecmd->Data)
	    		  {
	    			  Buffer[DataLen++] = currB;
	    		  }
	    		  if(ecmd->NeedCrc)
	    			  Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	    	  }
	    	  break;


	      default:
	        DataLen = 0;
	        break;
	    };

	    if(DataLen > 0)
	    {
			if(CMD != SET_PLC && plc == 3)
			{
				Byte* tmpBUF = new Byte[DataLen+5];
				Word dataInd = 0;
				tmpBUF[dataInd++] = 0x0A;
				tmpBUF[dataInd++] = 0x61;
				tmpBUF[dataInd++] = DataLen;

				for(int i=0; i < DataLen; i++){
					tmpBUF[dataInd++] = Buffer[i];
				}
		        Tmp.Data_s = Crc16(tmpBUF, dataInd);
		        tmpBUF[dataInd++] = Tmp.Data_b[0];
		        tmpBUF[dataInd++] = Tmp.Data_b[1];
				for(int i=0; i < dataInd; i++){
					Buffer[i] = tmpBUF[i];
				}
				DataLen = dataInd;
				delete [] tmpBUF;
			}
	    }
	    return DataLen;
	}
	//=============================================================================
	Word TEnergomeraCE::GetRecvMessageLen(Byte CMD)
	{
	  Word Len = 0;
	  switch(CMD)
	  {
	    case SET_PLC:
			  if(Index == 0)
			  {
		        	switch(plc)
		        	{
						case 1://nzif
							Len = 8;
							break;
						case 2://icbcom1f
							break;
						case 3://icbcom3f
							Len = 100;
							break;
						case 4://rm
							//Len = 100;
				        	break;
		        	};
			  }
	      break;
	    case OPEN_CHANNAL:
	    case CLOSE_CHANNAL:
	    case AUTORIZATE:
	    case GET_ETYPE:
	    case GET_EISP:
	    case GET_E:
	    case GET_U:
	    case GET_I:
	    case GET_P:
	    case GET_KM:
	    case GET_F:
	    case GET_SERIAL:
	      Len = 500;break;

	    default:
	      Len = 0;
	      break;
	  };
	  return Len;
	}
	//=============================================================================
	sWord   TEnergomeraCE::RecvData(IPort* por, Byte *Buf, sWord DataLen, Word MaxLen, Byte subfase, sWord &refLen)
	{
	  ComPort *port = (ComPort*)por;
	  sWord   RecvLen = 0;
	  sWord bytes = 0;
	  if(DataLen > 0)
	  {
		  switch(subfase)
		  {
			case OPEN_CHANNAL:
			  RecvLen = port->RecvTo( Buf, MaxLen, 0x0A);//plc == 5 ? 0x0D : 0x0A);// 0x0A);
			  if(RecvLen > 4){
				refLen = RecvLen;
			  }
			  break;

			case CLOSE_CHANNAL:
			  //RecvLen = port->Recv( Buf, DataLen );
			  RecvLen = port->Recv( Buf, MaxLen );
			  if(RecvLen <= 0)
				  RecvLen = 5;

			  refLen = RecvLen;
			  //Log::DEBUG("TEnergomeraCE::RecvData refLen="+toString(refLen));
			  break;

			default:
			  RecvLen = port->RecvTo( Buf, MaxLen, 0x03 );
			  if(RecvLen > 4)
			  {
					if(plc == 5)
					{
						int bytes_to_read = port->GetBytesInBuffer();

						if(bytes_to_read >= 1){//working normal
							RecvLen += port->Recv( &Buf[RecvLen], 1 );
						}
						else{
							sleep(1);
							bytes_to_read = port->GetBytesInBuffer();
							if(bytes_to_read >= 1){//working normal
								RecvLen += port->Recv( &Buf[RecvLen], 1 );
							}
							else{
								Log::DEBUG("CounterManager TEnergomeraCE::RecvData bytes_to_read="+toString(bytes_to_read));
								if(LastCmdPointer == NULL){
									LastCmdPointer = new TBuffer(RecvLen);
									LastCmdPointer->DataLen = TBuffer::CopyBuffer(LastCmdPointer->BUF, Buf, RecvLen);
								}
							}
						}
					}
					else{//working normal
						RecvLen += port->Recv( &Buf[RecvLen], 1 );
					}
					//
					refLen = RecvLen;
			  }
			  break;
		  };
	  }
	  return RecvLen;
	}
	//=============================================================================
	string TEnergomeraCE::GetStringValue(void)
	{
		string ReturnString = WordToString(Et)+","+ WordToString(En)+","+ E.GetValueString();
								  //E1.GetValueString()+
								  //E2.GetValueString()+
								  //E3.GetValueString()+
								  //E4.GetValueString()+
								  //E5.GetValueString()+
		ReturnString += CouterType.GetValueString();
		ReturnString += U.GetValueString();
		if(NeedFirstFlg == true)
		{
			//ReturnString += U.GetValueString();
			ReturnString += I.GetValueString();
			ReturnString += P.GetValueString();
			ReturnString += Q.GetValueString();
			ReturnString += S.GetValueString();
			ReturnString += KM.GetValueString();
			ReturnString += F.GetValueString();
		}
		ReturnString += SerialN.GetValueString()+WordToString(IsEnable)+',';
		ReturnString += "\r\n>,99,1,"+WordToString(Type)+',';
		return ReturnString;
	}
	//=============================================================================
	bool TEnergomeraCE::GetDateTime(void)
	{
		Log::DEBUG("TEnergomeraCE::GetDateTime");
		TRtcTime dt = SystemTime.GetTime();
		TEnergyCMD *newCmd;

		newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->DateTime = dt;
		newCmd->Type = 0;
		newCmd->RetLen = 255;
		newCmd->Data.push_back(0x06);
		newCmd->Data.push_back(0x30);
		newCmd->Data.push_back(0x35);
		newCmd->Data.push_back(0x31);
		newCmd->Data.push_back(0x0D);
		newCmd->Data.push_back(0x0A);
		newCmd->NeedCrc = false;
		newCmd->WithCloseChanal = false;
		//newCmd->NeedOpenChanal = true;
		ECMDMessages.push_back(newCmd);

        newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->DateTime = dt;
		newCmd->Type = 40;
		newCmd->RetLen = 255;
		newCmd->Data.push_back(0x01);
		newCmd->Data.push_back('R');
		newCmd->Data.push_back('1');
		newCmd->Data.push_back(0x02);
		newCmd->Data.push_back('D');
		newCmd->Data.push_back('A');
		newCmd->Data.push_back('T');
		newCmd->Data.push_back('E');
		newCmd->Data.push_back('_');
		newCmd->Data.push_back('(');
		newCmd->Data.push_back(')');
		newCmd->Data.push_back(0x03);
		newCmd->WithOpenChanal = false;
		newCmd->WithCloseChanal = false;
		ECMDMessages.push_back(newCmd);

		newCmd = new TEnergyCMD();
		newCmd->Etype = Et;
		newCmd->Ename = En;
		newCmd->DateTime = dt;
		newCmd->Type = 41;
		newCmd->RetLen = 255;
		newCmd->Data.push_back(0x01);
		newCmd->Data.push_back('R');
		newCmd->Data.push_back('1');
		newCmd->Data.push_back(0x02);
		newCmd->Data.push_back('T');
		newCmd->Data.push_back('I');
		newCmd->Data.push_back('M');
		newCmd->Data.push_back('E');
		newCmd->Data.push_back('_');
		newCmd->Data.push_back('(');
		newCmd->Data.push_back(')');
		newCmd->Data.push_back(0x03);
		newCmd->WithOpenChanal = false;
		newCmd->WithCloseChanal = true;;
		ECMDMessages.push_back(newCmd);
		return true;
	}
	//=============================================================================
	bool TEnergomeraCE::CorrectDateTime(string difftime)
	{//ss:mm:hh
		bool ret = false;
		//vector<string> data = Split(time, ":");
		//if(data.size() == 3)
		{
			Log::DEBUG("TEnergomeraCE::CorrectDateTime difftime="+difftime);
			TRtcTime dt = SystemTime.GetTime();
			TEnergyCMD *newCmd;

			newCmd = new TEnergyCMD();
			newCmd->Etype = Et;
			newCmd->Ename = En;
			newCmd->DateTime = dt;
			newCmd->Type = 0;
			newCmd->RetLen = 255;
			newCmd->Data.push_back(0x06);
			newCmd->Data.push_back(0x30);
			newCmd->Data.push_back(0x35);
			newCmd->Data.push_back(0x31);
			newCmd->Data.push_back(0x0D);
			newCmd->Data.push_back(0x0A);
			newCmd->NeedCrc = false;
			newCmd->WithCloseChanal = false;
			ECMDMessages.push_back(newCmd);

			newCmd = new TEnergyCMD();
			newCmd->Etype = Et;
			newCmd->Ename = En;
			newCmd->DateTime = dt;
			newCmd->Type = 41;
			newCmd->RetLen = 255;
			newCmd->Data.push_back(0x01);
			newCmd->Data.push_back('W');
			newCmd->Data.push_back('1');
			newCmd->Data.push_back(0x02);
			newCmd->Data.push_back('C');
			newCmd->Data.push_back('T');
			newCmd->Data.push_back('I');
			newCmd->Data.push_back('M');
			newCmd->Data.push_back('E');
			newCmd->Data.push_back('(');
			//diff
			for(int t = 0; t < difftime.size(); t++)
				newCmd->Data.push_back( difftime.c_str()[t] );

			newCmd->Data.push_back(')');
			newCmd->Data.push_back(0x03);
			newCmd->WithOpenChanal = false;
			newCmd->WithCloseChanal = true;;
			ECMDMessages.push_back(newCmd);
			ret = true;
		}
		return ret;
	}
	//=============================================================================
	bool TEnergomeraCE::SetDateTime(string datetime, string password)
	{//ss:mm:hh
		bool ret = false;
		vector<string> dt = Split(datetime, " ");
		Log::DEBUG( "SetDateTime datetime=["+datetime+"]");
		if(dt.size() == 2)
		{
			string date = dt[0];
			string time = dt[1];
			vector<string> data = Split(time, ":");
			vector<string> data1 = Split(date, ".");
			if(data.size() == 3 && data1.size() == 4)
			{
				time = data[2] + ":" + data[1] + ":" + data[0];
				Log::DEBUG("TEnergomeraCE::SetDateTime date=" + date + " time=" + time);
				TRtcTime dt = SystemTime.GetTime();
				TEnergyCMD *newCmd;

				newCmd = new TEnergyCMD();
				newCmd->Etype = Et;
				newCmd->Ename = En;
				newCmd->DateTime = dt;
				newCmd->Type = 0;
				newCmd->RetLen = 255;
				newCmd->Data.push_back(0x06);
				newCmd->Data.push_back(0x30);
				newCmd->Data.push_back(0x35);
				newCmd->Data.push_back(0x31);
				newCmd->Data.push_back(0x0D);
				newCmd->Data.push_back(0x0A);
				newCmd->NeedCrc = false;
				newCmd->WithCloseChanal = false;
				ECMDMessages.push_back(newCmd);

		        newCmd = new TEnergyCMD();
				newCmd->Etype = Et;
				newCmd->Ename = En;
				newCmd->DateTime = dt;
				newCmd->Type = 40;
				newCmd->RetLen = 255;
				newCmd->Data.push_back(0x01);
				newCmd->Data.push_back('W');//W
				newCmd->Data.push_back('1');
				newCmd->Data.push_back(0x02);
				newCmd->Data.push_back('D');
				newCmd->Data.push_back('A');
				newCmd->Data.push_back('T');
				newCmd->Data.push_back('E');
				newCmd->Data.push_back('_');
				newCmd->Data.push_back('(');
				//date
				for(int t = 0; t < date.size(); t++)
					newCmd->Data.push_back( date.c_str()[t] );

				newCmd->Data.push_back(')');
				newCmd->Data.push_back(0x03);
				newCmd->WithOpenChanal = false;
				newCmd->WithCloseChanal = false;
				ECMDMessages.push_back(newCmd);

				newCmd = new TEnergyCMD();
				newCmd->Etype = Et;
				newCmd->Ename = En;
				newCmd->DateTime = dt;
				newCmd->Type = 41;
				newCmd->RetLen = 255;
				newCmd->Data.push_back(0x01);
				newCmd->Data.push_back('W');//W
				newCmd->Data.push_back('1');
				newCmd->Data.push_back(0x02);
				newCmd->Data.push_back('T');
				newCmd->Data.push_back('I');
				newCmd->Data.push_back('M');
				newCmd->Data.push_back('E');
				newCmd->Data.push_back('_');
				newCmd->Data.push_back('(');
				//time
				for(int t = 0; t < time.size(); t++)
					newCmd->Data.push_back( time.c_str()[t] );

				newCmd->Data.push_back(')');
				newCmd->Data.push_back(0x03);
				newCmd->WithOpenChanal = false;
				newCmd->WithCloseChanal = true;;
				ECMDMessages.push_back(newCmd);
				ret = true;
			}
		}
		return ret;
	}
	//=============================================================================
	string TEnergomeraCE::GetEcmd(TEnergyCMD* ecmd, Byte *Buffer, Word len, TReturnValue *r)
	{
		//Log::DEBUG("[CounterManager] TEnergomeraCE::GetEcmd");
		string framStr = "";
		if(len > 0)
		{
			if (ecmd->Type == 40)//get date
			{
				vector<string> Result = GetDataFromMessage(Buffer, len);
				this->DateTime = "";
				if(Result.size() >= 1)
				{
					vector<string> lst = TBuffer::Split(Result[0], ".");
					if(lst.size() == 4)
					{
						  string day = toString(lst[1]);
						  string mon = toString(lst[2]);
						  string yea = toString(lst[3]);

						  int y = atoi(yea.c_str()) + 2000;
						  this->DateTime = day+"."+mon+"."+toString(y);

						  //Log::DEBUG("[CounterManager] TEnergomeraCE::GetEcmd DateTime="+DateTime);
					}
				}
				if(r!=NULL)
				{
					//r->IsComplite(true);
				}
			}
			else if(ecmd->Type == 41)
			{
				vector<string> Result = GetDataFromMessage(Buffer, len);
				if(Result.size() >= 1 && this->DateTime.size() > 0)
				{
					string date = this->DateTime;
					string time = Result[0];
					vector<string> lst = TBuffer::Split(time, ":");
					if(lst.size() == 3)
					{
						string sec = toString(lst[2]);
						string min = toString(lst[1]);
					 	string hou = toString(lst[0]);

						int t = 1;//winter type
						string timetype = toString(t);
						string tt = WordToString(40);// ecmd->Type );

						string ret = this->DateTime + " " + hou + ":" + min + ":" + sec;
						framStr += WordToString(ecmd->Etype)+',';
						framStr += WordToString( ecmd->Ename )+',';
						framStr +=">,51,1,"+tt+','+ret+","+timetype+',';//timetype=zima|leto

						//Log::DEBUG("[CounterManager] TEnergomeraCE::GetEcmd framStr="+framStr);

						if(r!=NULL)
						{
							r->Data.push_back(tt);
							r->Data.push_back(ret);
							r->Data.push_back(timetype);
							//r->IsComplite(true);
						}
					}
					if(r!=NULL)
					{
						r->IsComplite(true);
					}
				}
			}
		}
		return framStr;
	}
	//=============================================================================
	void TEnergomeraCE::GetType(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			Log::DEBUG( "[TEnergomeraCE] GetType LastCmd="+LastCmd);
			string Tmp = "Energomera";
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0){
			 	Tmp = Result[0];
			 	CouterType.Type = Tmp;
			}
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetEisp(Byte *Buffer, Word len)
	{
		/*
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			Log::DEBUG( "[TEnergomeraCE] GetType LastCmd="+LastCmd);
			string Tmp = "Energomera";
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0){
			 	Tmp = Result[0];
			 	CouterType.Type = Tmp;
			}
		}*/
	}
	//=============================================================================
	void TEnergomeraCE::GetEa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetEa LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
			{
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) E.Ea = tmp;

				if(Result.size() > 1){
					tmp = atof( Result[1].c_str() );
					if(tmp < 3000000.0) E1.Ea = tmp;
				}
				if(Result.size() > 2){
					tmp = atof( Result[2].c_str() );
					if(tmp < 3000000.0) E2.Ea = tmp;
				}
				if(Result.size() > 3){
					tmp = atof( Result[3].c_str() );
					if(tmp < 3000000.0) E3.Ea = tmp;
				}
				if(Result.size() > 4){
					tmp = atof( Result[4].c_str() );
					if(tmp < 3000000.0) E4.Ea = tmp;
				}
			}
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetEr(Byte *Buffer, Word len)
	{
		if(len > 0)// && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 //Log::DEBUG( "[TEnergomeraCE] GetEr LastCmd="+LastCmd);
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) E.Er = tmp;
			 }
		}
	}
	//=============================================================================
	float TEnergomeraCE::GetEt(int tindex, Byte etype, Byte *Buffer, Word len)
	{
		float ret = 0.0;
		if(len > 0)
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > tindex){
				 ret = atof( Result[tindex].c_str() );
			 }
		}
		return ret;
	}
	//=============================================================================
	void TEnergomeraCE::GetEa_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}
	//=============================================================================
	void TEnergomeraCE::GetEr_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}
	//=============================================================================
	float TEnergomeraCE::GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len)
	{
		float ret = 0.0;
		if(len > 0){

		}
		return ret;
	}
	//=============================================================================
	void TEnergomeraCE::GetUa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetUa LastCmd="+LastCmd);
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) U.Ua.CalcMinAverMax(tmp);

				//U.Ua.CalcMinAverMax(atof( Result[0].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetUb(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetUb LastCmd="+LastCmd);
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 1){
				float tmp = 0.0;
				tmp = atof( Result[1].c_str() );
				if(tmp < 3000000.0) U.Ub.CalcMinAverMax(tmp);
				 //U.Ub.CalcMinAverMax(atof( Result[1].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetUc(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetUc LastCmd="+LastCmd);
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 2){
				float tmp = 0.0;
				tmp = atof( Result[2].c_str() );
				if(tmp < 3000000.0) U.Uc.CalcMinAverMax(tmp);
				 //U.Uc.CalcMinAverMax(atof( Result[2].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetIa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetIa LastCmd="+LastCmd);
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) I.Ia.CalcMinAverMax(tmp);
				 //I.Ia.CalcMinAverMax(atof( Result[0].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetIb(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetIb LastCmd="+LastCmd);
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 1){
				float tmp = 0.0;
				tmp = atof( Result[1].c_str() );
				if(tmp < 3000000.0) I.Ib.CalcMinAverMax(tmp);
				 //I.Ib.CalcMinAverMax(atof( Result[1].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetIc(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetIc LastCmd="+LastCmd);
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 2){
				float tmp = 0.0;
				tmp = atof( Result[2].c_str() );
				if(tmp < 3000000.0) I.Ic.CalcMinAverMax(tmp);
				 //I.Ic.CalcMinAverMax(atof( Result[2].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetPa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetPa LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				P.Pa.CalcMinAverMax( atof(Result[0].c_str())*1000.0 );
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetPb(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetPb LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 1)
				P.Pb.CalcMinAverMax( atof(Result[1].c_str())*1000.0 );
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetPc(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetPc LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 2)
				P.Pc.CalcMinAverMax( atof(Result[2].c_str())*1000.0 );
		}
	}
	//=============================================================================
	void TEnergomeraCE::GetQa(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 0)
		 Q.Qa.CalcMinAverMax( atof(Result[0].c_str() )*1000.0 );
	}
	//=============================================================================
	void TEnergomeraCE::GetQb(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 1)
		 Q.Qb.CalcMinAverMax( atof( Result[1].c_str() )*1000.0 );
	}
	//=============================================================================
	void TEnergomeraCE::GetQc(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 2)
		 Q.Qc.CalcMinAverMax( atof( Result[2].c_str() )*1000.0 );

	}
	//=============================================================================
	float TEnergomeraCE::GetKM(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetKM LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				return atof( Result[0].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	float TEnergomeraCE::GetKMa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetKMa LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 1)
				return atof( Result[1].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	float TEnergomeraCE::GetKMb(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetKMb LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 2)
				return atof( Result[2].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	float TEnergomeraCE::GetKMc(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetKMc LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 3)
				return atof( Result[3].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	float TEnergomeraCE::GetF(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetF LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				return atof( Result[0].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	string TEnergomeraCE::GetSerial(Byte *Buffer, Word len)
	{
		string ret = "00000000";
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetType LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			//Log::DEBUG("TEnergomeraCE::GetSerial GetDataFromMessage="+Result[0]);
			if(Result.size() > 0 && !isNullOrWhiteSpace(Result[0]))
				ret= Result[0];
			//Log::DEBUG("TEnergomeraCE::GetSerial="+toString(ret));
		}
		return ret;
	}
	//=============================================================================
	Byte TEnergomeraCE::CalcBCC(Byte *BUF, Word Len )
	{
		//Byte bf[] = {0,0};
		//string ccc;
		Byte ret = 0;
        for (int i = 1; i < Len; i++)
        {
        	Byte b = BUF[i];
        	//ByteToHex(bf, b);
        	//ccc += "0x"+string((char*)bf, 2)+" ";
        	ret += b;
        	if (b == 0x03)
        		break;
        }
        ret &= 0x7F;
        //Log::DEBUG( "TEnergomeraCE::CalcBCC Len="+toString((int)Len));
        //Log::DEBUG( "TEnergomeraCE::CalcBCC="+ccc);
		return ret;
	}
	//=============================================================================
	bool TEnergomeraCE::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
	{
		//Log::DEBUG( "TEnergomeraCE::ParsingAnswer subFase="+toString((int)(subFase))+" Len="+toString(Len));
	  bool ret = false;

	  if(subFase == SET_PLC || subFase == GET_EISP)
		  return true;
	  if(Len == 0)
		  return true;

	  //if(Len == 1 && BUF[0] == 0x15)//wrong parameter
		//  return true;

	  if(Len > 0 && Len < 4)
		  return false;
	  else
	  {
		  switch(subFase)
		  {
		    case OPEN_CHANNAL:
		    case CLOSE_CHANNAL:
		    case AUTORIZATE:
				if(LastCmdPointer != NULL){
					delete LastCmdPointer;
					LastCmdPointer = NULL;
				}
		    	ret = true;
		      break;

		    default:

					int ind1 = TBuffer::find_first_of(BUF, Len, 0x01);
					int ind2 = TBuffer::find_first_of(BUF, Len, 0x02);
					if(  ind1 < 0 && ind2 < 0 )
						ret = false;
					else
					{
						int ind = 0;
						if(ind1 < ind2){
							ind = ind1;
							if(ind < 0){
								ind = ind2;
							}
						}
						Byte calcCrc = CalcBCC(&BUF[ind], Len-ind);
						Byte messCrc = (Word)BUF[Len-1];
						if(plc == 5 && BUF[Len-2] != 0x03 && ind > 0)
						{
							messCrc = (Word)BUF[ind-1];
							Log::DEBUG("Counter detect PLC error new CRC="+toString((Word)messCrc) + " Len="+toString(Len));
							if(LastCmdPointer != NULL )
							{
								Log::DEBUG("Counter detect PLC error Len="+toString(Len) + " LastCmdPointer->DataLen="+toString(LastCmdPointer->DataLen));
								if(Len >= LastCmdPointer->DataLen){
									ind1 = TBuffer::find_first_of(LastCmdPointer->BUF, LastCmdPointer->DataLen, 0x01);
									ind2 = TBuffer::find_first_of(LastCmdPointer->BUF, LastCmdPointer->DataLen, 0x02);
									if(  ind1 < 0 && ind2 < 0 )
										ret = false;
									else
									{
										ind = 0;
										if(ind1 < ind2){
											ind = ind1;
											if(ind < 0){
												ind = ind2;
											}
										}

										Len = TBuffer::CopyBuffer(BUF, &LastCmdPointer->BUF[ind], (LastCmdPointer->DataLen-ind));
										calcCrc = CalcBCC(BUF, Len);
										Log::DEBUG("Counter detect PLC error new BUF LEN="+toString(Len) );

										delete LastCmdPointer;
										LastCmdPointer = NULL;
									}
								}

							}
						}
						if(messCrc == calcCrc)
							ret = true;
						else
						{
							  ///////////////////////////
							  Byte bf[2] {0,0};
							  string recv = "";
							  for(int i = 0; i < Len; i++)
							  {
								  ByteToHex(bf, BUF[i]);
								  recv += "0x"+string((char*)bf, 2)+" ";
							  }
							  Log::DEBUG( "[CounterManager] messCrc=" + toString((int)messCrc) +  " calcCrc=" + toString((int)calcCrc) + "  BUF: [" + recv +"]");
							ret = false;
						}
					}
		      break;
		  };
	  }
	  return ret;
	}
	//=============================================================================
	vector<string> TEnergomeraCE::GetDataFromMessage( Byte *BUF, Word Len)
	{
	  vector<string> retVal;
	  vector<string> Result;
	  string sep = "\r\n";

	  TBuffer::Split(BUF, Len, sep, Result);
	  for(int i = 0; i < Result.size(); i++)
	  {
	      string curr = Result[i];
	      if(curr.size() > 2)
	      {
	        char stx = '(';
	        char etx = ')';
	        int start = TBuffer::find_first_of( (Byte*)curr.c_str(), Len, stx );
	        int end   = TBuffer::find_first_of( (Byte*)curr.c_str(), Len, etx );
	        if(start >=0 && end > 0)
	        {//ok
	          start++;
	          //end--;
	          if( start < end )
	          {
	            int size = (end-start);
	            retVal.push_back( string( (char*)&curr.c_str()[start], size) );
	          }
	        }
	      }
	  }
	  return retVal;
	}
	//=============================================================================
	//*****************************************************************************
	//*** TNevaMT class
	//*****************************************************************************
	TNevaMT::TNevaMT(Byte Ename, string adr):ICounter(Ename, atoi(adr.c_str())), Address(adr)
	{
	  Modify = false;
	  Type = 5;
	  LastCmd = "";
	  LastCmdPointer = NULL;
	}
	//=============================================================================
	TNevaMT::~TNevaMT( )
	{
	}
	//=============================================================================
	Word TNevaMT::CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *ecmd)
	{
	 Word DataLen = 0;
	 Bshort Tmp;
	 LastCmd = "";

	    switch(CMD)
	    {
		  case SET_PLC: // test connection
		        if(Index == 0)
		        {
		        	switch(plc)
		        	{
						case 1://nzif
					          Buffer[DataLen++] = 0xFC;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x00;
					          Buffer[DataLen++] = 0x03;
					          Buffer[DataLen++] = 0x2E;
					          Buffer[DataLen++] = 0x0B;
					          Buffer[DataLen++] = 0x02;
					          Buffer[DataLen++] = 0x42;
					          Buffer[DataLen++] = 0x3A;
					          Buffer[DataLen++] = 0x35;
					          Buffer[DataLen++] = 0xC7;
					          Tmp.Data_s = Crc16(Buffer, DataLen);
					          Buffer[DataLen++] = Tmp.Data_b[0];
					          Buffer[DataLen++] = Tmp.Data_b[1];
							break;
						case 2://icbcom1f
							break;

						case 3://icbcom3f
				        	if(cmd.size() > 0 && cmd.size() % 2 == 0)
				        	{
				        		for(int ind = 0; ind < cmd.size()/2; ind++)
				        		{
				        			Buffer[DataLen++] = HexToByte( (char*)&cmd.c_str()[ind*2] );
				        		}
				    	        //Tmp.Data_s = Crc16(Buffer, DataLen);
				    	        //Buffer[DataLen++] = Tmp.Data_b[0];
				    	        //Buffer[DataLen++] = Tmp.Data_b[1];
				        	}
							break;
						case 4://rm
				        	if(cmd.size() > 0)
				        	{
				        		for(int ind = 0; ind < cmd.size(); ind++){
				        			Buffer[DataLen++] = cmd.c_str()[ind];
				        		}
				        	}
				        	break;
		        	};
		        }
			break;

	      case OPEN_CHANNAL:
	        DataLen = 0;
	        Buffer[DataLen++] = '/';
	        Buffer[DataLen++] = '?';
	        if(Address.size() > 0 && atoi(Address.c_str()) > 0){
	          for(int ind = 0; ind < Address.size(); ind++){
	            Buffer[DataLen++] = Address.c_str()[ind];
	          }
	        }
	        Buffer[DataLen++] = '!';
	        Buffer[DataLen++] = 0x0D;
	        Buffer[DataLen++] = 0x0A;
	        break;

	      case AUTORIZATE:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x06;
	        Buffer[DataLen++] = 0x30;
	        Buffer[DataLen++] = 0x35;
	        Buffer[DataLen++] = 0x31;
	        Buffer[DataLen++] = 0x0D;
	        Buffer[DataLen++] = 0x0A;
	        break;

	      case GET_ETYPE:
	    	DataLen = 0;
			Buffer[DataLen++] = 0x01;
			Buffer[DataLen++] = 'P';
			Buffer[DataLen++] = '1';
			Buffer[DataLen++] = 0x02;
			Buffer[DataLen++] = '(';

			if(ecmd!=NULL && ecmd->Password.size() > 0){
				for(auto currB: ecmd->Password)
		    		Buffer[DataLen++] = currB;
				ecmd->Password.clear();
		    }
		    else{
		    	for(int i = 0; i < 8;i++)
		    		Buffer[DataLen++] = 0x30;
		    }
			Buffer[DataLen++] = ')';
			Buffer[DataLen++] = 0x03;
			Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_E:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "0F0880FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;


	      case GET_Ua:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "200700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Ub:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "340700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Uc:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "480700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Ia:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "1F0700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Ib:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "330700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Ic:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "470700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Pa:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "240700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Pb:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "380700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Pc:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "4C0700FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Qa:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "170701FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Qb:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "2B0701FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Qc:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "3F0701FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_KM:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "0D07FFFF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_F:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "0E0701FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];


	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_SERIAL:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;

		    LastCmd = "600100FF";
		    for(int i=0; i < LastCmd.size(); i++)
		    	Buffer[DataLen++] = LastCmd.c_str()[i];

	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x5E;
	        break;

	      case CLOSE_CHANNAL:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'B';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x75;
	        break;

	      default:
	        DataLen = 0;
	        break;
	    };

	    if(DataLen > 0)
	    {
			if(CMD != SET_PLC && plc == 3)
			{
				Byte* tmpBUF = new Byte[DataLen+5];
				Word dataInd = 0;
				tmpBUF[dataInd++] = 0x0A;
				tmpBUF[dataInd++] = 0x61;
				tmpBUF[dataInd++] = DataLen;

				for(int i=0; i < DataLen; i++){
					tmpBUF[dataInd++] = Buffer[i];
				}
		        Tmp.Data_s = Crc16(tmpBUF, dataInd);
		        tmpBUF[dataInd++] = Tmp.Data_b[0];
		        tmpBUF[dataInd++] = Tmp.Data_b[1];
				for(int i=0; i < dataInd; i++){
					Buffer[i] = tmpBUF[i];
				}
				DataLen = dataInd;
				delete [] tmpBUF;
			}
	    }
	    return DataLen;
	}
	//=============================================================================
	Word TNevaMT::GetRecvMessageLen(Byte CMD)
	{
	  Word Len = 0;
	  switch(CMD)
	  {
	    case SET_PLC:
			  if(Index == 0)
			  {
		        	switch(plc)
		        	{
						case 1://nzif
							Len = 8;
							break;
						case 2://icbcom1f
							break;
						case 3://icbcom3f
							Len = 100;
							break;
						case 4://rm
							//Len = 100;
				        	break;
		        	};
			  }
	      break;
	    case GET_ETYPE:
	    	Len = 1;break;

	    case OPEN_CHANNAL:
	    case CLOSE_CHANNAL:
	    case AUTORIZATE:
	    case GET_E:
	    case GET_ER:
	    case GET_Ua:
	    case GET_Ub:
	    case GET_Uc:
	    case GET_Ia:
	    case GET_Ib:
	    case GET_Ic:
	    case GET_Pa:
	    case GET_Pb:
	    case GET_Pc:
	    case GET_Qa:
	    case GET_Qb:
	    case GET_Qc:
	    case GET_KM:
	    case GET_SERIAL:
	    case GET_F:
	      Len = 500;break;

	    default:
	      Len = 0;
	      break;
	  };
	  return Len;
	}
	//=============================================================================
	sWord   TNevaMT::RecvData(IPort* por, Byte *Buf, sWord DataLen, Word MaxLen, Byte subfase, sWord &refLen)
	{
	  ComPort *port = (ComPort*)por;
	  sWord   RecvLen = 0;
	  sWord bytes = 0;
	  if(DataLen > 0)
	  {
		  switch(subfase)
		  {
			case OPEN_CHANNAL:
			  RecvLen = port->RecvTo( Buf, MaxLen, 0x0A);//plc == 5 ? 0x0D : 0x0A);// 0x0A);
			  if(RecvLen > 4){
				refLen = RecvLen;
			  }
			  break;

			case GET_ETYPE:
			  RecvLen = port->Recv( Buf, DataLen );
			  if(RecvLen > 0){
				refLen = RecvLen;
			  }
			  break;

			case CLOSE_CHANNAL:
			  //RecvLen = port->Recv( Buf, DataLen );
			  RecvLen = port->Recv( Buf, MaxLen );
			  if(RecvLen <= 0)
				  RecvLen = 5;
			  refLen = RecvLen;
			  break;

			default:
			  RecvLen = port->RecvTo( Buf, MaxLen, 0x03 );
			  if(RecvLen > 4)
			  {
					if(plc == 5)
					{
						int bytes_to_read = port->GetBytesInBuffer();

						if(bytes_to_read >= 1){//working normal
							RecvLen += port->Recv( &Buf[RecvLen], 1 );
						}
						else{
							sleep(1);
							bytes_to_read = port->GetBytesInBuffer();
							if(bytes_to_read >= 1){//working normal
								RecvLen += port->Recv( &Buf[RecvLen], 1 );
							}
							else{
								Log::DEBUG("CounterManager TNevaMT::RecvData bytes_to_read="+toString(bytes_to_read));
								if(LastCmdPointer == NULL){
									LastCmdPointer = new TBuffer(RecvLen);
									LastCmdPointer->DataLen = TBuffer::CopyBuffer(LastCmdPointer->BUF, Buf, RecvLen);
								}
							}
						}
					}
					else{//working normal
						RecvLen += port->Recv( &Buf[RecvLen], 1 );
					}
					//
					refLen = RecvLen;
			  }
			  break;
		  };
	  }
	  return RecvLen;
	}
	//=============================================================================
	string TNevaMT::GetStringValue(void)
	{
		string ReturnString = WordToString(Et)+","+ WordToString(En)+","+ E.GetValueString();
								  //E1.GetValueString()+
								  //E2.GetValueString()+
								  //E3.GetValueString()+
								  //E4.GetValueString()+
								  //E5.GetValueString()+
		ReturnString += CouterType.GetValueString();
		ReturnString += U.GetValueString();
		if(NeedFirstFlg == true)
		{
			//ReturnString += U.GetValueString();
			ReturnString += I.GetValueString();
			ReturnString += P.GetValueString();
			ReturnString += Q.GetValueString();
			ReturnString += S.GetValueString();
			ReturnString += KM.GetValueString();
			ReturnString += F.GetValueString();
		}
		ReturnString += SerialN.GetValueString()+WordToString(IsEnable)+',';
		ReturnString += "\r\n>,99,1,"+WordToString(Type)+',';
		return ReturnString;
	}
	//=============================================================================
	void TNevaMT::GetAdditionalData(Byte *Buffer, Word len){
		if(len > 0){
			int ind = TBuffer::find_first_of(Buffer, len, '/');
			if(ind >= 0 && len > (ind + 3)){
				CouterType.Type = string((char*)&Buffer[ind+1], (len - (ind+3)));
			}
		}
	}
	//=============================================================================
	void TNevaMT::GetType(Byte *Buffer, Word len)
	{
	}
	//=============================================================================
	void TNevaMT::GetEa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetEa LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
			{
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) E.Ea = tmp;

				if(Result.size() > 1){
					tmp = atof( Result[1].c_str() );
					if(tmp < 3000000.0) E1.Ea = tmp;
				}
				if(Result.size() > 2){
					tmp = atof( Result[2].c_str() );
					if(tmp < 3000000.0) E2.Ea = tmp;
				}
				if(Result.size() > 3){
					tmp = atof( Result[3].c_str() );
					if(tmp < 3000000.0) E3.Ea = tmp;
				}
				if(Result.size() > 4){
					tmp = atof( Result[4].c_str() );
					if(tmp < 3000000.0) E4.Ea = tmp;
				}
			}
		}
	}
	//=============================================================================
	void TNevaMT::GetEr(Byte *Buffer, Word len)
	{
		if(len > 0)// && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 //Log::DEBUG( "[TEnergomeraCE] GetEr LastCmd="+LastCmd);
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) E.Er = tmp;
			 }
		}
	}
	//=============================================================================
	float TNevaMT::GetEt(int tindex, Byte etype, Byte *Buffer, Word len)
	{
		float ret = 0.0;
		if(len > 0)
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > tindex){
				 ret = atof( Result[tindex].c_str() );
			 }
		}
		return ret;
	}
	//=============================================================================
	void TNevaMT::GetEa_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}
	//=============================================================================
	void TNevaMT::GetEr_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}
	//=============================================================================
	float TNevaMT::GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len)
	{
		float ret = 0.0;
		if(len > 0){

		}
		return ret;
	}
	//=============================================================================
	void TNevaMT::GetUa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0)
					U.Ua.CalcMinAverMax(tmp);
			 }
		}
	}
	//=============================================================================
	void TNevaMT::GetUb(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) U.Ub.CalcMinAverMax(tmp);
			 }
		}
	}
	//=============================================================================
	void TNevaMT::GetUc(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) U.Uc.CalcMinAverMax(tmp);
			 }
		}
	}
	//=============================================================================
	void TNevaMT::GetIa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) I.Ia.CalcMinAverMax(tmp);
			 }
		}
	}
	//=============================================================================
	void TNevaMT::GetIb(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) I.Ib.CalcMinAverMax(tmp);
			 }
		}
	}
	//=============================================================================
	void TNevaMT::GetIc(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0) I.Ic.CalcMinAverMax(tmp);
			 }
		}
	}
	//=============================================================================
	void TNevaMT::GetPa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				P.Pa.CalcMinAverMax( atof(Result[0].c_str())*1.0 );
		}
	}
	//=============================================================================
	void TNevaMT::GetPb(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				P.Pb.CalcMinAverMax( atof(Result[0].c_str())*1.0 );
		}
	}
	//=============================================================================
	void TNevaMT::GetPc(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				P.Pc.CalcMinAverMax( atof(Result[0].c_str())*1.0 );
		}
	}
	//=============================================================================
	void TNevaMT::GetQa(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 0)
		 Q.Qa.CalcMinAverMax( atof(Result[0].c_str() )*1.0 );
	}
	//=============================================================================
	void TNevaMT::GetQb(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 0)
		 Q.Qb.CalcMinAverMax( atof( Result[0].c_str() )*1.0 );
	}
	//=============================================================================
	void TNevaMT::GetQc(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 0)
		 Q.Qc.CalcMinAverMax( atof( Result[0].c_str() )*1.0 );

	}
	//=============================================================================
	float TNevaMT::GetKM(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				float tmp = 0.0;
				tmp = atof( Result[0].c_str() );
				if(tmp < 3000000.0)
					return tmp;
			 }
		}
		return 0.0;
	}
	//=============================================================================
	float TNevaMT::GetKMa(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetKMa LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				return atof( Result[0].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	float TNevaMT::GetKMb(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetKMb LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				return atof( Result[0].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	float TNevaMT::GetKMc(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetKMc LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				return atof( Result[0].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	float TNevaMT::GetF(Byte *Buffer, Word len)
	{
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			//Log::DEBUG( "[TEnergomeraCE] GetF LastCmd="+LastCmd);
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0)
				return atof( Result[0].c_str() );
		}
		return 0.0;
	}
	//=============================================================================
	string TNevaMT::GetSerial(Byte *Buffer, Word len)
	{
		string ret = "00000000";
		if(len > 0 && !isNullOrWhiteSpace(LastCmd) && TBuffer::find(Buffer, len, LastCmd) >=0 )
		{
			vector<string> Result = GetDataFromMessage(Buffer, len);
			if(Result.size() > 0 && !isNullOrWhiteSpace(Result[0]))
				ret= Result[0];
		}
		return ret;
	}
	//=============================================================================
	Byte TNevaMT::CalcBCC(Byte *BUF, Word Len )
	{
		Byte ret = 0;
        for (int i = 1; i < Len; i++){
        	Byte b = BUF[i];
        	ret ^= b;
        	if (b == 0x03)
        		break;
        }
        ret &= 0x7F;
		return ret;
	}
	//=============================================================================
	bool TNevaMT::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
	{
		//Log::DEBUG( "TEnergomeraCE::ParsingAnswer subFase="+toString((int)(subFase))+" Len="+toString(Len));
	  bool ret = false;

	  if(subFase == SET_PLC)
		  return true;
	  if(Len == 0)
		  return true;

	  if(Len > 1 && Len < 4)
		  return false;
	  else
	  {
		  switch(subFase)
		  {
		    case OPEN_CHANNAL:
		    case CLOSE_CHANNAL:
		    case AUTORIZATE:
				if(LastCmdPointer != NULL){
					delete LastCmdPointer;
					LastCmdPointer = NULL;
				}
		    	ret = true;
		      break;

		    case GET_ETYPE:
		    	if(Len == 1) ret = true;
		      break;


		    default:
					int ind1 = TBuffer::find_first_of(BUF, Len, 0x01);
					int ind2 = TBuffer::find_first_of(BUF, Len, 0x02);
					if(  ind1 < 0 && ind2 < 0 )
						ret = false;
					else
					{
						int ind = 0;
						if(ind1 < ind2){
							ind = ind1;
							if(ind < 0){
								ind = ind2;
							}
						}
						Byte calcCrc = CalcBCC(&BUF[ind], Len-ind);
						Byte messCrc = (Word)BUF[Len-1];
						if(plc == 5 && BUF[Len-2] != 0x03 && ind > 0)
						{
							messCrc = (Word)BUF[ind-1];
							Log::DEBUG("Counter detect PLC error new CRC="+toString((Word)messCrc) + " Len="+toString(Len));
							if(LastCmdPointer != NULL )
							{
								Log::DEBUG("Counter detect PLC error Len="+toString(Len) + " LastCmdPointer->DataLen="+toString(LastCmdPointer->DataLen));
								if(Len >= LastCmdPointer->DataLen){
									ind1 = TBuffer::find_first_of(LastCmdPointer->BUF, LastCmdPointer->DataLen, 0x01);
									ind2 = TBuffer::find_first_of(LastCmdPointer->BUF, LastCmdPointer->DataLen, 0x02);
									if(  ind1 < 0 && ind2 < 0 )
										ret = false;
									else
									{
										ind = 0;
										if(ind1 < ind2){
											ind = ind1;
											if(ind < 0){
												ind = ind2;
											}
										}

										Len = TBuffer::CopyBuffer(BUF, &LastCmdPointer->BUF[ind], (LastCmdPointer->DataLen-ind));
										calcCrc = CalcBCC(BUF, Len);
										Log::DEBUG("Counter detect PLC error new BUF LEN="+toString(Len) );

										delete LastCmdPointer;
										LastCmdPointer = NULL;
									}
								}

							}
						}
						if(messCrc == calcCrc)
							ret = true;
						else
						{
							  ///////////////////////////
							  Byte bf[2] {0,0};
							  string recv = "";
							  for(int i = 0; i < Len; i++)
							  {
								  ByteToHex(bf, BUF[i]);
								  recv += "0x"+string((char*)bf, 2)+" ";
							  }
							  Log::DEBUG( "[CounterManager] messCrc=" + toString((int)messCrc) +  " calcCrc=" + toString((int)calcCrc) + "  BUF: [" + recv +"]");
							ret = false;
						}
					}
		      break;
		  };
	  }
	  return ret;
	}
	//=============================================================================
	vector<string> TNevaMT::GetDataFromMessage( Byte *BUF, Word Len)
	{
	  vector<string> retVal;
	  vector<string> Result;
	  string sep = "\r\n";

	  TBuffer::Split(BUF, Len, sep, Result);
	  for(int i = 0; i < Result.size(); i++)
	  {
	      string curr = Result[i];
	      if(curr.size() > 2)
	      {
	        char stx = '(';
	        char etx = ')';
	        int start = TBuffer::find_first_of( (Byte*)curr.c_str(), Len, stx );
	        int end   = TBuffer::find_first_of( (Byte*)curr.c_str(), Len, etx );
	        if(start >=0 && end > 0)
	        {//ok
	          start++;
	          //end--;
	          if( start < end )
	          {
	            int size = (end-start);
	            retVal.push_back( string( (char*)&curr.c_str()[start], size) );
	          }
	        }
	      }
	  }
	  return retVal;
	}
	//=============================================================================
	//*****************************************************************************
	//*** TChintDts class
	//*****************************************************************************
	TChintDts::TChintDts(Byte Ename, string adr):ICounter(Ename, atoi(adr.c_str())), Address(adr)
	{
	  Modify = false;
	  Type = 3;
	}
	//=============================================================================
	TChintDts::~TChintDts( )
	{
	}
	//=============================================================================
	Word TChintDts::CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *ecmd)
	{
	 Word DataLen = 0;
	 Bshort       Tmp;

	    switch(CMD)
	    {
	      case SET_PLC:
		        DataLen = 0;
		        Buffer[DataLen++] = 0x01;
		        Buffer[DataLen++] = 0x42;
		        Buffer[DataLen++] = 0x30;
		        Buffer[DataLen++] = 0x03;
		        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x71;
	        break;
	      case OPEN_CHANNAL:
	        DataLen = 0;
	        Buffer[DataLen++] = '/';
	        Buffer[DataLen++] = '?';
	        if(Address.size() > 0 && atoi(Address.c_str()) > 0)
	        {
	          for(int ind = 0; ind < Address.size(); ind++)
	          {
	            Buffer[DataLen++] = Address.c_str()[ind];
	          }
	        }
	        Buffer[DataLen++] = '!';
	        Buffer[DataLen++] = 0x0D;
	        Buffer[DataLen++] = 0x0A;
	        break;

	      case AUTORIZATE:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x06;
	        Buffer[DataLen++] = 0x30;
	        Buffer[DataLen++] = 0x32;
	        Buffer[DataLen++] = 0x31;
	        Buffer[DataLen++] = 0x0D;
	        Buffer[DataLen++] = 0x0A;
	        break;

	      case GET_ETYPE:
		    DataLen = 0;
		    Buffer[DataLen++] = 0x01;
		    Buffer[DataLen++] = 'P';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = 0x02;
		    Buffer[DataLen++] = '(';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = '1';
		    Buffer[DataLen++] = ')';
		    Buffer[DataLen++] = 0x03;
		    Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x61;
		    break;

	      case GET_E:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '8';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = '*';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x74;
	        break;

	      case GET_ER:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '2';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '8';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = '*';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = '(';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = ')';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x74;
	        break;

	      case GET_Ua:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '3';
	        Buffer[DataLen++] = '2';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x57;
	        break;

	      case GET_Ub:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '5';
	        Buffer[DataLen++] = '2';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x51;
	        break;

	      case GET_Uc:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '2';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x53;
	        break;

	      case GET_Ia:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '3';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x54;
	        break;

	      case GET_Ib:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '5';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x52;
	        break;

	      case GET_Ic:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x50;
	        break;

	      case GET_Pa:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '2';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Pb:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '4';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Pc:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '6';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Qa:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '2';
	        Buffer[DataLen++] = '9';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Qb:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '4';
	        Buffer[DataLen++] = '9';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_Qc:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 'R';
	        Buffer[DataLen++] = '1';
	        Buffer[DataLen++] = 0x02;
	        Buffer[DataLen++] = '6';
	        Buffer[DataLen++] = '9';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '7';
	        Buffer[DataLen++] = '.';
	        Buffer[DataLen++] = '0';
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
	        break;

	      case GET_SERIAL:
		        DataLen = 0;
		        Buffer[DataLen++] = 0x01;
		        Buffer[DataLen++] = 'R';
		        Buffer[DataLen++] = '1';
		        Buffer[DataLen++] = 0x02;
		        Buffer[DataLen++] = '9';
		        Buffer[DataLen++] = '6';
		        Buffer[DataLen++] = '.';
		        Buffer[DataLen++] = '1';
		        Buffer[DataLen++] = '.';
		        Buffer[DataLen++] = '0';
		        Buffer[DataLen++] = 0x03;
		        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
		        break;

	      case CLOSE_CHANNAL:
	        DataLen = 0;
	        Buffer[DataLen++] = 0x01;
	        Buffer[DataLen++] = 0x42;
	        Buffer[DataLen++] = 0x30;
	        Buffer[DataLen++] = 0x03;
	        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x71;
	        break;



	      default:
	        DataLen = 0;
	        break;
	    };
	  return DataLen;
	}
	//=============================================================================
	Word TChintDts::GetRecvMessageLen(Byte CMD)
	{
	  Word Len = 0;
	  switch(CMD)
	  {
	    case GET_ETYPE:
	    	Len = 1;break;

	    case OPEN_CHANNAL:
	    case CLOSE_CHANNAL:
	    case AUTORIZATE:
	    case GET_E:
	    case GET_ER:
	    case GET_Ua:
	    case GET_Ub:
	    case GET_Uc:
	    case GET_Ia:
	    case GET_Ib:
	    case GET_Ic:
	    case GET_Pa:
	    case GET_Pb:
	    case GET_Pc:
	    case GET_Qa:
	    case GET_Qb:
	    case GET_Qc:
	    case GET_SERIAL:
	      Len = 100;break;

	    default:
	      Len = 0;
	      break;
	  };
	  return Len;
	}
	//=============================================================================
	sWord TChintDts::RecvData(IPort* por, Byte *Buf, sWord DataLen, Word MaxLen, Byte subfase, sWord &refLen)
	{
	  ComPort *port = (ComPort*)por;
	  sWord   RecvLen = 0;
	  sWord bytes = 0;

	  if(DataLen > 0)
	  {
		  switch(subfase)
		  {
			case OPEN_CHANNAL:
			  RecvLen = port->RecvTo( Buf, MaxLen, 0x0A);
			  if(RecvLen > 4){
				refLen = RecvLen;
			  }
			  break;

			case GET_ETYPE:
			  RecvLen = port->Recv( Buf, DataLen );
			  if(RecvLen > 0){
				refLen = RecvLen;
			  }
			  break;

			case CLOSE_CHANNAL:
			  //RecvLen = port->Recv( Buf, DataLen );
			  RecvLen = port->Recv( Buf, MaxLen );
			  if(RecvLen <= 0)
				  RecvLen = 5;
			  refLen = RecvLen;
			  break;

			default:
			  RecvLen = port->RecvTo( Buf, MaxLen, 0x03 );
			  if(RecvLen > 4){
				RecvLen += port->Recv( &Buf[RecvLen], 1 );
				refLen = RecvLen;
			  }
			  break;
		  };
	  }
	  return RecvLen;
	}
	//=============================================================================
	string TChintDts::GetStringValue(void)
	{
		string ReturnString = WordToString(Et)+","+
								WordToString(En)+","+
								  E.GetValueString()+
								  CouterType.GetValueString()+
								  U.GetValueString()+
								  I.GetValueString()+
								  P.GetValueString()+
								  //Q.GetValueString()+
								  S.GetValueString()+
								  //KM.GetValueString()+
								  //F.GetValueString()+
								  SerialN.GetValueString()+WordToString(IsEnable)+','+
								  "\r\n>,99,1,"+WordToString(Type)+',';
			  return ReturnString;
	}
	//=============================================================================
	void TChintDts::GetAdditionalData(Byte *Buffer, Word len){
		//Log::DEBUG( "[TChintDts] GetAdditionalData len="+toString(len)+" buffer="+string((char*)Buffer,len));
		if(len > 0){
			int ind = TBuffer::find_first_of(Buffer, len, '/');
			if(ind>=0){
				string tmp = string((char*)&Buffer[ind+1], 4);
				if(!isNullOrWhiteSpace(tmp))
					CouterType.Type = tmp;
			}
		}
	}

	//=============================================================================
	void TChintDts::GetType(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
		/*
	  string Tmp = "CHT";
	  vector<string> Result = GetDataFromMessage(Buffer, len);
	  if(Result.size() > 0)
	  {
		  Tmp = Result[0];
		  CouterType.Type = Tmp;
	  }
	 return Tmp;*/
	}
	//=============================================================================
	void TChintDts::GetEa(Byte *Buffer, Word len)
	{
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				 E.Ea=atof( Result[0].c_str() );
			 }
		}
	}
	//=============================================================================
	void TChintDts::GetEr(Byte *Buffer, Word len)
	{
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				 E.Er=atof( Result[0].c_str() );
			 }
		}
	}
	//=============================================================================
	float TChintDts::GetEt(int tindex, Byte etype, Byte *Buffer, Word len)
	{
		float ret = 0.0;
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > tindex){
				 ret=atof( Result[tindex].c_str() );
			 }
		}
		return ret;
	}
	//=============================================================================
	void TChintDts::GetEa_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}
	//=============================================================================
	void TChintDts::GetEr_minus(Byte *Buffer, Word len)
	{
		if(len > 0){

		}
	}
	//=============================================================================
	float TChintDts::GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len)
	{
		float ret = 0.0;
		if(len > 0){

		}
		return ret;
	}
	//=============================================================================
	void TChintDts::GetUa(Byte *Buffer, Word len)
	{
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				 U.Ua.CalcMinAverMax(atof( Result[0].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TChintDts::GetUb(Byte *Buffer, Word len)
	{
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				 U.Ub.CalcMinAverMax(atof( Result[0].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TChintDts::GetUc(Byte *Buffer, Word len)
	{
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				 U.Uc.CalcMinAverMax(atof( Result[0].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TChintDts::GetIa(Byte *Buffer, Word len)
	{
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				 I.Ia.CalcMinAverMax(atof( Result[0].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TChintDts::GetIb(Byte *Buffer, Word len)
	{
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				 I.Ib.CalcMinAverMax(atof( Result[0].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TChintDts::GetIc(Byte *Buffer, Word len)
	{
		if(len > 0){
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0){
				 I.Ic.CalcMinAverMax(atof( Result[0].c_str() ));
			 }
		}
	}
	//=============================================================================
	void TChintDts::GetPa(Byte *Buffer, Word len)
	{
		 vector<string> Result = GetDataFromMessage(Buffer, len);
		 if(Result.size() > 0)
			 P.Pa.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
	}
	//=============================================================================
	void TChintDts::GetPb(Byte *Buffer, Word len)
	{
		 vector<string> Result = GetDataFromMessage(Buffer, len);
		 if(Result.size() > 0)
			 P.Pb.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
	}
	//=============================================================================
	void TChintDts::GetPc(Byte *Buffer, Word len)
	{
		 vector<string> Result = GetDataFromMessage(Buffer, len);
		 if(Result.size() > 0)
			 P.Pc.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
	}
	//=============================================================================
	//=============================================================================
	void TChintDts::GetQa(Byte *Buffer, Word len)
	{
		 vector<string> Result = GetDataFromMessage(Buffer, len);
		 if(Result.size() > 0)
			 Q.Qa.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
	}
	//=============================================================================
	//=============================================================================
	void TChintDts::GetQb(Byte *Buffer, Word len)
	{
		 vector<string> Result = GetDataFromMessage(Buffer, len);
		 if(Result.size() > 0)
			 Q.Qb.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
	}
	//=============================================================================
	//=============================================================================
	void TChintDts::GetQc(Byte *Buffer, Word len)
	{
		 vector<string> Result = GetDataFromMessage(Buffer, len);
		 if(Result.size() > 0)
			 Q.Qc.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
	}
	//=============================================================================
	//=============================================================================
	//=============================================================================
	//=============================================================================
	//=============================================================================
	float TChintDts::GetKM(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 0)
	  return atof( Result[0].c_str() );
	 return 0.0;
	}
	//=============================================================================
	float TChintDts::GetKMa(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 1)
	  return atof( Result[1].c_str() );
	 return 0.0;
	}
	//=============================================================================
	float TChintDts::GetKMb(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 2)
	  return atof( Result[2].c_str() );
	 return 0.0;
	}
	//=============================================================================
	float TChintDts::GetKMc(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 3)
	  return atof( Result[3].c_str() );
	 return 0.0;
	}
	//=============================================================================

	float TChintDts::GetF(Byte *Buffer, Word len)
	{
	 vector<string> Result = GetDataFromMessage(Buffer, len);
	 if(Result.size() > 0)
	  return atof( Result[0].c_str() );
	 return 0.0;
	}
	//=============================================================================
	string TChintDts::GetSerial(Byte *Buffer, Word len)
	{
		string ret = "00000000";
		vector<string> Result = GetDataFromMessage(Buffer, len);
		if(Result.size() > 0 && !isNullOrWhiteSpace(Result[0]))
			  ret= Result[0];
		return ret;
	}
	//=============================================================================
	Byte TChintDts::CalcBCC(Byte *BUF, Word Len )
	{
		Byte ret = 0;
        for (int i = 1; i < Len; i++){
        	Byte b = BUF[i];
        	ret ^= b;
        	if (b == 0x03)
        		break;
        }
        ret &= 0x7F;
		return ret;
	}
	bool TChintDts::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
	{
		//Log::DEBUG( "TEnergomeraCE::ParsingAnswer subFase="+toString((int)(subFase))+" Len="+toString(Len));
	  bool ret = false;

	  if(subFase == SET_PLC)
		  return true;
	  if(Len == 0)
		  return true;

	  if(Len > 1 && Len < 4)
		  return false;
	  else
	  {
		  switch(subFase)
		  {
		    case OPEN_CHANNAL:
		    case CLOSE_CHANNAL:
		    case AUTORIZATE:
		    case GET_ETYPE:
		    	ret = true;
		      break;

		    default:
		    	int ind1 = TBuffer::find_first_of(BUF, Len, 0x01);
		    	int ind2 = TBuffer::find_first_of(BUF, Len, 0x02);
		    	if(  ind1 < 0 && ind2 <0 ){
		    		ret = false;
		    	}
		    	else{
		    		int ind = 0;
		    		if(ind1 < ind2){
		    			ind = ind1;
		    			if(ind < 0){
		    				ind = ind2;
		    			}
		    		}
		    		Byte calcCrc = CalcBCC(&BUF[ind], Len-ind);
		    		Byte messCrc = (Word)BUF[Len-1];
		    		if(messCrc == calcCrc)
		    			ret = true;
		    		else
		    			ret = false;

		    		//Log::DEBUG( "TEnergomeraCE::ParsingAnswer ind="+toString(ind));
		    		//Log::DEBUG( "TEnergomeraCE::calculated BCC="+toString((int)calcCrc));
		    		//Log::DEBUG( "TEnergomeraCE::message BCC="+toString((int)messCrc));
		    		//Log::DEBUG( "TEnergomeraCE::ParsingAnswer ind="+toString(ind));
		    	}
		      break;
		  };
	  }
	  return ret;
	}
	//=============================================================================
	vector<string> TChintDts::GetDataFromMessage( Byte *BUF, Word Len)
	{
	  vector<string> retVal;
	  vector<string> Result;
	  string sep = "\r\n";

	  TBuffer::Split(BUF, Len, sep, Result);
	  for(int i = 0; i < Result.size(); i++)
	  {
	      string curr = Result[i];
	      if(curr.size() > 2)
	      {
	        char stx = '(';
	        char etx = ')';
	        int start = TBuffer::find_first_of( (Byte*)curr.c_str(), Len, stx );
	        int end   = TBuffer::find_first_of( (Byte*)curr.c_str(), Len, etx );
	        if(start >=0 && end > 0)
	        {//ok
	          start++;
	          //end--;
	          if( start < end )
	          {
	            int size = (end-start);
	            retVal.push_back( string( (char*)&curr.c_str()[start], size) );
	          }
	        }
	      }
	  }
	  return retVal;
	}
	//=============================================================================

	//*****************************************************************************
	//*** THuabangDts class
	//*****************************************************************************
		THuabangDts::THuabangDts(Byte Ename, string adr):ICounter(Ename, atoi(adr.c_str())), Address(adr)
		{
		  Modify = false;
		  Type = 4;
		}
		//=============================================================================
		THuabangDts::~THuabangDts( )
		{
		}
		//=============================================================================
		Word THuabangDts::CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *ecmd)
		{
		 Word DataLen = 0;
		 Bshort       Tmp;

		    switch(CMD)
		    {
		      case SET_PLC:
			        if(Index == 0)
			        {
			        	switch(plc)
			        	{
							case 1://nzif
						          Buffer[DataLen++] = 0xFC;
						          Buffer[DataLen++] = 0x00;
						          Buffer[DataLen++] = 0x00;
						          Buffer[DataLen++] = 0x00;
						          Buffer[DataLen++] = 0x00;
						          Buffer[DataLen++] = 0x03;
						          Buffer[DataLen++] = 0x2E;
						          Buffer[DataLen++] = 0x0B;
						          Buffer[DataLen++] = 0x02;
						          Buffer[DataLen++] = 0x42;
						          Buffer[DataLen++] = 0x3A;
						          Buffer[DataLen++] = 0x35;
						          Buffer[DataLen++] = 0xC7;
						          Tmp.Data_s = Crc16(Buffer, DataLen);
						          Buffer[DataLen++] = Tmp.Data_b[0];
						          Buffer[DataLen++] = Tmp.Data_b[1];
								break;
							case 2://icbcom1f
								break;

							case 3://icbcom3f
					        	if(cmd.size() > 0 && cmd.size() % 2 == 0)
					        	{
					        		for(int ind = 0; ind < cmd.size()/2; ind++)
					        		{
					        			Buffer[DataLen++] = HexToByte( (char*)&cmd.c_str()[ind*2] );
					        		}
					    	        //Tmp.Data_s = Crc16(Buffer, DataLen);
					    	        //Buffer[DataLen++] = Tmp.Data_b[0];
					    	        //Buffer[DataLen++] = Tmp.Data_b[1];
					        	}
								break;
							case 4://rm
					        	if(cmd.size() > 0)
					        	{
					        		for(int ind = 0; ind < cmd.size(); ind++){
					        			Buffer[DataLen++] = cmd.c_str()[ind];
					        		}
					        	}
					        	break;
			        	};
			        }
		        break;
		      case OPEN_CHANNAL:
		        DataLen = 0;
		        Buffer[DataLen++] = 0xFE;
		        Buffer[DataLen++] = 0xFE;
		        Buffer[DataLen++] = 0x68;
		        Buffer[DataLen++] = 0x99;
		        Buffer[DataLen++] = 0x99;
		        Buffer[DataLen++] = 0x99;
		        Buffer[DataLen++] = 0x99;
		        Buffer[DataLen++] = 0x99;
		        Buffer[DataLen++] = 0x99;
		        Buffer[DataLen++] = 0x68;
		        Buffer[DataLen++] = 0x11;
		        Buffer[DataLen++] = 0x0A;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = 0x33;
		        Buffer[DataLen++] = CalcBCC(&Buffer[2], DataLen-2);
		        Buffer[DataLen++] = 0x16;
		        break;

		      case AUTORIZATE:
			        DataLen = 0;
			        Buffer[DataLen++] = 0xFE;
			        Buffer[DataLen++] = 0xFE;
			        Buffer[DataLen++] = 0x68;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x68;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = 0x65;
			        Buffer[DataLen++] = 0xF3;
			        Buffer[DataLen++] = CalcBCC(&Buffer[2], DataLen-2);
			        Buffer[DataLen++] = 0x16;
		        break;

		      case GET_E:
			        DataLen = 0;
			        Buffer[DataLen++] = 0xFE;
			        Buffer[DataLen++] = 0xFE;
			        Buffer[DataLen++] = 0x68;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x68;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = 0x43;
			        Buffer[DataLen++] = 0xC3;
			        Buffer[DataLen++] = CalcBCC(&Buffer[2], DataLen-2);
			        Buffer[DataLen++] = 0x16;
		        break;

		      case GET_SERIAL:
			        DataLen = 0;
			        Buffer[DataLen++] = 0xFE;
			        Buffer[DataLen++] = 0xFE;
			        Buffer[DataLen++] = 0x68;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x68;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = 0x65;
			        Buffer[DataLen++] = 0xF3;
			        Buffer[DataLen++] = CalcBCC(&Buffer[2], DataLen-2);
			        Buffer[DataLen++] = 0x16;
		        break;

		      case CLOSE_CHANNAL:
			        DataLen = 0;
			        Buffer[DataLen++] = 0xFE;
			        Buffer[DataLen++] = 0xFE;
			        Buffer[DataLen++] = 0x68;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x99;
			        Buffer[DataLen++] = 0x68;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = 0x65;
			        Buffer[DataLen++] = 0xF3;
			        Buffer[DataLen++] = CalcBCC(&Buffer[2], DataLen-2);
			        Buffer[DataLen++] = 0x16;
		        break;

		      default:
		        DataLen = 0;
		        break;
		    };

		    if(DataLen > 0)
		    {
				if(CMD != SET_PLC && plc == 3)
				{
					Byte* tmpBUF = new Byte[DataLen+5];
					Word dataInd = 0;
					tmpBUF[dataInd++] = 0x0A;
					tmpBUF[dataInd++] = 0x61;
					tmpBUF[dataInd++] = DataLen;

					for(int i=0; i < DataLen; i++){
						tmpBUF[dataInd++] = Buffer[i];
					}
			        Tmp.Data_s = Crc16(tmpBUF, dataInd);
			        tmpBUF[dataInd++] = Tmp.Data_b[0];
			        tmpBUF[dataInd++] = Tmp.Data_b[1];
					for(int i=0; i < dataInd; i++){
						Buffer[i] = tmpBUF[i];
					}
					DataLen = dataInd;
					delete [] tmpBUF;
				}
		    }
		    return DataLen;
		}
		//=============================================================================
		Word THuabangDts::GetRecvMessageLen(Byte CMD)
		{
		  Word Len = 0;
		  switch(CMD)
		  {
		    case OPEN_CHANNAL:
		    case AUTORIZATE:
		    case GET_E:
		    case GET_SERIAL:
		    case CLOSE_CHANNAL:
		      Len = 100;break;

		    default:
		      Len = 0;
		      break;
		  };
		  return Len;
		}
		//=============================================================================
		sWord THuabangDts::RecvData(IPort* por, Byte *Buf, sWord DataLen, Word MaxLen, Byte subfase, sWord &refLen)
		{
		  ComPort *port = (ComPort*)por;
		  sWord   RecvLen = 0;
		  sWord bytes = 0;

		  if(DataLen > 0)
		  {
			  switch(subfase)
			  {
				case OPEN_CHANNAL:
			      RecvLen = port->RecvTo( Buf, MaxLen, 0x16 );
				  if(RecvLen > 4){
					refLen = RecvLen;
				  }
				  break;

				default:
				  RecvLen = port->RecvTo( Buf, MaxLen, 0x16 );
				  if(RecvLen > 4){
					refLen = RecvLen;
				  }
				  break;
			  };
		  }
		  return RecvLen;
		}
		//=============================================================================
		string THuabangDts::GetStringValue(void)
		{
			string ReturnString = WordToString(Et)+","+
									WordToString(En)+","+
									  E.GetValueString()+
									  CouterType.GetValueString()+
										SerialN.GetValueString()+WordToString(IsEnable)+','+
										"\r\n>,99,1,"+WordToString(Type)+',';
				  return ReturnString;
		}
		//=============================================================================
		void THuabangDts::GetAdditionalData(Byte *Buffer, Word len){
			//Log::DEBUG( "[THuabangDts] GetAdditionalData len="+toString(len)+" buffer="+string((char*)Buffer,len));
			if(len > 0){
				CouterType.Type = "HBG";
			}
		}

		//=============================================================================
		void THuabangDts::GetType(Byte *Buffer, Word len)
		{
			if(len > 0){

			}
		}
		//=============================================================================
		void THuabangDts::GetEa(Byte *Buffer, Word len)
		{
			if(len > 0){
				string tmp = GetDataFromMessage(Buffer, len);
				//Log::DEBUG( "[THuabangDts] GetEa="+tmp);
				if(!isNullOrWhiteSpace(tmp)){
					E.Ea = atol( tmp.c_str() )/100.0;
				}
			}
		}
		//=============================================================================
		void THuabangDts::GetEr(Byte *Buffer, Word len)
		{
			if(len > 0){
				/*
				 vector<string> Result = GetDataFromMessage(Buffer, len);
				 if(Result.size() > 0){
					 E.Er=atof( Result[0].c_str() );
				 }*/
			}
		}
		//=============================================================================
		float THuabangDts::GetEt(int tindex, Byte etype, Byte *Buffer, Word len)
		{
			float ret = 0.0;
			if(len > 0){
				/*
				 vector<string> Result = GetDataFromMessage(Buffer, len);
				 if(Result.size() > tindex){
					 ret=atof( Result[tindex].c_str() );
				 }*/
			}
			return ret;
		}
		//=============================================================================
		void THuabangDts::GetEa_minus(Byte *Buffer, Word len)
		{
			if(len > 0){

			}
		}
		//=============================================================================
		void THuabangDts::GetEr_minus(Byte *Buffer, Word len)
		{
			if(len > 0){

			}
		}
		//=============================================================================
		float THuabangDts::GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len)
		{
			float ret = 0.0;
			if(len > 0){

			}
			return ret;
		}
		//=============================================================================
		string THuabangDts::GetSerial(Byte *Buffer, Word len)
		{
			//Log::DEBUG( "[THuabangDts] THuabangDts len="+toString(len)+" buffer="+string((char*)Buffer,len));
			string ret = "00000000";
			if(len > 0){
				string tmp = GetDataFromMessage(Buffer, len);
				if(tmp.size() > 7){
					int ind = tmp.size()-7;
					tmp = tmp.substr(ind);
				}
				if(!isNullOrWhiteSpace(tmp))
					ret = tmp;
		  }
		  return ret;
		}
		//=============================================================================
		Byte THuabangDts::CalcBCC(Byte *BUF, Word Len )
		{
			Byte ret = 0;
	        for (int i = 0; i < Len; i++){
	        	Byte b = BUF[i];
	        	ret += b;
	        }
			return ret;
		}
		bool THuabangDts::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
		{
			//Log::DEBUG( "TEnergomeraCE::ParsingAnswer subFase="+toString((int)(subFase))+" Len="+toString(Len));
		  bool ret = false;

		  if(subFase == SET_PLC)
			  return true;
		  if(Len == 0)
			  return true;

		  if(Len > 0 && Len < 4)
			  return false;
		  else
		  {
			  switch(subFase)
			  {
			    case OPEN_CHANNAL:
			    case CLOSE_CHANNAL:
			    case AUTORIZATE:
			    	ret = true;
			      break;

			    default:
			    	int ind1 = TBuffer::find_first_of(BUF, Len, 0x68);
			    	int ind2 = TBuffer::find_first_of(BUF, Len, 0x16);
			    	if(  ind1 < 0 || ind2 <0 || ind2 < ind1){
			    		ret = false;
			    	}
			    	else{
			    		int ind = ind1;

			    		Byte calcCrc = CalcBCC( &BUF[ind], Len-ind-2 );
			    		Byte messCrc = (Word)BUF[ind2-1];
			    		if(messCrc == calcCrc)
			    			ret = true;
			    		else
			    			ret = false;

			    		/*
			    		Log::DEBUG( "THuabangDts::calculated ind="+toString((int)ind));
			    		Log::DEBUG( "THuabangDts::calculated ind2="+toString((int)ind2));
			    		Log::DEBUG( "THuabangDts::calculated Len="+toString((int)Len));

			    		Log::DEBUG( "THuabangDts::calculated BCC="+toString((int)calcCrc));
			    		Log::DEBUG( "THuabangDts::message BCC="+toString((int)messCrc));*/
			    	}
			      break;
			  };
		  }
		  return ret;
		}
		//=============================================================================
		string THuabangDts::GetDataFromMessage( Byte *BUF, Word Len)
		{
			string retVal;
			Byte bf[2] {0,0};
			int start = TBuffer::find_first_of( BUF, Len, 0x68 );
			if(start >=0){
				BUF += start;
				Len -= start;
				start = TBuffer::find_first_of( &BUF[start+1], Len-(start+1), 0x68 );
				if(start >=0){
					BUF += (start+4);
					Len -= (start+4);
					int end = TBuffer::find_first_of( BUF, Len, 0x16 );
					//Log::DEBUG("end="+toString((int)end));
					if(end > 0 && end > start){
						for(int i=(end-2); i >= start; i--){
							ByteToHex(bf, BUF[i]-0x33);
							retVal+= string((char*)bf, 2);
						}
					}
					/*
					if(end > 0 && end > start){
						for(int i=start; i < (end-1); i++){
							ByteToHex(bf, BUF[i]);
							retVal+= string((char*)bf, 2)+" ";
						}
					}*/
				}
			}
			return retVal;
		}


		//*****************************************************************************
			//*** THuabang2Dts class
			//*****************************************************************************
			THuabang2Dts::THuabang2Dts(Byte Ename, string adr):ICounter(Ename, atoi(adr.c_str())), Address(adr)
			{
			  Modify = false;
			  Type = 14;

			}
			//=============================================================================
			THuabang2Dts::~THuabang2Dts( )
			{
			}
			//=============================================================================
			Word THuabang2Dts::CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *ecmd)
			{
			 Word DataLen = 0;
			 Bshort       Tmp;

			    switch(CMD)
			    {
			      case SET_PLC:
				        DataLen = 0;
				        Buffer[DataLen++] = 0x01;
				        Buffer[DataLen++] = 0x42;
				        Buffer[DataLen++] = 0x30;
				        Buffer[DataLen++] = 0x03;
				        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x71;
			        break;
			      case OPEN_CHANNAL:
			        DataLen = 0;
			        Buffer[DataLen++] = '/';
			        Buffer[DataLen++] = '?';
			        if(Address.size() > 0 && atoi(Address.c_str()) > 0)
			        {
			          for(int ind = 0; ind < Address.size(); ind++)
			          {
			            Buffer[DataLen++] = Address.c_str()[ind];
			          }
			        }
			        Buffer[DataLen++] = '!';
			        Buffer[DataLen++] = 0x0D;
			        Buffer[DataLen++] = 0x0A;
			        break;

			      case AUTORIZATE:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x06;
			        Buffer[DataLen++] = 0x30;
			        Buffer[DataLen++] = 0x32;
			        Buffer[DataLen++] = 0x31;
			        Buffer[DataLen++] = 0x0D;
			        Buffer[DataLen++] = 0x0A;
			        break;

			      case GET_ETYPE:
				    DataLen = 0;
				    Buffer[DataLen++] = 0x01;
				    Buffer[DataLen++] = 'P';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = 0x02;
				    Buffer[DataLen++] = '(';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = '1';
				    Buffer[DataLen++] = ')';
				    Buffer[DataLen++] = 0x03;
				    Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x61;
				    break;

			      case GET_E:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '8';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '*';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x74;
			        break;

			      case GET_ER:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '8';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '*';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x74;
			        break;

			      case GET_Ua:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '3';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x57;
			        break;

			      case GET_Ub:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '5';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x51;
			        break;

			      case GET_Uc:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x53;
			        break;

			      case GET_Ia:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '3';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x54;
			        break;

			      case GET_Ib:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '5';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x52;
			        break;

			      case GET_Ic:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x50;
			        break;

			      case GET_Pa:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
			        break;

			      case GET_Pb:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '4';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
			        break;

			      case GET_Pc:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '6';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
			        break;

			      case GET_Qa:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = '9';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
			        break;

			      case GET_Qb:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '4';
			        Buffer[DataLen++] = '9';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
			        break;

			      case GET_Qc:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 'R';
			        Buffer[DataLen++] = '1';
			        Buffer[DataLen++] = 0x02;
			        Buffer[DataLen++] = '6';
			        Buffer[DataLen++] = '9';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '7';
			        Buffer[DataLen++] = '.';
			        Buffer[DataLen++] = '0';
			        Buffer[DataLen++] = '(';
			        Buffer[DataLen++] = '2';
			        Buffer[DataLen++] = ')';
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
			        break;

			      case GET_SERIAL:
				        DataLen = 0;
				        Buffer[DataLen++] = 0x01;
				        Buffer[DataLen++] = 'R';
				        Buffer[DataLen++] = '1';
				        Buffer[DataLen++] = 0x02;
				        Buffer[DataLen++] = '9';
				        Buffer[DataLen++] = '6';
				        Buffer[DataLen++] = '.';
				        Buffer[DataLen++] = '1';
				        Buffer[DataLen++] = '.';
				        Buffer[DataLen++] = '0';
				        Buffer[DataLen++] = '(';
				        Buffer[DataLen++] = '2';
				        Buffer[DataLen++] = ')';
				        Buffer[DataLen++] = 0x03;
				        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);
				        break;

			      case CLOSE_CHANNAL:
			        DataLen = 0;
			        Buffer[DataLen++] = 0x01;
			        Buffer[DataLen++] = 0x42;
			        Buffer[DataLen++] = 0x30;
			        Buffer[DataLen++] = 0x03;
			        Buffer[DataLen++] = CalcBCC(Buffer, DataLen);//0x71;
			        break;



			      default:
			        DataLen = 0;
			        break;
			    };
			  return DataLen;
			}
			//=============================================================================
			Word THuabang2Dts::GetRecvMessageLen(Byte CMD)
			{
			  Word Len = 0;
			  switch(CMD)
			  {
			    case GET_ETYPE:
			    	Len = 1;break;

			    case OPEN_CHANNAL:
			    case CLOSE_CHANNAL:
			    case AUTORIZATE:
			    case GET_E:
			    case GET_ER:
			    case GET_Ua:
			    case GET_Ub:
			    case GET_Uc:
			    case GET_Ia:
			    case GET_Ib:
			    case GET_Ic:
			    case GET_Pa:
			    case GET_Pb:
			    case GET_Pc:
			    case GET_Qa:
			    case GET_Qb:
			    case GET_Qc:
			    case GET_SERIAL:
			      Len = 100;break;

			    default:
			      Len = 0;
			      break;
			  };
			  return Len;
			}
			//=============================================================================
			sWord THuabang2Dts::RecvData(IPort* por, Byte *Buf, sWord DataLen, Word MaxLen, Byte subfase, sWord &refLen)
			{
			  ComPort *port = (ComPort*)por;
			  sWord   RecvLen = 0;
			  sWord bytes = 0;

			  if(DataLen > 0)
			  {
				  switch(subfase)
				  {
					case OPEN_CHANNAL:
					  RecvLen = port->RecvTo( Buf, MaxLen, 0x0A);
					  if(RecvLen > 4){
						refLen = RecvLen;
					  }
					  break;

					case GET_ETYPE:
					  RecvLen = port->Recv( Buf, DataLen );
					  if(RecvLen > 0){
						refLen = RecvLen;
					  }
					  break;

					case CLOSE_CHANNAL:
					  //RecvLen = port->Recv( Buf, DataLen );
					  RecvLen = port->Recv( Buf, MaxLen );
					  if(RecvLen <= 0)
						  RecvLen = 5;
					  refLen = RecvLen;
					  break;

					default:
					  RecvLen = port->RecvTo( Buf, MaxLen, 0x03 );
					  if(RecvLen > 4){
						RecvLen += port->Recv( &Buf[RecvLen], 1 );
						refLen = RecvLen;
					  }
					  break;
				  };
			  }
			  return RecvLen;
			}
			//=============================================================================
			string THuabang2Dts::GetStringValue(void)
			{
				string ReturnString = WordToString(Et)+","+
										WordToString(En)+","+
										  E.GetValueString()+
										  CouterType.GetValueString()+
											U.GetValueString()+
											I.GetValueString()+
											P.GetValueString()+
											//Q.GetValueString()+
											S.GetValueString()+
											//KM.GetValueString()+
											//F.GetValueString()+
											SerialN.GetValueString()+WordToString(IsEnable)+','+
											"\r\n>,99,1,"+WordToString(Type)+',';
					  return ReturnString;
			}
			//=============================================================================
			void THuabang2Dts::GetAdditionalData(Byte *Buffer, Word len){
				//Log::DEBUG( "[TChintDts] GetAdditionalData len="+toString(len)+" buffer="+string((char*)Buffer,len));
				if(len > 0){
					int ind = TBuffer::find_first_of(Buffer, len, '/');
					if(ind>=0){
						CouterType.Type = string((char*)&Buffer[ind+1], 4);
					}
				}
			}

			//=============================================================================
			void THuabang2Dts::GetType(Byte *Buffer, Word len)
			{
				if(len > 0){

				}
				/*
			  string Tmp = "CHT";
			  vector<string> Result = GetDataFromMessage(Buffer, len);
			  if(Result.size() > 0)
			  {
				  Tmp = Result[0];
				  CouterType.Type = Tmp;
			  }
			 return Tmp;*/
			}
			//=============================================================================
			void THuabang2Dts::GetEa(Byte *Buffer, Word len)
			{
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > 0){
						 E.Ea=atof( Result[0].c_str() );
					 }
				}
			}
			//=============================================================================
			void THuabang2Dts::GetEr(Byte *Buffer, Word len)
			{
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > 0){
						 E.Er=atof( Result[0].c_str() );
					 }
				}
			}
			//=============================================================================
			float THuabang2Dts::GetEt(int tindex, Byte etype, Byte *Buffer, Word len)
			{
				float ret = 0.0;
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > tindex){
						 ret=atof( Result[tindex].c_str() );
					 }
				}
				return ret;
			}
			//=============================================================================
			void THuabang2Dts::GetEa_minus(Byte *Buffer, Word len)
			{
				if(len > 0){

				}
			}
			//=============================================================================
			void THuabang2Dts::GetEr_minus(Byte *Buffer, Word len)
			{
				if(len > 0){

				}
			}
			//=============================================================================
			float THuabang2Dts::GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len)
			{
				float ret = 0.0;
				if(len > 0){

				}
				return ret;
			}
			//=============================================================================
			void THuabang2Dts::GetUa(Byte *Buffer, Word len)
			{
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > 0){
						 U.Ua.CalcMinAverMax(atof( Result[0].c_str() ));
					 }
				}
			}
			//=============================================================================
			void THuabang2Dts::GetUb(Byte *Buffer, Word len)
			{
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > 0){
						 U.Ub.CalcMinAverMax(atof( Result[0].c_str() ));
					 }
				}
			}
			//=============================================================================
			void THuabang2Dts::GetUc(Byte *Buffer, Word len)
			{
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > 0){
						 U.Uc.CalcMinAverMax(atof( Result[0].c_str() ));
					 }
				}
			}
			//=============================================================================
			void THuabang2Dts::GetIa(Byte *Buffer, Word len)
			{
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > 0){
						 I.Ia.CalcMinAverMax(atof( Result[0].c_str() ));
					 }
				}
			}
			//=============================================================================
			void THuabang2Dts::GetIb(Byte *Buffer, Word len)
			{
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > 0){
						 I.Ib.CalcMinAverMax(atof( Result[0].c_str() ));
					 }
				}
			}
			//=============================================================================
			void THuabang2Dts::GetIc(Byte *Buffer, Word len)
			{
				if(len > 0){
					 vector<string> Result = GetDataFromMessage(Buffer, len);
					 if(Result.size() > 0){
						 I.Ic.CalcMinAverMax(atof( Result[0].c_str() ));
					 }
				}
			}
			//=============================================================================
			void THuabang2Dts::GetPa(Byte *Buffer, Word len)
			{
				 vector<string> Result = GetDataFromMessage(Buffer, len);
				 if(Result.size() > 0)
					 P.Pa.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
			}
			//=============================================================================
			void THuabang2Dts::GetPb(Byte *Buffer, Word len)
			{
				 vector<string> Result = GetDataFromMessage(Buffer, len);
				 if(Result.size() > 0)
					 P.Pb.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
			}
			//=============================================================================
			void THuabang2Dts::GetPc(Byte *Buffer, Word len)
			{
				 vector<string> Result = GetDataFromMessage(Buffer, len);
				 if(Result.size() > 0)
					 P.Pc.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
			}
			//=============================================================================
			void THuabang2Dts::GetQa(Byte *Buffer, Word len)
			{
				 vector<string> Result = GetDataFromMessage(Buffer, len);
				 if(Result.size() > 0)
					 Q.Qa.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
			}
			//=============================================================================
			void THuabang2Dts::GetQb(Byte *Buffer, Word len)
			{
				 vector<string> Result = GetDataFromMessage(Buffer, len);
				 if(Result.size() > 0)
					 Q.Qb.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
			}
			//=============================================================================
			//=============================================================================
			void THuabang2Dts::GetQc(Byte *Buffer, Word len)
			{
				 vector<string> Result = GetDataFromMessage(Buffer, len);
				 if(Result.size() > 0)
					 Q.Qc.CalcMinAverMax( atof( Result[0].c_str() )*1000.0 );
			}
			//=============================================================================
			float THuabang2Dts::GetKM(Byte *Buffer, Word len)
			{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0)
			  return atof( Result[0].c_str() );
			 return 0.0;
			}
			//=============================================================================
			float THuabang2Dts::GetKMa(Byte *Buffer, Word len)
			{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 1)
			  return atof( Result[1].c_str() );
			 return 0.0;
			}
			//=============================================================================
			float THuabang2Dts::GetKMb(Byte *Buffer, Word len)
			{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 2)
			  return atof( Result[2].c_str() );
			 return 0.0;
			}
			//=============================================================================
			float THuabang2Dts::GetKMc(Byte *Buffer, Word len)
			{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 3)
			  return atof( Result[3].c_str() );
			 return 0.0;
			}
			//=============================================================================

			float THuabang2Dts::GetF(Byte *Buffer, Word len)
			{
			 vector<string> Result = GetDataFromMessage(Buffer, len);
			 if(Result.size() > 0)
			  return atof( Result[0].c_str() );
			 return 0.0;
			}
			//=============================================================================
			string THuabang2Dts::GetSerial(Byte *Buffer, Word len)
			{
				string ret = "00000000";
				vector<string> Result = GetDataFromMessage(Buffer, len);
				if(Result.size() > 0 && !isNullOrWhiteSpace(Result[0]))
					ret= Result[0];
				return ret;
			}
			//=============================================================================
			Byte THuabang2Dts::CalcBCC(Byte *BUF, Word Len )
			{
				Byte ret = 0;
		        for (int i = 1; i < Len; i++){
		        	Byte b = BUF[i];
		        	ret ^= b;
		        	if (b == 0x03)
		        		break;
		        }
		        ret &= 0x7F;
				return ret;
			}
			//=============================================================================
			bool THuabang2Dts::ParsingAnswer( Byte *BUF, Word Len, Byte subFase )
			{
				//Log::DEBUG( "TEnergomeraCE::ParsingAnswer subFase="+toString((int)(subFase))+" Len="+toString(Len));
			  bool ret = false;

			  if(subFase == SET_PLC)
				  return true;
			  if(Len == 0)
				  return true;

			  if(Len > 1 && Len < 4)
				  return false;
			  else
			  {
				  switch(subFase)
				  {
				    case OPEN_CHANNAL:
				    case CLOSE_CHANNAL:
				    case AUTORIZATE:
				    case GET_ETYPE:
				    	ret = true;
				      break;

				    default:
				    	int ind1 = TBuffer::find_first_of(BUF, Len, 0x01);
				    	int ind2 = TBuffer::find_first_of(BUF, Len, 0x02);
				    	if(  ind1 < 0 && ind2 <0 ){
				    		ret = false;
				    	}
				    	else{
				    		int ind = 0;
				    		if(ind1 < ind2){
				    			ind = ind1;
				    			if(ind < 0){
				    				ind = ind2;
				    			}
				    		}
				    		Byte calcCrc = CalcBCC(&BUF[ind], Len-ind);
				    		Byte messCrc = (Word)BUF[Len-1];
				    		if(messCrc == calcCrc)
				    			ret = true;
				    		else
				    			ret = false;

				    		//Log::DEBUG( "TEnergomeraCE::ParsingAnswer ind="+toString(ind));
				    		//Log::DEBUG( "TEnergomeraCE::calculated BCC="+toString((int)calcCrc));
				    		//Log::DEBUG( "TEnergomeraCE::message BCC="+toString((int)messCrc));
				    		//Log::DEBUG( "TEnergomeraCE::ParsingAnswer ind="+toString(ind));
				    	}
				      break;
				  };
			  }
			  return ret;
			}
			//=============================================================================
			vector<string> THuabang2Dts::GetDataFromMessage( Byte *BUF, Word Len)
			{
			  vector<string> retVal;
			  vector<string> Result;
			  string sep = "\r\n";

			  TBuffer::Split(BUF, Len, sep, Result);
			  for(int i = 0; i < Result.size(); i++)
			  {
			      string curr = Result[i];
			      if(curr.size() > 2)
			      {
			        char stx = '(';
			        char etx = ')';
			        int start = TBuffer::find_first_of( (Byte*)curr.c_str(), Len, stx );
			        int end   = TBuffer::find_first_of( (Byte*)curr.c_str(), Len, etx );
			        if(start >=0 && end > 0)
			        {//ok
			          start++;
			          //end--;
			          if( start < end )
			          {
			            int size = (end-start);
			            retVal.push_back( string( (char*)&curr.c_str()[start], size) );
			          }
			        }
			      }
			  }
			  return retVal;
			}
			//=============================================================================

	//*****************************************************************************
	//***
	//*****************************************************************************
	TEnergyCounterManager::TEnergyCounterManager( void ):TFastTimer(4, &MilSecCount),
	                                                                Fase(CREATE_CMD),
	                                                                NewFase(99),
	                                                                SubFase(SET_PLC),
	                                                                CounterIndex(0),
	                                                                Period(1800),
	                                                                CurrentECMD(NULL),
	                                                                DataLen(0),fd(NULL),Port(NULL)
	{

	  SetTimer(PERIUD_TIMER, 50000);
	  SetTimer(ADDITIONAL_TIMER, 0);
	  SetTimer(LONG_TIMER, 0);

	  FirstInitFlg = true;
	  ModifyTime = Period;
	  needEcmd = false;
	  busyEcmd = false;
	  ReturnValue = NULL;
	  int st = pthread_mutex_init(&sych, NULL);
	  if(st != 0 )
	  {
		  Log::DEBUG("TEnergyCounterManager Error in pthread_mutex_init st="+toString(st));
	  }

	  ApiAddress = "";
	  ApiPort = 0;
	  ObjectId = "";
	}
	//=============================================================================
	TEnergyCounterManager::~TEnergyCounterManager( void )
	{
		try
		{
		  for(auto curr: Counters)
		  {
			 delete curr;
		  }
		  Counters.clear();
		  int st = pthread_mutex_destroy(&sych);
		  if(st != 0 )
		  {
			  Log::DEBUG("TEnergyCounterManager Error in pthread_mutex_destroy st="+toString(st));
		  }
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
		}
	}
	//=============================================================================
	void TEnergyCounterManager::SetFase(Byte nfase)
	{
		NewFase = nfase;
	}
	//=============================================================================
	Byte TEnergyCounterManager::GetFase(void)
	{
		if(NewFase != 99 && Fase == CREATE_CMD)
		{
			Fase = NewFase;
			NewFase = 99;
		}
		return Fase;
	}
	//=============================================================================
	void TEnergyCounterManager::Init(void *config, void *f )
	{
		//Port = (ComPort *)CurrCOM;
		vector<EcounterSettings> sett = *(vector<EcounterSettings>*)config;
		fd  = (TFifo<string> *)f;//!!

		//Log::DEBUG("TEnergyCounterManager sett.size()="+toString(sett.size()));
		//int _index = 0;
		for(int _index =0; _index < sett.size(); _index++)
		{
			EcounterSettings curr = sett[_index];
			Period = curr.period;
		 	ICounter *currCounter = NULL;
		 	switch(curr.type){
				case 0:
				    currCounter = new TMercury230Counter(curr.ename, curr.address);//
					break;
				case 1:
				    currCounter = new TMercury200Counter(curr.ename, curr.address);//
					break;
				case 2:
				    currCounter = new TEnergomeraCE(curr.ename, curr.address);//
					break;
				case 3:
				    currCounter = new TChintDts(curr.ename, curr.address);//
					break;
				case 4:
				    currCounter = new THuabangDts(curr.ename, curr.address);//
					break;
				case 5:
				    currCounter = new THuabang2Dts(curr.ename, curr.address);//
					break;
				case 6:
				    currCounter = new TPschCounter(curr.ename, curr.address);//
					break;

				case 7:
				    currCounter = new TNevaMT(curr.ename, curr.address);//
					break;

		 	}
		 	if(currCounter!=NULL)
		 	{
		 		currCounter->comm_settings  = curr.portsettings;
			    currCounter->Index 	= _index;
			    currCounter->BetweenTimeout = curr.betweentimeout;
			    currCounter->cntPort = new ComPort();
			    currCounter->cntPort->Init(&curr.portsettings);
			    Port = currCounter->cntPort;
			    //currCounter->SetPeriod( curr.period );
			    if(!isNullOrWhiteSpace(curr.poverka)){
			    	currCounter->Poverka = curr.poverka;
			    }
			    currCounter->cmd 		= ParseCmdString(curr.cmd);
			    currCounter->plc 		= curr.plc;
			    currCounter->timezone 	= curr.timezone;
			    currCounter->index 		= curr.index;
			    currCounter->hexPassword= curr.hex;
			    Counters.push_back( currCounter );
		 	}
		}

		for(int ind=0; ind < Counters.size(); ind++)
		{
			ICounter *currCounter = Counters[ind];
		    Log::DEBUG("TEnergyCounterManager currCounter ind="+toString(ind));
		    Log::DEBUG("Index="+toString((int)currCounter->Index));
		    Log::DEBUG("Type="+toString((int)currCounter->Type));
		    Log::DEBUG("Addr="+toString((int)currCounter->Addr));
		    Log::DEBUG("Plc="+toString(currCounter->plc));
		    Log::DEBUG("Hex="+toString(currCounter->hexPassword));
		    Log::DEBUG("TEnergyCounterManager currCounter->comm_settings:");
		    Log::DEBUG("DeviceName="+toString(currCounter->comm_settings.DeviceName));
		    Log::DEBUG("BaudRate="+toString((int)currCounter->comm_settings.BaudRate));
		    Log::DEBUG("DataBits="+toString((int)currCounter->comm_settings.DataBits));
		    Log::DEBUG("StopBit="+toString((int)currCounter->comm_settings.StopBit));
		    Log::DEBUG("Parity="+toString((int)currCounter->comm_settings.Parity));
		    Log::DEBUG("RecvTimeout="+toString(currCounter->comm_settings.RecvTimeout));
		    Log::DEBUG("SendTimeout="+toString(currCounter->comm_settings.SendTimeout));
		}

		//DWord tim = SystemTime.GetFullGlobalSecond();
		//ModifyTime = ((DWord)(tim / Period + 1)) * Period;
		//if(ModifyTime >= 86400) ModifyTime = 0;
	}
	//=============================================================================
	ICounter* TEnergyCounterManager::GetSimpleCounter( Word _index )
	{
	  ICounter *counter = NULL;

	  /*
	  for(auto curr: Counters)
	  {
		  if(curr->Index == _index)
		  {
			  Log::DEBUG("TEnergyCounterManager::GetSimpleCounter index="+toString(index));
		  }
	  }*/
  	  if(_index < Counters.size())
  	  {
  		  counter = Counters[_index];
  		  //Log::DEBUG("TEnergyCounterManager::GetSimpleCounter index="+toString(index) + " Index=" + toString((int)counter->Index)+" type="+toString((int)counter->Type)+" addr="+toString((int)counter->Addr));

  	  }

	  if(counter == NULL)
	  {
		  Log::ERROR("TEnergyCounterManager::GetSimpleCounter counter=NULL _index="+toString(_index));
	  }

	  return counter;
	}
	//=============================================================================
	//=============================================================================
	void TEnergyCounterManager::ENERGY_CreateCMD( void)
	{
	  int size = Counters.size();
	  if( size > 0)
	  {
	    if( CounterIndex >=  size){
	    	//Log::DEBUG( "[CounterManager] ENERGY_CreateCMD BIG CounterIndex="+toString(CounterIndex) + " size="+toString(size) + " set 0");
	    	CounterIndex = 0;//(size - 1);
	    }
	    ICounter *CurrCounter = GetSimpleCounter(CounterIndex);//new
	    if(CurrCounter != NULL)
	    {
	    	//Log::DEBUG("TEnergyCounterManager::ENERGY_CreateCMD CurrCounter Index=" + toString((int)CurrCounter->Index)+" type="+toString((int)CurrCounter->Type)+" addr="+toString((int)CurrCounter->Addr));
	    	if(needEcmd)
	    	{
	    		  needEcmd = false;
	    		  SetTimer(LONG_TIMER, 900000);//15 min = max time in DO_ECMD cycle
	    		  SetFase(DO_ECMD);
	    		  return;
	    	}

	    	if(SubFase == SET_PLC)
	    	{
	    		//Log::DEBUG( "[CounterManager] ENERGY_CreateCMD SubFase=SET_PLC CounterIndex="+toString(CounterIndex));
	    		if( CurrCounter->cntPort != Port )
	    		{
	    			if(Port != NULL)
	    				Port->Close();
	    			Port = CurrCounter->cntPort;//change
	    			sleep(1);
	    		}
	    	}

	    	DataLen = CurrCounter->CreateCMD(SubFase, Buffer);
	    	//Log::DEBUG( "[CounterManager] ENERGY_CreateCMD len=" + toString(DataLen)+" SubFase="+toString((int)SubFase));
	    }
	    Fase    = SEND_CMD;
	  }
	}
	//=============================================================================
	void TEnergyCounterManager::ENERGY_SendCMD( void )
	{
	  if( Port->Send(Buffer, DataLen) > 0 )
	  {//

////////////////

		  int t = GetSimpleCounter(CounterIndex)->Type;
		  if(t == 0 || t == 1 || t == 4|| t == 6 )
		  {
			  Byte bf[2] {0,0};
			  string sended = "";
			  for(int i = 0; i < DataLen; i++){
				  ByteToHex(bf, Buffer[i]);
				  sended += "0x"+string((char*)bf, 2)+" ";
			  }
			//Log::DEBUG( "[CounterManager] send ok: [" + sended +"] len="+toString(DataLen)+" SubFase="+toString((int)SubFase));
		  }
		  else{
			  //Log::DEBUG("[CounterManager] send ok: ["+ string( (char*)Buffer, DataLen ) + "] len="+toString(DataLen)+" SubFase="+toString((int)SubFase));
		  }

//////////////////////////
	    TBuffer::ClrBUF(Buffer, DataLen);
	    Fase    = RECV_CMD;
	  }
	  else{
		  //Log::DEBUG( "[TEnergyCounterManager] goto EXEC_CMD");
		  Fase    = EXEC_CMD;
	  }
	}
	//=============================================================================
	void TEnergyCounterManager::ENERGY_RecvCMD( void )
	{
	  static int 	_answerErrorCNT = 0;
	  sWord 		Len 			= 0;
	  ICounter* 	CurrCounter 	= GetSimpleCounter(CounterIndex);

	  if(CurrCounter == NULL)
	  {
		  Log::ERROR("TEnergyCounterManager::ENERGY_RecvCMD CurrCounter = NULL CounterIndex="+toString(CounterIndex));
		  CounterIndex = 0;
		  return;
	  }
	  Len 			= CurrCounter->GetRecvMessageLen(SubFase);
	  //sWord eLen 	= CurrCounter->GetErrorMessageLen(SubFase);
	  Word  timeout = CurrCounter->BetweenTimeout;

	  sWord RecvLen = CurrCounter->RecvData(Port, Buffer, Len, sizeof(Buffer), SubFase, Len);//
	  if( RecvLen > 0 || SubFase == SET_PLC || SubFase == GET_EISP || SubFase == GET_E5 ||
			  SubFase == GET_KTRAN || SubFase == GET_Ef || SubFase == GET_E1f || SubFase == GET_E2f || SubFase == GET_E3f || SubFase == GET_E4f)
	  {
	      DataLen     = RecvLen;
	      Fase        = EXEC_CMD;
	      if(SubFase != SET_PLC )
	    	  _answerErrorCNT = 0;
	      SetTimer( COMMON_TIMER, timeout);

		  ///////////////////////////

		  int t = GetSimpleCounter(CounterIndex)->Type;
		  if(t == 0 || t == 1 || t == 4|| t == 6 )
		  {
			  Byte bf[2] {0,0};
			  string recv = "";
			  for(int i = 0; i < RecvLen; i++){
				  ByteToHex(bf, Buffer[i]);
				  recv += "0x"+string((char*)bf, 2)+" ";
			  }
			  //Log::DEBUG( "[CounterManager] RecvData ok: [" + recv +"] RecvLen="+toString(RecvLen)+ " Len="+toString(Len)+" SubFase="+toString((int)SubFase));
		  }
		  else{
			  //Log::DEBUG("[CounterManager] RecvData ok: ["+ string( (char*)Buffer, RecvLen ) + "] len="+toString(RecvLen)+" SubFase="+toString((int)SubFase));
		  }
		  //Log::DEBUG("Wait len="+toString(Len) + " RecvLen="+toString(RecvLen)+" SubFase="+toString((int)SubFase));

		  //////////////////////

	  }
	  else
	  {
	      if(_answerErrorCNT++ >= 20)
	      { //err
	    	  CurrCounter->ErrCnt++;
	        _answerErrorCNT       = 0;
	        if( CurrCounter->IsEnable == true && SubFase == OPEN_CHANNAL )//
	        {//
	        	/////////////////////////////
	        	CurrCounter->E.ClearValues();
	        	CurrCounter->E1.ClearValues();
	        	CurrCounter->E2.ClearValues();
	        	CurrCounter->E3.ClearValues();
	        	CurrCounter->E4.ClearValues();
	        	CurrCounter->E5.ClearValues();
	        	CurrCounter->SerialN.SerialNumber = "0";
	        	/////////////////////////////


	        	CurrCounter->U.ClearValues();
	        	CurrCounter->I.ClearValues();
	        	CurrCounter->P.ClearValues();
	        	CurrCounter->Q.ClearValues();
	        	CurrCounter->S.ClearValues();
	        	CurrCounter->KM.ClearValues();
	        	CurrCounter->F.ClearValues();
	        	CurrCounter->LastTime = "";


	        	CurrCounter->Modify     = true;
	        	CurrCounter->IsEnable   = false;
	        	CurrCounter->NeedFirstFlg = true;
	        	DateTime = SystemTime.GetTime();
	        	CurrCounter->SetOrigAddr();
	        }
	        Port->Close();//!!
	        DataLen         = 0;
	        if( ++CounterIndex >=  Counters.size())//progon po ostalnim t.k. dalee
	        	CounterIndex = 0;
	        SubFase     = SET_PLC;
	        timeout = 10000;
	        Log::ERROR("[TEnergyCounterManager] READ ERROR Counter:"+ toString((int)CurrCounter->En));
	      }
	      else
	      {
	        if(_answerErrorCNT == 15  || (SubFase == AUTORIZATE && _answerErrorCNT == 3) || (SubFase == CLOSE_CHANNAL && Len == 0) )
	        {
	        	SubFase     = SET_PLC;//CLOSE_CHANNAL
	        	Port->Close();//!!
	        	timeout = 10000;
	        }
	      }
	      DataLen 	= 0;
	      Fase    = CREATE_CMD;
	      SetTimer( COMMON_TIMER, timeout);
	  }
	}
	//=============================================================================
	void TEnergyCounterManager::ENERGY_ExecCMD( void )
	{
	  static int 	_parsingErrorCNT = 0;
	  int size = Counters.size();
	  //Log::DEBUG("ENERGY_ExecCMD Counters.size()="+toString((int)Counters.size()));
	  if( size > 0)
	  {
	    if( CounterIndex >=  size){ CounterIndex = (size - 1); }
	    ICounter *CurrCounter 	= GetSimpleCounter(CounterIndex);//new
	    //Log::DEBUG("TEnergyCounterManager::ENERGY_ExecCMD CurrCounter Index=" + toString((int)CurrCounter->Index)+" type="+toString((int)CurrCounter->Type)+" addr="+toString((int)CurrCounter->Addr));

	    Word  timeout 			= CurrCounter->BetweenTimeout;
	    DataLen = CurrCounter->CorrectData(Buffer, DataLen, SubFase);

	    if( CurrCounter->ParsingAnswer(Buffer, DataLen, SubFase) )// ( > 3)
	    {
	    	if(SubFase != SET_PLC )
	    		_parsingErrorCNT = 0;

	    	if(CurrCounter->Type == MERCURY && SubFase != SET_PLC &&  DataLen == 4 && Buffer[1] == 0x05)//need open chanal cmd answer fo mercury
	    	{
	    		SubFase = 255;//go to default
	    		Log::ERROR("[TEnergyCounterManager] need open chanal cmd answer fo mercury Counter:"+ toString((int)CurrCounter->En));
	    	}
	    	switch(SubFase)
	      {
	        case SET_PLC:
	          SubFase = OPEN_CHANNAL;
	          break;
	        case OPEN_CHANNAL:
	          CurrCounter->GetAdditionalData(Buffer, DataLen);
	          SubFase = AUTORIZATE;
	          break;

	        case AUTORIZATE:
	          SubFase = GET_EISP;
	          break;

	        case GET_EISP:
	          if(CurrCounter->Modify == false){
	        	  //CurrCounter->GetType(Buffer, DataLen);
	        	  //Log::DEBUG("GET_EISP DataLen="+toString(DataLen));
	        	  CurrCounter->GetEisp(Buffer, DataLen);
		      }
	          SubFase = GET_ETYPE;
	          break;
	        case GET_ETYPE:
	          if(CurrCounter->Modify == false){
	        	  CurrCounter->GetType(Buffer, DataLen);
	        	  //Log::DEBUG("GET_EISP DataLen="+toString(DataLen));
		      }
	          SubFase = GET_E;
	          break;
	        case GET_E:
	          if(CurrCounter->Modify == false){
	        	  CurrCounter->GetEa(Buffer, DataLen);
	              CurrCounter->GetEa_minus(Buffer, DataLen);
	          }

	          SubFase = GET_ER;
	          break;
	        case GET_ER:
	          if(CurrCounter->Modify == false){
	        	  CurrCounter->GetEr(Buffer, DataLen);
                  CurrCounter->GetEr_minus(Buffer, DataLen);
	          }
	          SubFase = GET_E1;
	          break;
	        case GET_E1:
	          if(DataLen > 4)//>=3
	          {
	            if(CurrCounter->Modify == false)
	            {
	              CurrCounter->E1.Ea = CurrCounter->GetEt(1,0, Buffer, DataLen);
	              CurrCounter->E1.Ea_minus = CurrCounter->GetEt_minus(1,0, Buffer, DataLen);
	              switch( CurrCounter->Type )
	              {
	                case MERCURY:
	                case PSCH:
	                  CurrCounter->E1.Er = CurrCounter->GetEt(1,1, Buffer, DataLen);
	                  CurrCounter->E1.Er_minus = CurrCounter->GetEt_minus(1,1, Buffer, DataLen);
	              };
	            }
	          }
	          SubFase = GET_E2;
	          break;
	        case GET_E2:
	          if(DataLen > 4)
	          {
	            if(CurrCounter->Modify == false)
	            {
	              CurrCounter->E2.Ea = CurrCounter->GetEt(2,0, Buffer, DataLen);
	              CurrCounter->E2.Ea_minus = CurrCounter->GetEt_minus(2,0, Buffer, DataLen);
	              switch( CurrCounter->Type )
	              {
	                case MERCURY:
	                case PSCH:
		                  CurrCounter->E2.Er = CurrCounter->GetEt(2, 1, Buffer, DataLen);
		                  CurrCounter->E2.Er_minus = CurrCounter->GetEt_minus(2, 1, Buffer, DataLen);
	              };
	            }
	          }
	          SubFase = GET_E3;
	          break;
	        case GET_E3:
	          if(DataLen > 4)
	          {
	            if(CurrCounter->Modify == false)
	            {
	              CurrCounter->E3.Ea = CurrCounter->GetEt(3, 0, Buffer, DataLen);
	              CurrCounter->E3.Ea_minus = CurrCounter->GetEt_minus(3, 0, Buffer, DataLen);
	              switch( CurrCounter->Type )
	              {
	                case MERCURY:
	                case PSCH:
		              CurrCounter->E3.Er = CurrCounter->GetEt(3, 1, Buffer, DataLen);
		              CurrCounter->E3.Er_minus = CurrCounter->GetEt_minus(3, 1, Buffer, DataLen);
	              };
	            }
	          }
	          SubFase = GET_E4;
	          break;
	        case GET_E4:
	          if(DataLen > 4)
	          {
	            if(CurrCounter->Modify == false)
	            {
		          CurrCounter->E4.Ea = CurrCounter->GetEt(4, 0, Buffer, DataLen);
		          CurrCounter->E4.Ea_minus = CurrCounter->GetEt_minus(4, 0, Buffer, DataLen);
	              switch( CurrCounter->Type )
	              {
	                case MERCURY:
	                case PSCH:
		              CurrCounter->E4.Er = CurrCounter->GetEt(4, 1, Buffer, DataLen);
		              CurrCounter->E4.Er_minus = CurrCounter->GetEt_minus(4, 1, Buffer, DataLen);
	              };
	            }
	          }
	          SubFase = GET_E5;
	          break;
	        case GET_E5:
	          if(DataLen > 4)
	          {
	            if(CurrCounter->Modify == false)
	            {
			      CurrCounter->E5.Ea = CurrCounter->GetEt(5, 0, Buffer, DataLen);
			      CurrCounter->E5.Ea_minus = CurrCounter->GetEt_minus(5, 0, Buffer, DataLen);
	              switch( CurrCounter->Type )
	              {
	                case MERCURY:
	                case PSCH:
			              CurrCounter->E5.Er = CurrCounter->GetEt(5, 1, Buffer, DataLen);
			              CurrCounter->E5.Er_minus = CurrCounter->GetEt_minus(5, 1, Buffer, DataLen);
	              };
	            }
	          }
	          SubFase = GET_Ef;
	          break;
	        case GET_Ef:
	          if(DataLen > 4 && CurrCounter->GetIsQ())
	          {
	            if(CurrCounter->Modify == false && CurrCounter->Type == MERCURY)
	            {
	              CurrCounter->E.Efa = CurrCounter->GetEt(0,0, Buffer, DataLen);
	              CurrCounter->E.Efb = CurrCounter->GetEt_minus(0,0, Buffer, DataLen);
	              CurrCounter->E.Efc = CurrCounter->GetEt(0,1, Buffer, DataLen);
	            }
	          }
	          SubFase = GET_E1f;
	          //Log::DEBUG("GET_Ef DataLen="+toString(DataLen)+" SubFase="+toString((int)SubFase));
	          break;
	        case GET_E1f:
	          if(DataLen > 4 && CurrCounter->GetIsQ())
	          {
	            if(CurrCounter->Modify == false && CurrCounter->Type == MERCURY)
	            {
	              CurrCounter->E1.Efa = CurrCounter->GetEt(1,0, Buffer, DataLen);
	              CurrCounter->E1.Efb = CurrCounter->GetEt_minus(1,0, Buffer, DataLen);
	              CurrCounter->E1.Efc = CurrCounter->GetEt(1,1, Buffer, DataLen);
	            }
	          }
	          SubFase = GET_E2f;
	          break;
	        case GET_E2f:
	          if(DataLen > 4 && CurrCounter->GetIsQ())
	          {
	            if(CurrCounter->Modify == false && CurrCounter->Type == MERCURY)
	            {
	              CurrCounter->E2.Efa = CurrCounter->GetEt(2,0, Buffer, DataLen);
	              CurrCounter->E2.Efb = CurrCounter->GetEt_minus(2,0, Buffer, DataLen);
	              CurrCounter->E2.Efc = CurrCounter->GetEt(2,1, Buffer, DataLen);
	            }
	          }
	          SubFase = GET_E3f;
	          break;
	        case GET_E3f:
	          if(DataLen > 4 && CurrCounter->GetIsQ())
	          {
	            if(CurrCounter->Modify == false && CurrCounter->Type == MERCURY)
	            {
	              CurrCounter->E3.Efa = CurrCounter->GetEt(3,0, Buffer, DataLen);
	              CurrCounter->E3.Efb = CurrCounter->GetEt_minus(3,0, Buffer, DataLen);
	              CurrCounter->E3.Efc = CurrCounter->GetEt(3,1, Buffer, DataLen);
	            }
	          }
	          SubFase = GET_E4f;
	          break;
	        case GET_E4f:
	          if(DataLen > 4 && CurrCounter->GetIsQ())
	          {
	            if(CurrCounter->Modify == false && CurrCounter->Type == MERCURY)
	            {
	              CurrCounter->E4.Efa = CurrCounter->GetEt(4,0, Buffer, DataLen);
	              CurrCounter->E4.Efb = CurrCounter->GetEt_minus(4,0, Buffer, DataLen);
	              CurrCounter->E4.Efc = CurrCounter->GetEt(4,1, Buffer, DataLen);
	            }
	          }
	          SubFase = GET_U;
	          break;

	        case GET_U:
	          CurrCounter->GetUa(Buffer, DataLen);
	          CurrCounter->GetUb(Buffer, DataLen);
	          CurrCounter->GetUc(Buffer, DataLen);
	          SubFase = GET_Ua;
	          break;
	        case GET_Ua:
	          CurrCounter->GetUa(Buffer, DataLen);
	          SubFase = GET_Ub;
	          break;
	        case GET_Ub:
	          CurrCounter->GetUb(Buffer, DataLen);
	          SubFase = GET_Uc;
	          break;
	        case GET_Uc:
	          CurrCounter->GetUc(Buffer, DataLen);
	          SubFase = GET_I;
	          break;

	        case GET_I:
	          CurrCounter->GetIa(Buffer, DataLen);
	          CurrCounter->GetIb(Buffer, DataLen);
	          CurrCounter->GetIc(Buffer, DataLen);
	          SubFase = GET_Ia;
	          break;
	        case GET_Ia:
	          CurrCounter->GetIa(Buffer, DataLen);
	          SubFase = GET_Ib;
	          break;
	        case GET_Ib:
	          CurrCounter->GetIb(Buffer, DataLen);
	          SubFase = GET_Ic;
	          break;
	        case GET_Ic:
	          CurrCounter->GetIc(Buffer, DataLen);
	          SubFase = GET_Pa;
	          break;

	        case GET_Pa:
	          CurrCounter->GetPa(Buffer, DataLen);
	          SubFase = GET_Pb;
	          break;
	        case GET_Pb:
	          CurrCounter->GetPb(Buffer, DataLen);
	          SubFase = GET_Pc;
	          break;
	        case GET_Pc:
	          CurrCounter->GetPc(Buffer, DataLen);
	          SubFase = GET_P;
	          break;
	        case GET_P:
	          CurrCounter->GetPa(Buffer, DataLen);
	          CurrCounter->GetPb(Buffer, DataLen);
	          CurrCounter->GetPc(Buffer, DataLen);
	          CurrCounter->GetP(Buffer,  DataLen);
	          SubFase = GET_Qa;
	          break;

	        case GET_Qa:
	          CurrCounter->GetQa(Buffer, DataLen);
	          SubFase = GET_Qb;
	          break;
	        case GET_Qb:
	          CurrCounter->GetQb(Buffer, DataLen);
	          SubFase = GET_Qc;
	          break;
	        case GET_Qc:
	          CurrCounter->GetQc(Buffer, DataLen);
	          SubFase = GET_Q;
	          break;
	        case GET_Q:
	          CurrCounter->GetQa(Buffer, DataLen);
	          CurrCounter->GetQb(Buffer, DataLen);
	          CurrCounter->GetQc(Buffer, DataLen);
	          CurrCounter->GetQ(Buffer,  DataLen);
	          SubFase = GET_S;
	          break;

	        case GET_S:
	          CurrCounter->GetSa(Buffer, DataLen);
	          CurrCounter->GetSb(Buffer, DataLen);
	          CurrCounter->GetSc(Buffer, DataLen);
	          CurrCounter->GetS(Buffer,  DataLen);
	          SubFase = GET_KM;
	          break;

	        case GET_KM:
	          if(DataLen >= 3)
	          {
	            CurrCounter->KM.KMa.CalcMinAverMax(CurrCounter->GetKMa(Buffer,  DataLen));
	            CurrCounter->KM.KMb.CalcMinAverMax(CurrCounter->GetKMb(Buffer,  DataLen));
	            CurrCounter->KM.KMc.CalcMinAverMax(CurrCounter->GetKMc(Buffer,  DataLen));
	            CurrCounter->KM.KM.CalcMinAverMax(CurrCounter->GetKM(Buffer,    DataLen));
	          }
	          SubFase = GET_F;
	          break;
	        case GET_F:
	          if(DataLen >= 3)
	            CurrCounter->F.F.CalcMinAverMax(CurrCounter->GetF(Buffer, DataLen));
	          SubFase = GET_KTRAN;
	          break;

	        case GET_KTRAN:
	          CurrCounter->KtranI = CurrCounter->GetKtranI(Buffer, DataLen);
	          CurrCounter->KtranU = CurrCounter->GetKtranU(Buffer, DataLen);
	          SubFase = GET_NETADDR;
	          break;

	        case GET_NETADDR:
	          //CurrCounter->KtranI = CurrCounter->GetKtranI(Buffer, DataLen);
	          //CurrCounter->KtranU = CurrCounter->GetKtranU(Buffer, DataLen);
	          CurrCounter->GetNetAddr(Buffer, DataLen);
	          SubFase = GET_SERIAL;
	          break;

	        case GET_SERIAL:
	          if(DataLen >= 3)
	          {
	        	  CurrCounter->SerialN.SerialNumber = CurrCounter->GetSerial(Buffer, DataLen);
	        	  if(CurrCounter->Type == MERCURY)
	        	  {
	        		  long serl = atol(CurrCounter->SerialN.SerialNumber.c_str());
	        		  if(serl <= 0)
	        		  {
	        			  CurrCounter->ErrCnt++;
	      	        	/////////////////////////////
	      	        	CurrCounter->E.ClearValues();
	      	        	CurrCounter->E1.ClearValues();
	      	        	CurrCounter->E2.ClearValues();
	      	        	CurrCounter->E3.ClearValues();
	      	        	CurrCounter->E4.ClearValues();
	      	        	CurrCounter->E5.ClearValues();
	      	        	CurrCounter->SerialN.SerialNumber = "0";
	      	        	/////////////////////////////
	      	        	CurrCounter->IsEnable   = false;
	      	        	CurrCounter->SetOrigAddr();

	      		        Port->Close();//!!
	      		        DataLen         = 0;
	      		        if( ++CounterIndex >=  Counters.size())//progon po ostalnim t.k. dalee
	      		        	CounterIndex = 0;

	      		        timeout = 5000;
	      		        SetTimer( COMMON_TIMER, timeout);
	      		        Log::ERROR("[TEnergyCounterManager] serialnumber ERROR Counter:"+ toString((int)CurrCounter->En));
	      		        SubFase     = SET_PLC;
	      	        	break;
	        		  }
	        	  }

	        	  CurrCounter->AckCnt++;
				  if( CurrCounter->IsEnable == false)//
				  {//
					if(!FirstInitFlg)
					{
						CurrCounter->Modify     = true;
						DateTime = SystemTime.GetTime();
					}
					CurrCounter->IsEnable   = true;
					CurrCounter->NeedFirstFlg = true;
				  }
	          }

	          SubFase = CLOSE_CHANNAL;
	          break;

	        default:
	        case CLOSE_CHANNAL://mast be in all counters!!!
	        	CounterIndex++;
	        	//Log::DEBUG( "[TEnergyCounterManager] Inc CounterIndex="+ toString(CounterIndex));
		        if( CounterIndex >=  Counters.size())//progon po ostalnim t.k. dalee
		        {
		        	//Log::DEBUG( "[TEnergyCounterManager] CounterIndex was: "+ toString(CounterIndex) + " set 0");
		        	CounterIndex = 0;
		        }
	          SetTimer( COMMON_TIMER, timeout);
	          SubFase = SET_PLC;
	          break;
	      }
	    }
	    else
	    {
	    	Log::DEBUG( "[TEnergyCounterManager] ParsingAnswer ERROR SubFase="+toString((int)SubFase) + " _parsingErrorCNT="+toString(_parsingErrorCNT));
	    	//Port->Close();
			if(_parsingErrorCNT++ >= 20)
			{
				CurrCounter->ErrCnt++;
				_parsingErrorCNT       = 0;
		        if( CurrCounter->IsEnable == true && SubFase == OPEN_CHANNAL )//
		        {//
		        	/////////////////////////////
		        	CurrCounter->E.ClearValues();
		        	CurrCounter->E1.ClearValues();
		        	CurrCounter->E2.ClearValues();
		        	CurrCounter->E3.ClearValues();
		        	CurrCounter->E4.ClearValues();
		        	CurrCounter->E5.ClearValues();
		        	CurrCounter->SerialN.SerialNumber = "0";
		        	/////////////////////////////
		        	CurrCounter->U.ClearValues();
		        	CurrCounter->I.ClearValues();
		        	CurrCounter->P.ClearValues();
		        	CurrCounter->Q.ClearValues();
		        	CurrCounter->S.ClearValues();
		        	CurrCounter->KM.ClearValues();
		        	CurrCounter->F.ClearValues();
		        	CurrCounter->LastTime = "";
		        	CurrCounter->Modify     = true;
		        	CurrCounter->IsEnable   = false;

		        	CurrCounter->NeedFirstFlg = true;
		        	DateTime = SystemTime.GetTime();
		        	CurrCounter->SetOrigAddr();
		        }
		        Port->Close();//!!

		        if( ++CounterIndex >=  size)//progon po ostalnim t.k. dalee
		        	CounterIndex = 0;
		        SubFase = SET_PLC;
		        timeout = 5000;
		        Log::ERROR("[TEnergyCounterManager] Parsing ERROR Counter:"+ toString((int)CurrCounter->En));
			}
		    else
		    {
		    	timeout = timeout*2;

		        if(_parsingErrorCNT == 15)
		        {
		        	//SubFase     = SET_PLC;
		        	Port->Close();//!!
		        	timeout = 5000;
		        }
		    }
			SetTimer( COMMON_TIMER, timeout);
	    }
	  }
	  Fase    = CREATE_CMD;
	}
	//=============================================================================
	void TEnergyCounterManager::ENERGY_DoECMD( void *Par )//Mercury only
	{
		static int 	_answerErrorCNT = 0;
		static string tmps = "";

		if( Par != NULL && Enable() )
		{
			bool flg = false;

			if( CurrentECMD != NULL)
			{
				if(CurrentECMD->counter != NULL)
				{
					ICounter *counter = CurrentECMD->counter;
					  Word  timeout 			= 0; //counter->BetweenTimeout;
					  Log::DEBUG("[CounterManager] ENERGY_DoECMD Fase="+toString((int)CurrentECMD->Fase) +" SubFase="+toString((int)CurrentECMD->SubFase ));
					  switch(CurrentECMD->Fase)
					  {
			           case CREATE_CMD:
			        	   if(CurrentECMD->SubFase == OPEN_CHANNAL)
			        	   {
							  if( counter->cntPort != Port )
							  {
								  if(Port != NULL)
									  Port->Close();
								  Port = counter->cntPort;
								  //sleep(1);
							  }
			        	   }
			        	   DataLen = counter->CreateCMD(CurrentECMD->SubFase, Buffer, CurrentECMD);
			        	   if(CurrentECMD->SubFase == OPEN_CHANNAL)
			        	   {
			        		   if(CurrentECMD->WithOpenChanal == false && CurrentECMD->NeedOpenChanal == false)
			        			   DataLen = 0;
			        		   else{
			        			   CurrentECMD->NeedOpenChanal = false;
			        		   }
			        	   }
			        	   else if(CurrentECMD->SubFase == CLOSE_CHANNAL)
			        	   {
			        		   if(CurrentECMD->WithCloseChanal == false)
			        			   DataLen = 0;
			        	   }
			        	   CurrentECMD->Fase    = SEND_CMD;
			             break;

			           case SEND_CMD:
			           {
			        	   int sended = Port->Send(Buffer, DataLen);
			        	   if( sended > 0 )
			        	   {
							 //////////////
							  int t = counter->Type;
							  Byte bf[2] {0,0};
							  string sended = "";
							  for(int i = 0; i < DataLen; i++){
								  ByteToHex(bf, Buffer[i]);
								  sended += "0x"+string((char*)bf, 2)+" ";
							  }
							  Log::DEBUG("[CounterManager] ENERGY_DoECMD::send bytes: [" + sended +"] len="+toString(DataLen));
							  if(t == ENERGOMERA)
								  Log::DEBUG("[CounterManager] ENERGY_DoECMD::send string: ["+ string( (char*)Buffer, DataLen ) + "] len="+toString(DataLen)+" SubFase="+toString((int)CurrentECMD->SubFase));

							  /*
							  if(t == 0 || t == 1 || t == 4|| t == 6 ){
								  Byte bf[2] {0,0};
								  string sended = "";
								  for(int i = 0; i < DataLen; i++){
									  ByteToHex(bf, Buffer[i]);
									  sended += "0x"+string((char*)bf, 2)+" ";
								  }
								  Log::DEBUG("[CounterManager] ENERGY_DoECMD send ECMD ok: [" + sended +"] len="+toString(DataLen));
							  }
							  else{
								  Log::DEBUG("[CounterManager] send ok: ["+ string( (char*)Buffer, DataLen ) + "] len="+toString(DataLen)+" SubFase="+toString((int)CurrentECMD->SubFase));
							  }*/

							 //////////////
							 TBuffer::ClrBUF(Buffer, DataLen);
							 CurrentECMD->Fase = RECV_CMD;
			        	   }
			        	   else if(sended == 0)
			        	   {
			        		   CurrentECMD->Fase = EXEC_CMD;
			        	   }
			        	   else
			        	   {
							 CurrentECMD->Fase 	= EXEC_CMD;
					      	 CurrentECMD->SubFase = END_ECMD;
			        	   }
			           }
			             break;

			           case RECV_CMD:
						 {
							 sWord Len = (CurrentECMD->RetLen > 0 && CurrentECMD->SubFase == SET_ECMD) ?
																						 CurrentECMD->RetLen :
																						 counter->GetRecvMessageLen(CurrentECMD->SubFase);

	 						 Log::DEBUG("[CounterManager] ENERGY_DoECMD RecvData needLen=" + toString(Len));
							 DataLen = counter->RecvData(Port, Buffer, Len, sizeof(Buffer), CurrentECMD->SubFase, Len);//
	 						  ///////////////////////////
	 						  int t = counter->Type;

 							  Byte bf[2] {0,0};
 							  string recv = "";
 							  for(int i = 0; i < DataLen; i++){
 								  ByteToHex(bf, Buffer[i]);
 								  recv += "0x"+string((char*)bf, 2)+" ";
 							  }
 							 Log::DEBUG("[CounterManager] ENERGY_DoECMD::RecvData bytes: [" + recv +"] RecvLen="+toString(DataLen)+" needLen=" + toString(Len) + " SubFase="+toString((int)CurrentECMD->SubFase));
 							 if (t == ENERGOMERA)
 								 Log::DEBUG("[CounterManager] ENERGY_DoECMD::RecvData string: ["+ string( (char*)Buffer, DataLen ) + "] len="+toString(DataLen)+" SubFase="+toString((int)CurrentECMD->SubFase));



	 						  /*
	 						  if(t == 0 || t == 1 || t == 4|| t == 6 || DataLen < 4)
	 						  {
	 							  Byte bf[2] {0,0};
	 							  string recv = "";
	 							  for(int i = 0; i < DataLen; i++){
	 								  ByteToHex(bf, Buffer[i]);
	 								  recv += "0x"+string((char*)bf, 2)+" ";
	 							  }
	 							 Log::DEBUG("[CounterManager] ENERGY_DoECMD::RecvData: [" + recv +"] RecvLen="+toString(DataLen)+" needLen=" + toString(Len) + " SubFase="+toString((int)CurrentECMD->SubFase));
	 						  }
	 						  else{
	 							  Log::DEBUG("[CounterManager] RecvData ok: ["+ string( (char*)Buffer, DataLen ) + "] len="+toString(DataLen)+" SubFase="+toString((int)CurrentECMD->SubFase));
	 						  }*/
	 						  ///////////////////////////
			        		 if( DataLen > 0 && Len == DataLen)
			        		 {
			 					if( counter->ParsingAnswer(Buffer, DataLen, CurrentECMD->SubFase) )
			 					{
			 						//Log::ERROR("[CounterManager] ENERGY_DoECMD ParsingAnswer ERROR _answerErrorCNT="+toString((int)_answerErrorCNT));
			 						if(CurrentECMD->SubFase != OPEN_CHANNAL){
			 							_answerErrorCNT       = 0;
			 						}
			 						CurrentECMD->Fase = EXEC_CMD;
			 					}
			 					else
			 					{
			 						Log::ERROR("[CounterManager] ENERGY_DoECMD ParsingAnswer ERROR _answerErrorCNT="+toString((int)_answerErrorCNT));
			 					    if(_answerErrorCNT++ >= 20)
			 					    { //err
			 					       _answerErrorCNT       = 0;
			 					       CurrentECMD->SubFase 	= END_ECMD;
			 					       CurrentECMD->Fase 		= EXEC_CMD;
			 					    }
			 					    else
			 					    {
			 					    	CurrentECMD->Fase = CREATE_CMD;
			 					    }
			 					}
			        		 }
			        		 else
			        		 {
			        			 if(CurrentECMD->SubFase == CLOSE_CHANNAL)
			        			 {
									CurrentECMD->SubFase 	= END_ECMD;
									CurrentECMD->Fase 		= EXEC_CMD;
			        			 }
			        			 else
			        			 {
			        				 Log::DEBUG("[CounterManager] ENERGY_DoECMD ERROR _answerErrorCNT=" + toString(_answerErrorCNT)+"  SubFase="+toString((int)CurrentECMD->SubFase)+" Fase="+toString((int)CurrentECMD->Fase));
									 if(_answerErrorCNT++ >= 20)
									 { //err
										_answerErrorCNT       	= 0;
										CurrentECMD->SubFase 	= END_ECMD;
										CurrentECMD->Fase 		= EXEC_CMD;
									 }
									 else
									 {
										  if( DataLen == 4 && counter->ParsingAnswer(Buffer, DataLen, CurrentECMD->SubFase) )
										  {
											  if( Buffer[1] == 0x05 )
											  {
												  CurrentECMD->NeedOpenChanal 	= true;
											  	  CurrentECMD->SubFase 			= OPEN_CHANNAL;
											  }
											  else if( Buffer[1] == 0x01 )
											  {
												  CurrentECMD->SubFase  	= CLOSE_CHANNAL;
											  }
										  }
										  CurrentECMD->Fase 				= CREATE_CMD;
									 }
			        			 }
			        		 }
			        		 //CurrentECMD->Fase = EXEC_CMD;
	 						 SetTimer( COMMON_TIMER, timeout);
			        		 //Log::DEBUG("[CounterManager] ENERGY_DoECMD  SubFase="+toString((int)CurrentECMD->SubFase)+" Fase="+toString((int)CurrentECMD->Fase));
						 }
			             break;

			           case EXEC_CMD:
			        	 switch(CurrentECMD->SubFase)
			        	 {
							case WAIT_ECMD:
								CurrentECMD->Fase 		= CREATE_CMD;
								CurrentECMD->SubFase  	= OPEN_CHANNAL;

								// pri CLOSE_CHANNAL i
								//if(CurrentECMD->NeedOpenChanal == true)CurrentECMD->SubFase  = OPEN_CHANNAL;//!!!

								//Log::DEBUG("[CounterManager] ENERGY_DoECMD::WAIT_ECMD");
							  break;

							case OPEN_CHANNAL:
								CurrentECMD->Fase 		= CREATE_CMD;
								CurrentECMD->SubFase  = SET_ECMD;
								Log::DEBUG("[CounterManager] ENERGY_DoECMD::OPEN_CHANNAL");
							  break;

							case SET_ECMD:
								{
									string header = TBuffer::DateTimeToString( &CurrentECMD->DateTime )+">>";
									string framStr = counter->GetEcmd(CurrentECMD, Buffer, DataLen, ReturnValue);
									if(framStr.size() > 0)
									{
										if(CurrentECMD->Index >= 0)
										{
											tmps += framStr;
											//Log::DEBUG("ENERGY_DoECMD index="+toString(CurrentECMD->Index) + " framStr="+framStr);
											if(CurrentECMD->Index == 23)
											{
												framStr = WordToString(CurrentECMD->Etype)+',' + WordToString( CurrentECMD->Ename )+',' + tmps;
												framStr = header + framStr;
												framStr += ">>NeedCmdAnswer=0";
												TFifo<string> *framFifo = (TFifo<string> *)Par;

												if(!framFifo->empty())
												{
													string last = framFifo->back();
													if(framStr != last){
														framFifo->push( framStr );
														Log::DEBUG("ENERGY_DoECMD framFifo->push framStr="+framStr+" Index="+toString(CurrentECMD->Index));
													}
													else{
														Log::DEBUG("[CounterManager] ENERGY_DoECMD framStr==last [" + framStr+"]");
													}
												}
												else
												{
													framFifo->push( framStr );
													Log::DEBUG("ENERGY_DoECMD framFifo->push to empty framStr="+framStr+" Index="+toString(CurrentECMD->Index));
												}
												tmps = "";
											}
											else if(CurrentECMD->Index == 0)
											{
												tmps = framStr;
											}
										}
										else
										{
											framStr = header + framStr;
											framStr += ">>NeedCmdAnswer=0";
											TFifo<string> *framFifo = (TFifo<string> *)Par;
											framFifo->push( framStr );
											tmps = "";
										}
									}

									 CurrentECMD->Fase 		= CREATE_CMD;
									 CurrentECMD->SubFase  	= CLOSE_CHANNAL;
									 //Log::DEBUG("[CounterManager] ENERGY_DoECMD::SET_ECMD");
								}
							  break;

							 case END_ECMD:
								//Log::DEBUG("[CounterManager] ENERGY_DoECMD::END_ECMD");
								if(CurrentECMD != NULL)
								{
									delete CurrentECMD;
									CurrentECMD = NULL;
								}
								SetTimer( COMMON_TIMER, 100);
								break;

							 default:
								 //Log::DEBUG("[CounterManager] ENERGY_DoECMD::default CurrentECMD->SubFase="+toString((int)CurrentECMD->SubFase));
								 CurrentECMD->Fase = EXEC_CMD;
								 CurrentECMD->SubFase  = END_ECMD;
								break;
			        	 };
			             break;

			           default:
			        	   CurrentECMD->Fase 	= EXEC_CMD;
			        	   CurrentECMD->SubFase = END_ECMD;
			        	   break;
					  }
					  flg = true;
				}
				else
				{
					delete CurrentECMD;
					CurrentECMD = NULL;
				}
			}
			else
			{
				for(auto counter: Counters)
				{
					if(counter->ECMDMessages.size()> 0)
					{
						CurrentECMD = counter->ECMDMessages.front();
						CurrentECMD->counter = counter;
						counter->ECMDMessages.pop_front();
						flg = true;
					}
				}
			}

			if( GetTimValue(LONG_TIMER) <= 0 )
			{
				Log::DEBUG("[CounterManager] ECMDMessages LONG_TIMER exit");
				flg = false;
			}

			if(!flg)
			{//exit
				Log::DEBUG("[CounterManager] ECMDMessages exit");
				Fase    = CREATE_CMD;
				SubFase = CLOSE_CHANNAL;
				//SubFase = SET_PLC;
			}
		}
	}
	//=============================================================================
	void TEnergyCounterManager::ENERGY_CreateMessageCMD( void *Par )
	{
	if( Par != NULL && Enable() )
	  {
	    if(ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0)
	    {
	      ICounter *CurrCounter = NULL;//new
	      int size = Counters.size();
	      int index;
	      string framStr = "";

	      for( index = 0; index < size; index++ )
	      {
	    	CurrCounter = GetSimpleCounter(index);
	    	if(CurrCounter == NULL) continue;
	        if( CurrCounter->Modify == true )//new
	        {
	        	Log::DEBUG( "[CounterManager] ENERGY_CreateMessageCMD CurrCounterIndex="+toString(index));
	        	framStr = CurrCounter->GetStringValue();


	        	if(CurrCounter->NeedFirstFlg == true)
	        	{
					if(!isNullOrWhiteSpace(CurrCounter->Poverka)){
						framStr+="\r\n>,99,2,"+CurrCounter->Poverka+',';
					}
					//if(CurrCounter->IsEnable)//plc
					framStr+="\r\n>,99,3,"+toString(CurrCounter->plc)+','+toString(CurrCounter->ProfilCounter);
					CurrCounter->NeedFirstFlg = false;
	        	}

                if(framStr.size() > 0){
                	framStr  = TBuffer::DateTimeToString( &DateTime )+">>"+framStr;
                	TFifo<string> *framFifo = (TFifo<string> *)Par;
                    framFifo->push( framStr );
                }
	            framStr.clear();
	            CurrCounter->Modify = false;
	        }
	      }//for
	    }
	  }
	}
	//=============================================================================
	void TEnergyCounterManager::ENERGY_DetectStates( void  )
	{
		if(! Enable()) return;
		bool flag = false;
		if( GetTimValue(PERIUD_TIMER) <= 0 )
		{
		    DWord tim = SystemTime.GetGlobalSeconds();
		    if(FirstInitFlg == true)
		    {
		    		Log::DEBUG("ENERGY_DetectStates FirstInitFlg="+toString(FirstInitFlg));
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
		    			//Log::DEBUG("ENERGY_DetectStates err="+toString(err)+" ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
		    			flag     = true;
		    			ModifyTime = ((DWord)((DWord)(tim / Period) + 1)) * Period;
		    			if(ModifyTime > MAX_SECOND_IN_DAY)
		    				ModifyTime = Period;
		    		}
		    }
			if( flag == true  && Enable() )
			{
				  DateTime = SystemTime.GetTime();
				  Log::DEBUG("ENERGY_DetectStates Complite ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
				  for(auto curr: Counters)
				  {
					  curr->Modify = true;//new
					  curr->NeedFirstFlg = true;
				  }
				  SetTimer( PERIUD_TIMER, 30000 );//30s
			}
			else
			{
				  SetTimer( PERIUD_TIMER, 500 );
			}
		}
	}
	//=============================================================================
	bool TEnergyCounterManager::Enable( void )
	{
		return Counters.size() > 0;
	}
	//=============================================================================
	//=============================================================================
	string TEnergyCounterManager::CreateCfgString()
	{
	  string tmp = "";

	  for(auto curr: Counters)
	  {
	    tmp += WordToString(curr->Addr)+",";//new
	  }

	  return "41,1,\r\n>,120,1,"+tmp;
	}
	//=============================================================================
	void TEnergyCounterManager::ExeCmd(Byte et, Byte en, int cmd, vector<string> params, void* p3)
	{
		if(busyEcmd == true) return;
		busyEcmd = true;
		Log::DEBUG("TEnergyCounterManager::ExeCmd start et="+toString((int)et) + " en="+toString((int)en) + " cmd="+toString(cmd) + " busyEcmd=" + toString(busyEcmd));
		try
		{
			ICounter *curr_counter = NULL;
			for(auto counter: Counters)
			{
				if(counter->Et == et && counter->En == en)
				{
					curr_counter = counter;
					break;
				}
			}
			if(curr_counter == NULL)
			{
				if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
				return;
			}

			string framStr = "";
			bool flg = false;
			switch(cmd)
			{
			case 0://gettime send: >>,41,0,>,51,1,0 answer: >>,41,0, >,51,1,40,14.02.2019 19:15:57,1,
				flg = curr_counter->GetDateTime();
				break;

			case 1://correcttime: >>,41,0,>,51,1,1,3,1,
				if(params.size()>=1)
				{
					int tz = atoi(params[0].c_str());
					if(tz >= 0 && tz <= 12)
					{
						string uci_base_str = "@ecounter["+toString(curr_counter->index)+"]";
						string tmp_str = uci::Get(CONFIG_FILE, uci_base_str+".timezone");
						if(tmp_str.size() > 0){
							curr_counter->timezone= atoi(tmp_str.c_str());
						}

						if(curr_counter->timezone != tz){
							curr_counter->timezone = tz;

							uci::Set(CONFIG_FILE, uci_base_str, "timezone", toString(tz));

						}

						curr_counter->CouterType.UTC = tz;
						struct tm nowutc = SystemTime.NowUtc();
						Log::DEBUG( "CorrectDateTime utcTime="+TimeToString(nowutc) );
						nowutc.tm_hour += tz;
						time_t next = mktime(&nowutc);
						struct tm need = *localtime(&next);
						Log::DEBUG( "CorrectDateTime neeCounterTime="+TimeToString(&need) );
						if(curr_counter->GetDateTime())
						{
							Fase    = CREATE_CMD;//
							SubFase = SET_PLC;//
							needEcmd = true;
							TReturnValue rval = GetReturnValue();
							if(rval.Data.size() >= 3)
							{
								bool needcorrect = false;
								if(params.size()>=2)
								{
									needcorrect = atoi(params[1].c_str());
									Log::DEBUG("CorrectDateTime needcorrect="+toString(needcorrect));
								}

								tm time = 	StringToTime2(rval.Data[1]);
								int diff =  (int)difftime( mktime(&time), mktime(&need) );
								int origdiff = diff;
								string difftime = toString(diff);
								Log::DEBUG("difftime="+toString(difftime));
								if(diff < 0)
									diff *=-1;

								int mindiff = 5;
								int maxdiff = 7200;//2h  //43200;//12h

								if( (diff > mindiff && diff <= maxdiff) || needcorrect)
								{//diff > 1 min
									string newtime = IntToString(need.tm_sec, 2)+":"+IntToString(need.tm_min, 2)+":"+IntToString(need.tm_hour, 2);////ss:mm:hh
									if(curr_counter->Type == ENERGOMERA)
									{
										newtime = difftime;
										if(diff > 30)
										{
											newtime = origdiff < 0 ? "-29" : "29";
										}
									}
									flg = curr_counter->CorrectDateTime(newtime);
									Log::DEBUG( "CorrectDateTime newtime="+newtime + " flg="+toString(flg));

									framStr += WordToString(curr_counter->Et)+',';
									framStr += WordToString( curr_counter->En )+',';
									framStr +=  flg == true ? ">,51,3,0" : ">,51,3,1";//0-ok 1 err
									framStr += ">>NeedCmdAnswer=0";
								}
								else if (diff > maxdiff)
								{// diff > 2 h
									Log::DEBUG("CorrectDateTime Correct time error diff="+toString(diff));
									framStr += WordToString(curr_counter->Et)+',';
									framStr += WordToString( curr_counter->En )+',';
									framStr +=">,51,3,11,"+TimeToString(time)+","+TimeToString(need);
									framStr += ">>NeedCmdAnswer=0";
									if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
								}
								else
								{
									Log::DEBUG("CorrectDateTime Do not need correct time diff="+toString(diff));
									framStr += WordToString(curr_counter->Et)+',';
									framStr += WordToString( curr_counter->En )+',';
									framStr +=">,51,3,1";//ok
									framStr += ">>NeedCmdAnswer=0";
									if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
								}
							}
						}
						else{
							if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
						}
					}
					else{
						if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
					}
				}
				else{
					if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
				}
				break;
			case 2://get profil
				if(params.size() >= 2)
				{//>>,41,0,>,51,1,2,3,06.08.2018
					int tz = atoi(params[0].c_str());
					if(tz >= 0 && tz <= 12)
					{
						Log::DEBUG( "GetProfil params index1 ="+params[1]);
						curr_counter->CouterType.UTC = tz;
						curr_counter->ProfilCounter++;

						tm nowutc 	= SystemTime.NowUtc();
						tm starttime = StringToDate2(params[1]);

						nowutc.tm_hour += tz;
						time_t next = mktime(&nowutc);
						tm nowtime = *localtime(&next);

						if(curr_counter->CouterType.Const > 0)
						{
							string ctype = curr_counter->CouterType.Type;
							size_t pos = ctype.find("P");
							if(curr_counter->HasProfile() == false)
							{
								Log::DEBUG( "GetProfil Counter have not profile="+ctype);
								framStr += WordToString(curr_counter->Et)+',';
								framStr += WordToString( curr_counter->En )+',';
								framStr +=">,51,5,13,"+ctype;
								framStr += ">>NeedCmdAnswer=0";
								if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
							}
							else
							{
								double diff =  difftime(mktime(&nowtime), mktime(&starttime));
								Log::DEBUG( "GetProfil starttime="+TimeToString(&starttime) +" nowtime="+TimeToString(&nowtime) + " diff="+toString(diff));
								if(diff < 5184000)//60 days in seconds  60*(24*60*60) = 5184000
								{
									if(curr_counter->GetDateTime())
									{
										Fase    = CREATE_CMD;//
										SubFase = SET_PLC;//
										needEcmd = true;

										TReturnValue rval = GetReturnValue();
										if(rval.Data.size() >= 3)
										{
											tm timeFromCounter = 	StringToTime2(rval.Data[1]);
											Log::DEBUG( "GetProfil timeFromCounter="+TimeToString(&timeFromCounter));

											int counter_diff =  (int)difftime(mktime(&nowtime), mktime(&timeFromCounter));

											if(counter_diff < 0)
												counter_diff *= -1;

											Log::DEBUG("GetProfil counter_diff="+toString(counter_diff));
											if(counter_diff <= 7200) //2h //1800sec=30min
											{
												if(curr_counter->GetLastProfile())
												{
													Fase    = CREATE_CMD;//
													SubFase = SET_PLC;//
													needEcmd = true;
													TReturnValue rval = GetReturnValue();
													if(rval.Data.size() >= 4)
													{
														tm lasttime = StringToTime2(rval.Data[1]);
														unsigned int last_addr = atoi(rval.Data[2].c_str());

														long profil_diff =  (long)difftime(mktime(&lasttime), mktime(&starttime));///30*60
														Log::DEBUG("GetProfil profil_diff in sec="+toString(profil_diff));

														profil_diff = profil_diff / 60;
														Log::DEBUG("GetProfil profil_diff in min="+toString(profil_diff));

														profil_diff = profil_diff / 30;
														Log::DEBUG("GetProfil profil_diff in 30min="+toString(profil_diff));

														Log::DEBUG("GetProfil Lasttime="+TimeToString(&lasttime)+ " Starttime="+ TimeToString(&starttime) +" last_addr="+toString(last_addr)+" state="+rval.Data[3] + " profil_diff="+toString(profil_diff) );

														if(profil_diff <= 0)//in half of hour
														{
															Log::DEBUG("GetProfil eror profil_diff="+toString(profil_diff));
															framStr += WordToString(curr_counter->Et)+',';
															framStr += WordToString( curr_counter->En )+',';
															framStr +=">,51,5,14,"+toString(profil_diff);
															framStr += ">>NeedCmdAnswer=0";
														}
														else
														{
															int maxcnt = 4096;
															string cmodel = ctype;//curr_counter->CouterType.Model;
															pos = cmodel.find("230");
															bool is234 = false;
															if(pos == string::npos)
															{//234, 236
																maxcnt = 8192;
																is234 = true;
															}
															int div = is234 == true ? 1: 16;
															long new_addr = 	( last_addr/div - (profil_diff-1) );

															Log::DEBUG("GetProfil new_addr="+toString(new_addr) + " cmodel="+cmodel + " div="+toString(div));

															if(new_addr < 0)
																new_addr = maxcnt + new_addr;

															Log::DEBUG("GetProfil CounterModel=" +cmodel + " maxcnt="+ toString(maxcnt) + " new_addr="+toString(new_addr));
															for(int ind = 0; ind < 24; ind++)
															{
																Log::DEBUG("GetProfil ind="+toString(ind));
																Word new_addr_full = (new_addr + ind*2)*16;
																if(is234 == false)
																{//!!!
																	new_addr_full = new_addr_full > 0xFFFF ? (new_addr_full - 0x10000) :  new_addr_full;
																}
																Log::DEBUG("GetProfil new_addr_full="+toString(new_addr_full));
																if(curr_counter->GetProfileByAddr(ind, new_addr_full, params[1], ind == 0, ind == 23) == true)
																{
																	Fase    = CREATE_CMD;//
																	SubFase = SET_PLC;//
																	needEcmd = true;
																}
															}

															for(int ii = 0 ; ii < 15; ii++)
															{
																sleep(1);
																if(curr_counter->ECMDMessages.size() ==  0){
																	sleep(1);
																	break;
																}

															}
														}
													}
												}
											}
											else
											{
												Log::DEBUG("GetProfil error: time diff error="+toString(diff));
												framStr += WordToString(curr_counter->Et)+',';
												framStr += WordToString( curr_counter->En )+',';
												framStr +=">,51,5,12,"+toString(diff);
												framStr += ">>NeedCmdAnswer=0";
												//if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
											}
										}
										else{
											//if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
										}
									}
									else{
										//if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
									}
								}
								else{
									//if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
								}
							}
						}
						else
						{
							Log::DEBUG("GetProfil error: CouterType.Const  error="+toString(curr_counter->CouterType.Const ));
							framStr += WordToString(curr_counter->Et)+',';
							framStr += WordToString( curr_counter->En )+',';
							framStr +=">,51,5,10,"+toString(curr_counter->CouterType.Const );
							framStr += ">>NeedCmdAnswer=0";
							if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
						}
					}
					else{
						if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
					}
				}
				else{
					if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
				}
				break;

			case 3://setttime: >>,41,0,>,51,1,3,3,222222,
				if(params.size()>=1)
				{
					int tz = atoi(params[0].c_str());
					if(tz >= 0 && tz <= 12)
					{
						curr_counter->CouterType.UTC = tz;
						struct tm nowutc = SystemTime.NowUtc();
						Log::DEBUG( "SetDateTime utcTime="+TimeToString(nowutc) );
						nowutc.tm_hour += tz;
						time_t next = mktime(&nowutc);
						struct tm need = *localtime(&next);
						Log::DEBUG( "SetDateTime neeCounterTime="+TimeToString(&need) );
						if(curr_counter->GetDateTime())
						{
							Fase    = CREATE_CMD;//
							SubFase = SET_PLC;//
							needEcmd = true;
							TReturnValue rval = GetReturnValue();
							if(rval.Data.size() >= 3)
							{
								tm time = 	StringToTime2(rval.Data[1]);
								int diff =  (int)difftime(mktime(&time), mktime(&need));
								if(diff < 0)
									diff *=-1;
								Log::DEBUG("SetDateTime diff="+toString(diff));
								int mindiff = 30;//7200;//2h
								//int maxdiff = 43200;//12h


								if( diff > mindiff )
								{//diff > 2 h
									string newdate = IntToString(need.tm_wday, 2)+"."+IntToString(need.tm_mday, 2)+"."+IntToString(need.tm_mon+1, 2)+"."+IntToString(need.tm_year-100, 2);
									string newtime = IntToString(need.tm_sec, 2)+":"+IntToString(need.tm_min, 2)+":"+IntToString(need.tm_hour, 2);////ss:mm:hh
									string newdt = newdate + " " + newtime; //wd.dd.MM.yyyy ss:mm:hh

									string password = "";
									if(params.size() >= 2){
										password = params[1];
									}

									flg = curr_counter->SetDateTime(newdt, password);
									Log::DEBUG( "SetDateTime newdt="+newdt + "(wd.dd.MM.yyyy ss:mm:hh) flg="+toString(flg));

									framStr += WordToString(curr_counter->Et)+',';
									framStr += WordToString( curr_counter->En )+',';
									framStr +=  flg == true ? ">,51,4,0" : ">,51,4,1";//0-ok 1 err
									framStr += ">>NeedCmdAnswer=0";
									//curr_counter->GetDateTime();
								}
								else
								{
									Log::DEBUG("SetDateTime Do not need set time diff="+toString(diff));
									framStr += WordToString(curr_counter->Et)+',';
									framStr += WordToString( curr_counter->En )+',';
									framStr +=">,51,4,1";//ok
									framStr += ">>NeedCmdAnswer=0";

									if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
								}
							}
							else{
								if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
							}
						}
						else{
							if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
						}
					}
					else
					{
						if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
					}
				}
				else{
					if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
				}
				break;

			default:
				if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
				break;
			};

			if(framStr.size() > 0 && p3 != NULL)
			{
				TRtcTime dt = SystemTime.GetTime();
				string header = TBuffer::DateTimeToString( &dt )+">>";
				framStr = header + framStr;
				TFifo<string> *framFifo = (TFifo<string> *)p3;
				framFifo->push( framStr );
			}

			if(flg)
			{
				Fase    = CREATE_CMD;//
				SubFase = SET_PLC;//
				needEcmd = flg;
			}
			else
			{
				//if(ReturnValue!=NULL) ReturnValue->IsComplite(true);
			}
		}
		catch(exception &e)
		{
			Log::ERROR( e.what() );
		}
		busyEcmd = false;
		Log::DEBUG("TEnergyCounterManager::ExeCmd end et="+toString((int)et) + " en="+toString((int)en) + " cmd="+toString(cmd) + " busyEcmd=" + toString(busyEcmd));
	}
	//=============================================================================
	void TEnergyCounterManager::SetNewEcmd(vector<EcounterCmdParameter> cmdList)
	{
		Log::DEBUG("TEnergyCounterManager::SetNewEcmd size ="+ toString(cmdList.size()));
		TRtcTime dt = SystemTime.GetTime();
		bool flg = false;
		for(auto curr: cmdList)
		{
			for(auto counter: Counters)
			{
				if(counter->Et == curr.Et && counter->En == curr.En)
				{
					flg = counter->SetNewEcmd(dt, curr);
				}
			}
		}
		if(flg)
		{
			Fase    = CREATE_CMD;//
			SubFase = SET_PLC;//
			needEcmd = flg;
		}
	}
	//=============================================================================
	void TEnergyCounterManager::SetGetProfilCmd(int daysbefore, void* p3)
	{
		Log::DEBUG("TEnergyCounterManager::SetGetProfilCmd");

		for(auto counter: Counters)
		{////>>,41,0,>,51,1,2,3,06.08.2018
			//param[0] = timezone, param[1] = date
			if(counter->timezone > 0)
			{
				tm nowutc 	= SystemTime.NowUtc();
				nowutc.tm_mday += daysbefore;
				if( mktime(&nowutc) != -1)
				{
					vector<string> params;
					params.push_back( toString(counter->timezone) );
					params.push_back( TimeToDateString2(nowutc) );
					Log::DEBUG("TEnergyCounterManager::GetProfilCmd for Et="+
							toString((int)counter->Et)+ " En="+toString((int)counter->En) +" timezone="+toString(counter->timezone) + " date="+params[1]);
					Word st = 2;//st=0 - get time; st=1 - set time; st=2 - getProfil
					ExeCmd(counter->Et, counter->En, st, params, p3);
				}
			}
		}
	}
	//=============================================================================
	bool TEnergyCounterManager::CheckProfil(int day)
	{
		Log::DEBUG("TEnergyCounterManager::CheckProfil day="+toString(day)+" ApiAddress="+ApiAddress + " ApiPort="+toString(ApiPort) + " ObjectId="+ObjectId);
		bool ret = false;
		if(ApiAddress.length() == 0 || ApiPort == 0 || ObjectId.length() == 0)
		{
			return false;
		}
		for(auto counter: Counters)
		{//
			if(counter->timezone > 0)
			{
				tm nowutc 	= SystemTime.NowUtc();
				int daysbefore = day;
				if(daysbefore > 0)
					daysbefore *= -1;

				nowutc.tm_mday += daysbefore;
				if( mktime(&nowutc) != -1)
				{
					string url = "http://"+ApiAddress+":"+toString(ApiPort)+"/api/GetData/"+ObjectId+"/99/"+ toString(counter->timezone)+"/"+toString((int)counter->En)+"_"+ TimeToDateString2(nowutc);
					Log::DEBUG("TEnergyCounterManager::CheckProfil url="+url);

					char buffer[1024];
					HTTPResult res = HttpClient.get(url.c_str(), buffer, 1024, 20000);

					//Log::DEBUG("TEnergyCounterManager::CheckProfil res="+toString(res));
					if(res == HTTP_OK)
					{
						Log::DEBUG("HTTP_OK");
						string answer = JsonParse("Value", buffer);
						if(!isNullOrWhiteSpace(answer))
						{
							Log::DEBUG("TEnergyCounterManager::CheckProfil answer ("+answer+")");
							int value = atoi(answer.c_str());

							if(value == 0 || value == 2)
							{
								Log::INFO("TEnergyCounterManager::CheckProfil need get profil for day="+toString(day));
								ret = true;
							}
						}
					}
				}
			}
		}
		return ret;
	}
	//=============================================================================

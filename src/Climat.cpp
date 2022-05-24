/*
 * Climat.cpp
 *
 *  Created on: May 15, 2015
 *      Author: user
 */

#include <Climat.h>

IClimat::IClimat():TFastTimer(2,&MilSecCount)
{
	// TODO Auto-generated constructor stub
	  Modify = false;
	  Period.SetData( 600 );
	  SetTimer(PERIUD_TIMER, 10000);
	  FirstInitFlg.SetData(true);
	  NeedModify.SetData(false);
	  ModifyTime = Period.GetData();
}

IClimat::~IClimat() {
	// TODO Auto-generated destructor stub
}
//*****************************************************************************
//***
//*****************************************************************************
TMkkClimatManager::TMkkClimatManager()
{
	LastErrorState = 0;
	LastState = 0xF;
	isSqlDB = false;
}
//=============================================================================
bool TMkkClimatManager::InitDB(){
	isSqlDB = Repository::Mkk2Table.InitTable();
	return isSqlDB;
}
//=============================================================================
Word TMkkClimatManager::GetLastState(int lastState){
	Word retState = lastState;
	if(isSqlDB){
		vector<Mkk2> data = Repository::Mkk2Table.Select("", "DataIndex");
		if(data.size() == 4){
			for(auto curr: data){
				retState ^=  (curr.Inverted << curr.DataIndex);//XOR
			}
		}
	}
	return retState;
}
//=============================================================================
void TMkkClimatManager::ReloadConfig(void)
{
	try
	{
		ClimatSettings tmp = Settings();
		string climat = uci::Get(CONFIG_FILE, "climat", "enable");
		if(climat.size() > 0)
		{
			bool isEnable = atoi(climat.c_str());
			tmp.Enable = isEnable;
			if(isEnable)
			{
				string isauto = uci::Get(CONFIG_FILE, "climat", "isauto");
				if(isauto.size() > 0){
					bool isAuto = atoi(isauto.c_str());
					tmp.IsAuto = isAuto;
				}
				tmp.Cond1OnTemp = atof(uci::Get(CONFIG_FILE,    "climat", "cond1on").c_str());
				tmp.Cond1OffTemp = atof(uci::Get(CONFIG_FILE,   "climat", "cond1off").c_str());
				tmp.Cond2OnTemp = atof(uci::Get(CONFIG_FILE,    "climat", "cond2on").c_str());
				tmp.Cond2OffTemp = atof(uci::Get(CONFIG_FILE,   "climat", "cond2off").c_str());
				tmp.Heater1OnTemp = atof(uci::Get(CONFIG_FILE,  "climat", "heater1on").c_str());
				tmp.Heater1OffTemp = atof(uci::Get(CONFIG_FILE, "climat", "heater1off").c_str());
				tmp.Heater2OnTemp = atof(uci::Get(CONFIG_FILE,  "climat", "heater2on").c_str());
				tmp.Heater2OffTemp = atof(uci::Get(CONFIG_FILE, "climat", "heater2off").c_str());
			}
			string etype = uci::Get(CONFIG_FILE, "climat", "etype");
			if(etype.size() > 0){
				int et = atoi(etype.c_str());
				tmp.Etype = et;
			}
			else
				tmp.Etype = 60;

			string ename = uci::Get(CONFIG_FILE, "climat", "ename");
			if(ename.size() > 0){
				int en = atoi(ename.c_str());
				tmp.Ename = en;
			}
			else
				tmp.Ename = 1;
			///////
			string rotate = uci::Get(CONFIG_FILE, "climat", "rotate");
			if(rotate.size() > 0){
				long Rot = atol(rotate.c_str());
				tmp.RotateTime = Rot;
			}
			else
				tmp.RotateTime = 720;//720min=12h

			string clear = uci::Get(CONFIG_FILE, "climat", "needclear");
			if(clear.size() > 0){
				bool flg = atoi(clear.c_str());
				if(flg){
					uci::Set(CONFIG_FILE, "climat", "needclear", "0");
					Clear();
				}
			}
			else{
				uci::Set(CONFIG_FILE, "climat", "needclear", "0");
			}
			tmp.ImpulseCount = 3200;
			string impulses = uci::Get(CONFIG_FILE, "climat", "impulses");
			if(impulses.size() > 0){
				tmp.ImpulseCount = atoi(impulses.c_str());
			}
			SetSettings(tmp);
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "TMkkClimatManager::ReloadConfig " + string( e.what() ) );
	}
}
//=============================================================================
Word TMkkClimatManager::SetStates(Word newState)
{
		Word retState = newState;
		try
		{
			if(isSqlDB)
			{
				vector<Mkk2> data = Repository::Mkk2Table.Select("", "DataIndex");
				if(data.size() == 4){
					for(auto curr: data)
					{
						retState ^=  (curr.Inverted << curr.DataIndex);//XOR
						bool currNewState = (retState & (1 << curr.DataIndex));
						if(currNewState != curr.ActiveState)
						{
							if(currNewState){//vkluchili
								curr.ActiveTime = TDateTime::Now();
							}
							else{
								tm now = TDateTime::Now();
								DWord diff 	= difftime( mktime(&now), mktime(&curr.ActiveTime));
								curr.CNT		+= diff;
								if(curr.RotateCNT == 0){
									curr.RotateCNT = 1;
								}else{
									curr.RotateCNT	+= diff;
								}
							}
							curr.ActiveState = currNewState;
							Repository::Mkk2Table.Update(curr);
						}
					}
				}
				//RotateStates(newState);
			}
		}
		catch(exception &e)
		{
			Log::ERROR( "TMkkClimatManager::SetStates " + string( e.what() ) );
		}
		return retState;
}
//=============================================================================
Word TMkkClimatManager::RotateStates(Word newState)
{
	Word retState = newState;
	try
	{
		if(isSqlDB)
		{
			for(int i=0; i < 2; i++)
			{
				vector<Mkk2> data = Repository::Mkk2Table.Select("DataType="+toString(i), "DataIndex DESC");
				if(data.size() == 2)
				{
					Mkk2 curr1 = data[0];
					Mkk2 curr2 = data[1];
					bool ActiveState1 = (newState & (1 << curr1.DataIndex));
					bool ActiveState2 = (newState & (1 << curr2.DataIndex));
					//Log::DEBUG("ActiveState1="+toString(ActiveState1) + " ActiveState2="+toString(ActiveState2));
					bool allDisable 	= !(ActiveState1 || ActiveState2);
					bool rotateEnable 	= !(ActiveState1 && ActiveState2);
					if(allDisable || !rotateEnable)
					{//vikluchaem
						if(curr1.Inverted){
							curr1.Inverted = false;
							curr1.RotateCNT = 0;
							Repository::Mkk2Table.Update(curr1);
						}
						if(curr2.Inverted){
							curr2.Inverted = false;
							curr2.RotateCNT = 0;
							Repository::Mkk2Table.Update(curr2);
						}
					}
					else
					{
						if(rotateEnable)
						{
							bool wasAllDisable = !curr1.ActiveState && !curr2.ActiveState;
							bool wasAllEnable  =  curr1.ActiveState &&  curr2.ActiveState;
							if(wasAllDisable || wasAllEnable)
							{
								//Log::DEBUG("!!!!!!!!!!!!!!!rotate1 == rotate2!!!!!!!!!!!!!");
								DWord RotatePorog = Settings().RotateTime;
								//if(curr1.CNT > curr2.CNT)
								if(curr1.CNT > curr2.CNT && (curr1.CNT-curr2.CNT)>=RotatePorog)
								{
									curr1.Inverted = true;
									curr2.Inverted = true;
								}
								else
								{
									curr1.Inverted = false;
									curr2.Inverted = false;
								}
								//curr1.RotateCNT = 0;
								//curr2.RotateCNT = 0;
								Repository::Mkk2Table.Update(curr1);
								Repository::Mkk2Table.Update(curr2);
							}
							else
							{
								DWord rotate1 = curr1.RotateCNT;
								DWord rotate2 = curr2.RotateCNT;
								tm now = TDateTime::Now();
								if(curr1.ActiveState){
									DWord diff = difftime( mktime(&now), mktime(&curr1.ActiveTime) );
									rotate1+=diff;
								}
								if(curr2.ActiveState){
									DWord diff = difftime( mktime(&now), mktime(&curr2.ActiveTime) );
									rotate2+=diff;
								}
								//Log::DEBUG("rotate1="+toString(rotate1) + " rotate2="+toString(rotate2));
								DWord RotatePorog = Settings().RotateTime;
								if(rotate1 > rotate2)//vikl 1 vkl 2
								{
									if(rotate1 >= RotatePorog*60)
									{
										curr1.Inverted = true;
										curr2.Inverted = true;
										curr1.RotateCNT = 0;
										curr2.RotateCNT = 0;
										Repository::Mkk2Table.Update(curr1);
										Repository::Mkk2Table.Update(curr2);
									}
								}
								else// if(rotate1 < rotate2)
								{
									if(rotate2 >= RotatePorog*60){
										curr1.Inverted = false;
										curr2.Inverted = false;
										curr1.RotateCNT = 0;
										curr2.RotateCNT = 0;
										Repository::Mkk2Table.Update(curr1);
										Repository::Mkk2Table.Update(curr2);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "TMkkClimatManager::ReloadConfig " + string( e.what() ) );
	}
	return retState;
}
//=============================================================================
string TMkkClimatManager::GetStringValue(void)
{
	string ReturnString = "";
	try
	{
		if(isSqlDB)
		{
			vector<Mkk2> data = Repository::Mkk2Table.Select("", "DataIndex DESC");
			if(data.size() == 4){
				for(auto curr: data){
					DWord cnt = 0;
					if(curr.ActiveState){
						tm now = TDateTime::Now();
						DWord diff = difftime( mktime(&now), mktime(&curr.ActiveTime));
						cnt = curr.CNT+diff;
						//Log::DEBUG("difftime="+toString(diff));
					}else{
						cnt = curr.CNT;
					}
					ReturnString+=">,91,"+toString(4-curr.DataIndex)+","+
							toString(cnt)+","+TimeToString(curr.ClearTime)+",";
				}
			}
		}
		if(ClimatPower.Enable){
			ReturnString+=ClimatPower.GetValueString();
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "TMkkClimatManager::GetStringValue " + string( e.what() ) );
	}
	return ReturnString;
}
//=============================================================================
void TMkkClimatManager::SetSettings(ClimatSettings val){
	pthread_mutex_lock(&sychObject);
	try
	{
		_settings = val;
	}
	catch(exception &e)
	{
		Log::ERROR( "SetSettings " + string( e.what() ) );
	}
	pthread_mutex_unlock(&sychObject);
}
//=============================================================================
void TMkkClimatManager::Stop(void)
{
	try
	{
		system("echo '' > /tmp/mkk.log");
		if(!Settings().Enable)
			return;

		if(isSqlDB)
		{
			vector<Mkk2> data = Repository::Mkk2Table.Select("", "DataIndex");
			if(data.size() == 4){
				for(auto curr: data)
				{
					if(curr.ActiveState){
						tm now = TDateTime::Now();
						DWord diff 	= difftime( mktime(&now), mktime(&curr.ActiveTime));
						curr.CNT		+= diff;

						curr.Inverted 		= false;
						curr.ActiveState 	= false;
						Repository::Mkk2Table.Update(curr);
					}
				}
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "TMkkClimatManager::Stop " + string( e.what() ) );
	}
}
//=============================================================================
void TMkkClimatManager::Clear(void)
{
	try
	{
		if(!Settings().Enable)
			return;

		if(isSqlDB)
		{
			Log::DEBUG("Clearing CNT");
			vector<Mkk2> data = Repository::Mkk2Table.Select("", "DataIndex");
			if(data.size() == 4){
				for(auto curr: data)
				{
					tm now = TDateTime::Now();
					curr.ActiveTime = now;
					curr.ClearTime = now;
					curr.CNT = 0;
					curr.Inverted = false;
					curr.RotateCNT = 0;
					Repository::Mkk2Table.Update(curr);
					Log::DEBUG("Clear " + curr.DataName);
				}
			}
		}
	}
	catch(exception &e)
	{
		Log::ERROR( "TMkkClimatManager::Clear " + string( e.what() ) );
	}
}
//=============================================================================
void TMkkClimatManager::ChangeState(Word newState)
{
	Log::DEBUG("TMkkClimatManager::ChangeState");
	uci::Set(CONFIG_FILE, "climat", "enable", toString(newState));
	//ReloadConfig();
}
//=============================================================================
void TMkkClimatManager::ChangeParameters(vector<string> parameters)
{
	Log::DEBUG("TMkkClimatManager::ChangeParameters");
	for(int i=0; i<parameters.size(); i++){
		string value = parameters[i];
		switch(i)
		{
		case 0:
			uci::Set(CONFIG_FILE, "climat", "cond1on", value);
			break;
		case 1:
			uci::Set(CONFIG_FILE, "climat", "cond1off", value);
			break;
		case 2:
			uci::Set(CONFIG_FILE, "climat", "cond2on", value);
			break;
		case 3:
			uci::Set(CONFIG_FILE, "climat", "cond2off", value);
			break;
		case 4:
			uci::Set(CONFIG_FILE, "climat", "heater1on", value);
			break;
		case 5:
			uci::Set(CONFIG_FILE, "climat", "heater1off", value);
			break;
		case 6:
			uci::Set(CONFIG_FILE, "climat", "heater2on", value);
			break;
		case 7:
			uci::Set(CONFIG_FILE, "climat", "heater2off", value);
			break;
		case 8:
			uci::Set(CONFIG_FILE, "climat", "rotate", value);
			break;
			//ci::Get(CONFIG_FILE, "climat", "rotate");
		};
	}
	//ReloadConfig();
}

/*
 * InputOutputManager.cpp
 *
 *  Created on: Apr 14, 2020
 *      Author: user
 */

#include <InputOutputManager.h>

///*****************************************************************************
//***
//*****************************************************************************
TIOManager::TIOManager( void ):TFastTimer(3,&MilSecCount),Period(600)
{

		SetTimer(PERIUD_TIMER, 10000);
		SetTimer(ADDITIONAL_TIMER, 5000);
		FirstInitFlg = true;
		ModifyTime = Period;
}
//=============================================================================
TIOManager::~TIOManager()
{
	try
	{

	}
	catch(exception &e)
	{
		Log::ERROR( e.what() );
	}
}
//=============================================================================
void TIOManager::Init( void *config, void *driver )
{
  vector<InputSettings> sett = *((vector<InputSettings> *)config);
  Log::DEBUG("Init TIOManager sett.size="+toString(sett.size()));

  for(auto curr: sett)
  {
	 TSimpleDevice *device = NULL;
	 bool needadd = false;
	 if(curr.enable)
	 {
		 device = find(curr.etype, curr.ename);
		 if(device == NULL){
			 device = new TSimpleDevice(curr.etype, curr.ename);
			 needadd = true;
		 }
		 else {
			 needadd = false;
		 }
		 IDatchik *dat = NULL;
		 if(curr.type == 0)//discretr proc inputs
		 {
			 dat = new TDiscreteDatchik(curr.ptype, curr.pname, curr.inversia);
			 dat->Register 	= curr.port;
			 dat->Index 	= 0;
		 }
		 else if(curr.type == 1)//analog proc inputs
		 {

		 }
		 if(dat != NULL)
		 {
			 dat->InitGPIO();
			 device->DatchikList.push_back(dat);
		 }
	 }

	 if(device)
	 {
		 Period  = curr.period;
		 if(needadd)
			 Devices.push_back(device);
		 //Log::DEBUG("\r\nInit TIOManager.device:\r\n type="+toString((int)curr.type)+" et="+toString((int)modul->Et)+" en="+toString((int)modul->En) + " commands="+modul->GetStringCfgValue());
	 }
  }
  for(auto curr: Devices)
  {
	  Log::DEBUG("Init TIOManager etype="+toString((int)curr->Et.Etype)+" ename="+toString(curr->Et.Ename) + " DatchikList.size="+ toString(curr->DatchikList.size()));
  }
  //Log::DEBUG("Init TInnerRs485Manager Controllers.size="+toString(Controllers.size())+" Period="+toString(Period)+" Aperiod="+toString(Aperiod));
}
//=============================================================================
bool TIOManager::Enable( void )
{
	return Devices.size() > 0;
}
//=============================================================================
void TIOManager::DetectStates( void  )
{
	if(! Enable()) return;

	  bool flag = false;
	  if( GetTimValue(PERIUD_TIMER) <= 0 )
	  {
		    DWord tim = SystemTime.GetGlobalSeconds();
		    Word period = Period;

		    Log::DEBUG("TIOManager::DetectStates FirstInitFlg="+toString(FirstInitFlg));
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
			  Log::DEBUG("TIOManager::DetectStates SetModify(true) Period="+toString(period) + " ModifyTime="+toString(ModifyTime)+" tim="+toString(tim) );
			  DateTime = SystemTime.GetTime();
			  for(auto curr: Devices)
				  curr->SetModify(true);

			  SetTimer( PERIUD_TIMER, 30000 );
	    	}
	    	else
	    	{
	    		SetTimer( PERIUD_TIMER, 500 );
	    	}
	  }
}
//=============================================================================
void TIOManager::CreateMessageCMD( void *Par )
{
if( Par != NULL && Enable() )
  {
    if(ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0)
    {
    	TSimpleDevice *CurrController = NULL;//new
    	int size = Devices.size();
    	string framStr = "";
		for( int index = 0; index < size; index++ )
		{
		   CurrController = Devices[index];
		   if( CurrController->GetModify() )
		   {
			  if(DateTime.Year == 0)
				  DateTime = SystemTime.GetTime();
			  string DateStr = TBuffer::DateTimeToString( &DateTime )+">>";
			  DateTime.Year  = 0;

			  string framStr = CurrController->GetValueString();
			  Log::DEBUG("TIOManager CreateMessageCMD framStr=["+framStr+"]");
			  if(framStr.size() > 0)
			  {
					 TFifo<string> *framFifo = (TFifo<string> *)Par;
					 framFifo->push( DateStr+framStr );
			  }
			  CurrController->SetModify(false);
		   }
		}
    }
  }
}
//=============================================================================
void TIOManager::DoWork( void *runFlg )
{
	bool stFlg = *((bool*)runFlg);
	if(stFlg)
	{
		for(auto currController: Devices)
		{
			//Log::DEBUG("DoWork TIOManager etype="+toString((int)currController->Et.Etype)+" ename="+toString(currController->Et.Ename) + " DatchikList.size="+ toString(currController->DatchikList.size()));
			for(auto cDatchik: currController->DatchikList)
			{
				string sendStr = "cat /sys/class/gpio/gpio" + toString(cDatchik->Register) + "/value";
				string valuStr = sendToConsole(sendStr);
				if(valuStr.size() > 0)
				{
					//Log::DEBUG("DoWork TIOManager port="+toString(cDatchik->Register) + " value="+valuStr);
					Word cval = atoi(valuStr.c_str());
					cDatchik->CalculateValue((Word*)&cval); // attention!! need cast to (Word*)!!!!!
					cDatchik->StateValue = cDatchik->DetectPorogs((Word*)&cval);
					if(cDatchik->StateValue != cDatchik->HStateValue)
					{
						cDatchik->HStateValue = cDatchik->StateValue;
						cDatchik->Modify = true;
						Log::DEBUG("TIOManager changeValue for port="+toString(cDatchik->Register) + " value="+valuStr);
					}
					if( !currController->IsEnable == false )
						currController->IsEnable = true;
				}
			}
		}
	}
}
//=============================================================================





/*
 * Climat.h
 *
 *  Created on: May 15, 2015
 *      Author: user
 */

#ifndef CLIMAT_H_
#define CLIMAT_H_

#include <Log.h>
#include <Classes.h>
#include <DAL.h>

using namespace std;
using namespace log4net;
using namespace MyClassTamlates;
using namespace datalayer;
//=============================================================================
class TSimpleClimatPower:public IParams
{
	public:
		TSimpleClimatPower(Byte Pt, Byte Pn):IParams(Pt,Pn)
		{
			Params.push_back(&PowerValue);
		}

		~TSimpleClimatPower(){}

		TMamStruct PowerValue;
};
//=============================================================================
class TClimatPower
{
	public:
		TClimatPower()
		{
			for(int i=0; i < 4; i++){
				Params.push_back(new TSimpleClimatPower(44, i+1));
			}
			Enable=false;
		}

		~TClimatPower()
		{
			for(int i=0; i < 4; i++){
				delete Params[i];
			}
		}

	    string GetValueString(void)
	    {
	    	string ReturnStr = "";
	        for( int i=0; i < Params.size(); i++){
	        	ReturnStr+=Params[i]->GetValueString();
	        }
	        return ReturnStr;
	    }

		bool Enable;
		vector<TSimpleClimatPower*> Params;
};
//=============================================================================
class IClimat: public TFastTimer
{
public:
	IClimat();
	virtual ~IClimat();

	virtual  void CLIMAT_DetectStates( void  )
	{
	  static int cnt = 0;
	  bool flag = false;

	    if( GetTimValue(PERIUD_TIMER) <= 0 )
	    {
	    	DWord tim = SystemTime.GetGlobalSeconds();
		    if(FirstInitFlg.GetData() == true)
		    {
		    		Log::DEBUG("CLIMAT_DetectStates FirstInitFlg=true");
		    		FirstInitFlg.SetData(false);
		    		flag     = true;
		    		ModifyTime = ((DWord)( (DWord)(tim / Period.GetData()) + 1)) * Period.GetData();
		    		if(ModifyTime > MAX_SECOND_IN_DAY)
		    			ModifyTime = Period.GetData();
		    }
		    else
		    {
		    		long err = ModifyTime-tim;
		    		//Log::DEBUG("CLIMAT_DetectStates err="+toString(err)+" ModifyTime="+toString(ModifyTime)+" tim="+toString(tim));
		    		if( (ModifyTime <= tim) || (err > Period.GetData()*2) )
		    		{
		    			flag     = true;
		    			ModifyTime = ((DWord)((DWord)(tim / Period.GetData()) + 1)) * Period.GetData();
		    			if(ModifyTime > MAX_SECOND_IN_DAY)
		    				ModifyTime = Period.GetData();
		    		}
		    }

	    	if(NeedModify.GetData() == true)
	    	{
	    		if(cnt++ > 30)
	    		{
	    			cnt = 0;
	    			NeedModify.SetData(false);
	    			flag = true;
	    		}
	    	}

			if( flag == true  )
			{
			      DateTime = SystemTime.GetTime();
			      Modify = true;//new
				  SetTimer( PERIUD_TIMER, 10000 );
			}
			else
			{
				  SetTimer( PERIUD_TIMER, 500 );
			}


	    	/*
	      if(FirstInitFlg.GetData() == true)
	      {
	        FirstInitFlg.SetData(false);
	        flag     = true;
	      }
	      if( SystemTime.GetGlobalSeconds()%Period.GetData()  == 0)
	      {
	        flag     = true;
	        SetTimer( PERIUD_TIMER, 2000 );
	      }
	      else
	      { SetTimer( PERIUD_TIMER, 50 );}//attention need 50, not set 1000!!

	    	if(NeedModify.GetData() == true)
	    	{
	    		if(cnt++ > 30)
	    		{
	    			cnt = 0;
	    			NeedModify.SetData(false);
	    			flag = true;
	    		}
	    	}

	    }

	    if( flag == true )
	    {
	      DateTime = SystemTime.GetTime();
	      Modify = true;//new
	      //Log::DEBUG("TFreeCoolingManager reglament time");
	    }*/

	    }
	}

	virtual void CLIMAT_CreateMessageCMD( void *Par )
	{

	if( Par != NULL )
	  {
	    if(ObjectFunctionsTimer.GetTimValue(USB_RS485_ACTIV_WAIT_TIMER) <= 0)
	    {
	      string framStr = "";
	      if( Modify == true )//new
	      {
	        framStr += TBuffer::DateTimeToString( &DateTime )+">>";
	        framStr += GetStringValue();
	        if(framStr.size() > 0)
	        {
	           TFifo<string> *framFifo = (TFifo<string> *)Par;
	           framFifo->push( framStr );
	        }
	        framStr.clear();
	        Modify = false;
	      }
	    }
	  }
	}

	virtual string GetStringValue(void) = 0;

	sync_type<bool>   	FirstInitFlg;

	sync_type<bool>     NeedModify;

	sync_type<Word>		Period;

private:

    DWord       ModifyTime;
	TRtcTime    DateTime;
	bool        Modify;
};

//=============================================================================
class TMkkClimatManager:public IClimat
{
public:
	TMkkClimatManager();

	bool InitDB();

	Word GetLastState(int lastState);

	void ReloadConfig(void);

	Word SetStates(Word newState);

	Word RotateStates(Word newState);

	string GetStringValue(void);

	ClimatSettings Settings(void)
	{
		ClimatSettings ret;
		pthread_mutex_lock(&sychObject);
		try
		{
			ret = _settings;
		}
		catch(exception &e)
		{
			Log::ERROR( "Settings " + string( e.what() ) );
		}
		pthread_mutex_unlock(&sychObject);
		return ret;
	}

	bool GetDBstate(void){
		return isSqlDB;
	}

	void Stop(void);

    void ChangeState(Word newState);

    void ChangeParameters(vector<string> parameters);

    void SetSettings(ClimatSettings val);

	sWord LastState;

	Word LastErrorState;

	TClimatPower ClimatPower;

private:

	void Clear(void);

	pthread_mutex_t sychObject;

	ClimatSettings _settings;

	bool isSqlDB;
};


//=============================================================================
//extern TFreeCoolingManager  FreeCoolingManager;
//=============================================================================
class TExternClimatManager:public IClimat
{
public:
	TExternClimatManager()
	{
		State.SetData(0);
		//HState.SetData(0);
		MainTempParameter.SetData(0);
		FirstPointsStepStart.SetData(0);
		FirstPointsStepEnd.SetData(0);
		SecondPointsStepStart.SetData(0);
		SecondPointsStepEnd.SetData(0);
	}

	string GetStringValue(void)
	{
		string ReturnString = "";
		ReturnString += toString(Configuration.ClimatManagerSettings.Etype)+","+toString(Configuration.ClimatManagerSettings.Ename)+",";

		TMamStruct roomTemp = RoomTemp.GetData();
		ReturnString += ">,9,5,";
		ReturnString +=	FloatToString(roomTemp.Aver,0,2)+','+
							FloatToString(roomTemp.Min,0,2)+','+
								FloatToString(roomTemp.Max, 0, 2)+',';
		roomTemp.ClrDivider();
		RoomTemp.SetData(roomTemp);

		TMamStruct extTemp = ExtTemp.GetData();
		ReturnString += ">,9,6,";
		ReturnString +=	FloatToString(extTemp.Aver,0,2)+','+
							FloatToString(extTemp.Min,0,2)+','+
								FloatToString(extTemp.Max, 0, 2)+',';
		extTemp.ClrDivider();
		ExtTemp.SetData(extTemp);

		ReturnString+= ">,99,1,"+toString(Configuration.ClimatManagerSettings.Enable)+","+
							FloatToString(Cond1OnTemp.GetData())+","+
							FloatToString(Cond1OffTemp.GetData())+","+
							FloatToString(Cond2OnTemp.GetData())+","+
							FloatToString(Cond2OffTemp.GetData())+","+
							FloatToString(Heater1OnTemp.GetData())+","+
							FloatToString(Heater1OffTemp.GetData())+","+
							FloatToString(Heater2OnTemp.GetData())+","+
							FloatToString(Heater2OffTemp.GetData())+",";
		ReturnString += ">,100,1,"+ toString(Configuration.ClimatManagerSettings.IsAuto)+",";

		TMamStruct sf1 = Sf1.GetData();
		ReturnString += ">,140,1,";
		ReturnString +=	FloatToString(sf1.Aver,0,2)+','+
							FloatToString(sf1.Min,0,2)+','+
								FloatToString(sf1.Max, 0, 2)+',';
		sf1.ClrDivider();
		Sf1.SetData(sf1);

		TMamStruct sp1 = Sp1.GetData();
		ReturnString += ">,141,1,";
		ReturnString +=	FloatToString(sp1.Aver,0,2)+','+
							FloatToString(sp1.Min,0,2)+','+
								FloatToString(sp1.Max, 0, 2)+',';
		sp1.ClrDivider();
		Sp1.SetData(sp1);

		ReturnString += ">,201,1,"+ WordToString(State.GetData())+',';
		ReturnString+= ">,230,1,"+
							FloatToString(FirstPointsStepStart.GetData())+","+
							FloatToString(FirstPointsStepEnd.GetData())+","+
							FloatToString(SecondPointsStepStart.GetData())+","+
							FloatToString(SecondPointsStepEnd.GetData())+",";
		ReturnString += ">,231,1,"+FloatToString(MainTempParameter.GetData())+",";
		//ReturnString += PointsSteps.GetValueString();
		//ReturnString += SetPoint.GetValueString();
	    //ReturnString += "\r\n>,49,1,"+SerialNo+",";
	    //ReturnString += WordToString(IsEnable)+',';
		return ReturnString;
	}
	sync_type<TMamStruct>   RoomTemp;
	sync_type<TMamStruct> 	ExtTemp;
	sync_type<TMamStruct> 	Sf1;
	sync_type<TMamStruct> 	Sp1;

	sync_type<Byte> 		State;
	sync_type<float> MainTempParameter;
	sync_type<float> FirstPointsStepStart;
	sync_type<float> FirstPointsStepEnd;
	sync_type<float> SecondPointsStepStart;
	sync_type<float> SecondPointsStepEnd;
	sync_type<float> Cond1OnTemp;
	sync_type<float> Cond1OffTemp;
	sync_type<float> Cond2OnTemp;
	sync_type<float> Cond2OffTemp;
	sync_type<float> Heater1OnTemp;
	sync_type<float> Heater1OffTemp;
	sync_type<float> Heater2OnTemp;
	sync_type<float> Heater2OffTemp;
	sync_type<Word>  ClimatStateData;

    //TIbpAlarms 	Alarms;
    //TIbpAlarms 	Warnings;

    string 		SerialNo;
};




#endif /* CLIMAT_H_ */

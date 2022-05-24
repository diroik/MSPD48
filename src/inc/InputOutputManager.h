/*
 * InputOutputManager.h
 *
 *  Created on: Apr 14, 2020
 *      Author: user
 */

#ifndef INPUTOUTPUTMANAGER_H_
#define INPUTOUTPUTMANAGER_H_

#include <Classes.h>
#include <stdio.h>

using namespace std;
using namespace log4net;
using namespace MyClassTamlates;

//=============================================================================
class TIOManager:public TFastTimer, public IManager
{
public:
	TIOManager();
	~TIOManager();

	void 	Init( void *config, void *driver = NULL );

    void 	DetectStates(void);
    void 	CreateMessageCMD(void *Par = NULL);

    void 	DoWork( void *runFlg );

    bool 	Enable( void );
	bool    FirstInitFlg;

    string GetSelfTestData(int et, int en)
    {
    	 string ret = "";
    	 for(auto cm: Devices)
    	 {
    			  if(cm->Et.Etype == et && cm->Et.Ename==en)
    			  {
    				  ret += "{";
    				  ret += "\"Et\":"+toString((int)cm->Et.Etype)+",";
    				  ret += "\"En\":"+toString((int)cm->Et.Ename)+",";
    				  ret += "\"Address\":"+toString((int)cm->Address)+",";
    				  ret += "\"Number\":\""+cm->SerialNumber+"\""+",";
    				  ret += "\"IsEnable\":"+toString(cm->IsEnable)+",";
    				  ret += "\"AckCnt\":"+toString(0)+",";
    				  ret += "\"ErrCnt\":"+toString(0)+",";
    				  ret += "\"DatchikList\":[";

    				  for(auto dat: cm->DatchikList){
    					  ret+= dat->GetSimpleValueString();
    				  }

					  if(ret.c_str()[ret.size()-1]==',')
						ret=ret.substr(0,ret.size()-1);
    				  ret += "]";
    				  ret+="},";
    			  }
    	 }
    	 return ret;
    }

//protected:
	vector<TSimpleDevice*> 	Devices;

private:

	TSimpleDevice*			find(int et, int en)
	{
		TSimpleDevice* ret = NULL;
		for(auto curr: Devices)
		{
			if(curr->Et.Etype == et && curr->Et.Ename == en)
			{
				ret = curr;
				break;
			}
		}
		return ret;
	}


	TRtcTime    DateTime;
	DWord   	ModifyTime;
	Word 		Period;
};
//=============================================================================
#endif /* INPUTOUTPUTMANAGER_H_ */

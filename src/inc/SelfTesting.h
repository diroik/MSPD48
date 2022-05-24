/*
 * SelfTesting.h
 *
 *  Created on: Apr 19, 2017
 *      Author: user
 */

#ifndef SELFTESTING_H_
#define SELFTESTING_H_

#include <Classes.h>
#include <SocketPort.h>

using namespace std;
using namespace log4net;
using namespace MyClassTamlates;
//=============================================================================
class TSelfTestingManager
{
	public:
		TSelfTestingManager();
		~TSelfTestingManager();

		void Init( void *config );

		void DoWork( void *runFlg=NULL );
		void Stop( void );


		vector<sync_deque<TCmdMessage*>*> AltMessList;

	private:
		Byte 					Buffer[2048];
		IListener 				*listener;
		vector<IManager*>  		 managers;
		vector<IManager*>  		 monmanagers;
};


#endif /* SELFTESTING_H_ */

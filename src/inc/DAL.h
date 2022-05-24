/*
 * DAL.h
 *
 *  Created on: Aug 1, 2016
 *      Author: user
 */

//#include <sqlite3.h>
#include <string>
#include <sys/time.h>
#include <vector>
#include <list>
#include <pthread.h>

#include <TypeDefine.h>
#include <convert.h>
#include <Log.h>

#ifndef DAL_H__
#define DAL_H__

#include <Classes.h>

using namespace std;
using namespace log4net;

namespace datalayer {

class Mkk2
{
public:

	Mkk2(){
		ID 				= 0;
		DataName		= "";
		DataType 		= 0;
		DataIndex		= 0;
		CNT 			= 0;
		RotateCNT		= 0;
		ActiveState 	= 0;
		Inverted 		= 0;
		ClearTime 	= TDateTime::Now();
		ActiveTime 	= TDateTime::Now();
	}

	Mkk2(DWord id, string n, Byte t, Word in, DWord rcnt, DWord cnt, bool as, bool inv, tm ct, tm at){
		ID 				= id;
		DataName		= n;
		DataType 		= t;
		DataIndex		= in;
		RotateCNT		= rcnt;
		CNT 			= cnt;
		ActiveState 	= as;
		Inverted 		= inv;
		ClearTime 	= ct;
		ActiveTime 	= at;
	}

	DWord 	ID;
	string 	DataName;
	Word    DataType;
	Word 	DataIndex;
	DWord   RotateCNT;
	DWord 	CNT;
	bool    ActiveState;
	bool    Inverted;
	tm 		ClearTime;
	tm      ActiveTime;
};

template <class elemType>
class IRepositoryBase{
	public:

		virtual ~IRepositoryBase(){
			/*
			if(context != NULL) {
				close();
			}*/
			//Log::INFO("[IRepositoryBase] Destructor");
		}

		virtual void Init(string h, unsigned int p, pthread_mutex_t *s) = 0;

		virtual void Insert(elemType &item) = 0;

		virtual void Update(elemType &item) = 0;

		virtual void Delete(unsigned long id) = 0;

		virtual vector<elemType> Select(string expression, string order="") = 0;


		virtual bool InitTable() = 0;

	protected:
		//sqlite3		*context = NULL;
		char 		*err = 0;

		unsigned 	int port;
		string 		host;
		string 		file_name;

		string 		table_name;
		string 		dbs_name;


		pthread_mutex_t *sych_obj;

		string getDBS(string name){
			string ret = "";
			string query = "SHOW DATABASES LIKE '"+name+"'";
			simple_connect();
			//Log::DEBUG("GetDBS");
			/*
			if(mysql_ping(context) == 0){
				MYSQL_RES *result;
				if(mysql_query(context, query.c_str()) == 0){
					result = mysql_store_result(context);
					if(result){
						unsigned int num_rows =  mysql_num_rows(result);
						if(num_rows >= 1){
							MYSQL_ROW row = mysql_fetch_row(result);
							if(row != NULL)
								ret += toString(row[0]);
						}
						mysql_free_result(result);
					}

				}else{
					Log::ERROR("[IRepositoryBase] Faled to GetDBS: "+toString(mysql_error(context)));
				}
			}
			*/

			close();
			return ret;
		}

		bool createDBS(string name){
			//mysql_install_db();
			bool ret = false;
			string query = "CREATE DATABASE "+name;
			simple_connect();
			//Log::DEBUG("CreateDBS");
			char *err = 0;
			/*
			if( sqlite3_exec(context, query.c_str(), NULL, NULL, &err ) == SQLITE_OK)
			{
				ret = true;
			}else{
			    sqlite3_free(err);
			    sqlite3_close(context);
				Log::ERROR("[IRepositoryBase] Faled to CreateDBS: "+toString(err));
			}*/

			close();
			return ret;
		}

		void simple_connect(){
			//pthread_mutex_lock(&sych);

			/*
			if(context != NULL) {
				close();
			}*/
			/*
			context = mysql_init(NULL);
			if(context != NULL)
			{
				//if(!mysql_real_connect(context, host.c_str(), "root", NULL, "mspd48", port, NULL, 0)){
				if(!mysql_real_connect(context, host.c_str(), "root", NULL, NULL, port, NULL, 0)){
					Log::ERROR("[SQLmspd48Repository] "+ toString(mysql_error(context)));
				}
				else{
					//Log::INFO("[SQLmspd48Repository] DB CONNECTED! Host="+host+" Port="+toString(port));
				}
			}
			else{
				Log::ERROR("[SQLmspd48Repository] Cant create MySql-descriptor!");
			}
			*/

			//pthread_mutex_unlock(&sych);
		}

		void connect(){
			//pthread_mutex_lock(&sych);

			/*
			if(context != NULL) {
				close();
			}*/
			/*
			context = mysql_init(NULL);
			if(context != NULL)
			{
				if(!mysql_real_connect(context, host.c_str(), "root", NULL, dbs_name.c_str(), port, NULL, 0)){
				//if(!mysql_real_connect(context, host.c_str(), "root", NULL, NULL, port, NULL, 0)){
					Log::ERROR("[SQLmspd48Repository] "+ toString(mysql_error(context)));
				}
				else{
					//Log::INFO("[SQLmspd48Repository] DB CONNECTED! Host="+host+" Port="+toString(port));
				}
			}
			else{
				Log::ERROR("[SQLmspd48Repository] Cant create MySql-descriptor!");
			}
			 	*/
			//pthread_mutex_unlock(&sych);
		}

		void close(){
			//pthread_mutex_lock(&sych);
			/*
			if(context != NULL){
				sqlite3_close(context);
				context = NULL;
				//Log::INFO("[SQLmspd48Repository] DB CLOSED!");

				//pthread_mutex_unlock(&sych);
			}*/
		}

};

class SQL_Mkk2Repository:public IRepositoryBase<Mkk2>{
 public:

	SQL_Mkk2Repository(void *con);

	SQL_Mkk2Repository(string h, unsigned int p);

	SQL_Mkk2Repository();

	~SQL_Mkk2Repository();

	void Init(string h, unsigned int p, pthread_mutex_t *s);

	bool InitTable();

	void Insert(Mkk2 &item);

	void Update(Mkk2 &item);

	void Delete(unsigned long id);

	vector<Mkk2> Select(string expression, string order="");

	vector<Mkk2> SelectTop(int top, string expression, string order);

	Mkk2* FirstOrDefault(string expression, string order="");

 private:

	bool tableExist();

	bool createTable();

	void fillTable();

};

class Repository
{
	public:

		static void Init(string h, unsigned int p);

		static SQL_Mkk2Repository 	Mkk2Table;

		static pthread_mutex_t sych_obj;

	private:

};

}

#endif /* DAL_H_ */

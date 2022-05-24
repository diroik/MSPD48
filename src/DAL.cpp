/*
 * DAL.cpp
 *
 *  Created on: Aug 1, 2016
 *      Author: user
 */
#include <DAL.h>

namespace datalayer {

	// ***********************************************
	// class SQL_Mkk2Repository
	// ===============================================
	//public
	SQL_Mkk2Repository::SQL_Mkk2Repository(void *con){

		//context = con;
		host = "127.0.0.1";
		port = 0;
		table_name = "mkk2";
		dbs_name = "mspd48";

	}

	SQL_Mkk2Repository::SQL_Mkk2Repository(string h, unsigned int p){
		//context = NULL;
		host = h;
		port = p;
		table_name = "mkk2";
		dbs_name = "mspd48";
	}

	SQL_Mkk2Repository::SQL_Mkk2Repository(){
		//context = NULL;
		host = "127.0.0.1";
		port = 0;
		table_name = "mkk2";
		dbs_name = "mspd48";
	}

	SQL_Mkk2Repository::~SQL_Mkk2Repository(){
		//Log::INFO("[SQLmspd48Repository] Destructor");
	}

	void SQL_Mkk2Repository::Init(string h, unsigned int p, pthread_mutex_t *s){
		//context = NULL;
		host = h;
		port = p;
		sych_obj = s;
		if(port == 0){
			file_name = host + "mspd48.db";
		}

		Log::INFO("[SQLmspd48Repository] Init complited!");
	}

	bool SQL_Mkk2Repository::InitTable()
	{
		bool ret = false;
		// открываем соединение
		/*
		if( sqlite3_open(file_name.c_str(), &context) ){
			Log::INFO( "[SQL_Mkk2Repository] Ошибка открытия/создания БД!"+ toString(sqlite3_errmsg(context)) );
			return false;
		}*/

		if(!tableExist()){
			if(createTable()){
				fillTable();
				ret = true;
			}
		}
		else{
			ret = true;
		}
		return ret;
	}

	void SQL_Mkk2Repository::Insert(Mkk2 &item){

		string clear  ="'"+TimeToString(item.ClearTime)+"'";
		string active ="'"+TimeToString(item.ActiveTime)+"'";

		string query = "INSERT INTO mkk2 VALUES(NULL,'"
												+item.DataName+"', "
												+toString((Word)item.DataType)+", "
												+toString(item.DataIndex)+", "
												+toString(item.RotateCNT)+", "
												+toString(item.CNT)+", "
												+toString((Word)item.ActiveState)+", "
												+toString((Word)item.Inverted)+", "
												+clear+", "+active+")";
		connect();
/*
		if(sqlite3_exec(context, query.c_str(), NULL, NULL, &err)!= SQLITE_OK){
			Log::ERROR("[SQLmspd48Repository] Faled sqlite3_exec: "+toString(err));
			sqlite3_free(err);
		}
		else{
			Log::INFO("[SQLmspd48Repository] query=["+query+"]");
		}
		*/

		close();
	}

	void SQL_Mkk2Repository::Update(Mkk2 &item){
		string clear  ="'"+TimeToString(item.ClearTime)+"'";
		string active ="'"+TimeToString(item.ActiveTime)+"'";

		string query = "UPDATE mkk2 SET DataName='"+item.DataName+"', DataType="
												+toString((Word)item.DataType)+", DataIndex="
												+toString(item.DataIndex)+", RotateCNT="
												+toString(item.RotateCNT)+", CNT="
												+toString(item.CNT)+", ActiveState="
												+toString((Word)item.ActiveState)+", Inverted="
												+toString((Word)item.Inverted)+
												", ClearTime="+clear+
												", ActiveTime="+active+" WHERE ID="+toString(item.ID);
		connect();

		/*
		sqlite3_exec(context, query.c_str(), NULL, NULL, &err );
		if(sqlite3_exec(context, query.c_str(), NULL, NULL, &err)!= SQLITE_OK){
			Log::ERROR("[SQLmspd48Repository] Faled sqlite3_exec: "+toString(err));
			sqlite3_free(err);
		}
		else{
			//Log::DEBUG("[SQLmspd48Repository] query=["+query+"]");
		}*/
		close();
	}

	void SQL_Mkk2Repository::Delete(unsigned long id){

		string query = "DELETE FROM mkk2 WHERE ID="+toString(id);
		connect();
		/*
		if(sqlite3_exec(context, query.c_str(), NULL, NULL, &err)!= SQLITE_OK){
			Log::ERROR("[SQLmspd48Repository] Faled sqlite3_exec: "+toString(err));
			sqlite3_free(err);
		}
		else{}
		 	 */
		close();
	}

	vector<Mkk2> SQL_Mkk2Repository::Select(string expression, string order){
		vector<Mkk2> ret;
		//string query = "SELECT * FROM "+table_name+ " WHERE "+expression;
		string query = "SELECT * FROM "+table_name;//+ " WHERE "+expression;
		if(expression.length() > 0){
			query+=" WHERE "+expression;
		}
		if(order.length() > 0){
			query +=" ORDER BY "+order;
		}
		//Log::DEBUG("[SQL_Mkk2Repository] query="+query);

		connect();
/*
			MYSQL_RES *result;
			if(mysql_query(context, query.c_str()) == 0){
				result = mysql_store_result(context);
				if(result){
					unsigned int num_rows =  mysql_num_rows(result);
					//Log::DEBUG("[SQL_Mkk2Repository] num_rows="+toString(num_rows));
					if(num_rows > 0){
						unsigned int num_fields =  mysql_num_fields(result);
						//Log::DEBUG("[SQL_Mkk2Repository] num_fields="+toString(num_fields));
						MYSQL_ROW row;
						while( (row = mysql_fetch_row(result)) )
						{
							Mkk2 tmp;
							tmp.ID 			= fromString<DWord>(row[0]);
							tmp.DataName	= row[1];
							tmp.DataType	= fromString<Word>(row[2]);
							tmp.DataIndex	= fromString<Word>(row[3]);
							tmp.RotateCNT	= fromString<DWord>(row[4]);
							tmp.CNT			= fromString<DWord>(row[5]);
							tmp.ActiveState	= fromString<bool>(row[6]);
							tmp.Inverted	= fromString<bool>(row[7]);
							tmp.ClearTime   = StringToTime(row[8]);
							tmp.ActiveTime  = StringToTime(row[9]);

							ret.push_back(tmp);
						}
						//Log::DEBUG("[SQL_Mkk2Repository] ret.size="+toString(ret.size()));
					}
					mysql_free_result(result);
				}
			}
			else{
				Log::ERROR("[SQL_Mkk2Repository] Faled to tableExist: "+toString(mysql_error(context)));
			}
*/
		close();
		return ret;
	}

	vector<Mkk2> SQL_Mkk2Repository::SelectTop(int top, string expression, string order){
		vector<Mkk2> ret;
		//string query = "SELECT * FROM "+table_name+ " WHERE "+expression;
		string query = "SELECT * FROM "+table_name;//+ " WHERE "+expression;
		if(expression.length() > 0){
			query+=" WHERE "+expression;
		}
		if(order.length() > 0){
			query +=" ORDER BY "+order;
		}
		query+=" LIMIT "+toString(top);

		//Log::DEBUG("[SQL_Mkk2Repository] query="+query);

		connect();
/*
			MYSQL_RES *result;
			if(mysql_query(context, query.c_str()) == 0){
				result = mysql_store_result(context);
				if(result){
					unsigned int num_rows =  mysql_num_rows(result);
					//Log::DEBUG("[SQL_Mkk2Repository] num_rows="+toString(num_rows));
					if(num_rows > 0){
						unsigned int num_fields =  mysql_num_fields(result);
						//Log::DEBUG("[SQL_Mkk2Repository] num_fields="+toString(num_fields));
						MYSQL_ROW row;
						while( (row = mysql_fetch_row(result)) )
						{
							Mkk2 tmp;
							tmp.ID 			= fromString<DWord>(row[0]);
							tmp.DataName 	= row[1];
							tmp.DataType	= fromString<Word>(row[2]);
							tmp.DataIndex	= fromString<Word>(row[3]);
							tmp.RotateCNT	= fromString<DWord>(row[4]);
							tmp.CNT			= fromString<DWord>(row[5]);
							tmp.ActiveState	= fromString<bool>(row[6]);
							tmp.Inverted	= fromString<bool>(row[7]);
							tmp.ClearTime	= StringToTime(row[8]);
							tmp.ActiveTime  = StringToTime(row[9]);

							ret.push_back(tmp);
						}
						//Log::DEBUG("[SQL_Mkk2Repository] ret.size="+toString(ret.size()));
					}
					mysql_free_result(result);
				}
			}
			else{
				Log::ERROR("[SQL_Mkk2Repository] Faled to tableExist: "+toString(mysql_error(context)));
			}
*/
		close();
		return ret;
	}

	Mkk2* SQL_Mkk2Repository::FirstOrDefault(string expression, string order){
		Mkk2* ret = NULL;
		string query = "SELECT * FROM "+table_name+ " WHERE "+expression;
		if(order.length() > 0){
			query +=" ORDER BY "+order;
		}
		query+=" LIMIT 1";
		//Log::DEBUG("[SQL_Mkk2Repository] query="+query);

		connect();
		/*
			MYSQL_RES *result;
			if(mysql_query(context, query.c_str()) == 0){
				result = mysql_store_result(context);
				if(result){
					unsigned int num_rows =  mysql_num_rows(result);
					if(num_rows > 0){
						unsigned int num_fields =  mysql_num_fields(result);
						MYSQL_ROW row;
						row = mysql_fetch_row(result);
						if(row != NULL)
						{
							ret = new Mkk2();
							ret->ID 			= fromString<DWord>(row[0]);
							ret->DataName 		= row[1];
							ret->DataType		= fromString<Word>(row[2]);
							ret->DataIndex		= fromString<Word>(row[3]);
							ret->RotateCNT		= fromString<DWord>(row[4]);
							ret->CNT			= fromString<DWord>(row[5]);
							ret->ActiveState	= fromString<bool>(row[6]);
							ret->Inverted		= fromString<bool>(row[7]);
							ret->ClearTime  	= StringToTime(row[8]);
							ret->ActiveTime 	= StringToTime(row[9]);
						}
					}
					mysql_free_result(result);
				}
			}
			else{
				Log::ERROR("[SQL_Mkk2Repository] Faled to tableExist: "+toString(mysql_error(context)));
			}
*/
		close();
		return ret;
	}

	//private
	bool SQL_Mkk2Repository::tableExist()
	{
		string t_name = "";
		string query = "SHOW TABLES LIKE '"+table_name+"'";

		connect();
		/*

			MYSQL_RES *result;
			if(mysql_query(context, query.c_str()) == 0){
				result = mysql_store_result(context);
				if(result){
					unsigned int num_rows =  mysql_num_rows(result);
					if(num_rows >= 1){
						MYSQL_ROW row = mysql_fetch_row(result);
						if(row != NULL)
							t_name = toString(row[0]);
					}
					mysql_free_result(result);
				}
			}
			else{
				Log::ERROR("[SQL_Mkk2Repository] Faled to tableExist: "+toString(mysql_error(context)));
			}
		 */
		close();

		//Log::INFO("[SQL_Mkk2Repository] t_name="+t_name+" table_name="+table_name);

		return table_name == t_name;
	}

	bool SQL_Mkk2Repository::createTable(){
		if(table_name.length() == 0) return false;
		bool ret = false;
		string query = "CREATE TABLE IF NOT EXISTS "+table_name+" ("+
				"ID bigint unsigned NOT NULL auto_increment,"+
				"DataName varchar(250) NOT NULL default '',"+
				"DataType smallint unsigned NOT NULL default 0,"+
				"DataIndex smallint unsigned NOT NULL default 0,"+
				"RotateCNT bigint unsigned NOT NULL default 0,"+
				"CNT bigint unsigned NOT NULL default 0,"+
				"ActiveState tinyint unsigned NOT NULL default 0,"+
				"Inverted tinyint unsigned NOT NULL default 0,"+
				"ClearTime TIMESTAMP,"+
				"ActiveTime TIMESTAMP,"+
				"PRIMARY KEY  (ID)"+
				")";

		connect();
		//Log::INFO("createTable");
/*
		if(sqlite3_exec(context, query.c_str(), NULL, NULL, &err ) == SQLITE_OK){
			ret = true;
		}
		else{
			Log::ERROR("[SQL_Mkk2Repository] Faled to createTable: "+toString(err));
			sqlite3_free(err);
		}*/

		close();
		return ret;
	}

	void SQL_Mkk2Repository::fillTable(){
		//Mkk2(DWord id, string name, Byte type, Word index, DWord cnt, bool astate, bool invert, tm cleart, tm activet)
		Mkk2 cond1(1, "cond1", 0, 3, 0, 0, 0, 0, TDateTime::Now(), TDateTime::Now());
		Mkk2 cond2(2, "cond2", 0, 2, 0, 0, 0, 0, TDateTime::Now(), TDateTime::Now());
		Mkk2 heat1(3, "heat1", 1, 1, 0, 0, 0, 0, TDateTime::Now(), TDateTime::Now());
		Mkk2 heat2(4, "heat2", 1, 0, 0, 0, 0, 0, TDateTime::Now(), TDateTime::Now());
		vector<Mkk2> vec {cond1, cond2, heat1, heat2};

		for(auto curr: vec){
			string expression = "DataType="+toString(curr.DataType)+" and DataIndex="+toString(curr.DataIndex);
			string order="DataIndex";
			Mkk2 *tmp =  FirstOrDefault(expression, order);
			if(tmp!=NULL){
				delete tmp;
			}
			else{
				Insert(curr);
			}
		}
	}
	// ===============================================

	// ***********************************************
	// class Repository
	// ===============================================
	//public
	void Repository::Init(string h, unsigned int p){
		Mkk2Table.Init(h, p, &sych_obj);
	}

	pthread_mutex_t 	Repository::sych_obj;

	SQL_Mkk2Repository 	Repository::Mkk2Table;
	// ===============================================



}


/*
 * netsnmp.h
 *
 *  Created on: Jun 1, 2017
 *      Author: user
 */

#ifndef NETSNMP_H_
#define NETSNMP_H_

#include <TypeDefine.h>
#include <convert.h>
#include <Log.h>

using namespace log4net;

class OidNumber
{
	public:

		OidNumber(Short d)
		{
			Value = d;
		}

		vector<Byte> GetValue()
		{
			vector<Byte> ret;
			Bshort tmp;
			Bshort tmp2;

			tmp.Data_b[0] = Value & 0x7F;
			tmp.Data_b[1] = Value >> 7;

			if(tmp.Data_b[1] > 0){
				tmp2.Data_s = tmp.Data_b[1] + 0x80;
				if(tmp2.Data_s > 255)
				{
					ret.push_back(tmp2.Data_b[1] + 0x80);
					ret.push_back(tmp2.Data_b[0] + 0x80);
				}
				else{
					ret.push_back(tmp.Data_b[1] + 0x80);
				}
			}
			/*
			if(tmp.Data_b[1] > 0){
				ret.push_back(tmp.Data_b[1] + 0x80);
			}*/

			ret.push_back(tmp.Data_b[0]);
			return ret;
		}

		string ToString(){
			return Value == 0x2B ? "1.3" : toString(Value);
		}

	private:

		Short Value;
};

class Oid
{
	public:
		Oid(string oid)
		{
			_oidText = oid;
			StrToOid(_oidText);
		}

		Oid(vector<Byte> oid_bytes)
		{
			ArrayToOid(oid_bytes);
			_oidText = ToString();
		}

		Oid(Byte* buf, Word len)
		{
			ArrayToOid(buf, len);
			_oidText = ToString();
		}

		~Oid()
		{
			for(auto curr : Oids){
				delete curr;
			}
		}

		int GetSize()
		{
			int ret = 0;

			for(auto curr : Oids){
				ret +=curr->GetValue().size();
			}
			return ret;
		}

		vector<Byte> GetByteValue()
		{
			vector<Byte> ret;
			for(auto oid : Oids){
				vector<Byte>  bs = oid->GetValue();
				for(auto b : bs){
					ret.push_back(b);
				}
			}
			return ret;
		}

		string ToString()
		{
			string ret = "";
			for(auto oid : Oids){
				ret+=oid->ToString()+".";
			}
			if(ret.size() > 0 && ret.c_str()[ret.size()-1] == '.')
				ret = ret.substr(0, ret.size()-1);
			return ret;
		}

		/*
		Oid *CreateClone(int len = 0)
		{
			Log::DEBUG("OidCreateClone size="+toString(len));

			Oid *ret = NULL;
			int sz = GetSize();
			if(len == 0)
				len = sz;

			if(len <= sz)
			{
				string new_oid =



				vector<Byte> oid_bytes = GetByteValue();
				oid_bytes.resize(sz);

				ret = new Oid(oid_bytes);
				Log::DEBUG("OidCreateClone size="+toString(ret->GetSize()));
			}
			return ret;
		}*/



	protected:

		int StrToOid(string str);

		int ArrayToOid(Byte* buf, Word len);

		int ArrayToOid(vector<Byte> oid_bytes);



		string _oidText;

		vector<OidNumber *> Oids;
};

class netsnmp
{
	public:
		netsnmp()
		{
			request_id.Data_s = 1;
			last_pdu_type = SNMP_GET_req;
		}

		vector<Byte> GetPDU(Byte p_type, Byte ver, string comm, Oid *oid, string d = "", Byte d_type=0x42);

		bool ParsingPDU(Byte *ptrPDU, Word lenPDU);

		string GetData(Byte *ptrPDU, Word lenPDU, string lst_oid);




	private:

		Bshort request_id;
		PDU_TYPE last_pdu_type;
};

#endif /* NETSNMP_H_ */

/*
 * netsnmp.cpp
 *
 *  Created on: Jun 1, 2017
 *      Author: user
 */

#include <netsnmp.h>

//=============[Oid::StrToOid(char *string, SmiLPOID dst) ]==============
// convert a string to an oid
int Oid::StrToOid(string str)
{
  unsigned int index = 0;

  string iso = "1.3.";
  std::size_t found = str.find(iso);
  if (found!=std::string::npos)
  {
	  Oids.push_back( new OidNumber(0x2b) );
	  string new_str = str.substr(found+iso.size());
	  vector<string> oids = Split(new_str, ".");
	  for(auto curr :  oids){
		  Oids.push_back( new OidNumber(atoi(curr.c_str())) );
	  }
	  index = Oids.size();
  }
  return (int) index;
}
//=============[Oid::ArrayToOid(Byte *buf, Word len) ]==============
// convert a string to an oid
int Oid::ArrayToOid(Byte* buf, Word len)
{
  unsigned int index = 0;
  Blong TMP;

  for(int i=0; i < len;)
  {
	  if(  !(buf[i] & 0x80)  )
		  Oids.push_back( new OidNumber(buf[i++]) );
	  else
	  {
		  TMP.Data_l = 0;
		  Byte b1 = buf[i++] - 0x80;
		  if(buf[i] & 0x80){
			  b1 = b1<< 7 | (buf[i++] - 0x80);
		  }
		  TMP.Data_b[1] = b1;
		  TMP.Data_b[0] = buf[i++] | (b1 << 7);
		  TMP.Data_b[1] = (b1 >> 1);
		  /*
		  TMP.Data_b[1] = buf[i++] - 0x80;
		  TMP.Data_b[0] = buf[i++] | TMP.Data_b[1] << 7;
		  */
		  Oids.push_back( new OidNumber(TMP.Data_l) );
	  }
  }
  return (int)index;
}
//=============[Oid::ArrayToOid(vector<Byte> oid_bytes) ]==============
int Oid::ArrayToOid(vector<Byte> oid_bytes)
{
  unsigned int index = 0;
  Blong TMP;
  Word len = oid_bytes.size();
  for(int i=0; i < len;)
  {
	  if(  !(oid_bytes[i] & 0x80)  )
		  Oids.push_back( new OidNumber(oid_bytes[i++]) );
	  else
	  {
		  TMP.Data_l = 0;
		  Byte b1 = oid_bytes[i++] - 0x80;
		  if(oid_bytes[i] & 0x80){
			  b1 = b1<< 7 | (oid_bytes[i++] - 0x80);
		  }
		  TMP.Data_b[1] = b1;
		  TMP.Data_b[0] = oid_bytes[i++] | (b1 << 7);
		  TMP.Data_b[1] = (b1 >> 1);
		  Oids.push_back( new OidNumber(TMP.Data_l) );
	  }
  }
  return (int)index;
}
//===========================
//Bshort netsnmp::request_id;
//===========================


//===========================
vector<Byte> netsnmp::GetPDU(Byte pdu_type, Byte snmp_ver, string community, Oid *oid, string data, Byte type)
{
	Byte FLG = 0x30;
	Byte END = 0x05;
	vector<Byte> ret;

	if(request_id.Data_s <= 0)
		request_id.Data_s = 1;

	last_pdu_type = (PDU_TYPE)pdu_type;

	vector<Byte> oid_bytes = oid->GetByteValue();
	ret.push_back(FLG);
	ret.push_back(0);//len all
	ret.push_back(0x02); //type next parameter (snmp_ver)
	ret.push_back(0x01); //len next parameter (snmp_ver)
	ret.push_back(snmp_ver);
	ret.push_back(0x04); //type next parameter (community)
	ret.push_back(community.size()); //len next parameter (community)
	for(int i =0; i < community.size();i++)
		ret.push_back(community.c_str()[i]);
	ret.push_back(pdu_type);
	ret.push_back(0); // len pdu
	Word pdu_len_div = ret.size(); //save index for count len pdu
	ret.push_back(0x02); //type next parameter (request_id)
	ret.push_back(0x02); //len next parameter (request_id)
	ret.push_back(request_id.Data_b[1]);
	ret.push_back(request_id.Data_b[0]);
	ret.push_back(0x02); //type next parameter (error_status)
	ret.push_back(0x01); //len next parameter (error_status)
	ret.push_back(0x00); //error_status
	ret.push_back(0x02); //type next parameter (error_index)
	ret.push_back(0x01); //len next parameter (error_index)
	ret.push_back(0x00); //error_index
	ret.push_back(FLG);
	ret.push_back(0);//oid_bytes.size()+6);
	Word oid_l1_index = ret.size(); //save index for count len oid
	ret.push_back(FLG);
	ret.push_back(0);//oid_bytes.size()+4);
	ret.push_back(0x06);//type next parameter (oid)
	ret.push_back(oid_bytes.size()); //len next parameter (oid)
	for(auto  curr : oid_bytes){
		ret.push_back(curr);
	}
	if(!isNullOrWhiteSpace(data)){
		ret.push_back(type);
		Bshort tmp;
		tmp.Data_s = atoi(data.c_str());
		ret.push_back(sizeof(tmp));
		ret.push_back(tmp.Data_b[1]);
		ret.push_back(tmp.Data_b[0]);
	}else{
		ret.push_back(END);
		ret.push_back(0x00);
	}
	ret[1] = ret.size()-2;
	ret[pdu_len_div-1] = ret.size()-pdu_len_div;

	ret[oid_l1_index-1] = ret.size()-oid_l1_index;
	ret[oid_l1_index-1+2] = ret.size()-oid_l1_index-2;



	if(oid != NULL)
	{
		Log::DEBUG("netsnmp::GetPDU oid="+oid->ToString() + " request_id="+toString(request_id.Data_s));
		delete oid;
	}

	request_id.Data_s++;
	return ret;
}

bool netsnmp::ParsingPDU(Byte *ptrPDU, Word lenPDU)
{
	bool ret = false;
	Word ind = 0;
	Bshort TMP;
	Blong  TMP_L;

	while(ptrPDU[ind++] != 0x30 && ind < lenPDU);
	if(ptrPDU[ind] == 0x82 && ptrPDU[ind+1] == 0x00)
		ind+=2;

	if( (lenPDU - ind) > 6)
	{
		Word allLen 	= ptrPDU[ind];
		Byte ver 		= ptrPDU[ind+3];
		Byte passLen 	= ptrPDU[ind+5];
		Byte cmd 		= ptrPDU[ind+5+passLen+1];
		ind += 5+passLen+1+1;
		if(ptrPDU[ind] == 0x82 && ptrPDU[ind+1] == 0x00)
			ind+=2;

		Word pdu_len 	= ptrPDU[ind];
		Log::DEBUG("netsnmp::ParsingPDU allLen="+toString(allLen)+" pdu_len="+toString(pdu_len) + " ind="+toString(ind));
		if( (lenPDU - ind) > pdu_len)
		{
			Word id_len = ptrPDU[ind+2];
			if(id_len > sizeof(TMP_L))
				return false;

			Log::DEBUG("netsnmp::ParsingPDU id_len="+toString(id_len)+" pdu_len="+toString(pdu_len) + " ind="+toString(ind));
			TMP_L.Data_l = 0;
			if(id_len == 0) id_len = 1;
			for(int i=id_len; i > 0; i--)	{
				TMP_L.Data_b[id_len-i] = ptrPDU[ind+2+i];
				Log::DEBUG("netsnmp::ParsingPDU ind="+toString(ind) + " data["+toString((int)(id_len-i))+"]="+toString((int)(TMP_L.Data_b[id_len-i])));
			}
			/*if(ptrPDU[ind+2] == 1){
				TMP.Data_b[0] = ptrPDU[ind+3];
				TMP.Data_b[1] = 0;
			}
			else{
				TMP.Data_b[1] = ptrPDU[ind+3];
				TMP.Data_b[0] = ptrPDU[ind+4];
			}
			ind+=3+ptrPDU[ind+2];*/

			DWord r_id = TMP_L.Data_l;
			DWord need_r_id = request_id.Data_s-1;

			Log::DEBUG("netsnmp::ParsingPDU r_id="+toString(r_id)+" need_r_id="+toString(need_r_id) + " id_len="+toString(id_len) + " ind="+toString(ind));

			if(r_id != need_r_id)
				return ret;


			ind+=3+id_len;//ptrPDU[ind+2];
			if(ptrPDU[ind+1] == 1){
				TMP.Data_b[0] = ptrPDU[ind+2];
				TMP.Data_b[1] = 0;
			}
			else{
				TMP.Data_b[1] = ptrPDU[ind+2];
				TMP.Data_b[0] = ptrPDU[ind+3];
			}
			ind+=2+ptrPDU[ind+1];
			Short err_state = TMP.Data_s;

			if(ptrPDU[ind+1] == 1){
				TMP.Data_b[0] = ptrPDU[ind+2];
				TMP.Data_b[1] = 0;
			}
			else{
				TMP.Data_b[1] = ptrPDU[ind+2];
				TMP.Data_b[0] = ptrPDU[ind+3];
			}
			ind+=2+ptrPDU[ind+1];
			Short err_index = TMP.Data_s;
			Log::DEBUG("netsnmp::ParsingPDU err_state="+toString(err_state)+" err_index="+toString(err_index));
			if(err_state > 0 || err_index > 0)
			{
				if(err_state == 2){
					Log::DEBUG("netsnmp::ParsingPDU err_status=noSuchName ");
				}
				else{
					return false;
				}
			}
			if(ptrPDU[ind++] == 0x30)
			{
				if(ptrPDU[ind] == 0x82 && ptrPDU[ind+1] == 0x00)
					ind+=2;

				Word L7 = ptrPDU[ind++];
				//Log::DEBUG("netsnmp::ParsingPDU L7="+toString(L7)+" ind="+toString(ind));
				if( (lenPDU - ind) >= L7){
					if(ptrPDU[ind++] == 0x30)
					{
						if(ptrPDU[ind] == 0x82 && ptrPDU[ind+1] == 0x00)
							ind+=2;

						Word L8 = ptrPDU[ind++];
						//Log::DEBUG("netsnmp::ParsingPDU (lenPDU - ind)="+toString((lenPDU - ind))+" L8="+toString(L8) );
						if( (lenPDU - ind) >= L8){
							if(ptrPDU[ind++] == 0x06){
								Word len_oid = ptrPDU[ind++];
								//Log::DEBUG("netsnmp::ParsingPDU L8="+toString(L8)+" len_oid="+toString(len_oid));
								ret = true;
							}
						}
					}
				}
			}
		}
	}
	return ret;
}

string netsnmp::GetData(Byte *ptrPDU, Word lenPDU, string lst_oid)
{
	string ret = "";
	Word ind = 0;
	Blong TMP;
	Blong  TMP_L;
	Log::DEBUG("netsnmp::GetData lst_oid="+lst_oid);
	while(ptrPDU[ind++] != 0x30 && ind < lenPDU);
	if(ptrPDU[ind] == 0x82 && ptrPDU[ind+1] == 0x00)
		ind+=2;

	if( (lenPDU - ind) > 6)
	{
		Word allLen 	= ptrPDU[ind];
		Byte ver 		=  ptrPDU[ind+3];
		Byte passLen 	= ptrPDU[ind+5];
		Byte cmd 		= ptrPDU[ind+5+passLen+1];
		ind += 5+passLen+1+1;
		if(ptrPDU[ind] == 0x82 && ptrPDU[ind+1] == 0x00)
			ind+=2;

		Word pdu_len 	= ptrPDU[ind];
		//Log::DEBUG("netsnmp::GetData allLen="+toString(allLen)+" pdu_len="+toString(pdu_len));
		if( (lenPDU - ind) > pdu_len){
			Word id_len = ptrPDU[ind+2];
			if(id_len > sizeof(TMP_L))
				return ret;
			TMP_L.Data_l = 0;
			/*
			if(ptrPDU[ind+2] == 1){
				TMP.Data_b[0] = ptrPDU[ind+3];
				TMP.Data_b[1] = 0;
			}
			else{
				TMP.Data_b[1] = ptrPDU[ind+3];
				TMP.Data_b[0] = ptrPDU[ind+4];
			}*/
			if(id_len == 0) id_len = 1;
			for(int i=id_len; i>0; i--)	{
				TMP_L.Data_b[id_len-i] = ptrPDU[ind+2+i];
			}
			DWord r_id = TMP_L.Data_l;
			DWord need_r_id = request_id.Data_s-1;
			//Log::DEBUG("netsnmp::GetData r_id="+toString(r_id)+" need_r_id="+toString(need_r_id) + " id_len="+toString(id_len) + " ind="+toString(ind));

			if(r_id != need_r_id)
				return ret;

			ind += 3+id_len;//ptrPDU[ind+2];
			TMP.Data_l = 0;
			if(ptrPDU[ind+1] == 1){
				TMP.Data_b[0] = ptrPDU[ind+2];
				TMP.Data_b[1] = 0;
			}
			else{
				TMP.Data_b[1] = ptrPDU[ind+2];
				TMP.Data_b[0] = ptrPDU[ind+3];
			}
			ind+=2+ptrPDU[ind+1];
			Short err_state = TMP.Data_l;

			TMP.Data_l = 0;
			if(ptrPDU[ind+1] == 1){
				TMP.Data_b[0] = ptrPDU[ind+2];
				TMP.Data_b[1] = 0;
			}
			else{
				TMP.Data_b[1] = ptrPDU[ind+2];
				TMP.Data_b[0] = ptrPDU[ind+3];
			}
			ind+=2+ptrPDU[ind+1];
			Short err_index = TMP.Data_l;
			Log::DEBUG("netsnmp::GetData err_state="+toString(err_state)+" err_index"+toString(err_index));
			if(err_state > 0 || err_index > 0)
				return ret;


			if(ptrPDU[ind++] == 0x30)
			{
				if(ptrPDU[ind] == 0x82 && ptrPDU[ind+1] == 0x00)
					ind+=2;

				Word L7 = ptrPDU[ind++];
				//Log::DEBUG("netsnmp::GetData L7="+toString(L7)+" ind="+toString(ind));
				if( (lenPDU - ind) >= L7){
					if(ptrPDU[ind++] == 0x30)
					{
						if(ptrPDU[ind] == 0x82 && ptrPDU[ind+1] == 0x00)
							ind+=2;

						Word L8 = ptrPDU[ind++];
						//Log::DEBUG( "netsnmp::GetData (lenPDU - ind)="+toString((lenPDU - ind))+" L8="+toString(L8) );
						if( (lenPDU - ind) >= L8){
							if(ptrPDU[ind++] == 0x06){
								Word len_oid = ptrPDU[ind++];
								//Log::DEBUG("netsnmp::GetData L8="+toString(L8)+" len_oid="+toString(len_oid));
								Oid curr_oid(&ptrPDU[ind], len_oid);
								Log::DEBUG("netsnmp::GetData curr_oid="+curr_oid.ToString() + " size=" + toString(curr_oid.GetSize()));

								Oid need_oid(lst_oid);
								Log::DEBUG("netsnmp::GetData need_oid="+need_oid.ToString() + " size=" + toString(need_oid.GetSize()));

								size_t found = curr_oid.ToString().find(need_oid.ToString());
								if (found == string::npos){
									return ret;//old


									///TODO: check new ver down here for errors
									//new ver
									/*
									vector<string> oids = Split(need_oid.ToString(), ".");
									if(oids.size() > 0)
									{
										oids.pop_back();
										string new_oid = "";
										for(auto oid : oids){
											new_oid+=oid+".";
										}

										if(new_oid.size() > 0 && new_oid.c_str()[new_oid.size()-1] == '.')
											new_oid = new_oid.substr(0, new_oid.size()-1);

										Oid reduse_oid(new_oid);
										found = curr_oid.ToString().find(reduse_oid.ToString());
										Log::DEBUG("netsnmp::GetData find reduse_oid="+reduse_oid.ToString() + " found="+toString(found));
										if (found == string::npos)
											return ret;

									}
									else
										return ret;
									*/
								}

								bool isnext = false;
								Log::DEBUG("netsnmp::GetData curr_oid size=" + toString(curr_oid.ToString().size()) + " need_oid size="+ toString(need_oid.ToString().size())   );
								if( curr_oid.ToString().size() > need_oid.ToString().size() )	{

									isnext = true;
									Log::DEBUG("netsnmp::GetData isnext=true");
								}

								ind+=len_oid;
								Byte data_type = ptrPDU[ind++];
								Log::DEBUG("netsnmp::GetData data_type="+toString((int)data_type));
								if (data_type > 0 && data_type != 5)
								{
									Word data_len = ptrPDU[ind++];
									if(data_len > 0)
									{
										Log::DEBUG("netsnmp::GetData data_value len="+toString((int)data_len));
										TMP.Data_l = 0;
										Word j = data_len-1;
										switch(data_type)
										{
										case 0x02:
										case 0x42:
											for(int i = 0; i < data_len; i++){
												TMP.Data_b[j--] = ptrPDU[ind+i];
											}
											ret = toString(TMP.Data_l);
											break;

										case 0x04:
											ret = string((char *)&ptrPDU[ind], data_len);
											break;
										};
									}
									if(isnext && !isNullOrWhiteSpace(ret))
										ret = "$"+ret;//???
								}
							}
						}
					}
				}
			}
		}
	}
	Log::DEBUG("netsnmp::GetData value=["+ret+"]");
	return ret;
}

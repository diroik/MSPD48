/*
 * convert.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: user
 */

#include <convert.h>


//=======================================
vector<string>  Split(string str, string separator)
{
	vector<string> results;
	results.clear();
    unsigned int found;
    found = str.find(separator);
    while(found != string::npos)
    {
        if(found > 0)
        {
          results.push_back(str.substr(0,found));
        }
        str = str.substr(found+separator.size());
        found = str.find(separator);
    }
    if(str.length() > 0)
    { results.push_back(str);}
    return results;
}
//=======================================
string WordToString(Word Val, int AllSymb)
{
  char buff[6] = {0x30,0x30,0x30,0x30,0x30,0};
  sByte index = 4;
  Word counter = 1;
  Word tmpcounter = 1;
  Word tmp1;
  Word tmp2;

  do
  {
    counter*=10;
    tmp1 = Val/counter;
    tmp2 = Val%counter;
    tmp2 = tmp2/tmpcounter;
    tmpcounter = counter;
    buff[index--] = tmp2+0x30;
  }
  while(tmp1 > 0 && index > 0);
  if(tmp1 > 0)
  { buff[index--] = tmp1+0x30;}
  if(AllSymb > 0)
  {
    sByte i = index+1;
    if(i > 0)
    {
      if(AllSymb > 5)
      { AllSymb = 5;}
      Byte siz = 5-i;
      i=AllSymb-siz;
      if(i>0)
      {
        index-=i;
      }
    }
  }
  return string(&buff[index+1]);
}
//=======================================
DWord ByteToLong(Byte* Buf)
{
  Blong   LBuf;
    for (Word i = 0; i < 4 ; i++)
      LBuf.Data_b[i] = Buf[i];
    return LBuf.Data_l;
}
//=======================================
Word ByteToShort(Byte* Buf)
{
 Bshort  SBuf;

 for (Word i = 0; i < 2 ; i++)
	 SBuf.Data_b[i] = Buf[i];
 return SBuf.Data_s;
}
//=======================================
Word DeByteToShort(Byte* Buf)
{
 Bshort  SBuf;

 for (Word i = 0; i < 2 ; i++)
	 SBuf.Data_b[1-i] = Buf[i];
 return SBuf.Data_s;
}
//=======================================
Word HexToWord(Byte *ptr)
{
  Word number = 0;

  sscanf((char*)ptr, "%X", &number);
  return number;
}
//=======================================
Word HexToWord(string val)
{
  Word number = 0;

  sscanf((char*)val.c_str(), "%X", &number);
  return number;
}
//=======================================
DWord HexToDWord(Byte *Ptr)
{
  DWord number = 0;

  sscanf((char*)Ptr, "%lX", &number);
  return number;
}
//=======================================
sWord HexToByte(Byte *ptr)
{
   Byte ch  = *ptr++;
   Byte k   = 0;
   Byte result[2] = {0,0};


  while(k < 2)
  {
    if( (ch >= 'A') && (ch <= 'F') )
    {  result[k] = (ch - 'A') + 10; }
    else if( (ch >= '0') && (ch <= '9') )
    {  result[k] = ch - '0';}
    else
    {  return -1;}
    ch = *ptr;
    k++;
  }
  return (result[0] <<= 4) + result[1];
}
//=======================================
sWord HexToByte(char *ptr)
{
   Byte ch  = *ptr++;
   Byte k   = 0;
   Byte result[2] = {0,0};


  while(k < 2)
  {
    if( (ch >= 'A') && (ch <= 'F') )
    {  result[k] = (ch - 'A') + 10; }
    else if( (ch >= '0') && (ch <= '9') )
    {  result[k] = ch - '0';}
    else
    {  return -1;}
    ch = *ptr;
    k++;
  }
  return (result[0] <<= 4) + result[1];
}
//=======================================
float HexToFloat(Byte *Ptr)
{
  Bfloat ret;
  int st;
  st = HexToByte(Ptr);
  ret.Data_b[0] = st < 0 ? 0: st;

  st = HexToByte(&Ptr[2]);
  ret.Data_b[1] = st < 0 ? 0: st;

  st = HexToByte(&Ptr[4]);
  ret.Data_b[2] = st < 0 ? 0: st;

  st = HexToByte(&Ptr[6]);
  ret.Data_b[3] = st < 0 ? 0: st;

  return ret.Data_f;
}

//=======================================
void ByteToHex(Byte *HEX, Byte BYTE)
{
 Byte ch = (BYTE >> 4) & 0x0F;
 Byte k  = 0;


   while(k < 2)
   {
     if(ch > 9)
     {  *HEX++ = ch + 'A' - 10;}
     else
     {  *HEX++ = ch + '0';}

     ch = BYTE & 0x0F;
     k++;
   }

   HEX = NULL;
}
//=======================================
string ByteToHex(Byte BYTE)
{
	Byte tmp[50];
	Byte *HEX = tmp;


	 Byte ch = (BYTE >> 4) & 0x0F;
	 Byte k  = 0;

	   while(k < 2)
	   {
		 if(ch > 9)
		 {  *HEX++ = ch + 'A' - 10;}
		 else
		 {  *HEX++ = ch + '0';}

		 ch = BYTE & 0x0F;
		 k++;
	   }

	   HEX = 0;
   return string((char*)tmp, 2);
}
//=======================================
void WordToHex(Byte *HEX, Word WORD)
{
  ByteToHex( HEX, (Byte)( WORD >> 8) );
  ByteToHex( &HEX[2], (Byte)( WORD) );
}
//=======================================
void DWordToHex(Byte *HEX, DWord DWORD)
{
  WordToHex(HEX, (Word)( DWORD >> 16) );
  WordToHex(&HEX[4], (Word)( DWORD) );
}
//=======================================
struct tm StringToTime(string st)
{
	struct tm 	ret;

	vector<string> tmp = Split(st, " ");

	if(tmp.size() == 2){

		vector<string> date = Split(tmp[0], "-");
		vector<string> time = Split(tmp[1], ":");
		if(date.size() == 3 && time.size() == 3){
			ret.tm_year = fromString<int>(date[0].c_str()) - 1900;
			ret.tm_mon 	= fromString<int>(date[1].c_str()) - 1;
			ret.tm_mday = fromString<int>(date[2].c_str());

			ret.tm_hour = fromString<int>(time[0].c_str());
			ret.tm_min 	= fromString<int>(time[1].c_str());
			ret.tm_sec 	= fromString<int>(time[2].c_str());
		}
	}

	return ret;
}
//=======================================
struct tm StringToTime2(string st)
{
	struct tm 	ret;

	vector<string> tmp = Split(st, " ");

	if(tmp.size() == 2)
	{

		vector<string> date = Split(tmp[0], ".");
		vector<string> time = Split(tmp[1], ":");
		if(date.size() == 3)
		{
			ret.tm_year = fromString<int>(date[2].c_str()) - 1900;
			ret.tm_mon 	= fromString<int>(date[1].c_str()) - 1;
			ret.tm_mday = fromString<int>(date[0].c_str());
		}
		if(time.size() == 3)
		{
			ret.tm_hour = fromString<int>(time[0].c_str());
			ret.tm_min 	= fromString<int>(time[1].c_str());
			ret.tm_sec 	= fromString<int>(time[2].c_str());
		}
	}

	return ret;
}
//=======================================
struct tm StringToDate2(string st)
{
	struct tm 	ret;
	vector<string> date = Split(st, ".");
	if(date.size() == 3)
	{
		ret.tm_year = fromString<int>(date[2].c_str()) - 1900;
		ret.tm_mon 	= fromString<int>(date[1].c_str()) - 1;
		ret.tm_mday = fromString<int>(date[0].c_str());

		ret.tm_hour = 0;
		ret.tm_min 	= 0;
		ret.tm_sec 	= 0;
	}
	return ret;
}
//=======================================
string TimeToDateString2(struct tm &t)
{
	return IntToString(t.tm_mday, 2)+"."+IntToString((t.tm_mon+1), 2)+"."+IntToString(t.tm_year+1900);
}
//=================================
string TimeToString(struct tm &t)
{
	return toString(t.tm_year+1900)+"-"+toString(t.tm_mon+1)+"-"+toString(t.tm_mday)+
		    "T"+toString(t.tm_hour)+":"+toString(t.tm_min)+":"+toString(t.tm_sec);

}
//=======================================

string TimeToString(struct tm *t)
{
	return toString(t->tm_year+1900)+"-"+toString(t->tm_mon+1)+"-"+toString(t->tm_mday)+
		    "T"+toString(t->tm_hour)+":"+toString(t->tm_min)+":"+toString(t->tm_sec);

}

//=======================================
string FloatToString(float Val, int AllSymb, int AfteComma)
{
  char buff[32];
  if(AllSymb == 0)
  { sprintf(buff,"%.*f",AfteComma,Val);}
  else
  { sprintf(buff,"%0*.*f",AllSymb,AfteComma,Val);}
  return string(buff);
}
//=======================================
Byte FloatToString(Byte *ReturnBUF, float Val, int AllSymb, int AfteComma)
{

  sprintf((char*)ReturnBUF,"%0*.*f",AllSymb,AfteComma,Val);
  return AllSymb;
}

//=======================================
Byte IntToString(Byte *ReturnBUF, int Val, int AllSymb)
{
  sprintf((char*)ReturnBUF,"%0*i",AllSymb,Val);
  return AllSymb;
}
//=======================================
string IntToString(int Val, int AllSymb)
{
char buff[16];
  if(AllSymb == 0)
  { sprintf(buff,"%i",Val);}
  else
  { sprintf(buff,"%0*i",AllSymb,Val);}
return string(buff);
}
//=======================================
string LongToString(long Val, int AllSymb)
{
char buff[32];
  if(AllSymb == 0)
  { sprintf(buff,"%li",Val);}
  else
  { sprintf(buff,"%0*li",AllSymb,Val);}

return string(buff);
}
//=======================================
string LongLongToString(long long Val, int AllSymb)
{
char buff[32];
  if(AllSymb == 0)
  { sprintf(buff,"%lli",Val);}
  else
  { sprintf(buff,"%0*lli",AllSymb, Val);}

return string(buff);
}
//=======================================
Short Crc16(Byte * pcBlock, Short len)
{
 Short crc = 0xFFFF;
    while (len--)
    {
      crc = (Short)(crc >> (Short)8) ^ (Short)Crc16Table[(Short)(crc & (Short)0xFF) ^ (Short)*pcBlock++];
    }
    return crc;
}
//=======================================
Short CHKSUM(Byte * pcBlock, Short len)
{

	Short crc = 0xFFFF;
	 for(int i=1; i < len; i++){
			 crc+=pcBlock[i];
	 }
	 return (Short)(0xFFFF-crc);
}
//=======================================
Short Crc16ccitt(Byte * pcBlock, Short len)
{
	Short crc = 0xFFFF;

    while (len--)
        crc = (crc << 8) ^ Crc16TableCcitt[(crc >> 8) ^ *pcBlock++];

    return crc;
}
//=======================================
Short Crc16ccitt0(Byte * pcBlock, Short len)
{
	Short crc = 0x0000;

    while (len--)
        crc = (crc << 8) ^ Crc16TableCcitt[(crc >> 8) ^ *pcBlock++];

    return crc;
}
//=======================================
string sendToConsole(string str)
{
 	string ret;
	FILE *fp;
	char buffer[2000];
	bzero(buffer, sizeof(buffer));
	fp=popen( str.c_str(), "r");
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	ret = string(buffer);
	if(ret.size()>0) ret = ret.erase(ret.size()-1, 1);

	//printf("DEBUG: [%s]\n", ret.c_str());
 return ret;
}
//=======================================
bool isNullOrWhiteSpace(string str)
{
	bool ret = true;
	if(str.length() == 0) return true;
	for(unsigned int i = 0; i < str.size(); i++)
	{
			if(str.c_str()[i] > ' ')
				return false;
	}
	return ret;
}
//=======================================
string replaceAll(string str, const string from, const string to)
{
  while(str.find(from) != string::npos)
    str.replace(str.find(from), from.length(), to);
  return str;
}
//=======================================
string trimAllNull(string trimstring)
{
	char dest[8192];
	int index = 0;

	//printf("DEBUG: %s\n", "trimAllNull");

	for(int i=0; i < trimstring.size();i++){
		if( trimstring.c_str()[i] > 0x20 )
			dest[index++] = trimstring.c_str()[i];
	}
	return string(dest, index);
}
/*
string trim(string trimstring){
	int start = 0;
	int end = 0;

	string ret = "";
	for(int i=0;i<trimstring.size();i++)
	{
		if(trimstring.c_str()[i] == ' '){
			continue;
		}
		else{
			if(start == 0){
				start = i;
			}
		}
	}
	for(int i=trimstring.size()-1; i>=0;i--)
	{
		if(trimstring.c_str()[i] == ' '){
			continue;
		}
		else{
			if(end == 0){
				end = i;
			}
		}
	}
	if(start >= end){
		ret = string(trimstring, start, (end-start)+1);
	}
	return ret;
}*/
void gets_utf8(char *buf, int buf_len)
{

}

//=======================================
bool StrCmp(string &str1, string &str2)
{


	if(str1.length() != str2.length()) return false;
	char *Str1 = (char *)str1.c_str();
	char *Str2 = (char *)str2.c_str();
	while(*Str1)
	{
		if(*Str1++ != *Str2++)
			return false;
	}
	if(*Str2 != 0)
		return false;
	return true;
}


string JsonParse(string name, char *data)
{
	string ret = "";
	char* ind = strstr(data, name.c_str());
	if(ind != NULL)
	{
		//printf("DEBUG!!: [%s]\n", ind);
		//Log::DEBUG("JsonParse find "+ toString(ind));
		char *ch = strstr(ind, ",");
		if(ch != NULL)
			*ch = 0;
		string ret1 = toString(ind);
		ret1 = replaceAll(ret1, "]", "");
		ret1 = replaceAll(ret1, "}", "");
		ret1 = replaceAll(ret1, "\"", "");

		vector<string> lst = Split(ret1, ":");
		if(lst.size() == 2){
			ret = lst[1];
		}
	}
	return ret;
}


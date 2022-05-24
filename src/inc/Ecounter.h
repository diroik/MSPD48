/*
 * Ecounter.h
 *
 *  Created on: Sep 30, 2014
 *      Author: user
 */

#ifndef ECOUNTER_H_
#define ECOUNTER_H_

#include <CommPort.h>
#include <Classes.h>
#include <HTTPClient.h>
#include <math.h>

using namespace std;
using namespace log4net;
using namespace MyClassTamlates;
//=============================================================================
  enum IFASE          { ONE   = 0, NUL };
  //enum ECMDSUBFASE    { WAIT_ECMD = 0, OPEN_ECHANNAL, SET_ECMD, END_ECMD };
  enum ECTYPE         { MERCURY = 0, MERC200 = 1, ENERGOMERA = 2, CHINT = 3, HUAB = 4, HUAB2 = 5, PSCH = 6};
//=============================================================================
  class ICounter;
  //=============================================================================

  class TEnergyCMD//:public TFastTimer
  {
  public:
	TEnergyCMD():Type(0), RetLen(0)
	  {
		Etype = 41;
		Ename = 0;
		//Fase = EXEC_CMD;//CREATE_CMD;
		//SubFase = WAIT_ECMD;
		Fase = CREATE_CMD;
		SubFase = OPEN_CHANNAL;

		Tmp[0] = 0;
		Tmp[1] = 0;
		WithOpenChanal = true;
		WithCloseChanal = true;
		NeedOpenChanal = false;
		NeedCrc = true;
		Index = -1;
		Addr = 0;
		counter = NULL;
	  }

	TRtcTime       DateTime;
	Byte           RetLen;
	Byte           Type;
	Byte           Addr;

	int Index;
	vector<Byte> 	Data;
	vector<Byte> 	Password;
	Word Tmp[3];
	string TmpStr[3];

	Byte Etype;
	Byte Ename;

	Byte Fase;
	Byte SubFase;

	bool NeedOpenChanal;

	bool WithOpenChanal;
	bool WithCloseChanal;


	bool NeedCrc;

	ICounter *counter;
  };
    //=============================================================================
	  class TEnergy:public IParams
	  {
	  public:
		  TEnergy():IParams(40,1),Ea(0),Er(0),Ea_minus(0),Er_minus(0)
	  	  {
			  Size = 4;
		  }
		  TEnergy(Byte pt, Byte pn):IParams(pt,pn),Ea(0),Er(0),Ea_minus(0),Er_minus(0)
	  	  {
			  Size = 4;
		  }

		 ~TEnergy(){}

		  string GetValueString(void)
		  {
			return string(">,"+
						  WordToString(Pt.Ptype, 0)+','+
						  WordToString(Pt.Pname, 0)+','+
						  FloatToString(Ea, 0, 3)+','+
						  FloatToString(Er, 0, 3)+','+
						  FloatToString(Ea_minus, 0, 3)+','+
					      FloatToString(Er_minus, 0, 3)+',');
		  }

		 float Ea;
		 float Ea_minus;
		 float Er;
		 float Er_minus;
	  };

    //=============================================================================
	  class TCounterType:public IParams
	  {
	      public:
		  TCounterType():IParams(41,1), Type("")
	        {
			  Size= 1;
			  Model = "";
			  Const=0;
			  UTC = 0;
	        }
	       ~TCounterType() {}


	    string GetValueString(void)
	    {
	    	//Log::DEBUG("TCounterType::GetValueString Type.size="+toString(Type.size()));
			  Byte bf[2] {0,0};
			  string recv = "";
			  for(int i = 0; i < Type.size(); i++)
			  {
				  ByteToHex(bf, Type.c_str()[i]);
				  recv += "0x"+string((char*)bf, 2)+" ";
			  }
			  //Log::DEBUG("TCounterType::GetValueString recv=["+recv+"]");
	    	if(isNullOrWhiteSpace(Type)){
	    		Type = "~";}
	        string ret = "\r\n>,";
	        ret += WordToString(Pt.Ptype)+',';
	        ret += WordToString(Pt.Pname)+',';
	        ret += Type+',';
	        return ret;
	    }
	    string Type;
	    string Model;

	    Word Const;
	    Word UTC;

	  };

    //=============================================================================
      class TSerialN:public IParams
      {
      public:
        TSerialN():IParams(49,1)
        {
          Size = 1;
          SerialNumber = "00000000";
        }
       ~TSerialN() {}


        string GetValueString(void)
        {
        	string ret = "\r\n>,";
        	ret += WordToString(Pt.Ptype)+',';
        	ret += WordToString(Pt.Pname)+',';
        	ret += SerialNumber+',';
            return ret;
        }

        string SerialNumber;
      };
    //=============================================================================
      class TSimpleCounter
      {
      public:
        TSimpleCounter(Byte addr, Byte type, CommSettings _comm_settings)
        {
          Addr = addr;
          Type = type;
          comm_settings = _comm_settings;
        }


        Byte          Addr;
        Byte          Type;
        CommSettings  comm_settings;

      };
      //=============================================================================
      class ICounter
        {
          friend class TEnergyCounterManager;
        public:
          ICounter(Byte Ename, Byte adr);
          virtual ~ICounter();

          virtual string  GetStringValue(void);
          virtual string  GetStringCfgValue(void);

          virtual Word    CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL) = 0;
          virtual Word    GetRecvMessageLen(Byte CMD) = 0;
          virtual Word    GetErrorMessageLen(Byte CMD){
        	  return 251;
          }

          virtual sWord   RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen) = 0;
          virtual bool    ParsingAnswer( Byte *BUF, Word Len, Byte subFase );
          virtual void    GetAdditionalData(Byte *Buffer, Word len = 0 ){

          }

          virtual sWord CorrectData(Byte *BUF, Word Len, Byte subFase )
          {
        	  if(Len > 5)
        	  {
        		  if(subFase != SET_PLC && plc == 3)
        		  {
        			  Len -= 5;
        			  for(int i=0; i < Len; i++)
        			  {
        				  BUF[i] = BUF[i+3];
        			  }
        		  }
        	  }
        	  return Len;
          }

          virtual bool SetDateTime(string tz, string password){
        	  return false;
          }

          virtual bool CorrectDateTime(string time)
          {
        	  return false;
          }

          virtual bool GetDateTime(){
        	  return false;
          }

          virtual bool GetLastProfile(void){
        	  return false;
          }

          virtual bool GetProfileByAddr(int ind, unsigned int address, string needdate, bool withopen = true, bool withclose = true){
        	  return false;
          }

          virtual bool GetStartOfLastDayEnergy(void)
          {
        	  return false;
          }

          virtual void GetType(Byte *Buffer, Word len = 0 ) 							= 0;

          virtual void GetEisp(Byte *Buffer, Word len = 0 ) {

          }


          virtual void GetEa(Byte *Buffer, Word len = 0 ) 								= 0;
          virtual void GetEa_minus(Byte *Buffer, Word len = 0 ) 						= 0;
          virtual void GetEr(Byte *Buffer, Word len = 0 ) 								= 0;
          virtual void GetEr_minus(Byte *Buffer, Word len = 0 ) 						= 0;

          virtual float GetEt(int tindex, Byte etype, Byte *Buffer, Word len = 0) 		= 0;
          virtual float GetEt_minus(int tindex,Byte etype, Byte *Buffer, Word len = 0) 	= 0;

          virtual void GetUa(Byte *Buffer, Word len = 0 ){
        	  if(len > 0){

        	  }
          }
          virtual void GetUb(Byte *Buffer, Word len = 0 ){
        	  if(len > 0){

        	  }
          }
          virtual void GetUc(Byte *Buffer, Word len = 0 ){
        	  if(len > 0){

        	  }
          }

          virtual void GetIa(Byte *Buffer, Word len = 0 ){
          }
          virtual void GetIb(Byte *Buffer, Word len = 0 ){
          }
          virtual void GetIc(Byte *Buffer, Word len = 0 ){
          }

          virtual void GetP(Byte *Buffer, Word len = 0 ){
        	 // Log::DEBUG("GetP Pa="+toString(P.Pa.Value)+" Pb="+toString(P.Pb.Value)+" Pc="+toString(P.Pc.Value));
        	  P.P.CalcMinAverMax(P.Pa.Value + P.Pb.Value + P.Pc.Value);
          }
          virtual void GetPa(Byte *Buffer, Word len = 0 ){
          }
          virtual void GetPb(Byte *Buffer, Word len = 0 ){

          }
          virtual void GetPc(Byte *Buffer, Word len = 0 ){
          }

          virtual void GetQ(Byte *Buffer, Word len = 0 ){
        	  Q.Q.CalcMinAverMax(Q.Qa.Value + Q.Qb.Value + Q.Qc.Value);
          }
          virtual void GetQa(Byte *Buffer, Word len = 0 ){
          }
          virtual void GetQb(Byte *Buffer, Word len = 0 ){

          }
          virtual void GetQc(Byte *Buffer, Word len = 0 ){
          }

          virtual void GetS(Byte *Buffer, Word len = 0 ){
        	  S.S.CalcMinAverMax(S.Sa.Value + S.Sb.Value + S.Sc.Value);
          }
          virtual void GetSa(Byte *Buffer, Word len = 0 ){
        	  S.Sa.CalcMinAverMax(GetSPower(P.Pa.Value, Q.Qa.Value));
          }
          virtual void GetSb(Byte *Buffer, Word len = 0 ){
        	  S.Sb.CalcMinAverMax(GetSPower(P.Pb.Value, Q.Qb.Value));
          }
          virtual void GetSc(Byte *Buffer, Word len = 0 ){
        	  S.Sc.CalcMinAverMax(GetSPower(P.Pc.Value, Q.Qc.Value));
          }

          virtual float GetKM(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }
          virtual float GetKMa(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }
          virtual float GetKMb(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }
          virtual float GetKMc(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }

          virtual float GetF(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }
          virtual Word GetKtranI(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }
          virtual Word GetKtranU(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }

          virtual Word GetNetAddr(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }

          virtual string GetSerial(Byte *Buffer, Word len = 0 ){
        	  return 0;
          }

          virtual bool GetIsQ()
          {
        	  return isQ;
          }


          virtual bool SetNewEcmd(TRtcTime &dt, EcounterCmdParameter curr)
          {
        	  return false;
          }

          virtual string GetEcmd(TEnergyCMD* ecmd, Byte *Buffer, Word len, TReturnValue *r=NULL)
          {
        	  string ret = "";
        	  return ret;
          }

          virtual void SetOrigAddr(void)
          {

          }

          virtual bool HasProfile(void)
          {
        	  return false;
          }


          Byte          Et;
          Byte          En;

          TFaseEnergy       E;
          TFaseEnergy       E1;
          TFaseEnergy       E2;
          TFaseEnergy       E3;
          TFaseEnergy       E4;
          TFaseEnergy       E5;

          TVoltage      U;
          TCurrent      I;
          TPower        P;
          TRpower       Q;
          TSpower       S;
          TCoefficient  KM;
          TFrequency    F;
          Word			KtranI;
          Word			KtranU;
          string        LastTime;

          TSerialN      SerialN;
          TCounterType  CouterType;
          string		Poverka;
          DWord         ProfilCounter;

          bool          Modify;
          bool          IsEnable;
          Byte          Type;
          Word 			BetweenTimeout;

          IPort        *cntPort;

          DWord			AckCnt;
          DWord			ErrCnt;

          bool      hexPassword;
          int 		plc;
          string 	cmd;

          int timezone;
          int index;

          sync_deque<TEnergyCMD*> 	ECMDMessages;

          bool			NeedFirstFlg;

        protected:
          float GetSPower( float P, float Q){
			  return sqrt( pow(P, 2) + pow(Q, 2) );
			}

	      Byte          Addr;
          CommSettings  comm_settings;
          Word          Index;
          bool 			isQ;



        private:

        };
      //=============================================================================
      class TMercury230Counter:public ICounter
        {
          //friend class TEnergyCounterManager;
        public:
          TMercury230Counter(Byte Ename, string adr);
         ~TMercury230Counter();

          Word 		CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL);
          Word 		GetRecvMessageLen(Byte CMD);
          sWord  	RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen);
          string  	GetStringValue(void);
          Word      GetErrorMessageLen(Byte CMD){
                  	  return 4;
                    }

          bool SetNewEcmd(TRtcTime &dt,  EcounterCmdParameter curr);
          bool GetDateTime(void);
          bool CorrectDateTime(string time);
          bool SetDateTime(string datetime, string password);

          bool GetLastProfile(void);
          bool GetProfileByAddr(int ind, unsigned int address, string needdate, bool withopen = true, bool withclose = true);

          bool GetStartOfLastDayEnergy(void);

          void GetEisp(Byte *Buffer, Word len = 0 );

          void GetType(Byte *Buffer, Word len = 0 );
          void GetEa(Byte *Buffer, Word len = 0);
          void GetEa_minus(Byte *Buffer, Word len = 0 );
          void GetEr(Byte *Buffer, Word len = 0);
          void GetEr_minus(Byte *Buffer, Word len = 0);
          float GetEt(int tindex, Byte etype, Byte *Buffer, Word len = 0);
          float GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len = 0);


          void GetUa(Byte *Buffer, Word len = 0);
          void GetUb(Byte *Buffer, Word len = 0);
          void GetUc(Byte *Buffer, Word len = 0);

          void GetIa(Byte *Buffer, Word len = 0);
          void GetIb(Byte *Buffer, Word len = 0);
          void GetIc(Byte *Buffer, Word len = 0);

          void GetP(Byte *Buffer, Word len = 0);
          void GetPa(Byte *Buffer, Word len = 0);
          void GetPb(Byte *Buffer, Word len = 0);
          void GetPc(Byte *Buffer, Word len = 0);

          void GetQ(Byte *Buffer, Word len = 0);
          void GetQa(Byte *Buffer, Word len = 0);
          void GetQb(Byte *Buffer, Word len = 0);
          void GetQc(Byte *Buffer, Word len = 0);

          void GetS(Byte *Buffer, Word len = 0);
          void GetSa(Byte *Buffer, Word len = 0);
          void GetSb(Byte *Buffer, Word len = 0);
          void GetSc(Byte *Buffer, Word len = 0);

          float GetKM(Byte *Buffer, Word len = 0);
          float GetKMa(Byte *Buffer, Word len = 0);
          float GetKMb(Byte *Buffer, Word len = 0);
          float GetKMc(Byte *Buffer, Word len = 0);

          float GetF(Byte *Buffer, Word len = 0);
          Word GetKtranI(Byte *Buffer, Word len = 0);
          Word GetKtranU(Byte *Buffer, Word len = 0);

          Word GetNetAddr(Byte *Buffer, Word len = 0);

          string GetSerial(Byte *Buffer, Word len = 0);
          string GetEcmd(TEnergyCMD* ecmd, Byte *Buffer, Word len, TReturnValue *r=NULL);

          void SetOrigAddr(void)
          {
        	  if(Addr != OrigAddr){
        		  Addr = OrigAddr;
        	  }
          }

          bool HasProfile(void)
          {
				string ctype = this->CouterType.Type;
				size_t pos = ctype.find("P");

				return pos == string::npos ? false : true;
          }


        protected:
        private:
          Byte OrigAddr;

        };

      //=============================================================================
      class TPschCounter:public ICounter
      {
          //friend class TEnergyCounterManager;
        public:
          TPschCounter(Byte Ename, string adr);
         ~TPschCounter();

          Word 		CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL);
          Word 		GetRecvMessageLen(Byte CMD);
          sWord  	RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen);
          string  	GetStringValue(void);
          Word      GetErrorMessageLen(Byte CMD){
                  	  return 4;
                    }

          bool SetNewEcmd(TRtcTime &dt,  EcounterCmdParameter curr);
          bool GetDateTime(void);
          bool CorrectDateTime(string time);
          bool SetDateTime(string datetime, string password);

          bool GetLastProfile(void);
          //bool GetProfileByAddr(int ind, unsigned int address, string needdate, bool withopen = true, bool withclose = true);

          void GetType(Byte *Buffer, Word len = 0 );
          void GetEa(Byte *Buffer, Word len = 0);
          void GetEa_minus(Byte *Buffer, Word len = 0 );
          void GetEr(Byte *Buffer, Word len = 0 );
          void GetEr_minus(Byte *Buffer, Word len = 0 );

          float GetEt(int tindex, Byte etype, Byte *Buffer, Word len = 0);
          float GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len = 0);

          void GetUa(Byte *Buffer, Word len = 0);
          void GetUb(Byte *Buffer, Word len = 0);
          void GetUc(Byte *Buffer, Word len = 0);

          void GetIa(Byte *Buffer, Word len = 0);
          void GetIb(Byte *Buffer, Word len = 0);
          void GetIc(Byte *Buffer, Word len = 0);

          void GetP(Byte *Buffer, Word len = 0);
          void GetPa(Byte *Buffer, Word len = 0);
          void GetPb(Byte *Buffer, Word len = 0);
          void GetPc(Byte *Buffer, Word len = 0);

          void GetQ(Byte *Buffer, Word len = 0);
          void GetQa(Byte *Buffer, Word len = 0);
          void GetQb(Byte *Buffer, Word len = 0);
          void GetQc(Byte *Buffer, Word len = 0);

          void GetS(Byte *Buffer, Word len = 0);
          void GetSa(Byte *Buffer, Word len = 0);
          void GetSb(Byte *Buffer, Word len = 0);
          void GetSc(Byte *Buffer, Word len = 0);

          float GetKM(Byte *Buffer, Word len = 0);
          float GetKMa(Byte *Buffer, Word len = 0);
          float GetKMb(Byte *Buffer, Word len = 0);
          float GetKMc(Byte *Buffer, Word len = 0);

          float GetF(Byte *Buffer, Word len = 0);
          string GetSerial(Byte *Buffer, Word len = 0);
          string GetEcmd(TEnergyCMD* ecmd, Byte *Buffer, Word len, TReturnValue *r=NULL);

          Word GetKtranI(Byte *Buffer, Word len = 0);
          Word GetKtranU(Byte *Buffer, Word len = 0);

          Word GetNetAddr(Byte *Buffer, Word len = 0);

          void SetOrigAddr(void)
          {
        	  if(Addr != OrigAddr){
        		  Addr = OrigAddr;
        	  }
          }

          bool HasProfile(void)
          {
				string ctype = this->CouterType.Type;
				size_t pos = ctype.find("P");

				return pos == string::npos ? false : true;
          }


        protected:
        private:
          Byte OrigAddr;
      };

      //=============================================================================
       class TMercury200Counter:public ICounter
         {
           //friend class TEnergyCounterManager;
         public:
    	  TMercury200Counter(Byte Ename, string adr);
          ~TMercury200Counter();

           Word   CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL);
           Word   GetRecvMessageLen(Byte CMD);
           sWord  RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen);
           string GetStringValue(void);

           void GetType(Byte *Buffer, Word len = 0 );
           void GetEa(Byte *Buffer, Word len = 0);
           void GetEa_minus(Byte *Buffer, Word len = 0 );
           void GetEr(Byte *Buffer, Word len = 0 );
           void GetEr_minus(Byte *Buffer, Word len = 0 );

           float GetEt(int tindex, Byte etype, Byte *Buffer, Word len = 0);
           float GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len = 0);

           void GetUa(Byte *Buffer, Word len = 0);
           void GetP(Byte *Buffer, Word len = 0);

           string GetSerial(Byte *Buffer, Word len = 0);

         protected:

           DWord Address;
           vector<TFaseEnergy *> Emass;

         private:

         };

      //=============================================================================
      class TEnergomeraCE:public ICounter
        {
        public:
           TEnergomeraCE(Byte Ename, string adr);
          ~TEnergomeraCE();

          Word CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL);
          Word GetRecvMessageLen(Byte CMD);
          sWord RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen);
          bool  ParsingAnswer( Byte *BUF, Word Len, Byte subFase );
          string  GetStringValue(void);

          //bool SetNewEcmd(TRtcTime &dt,  EcounterCmdParameter curr);
          bool GetDateTime(void);
          bool CorrectDateTime(string time);
          bool SetDateTime(string datetime, string password);

          void GetType(Byte *Buffer, Word len = 0 );
          void GetEisp(Byte *Buffer, Word len = 0 );


          void GetEa(Byte *Buffer, Word len = 0);
          void GetEa_minus(Byte *Buffer, Word len = 0 );
          void GetEr(Byte *Buffer, Word len = 0);
          void GetEr_minus(Byte *Buffer, Word len = 0);
          float GetEt(int tindex, Byte etype,Byte *Buffer, Word len = 0);
          float GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len = 0);

          void GetUa(Byte *Buffer, Word len = 0);
          void GetUb(Byte *Buffer, Word len = 0);
          void GetUc(Byte *Buffer, Word len = 0);

          void GetIa(Byte *Buffer, Word len = 0);
          void GetIb(Byte *Buffer, Word len = 0);
          void GetIc(Byte *Buffer, Word len = 0);

          void GetPa(Byte *Buffer, Word len = 0);
          void GetPb(Byte *Buffer, Word len = 0);
          void GetPc(Byte *Buffer, Word len = 0);

          void GetQa(Byte *Buffer, Word len = 0);
          void GetQb(Byte *Buffer, Word len = 0);
          void GetQc(Byte *Buffer, Word len = 0);

          float GetKM(Byte *Buffer, Word len = 0);
          float GetKMa(Byte *Buffer, Word len = 0);
          float GetKMb(Byte *Buffer, Word len = 0);
          float GetKMc(Byte *Buffer, Word len = 0);

          float GetF(Byte *Buffer, Word len = 0);
          string GetSerial(Byte *Buffer, Word len = 0);

          string GetEcmd(TEnergyCMD* ecmd, Byte *Buffer, Word len, TReturnValue *r=NULL);

        protected:
        private:

          Byte 				CalcBCC(Byte *BUF, Word Len );
          vector<string>    GetDataFromMessage( Byte *BUF, Word Len );
          string            Address;

          string 			LastCmd;

          TBuffer		   *LastCmdPointer;


          string DateTime;

        };
      //=============================================================================
      class TNevaMT:public ICounter
        {
        public:
    	  TNevaMT(Byte Ename, string adr);
          ~TNevaMT();

          Word 		CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL);
          Word 		GetRecvMessageLen(Byte CMD);
          sWord 	RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen);
          bool  	ParsingAnswer( Byte *BUF, Word Len, Byte subFase );
          string  	GetStringValue(void);

          void 		GetType(Byte *Buffer, Word len = 0 );
          void 		GetAdditionalData(Byte *Buffer, Word len = 0 );

          void 		GetEa(Byte *Buffer, Word len = 0);
          void 		GetEa_minus(Byte *Buffer, Word len = 0 );
          void 		GetEr(Byte *Buffer, Word len = 0);
          void 		GetEr_minus(Byte *Buffer, Word len = 0);
          float 	GetEt(int tindex, Byte etype,Byte *Buffer, Word len = 0);
          float 	GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len = 0);

          void GetUa(Byte *Buffer, Word len = 0);
          void GetUb(Byte *Buffer, Word len = 0);
          void GetUc(Byte *Buffer, Word len = 0);

          void GetIa(Byte *Buffer, Word len = 0);
          void GetIb(Byte *Buffer, Word len = 0);
          void GetIc(Byte *Buffer, Word len = 0);

          void GetPa(Byte *Buffer, Word len = 0);
          void GetPb(Byte *Buffer, Word len = 0);
          void GetPc(Byte *Buffer, Word len = 0);

          void GetQa(Byte *Buffer, Word len = 0);
          void GetQb(Byte *Buffer, Word len = 0);
          void GetQc(Byte *Buffer, Word len = 0);

          float GetKM(Byte *Buffer, Word len = 0);
          float GetKMa(Byte *Buffer, Word len = 0);
          float GetKMb(Byte *Buffer, Word len = 0);
          float GetKMc(Byte *Buffer, Word len = 0);

          float GetF(Byte *Buffer, Word len = 0);
          string GetSerial(Byte *Buffer, Word len = 0);

        protected:
        private:

          Byte 				CalcBCC(Byte *BUF, Word Len );
          vector<string>    GetDataFromMessage( Byte *BUF, Word Len );
          string            Address;

          string 			LastCmd;

          TBuffer		   *LastCmdPointer;


        };
      //=============================================================================
       class TChintDts:public ICounter
         {
         public:
    	  TChintDts(Byte Ename, string adr);
           ~TChintDts();

           Word CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL);
           Word GetRecvMessageLen(Byte CMD);
           sWord RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen);
           bool  ParsingAnswer( Byte *BUF, Word Len, Byte subFase );
           string  GetStringValue(void);

           void GetAdditionalData(Byte *Buffer, Word len = 0 );
           void GetType(Byte *Buffer, Word len = 0 );

           void GetEa(Byte *Buffer, Word len = 0);
           void GetEa_minus(Byte *Buffer, Word len = 0 );
           void GetEr(Byte *Buffer, Word len = 0);
           void GetEr_minus(Byte *Buffer, Word len = 0);
           float GetEt(int tindex, Byte etype, Byte *Buffer, Word len = 0);
           float GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len = 0);

           void GetUa(Byte *Buffer, Word len = 0);
           void GetUb(Byte *Buffer, Word len = 0);
           void GetUc(Byte *Buffer, Word len = 0);

           void GetIa(Byte *Buffer, Word len = 0);
           void GetIb(Byte *Buffer, Word len = 0);
           void GetIc(Byte *Buffer, Word len = 0);

           void GetPa(Byte *Buffer, Word len = 0);
           void GetPb(Byte *Buffer, Word len = 0);
           void GetPc(Byte *Buffer, Word len = 0);

           void GetQa(Byte *Buffer, Word len = 0);
           void GetQb(Byte *Buffer, Word len = 0);
           void GetQc(Byte *Buffer, Word len = 0);

           float GetKM(Byte *Buffer, Word len = 0);
           float GetKMa(Byte *Buffer, Word len = 0);
           float GetKMb(Byte *Buffer, Word len = 0);
           float GetKMc(Byte *Buffer, Word len = 0);

           float GetF(Byte *Buffer, Word len = 0);
           string GetSerial(Byte *Buffer, Word len = 0);

         protected:
         private:

           Byte 				CalcBCC(Byte *BUF, Word Len );
           vector<string>       GetDataFromMessage( Byte *BUF, Word Len );
           string               Address;
         };

       //=============================================================================
        class THuabangDts:public ICounter
          {
          public:
    	   THuabangDts(Byte Ename, string adr);
            ~THuabangDts();

            Word CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL);
            Word GetRecvMessageLen(Byte CMD);
            sWord RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen);
            bool  ParsingAnswer( Byte *BUF, Word Len, Byte subFase );
            string  GetStringValue(void);

            void GetAdditionalData(Byte *Buffer, Word len = 0 );
            void GetType(Byte *Buffer, Word len = 0 );

            void GetEa(Byte *Buffer, Word len = 0);
            void GetEa_minus(Byte *Buffer, Word len = 0 );
            void GetEr(Byte *Buffer, Word len = 0);
            void GetEr_minus(Byte *Buffer, Word len = 0);
            float GetEt(int tindex, Byte etype, Byte *Buffer, Word len = 0);
            float GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len = 0);

            string GetSerial(Byte *Buffer, Word len = 0);

          protected:
          private:

            Byte 				CalcBCC(Byte *BUF, Word Len );
            string       GetDataFromMessage( Byte *BUF, Word Len );
            string               Address;
          };
        //=============================================================================
        class THuabang2Dts:public ICounter
           {
           public:
        	THuabang2Dts(Byte Ename, string adr);
             ~THuabang2Dts();

             Word CreateCMD(Byte CMD, Byte *Buffer, TEnergyCMD *cmd= NULL);
             Word GetRecvMessageLen(Byte CMD);
             sWord RecvData(IPort* Port, Byte *Buf, sWord Len, Word MaxLen, Byte subfase, sWord &refLen);
             bool  ParsingAnswer( Byte *BUF, Word Len, Byte subFase );
             string  GetStringValue(void);

             void GetAdditionalData(Byte *Buffer, Word len = 0 );
             void GetType(Byte *Buffer, Word len = 0 );

             void GetEa(Byte *Buffer, Word len = 0);
             void GetEa_minus(Byte *Buffer, Word len = 0 );
             void GetEr(Byte *Buffer, Word len = 0);
             void GetEr_minus(Byte *Buffer, Word len = 0);
             float GetEt(int tindex, Byte etype, Byte *Buffer, Word len = 0);
             float GetEt_minus(int tindex, Byte etype, Byte *Buffer, Word len = 0);

             void GetUa(Byte *Buffer, Word len = 0);
             void GetUb(Byte *Buffer, Word len = 0);
             void GetUc(Byte *Buffer, Word len = 0);

             void GetIa(Byte *Buffer, Word len = 0);
             void GetIb(Byte *Buffer, Word len = 0);
             void GetIc(Byte *Buffer, Word len = 0);

             void GetPa(Byte *Buffer, Word len = 0);
             void GetPb(Byte *Buffer, Word len = 0);
             void GetPc(Byte *Buffer, Word len = 0);

             void GetQa(Byte *Buffer, Word len = 0);
             void GetQb(Byte *Buffer, Word len = 0);
             void GetQc(Byte *Buffer, Word len = 0);

             float GetKM(Byte *Buffer, Word len = 0);
             float GetKMa(Byte *Buffer, Word len = 0);
             float GetKMb(Byte *Buffer, Word len = 0);
             float GetKMc(Byte *Buffer, Word len = 0);

             float GetF(Byte *Buffer, Word len = 0);
             string GetSerial(Byte *Buffer, Word len = 0);

           protected:
           private:

             Byte 				CalcBCC(Byte *BUF, Word Len );
             vector<string>       GetDataFromMessage( Byte *BUF, Word Len );
             string               Address;
           };
        //=============================================================================


      class TEnergyCounterManager:public TFastTimer, public IManager
        {
        public:
          TEnergyCounterManager();
          ~TEnergyCounterManager();

          void Init( void *config, void *f );// , TFlashEnergyCounters const* flashEnergyCounters);

          void ENERGY_CreateCMD();
          void ENERGY_SendCMD();
          void ENERGY_RecvCMD();
          void ENERGY_ExecCMD();
          void ENERGY_DoECMD(void *Par = NULL);

          void ENERGY_DetectStates(void);
          void ENERGY_CreateMessageCMD(void *Par = NULL);
          void ENERGY_ChangeCounter(void)
           {
			   int size = Counters.size();
			   if( size > 0)
				{
				   Log::DEBUG("TEnergyCounterManager ENERGY_ChangeCounter CounterIndex="+toString(CounterIndex));
					  if( ++CounterIndex >=  size)//progon po ostalnim t.k. dalee
					  { CounterIndex = 0; }
					  ICounter *CurrController = GetSimpleCounter(CounterIndex);//new
					  if(CurrController != NULL){
						  if( CurrController->cntPort != Port ){
							  if(Port != NULL)
								  Port->Close();
							  Port = CurrController->cntPort;//change
						  }
					  }
				}
         		SubFase = OPEN_CHANNAL;
         		Fase    = CREATE_CMD;
           }

          string    CreateCfgString(void);

          void 		ExeCmd(Byte et, Byte en, int cmd, vector<string> params, void* p3 = 0);
          void      SetNewEcmd(vector<EcounterCmdParameter> cmdList);
          void 		SetGetProfilCmd(int daysbefore, void* p);
          bool 		CheckProfil(int day);

          bool 		Enable(void);

          Byte      GetFase();
          void      SetFase(Byte nfase);

          bool                FirstInitFlg;

          IPort	*GetPort()
          {
        	  IPort*ret = NULL;
        	  pthread_mutex_lock(&sych);
        	  ret = Port;
        	  pthread_mutex_unlock(&sych);
        	  return ret;
          }

          string GetSelfTestData(int et, int en){
        	  string ret = "";
        	  for(auto curr: Counters){
        		  if(curr->Et == et && curr->En==en)
    			  {
    				  ret += "{";
    				  ret += "\"Et\":"+toString((int)curr->Et)+",";
    				  ret += "\"En\":"+toString((int)curr->En)+",";
    				  ret += "\"Address\":"+toString((int)curr->Addr)+",";
    				  ret += "\"Value\":\""+toString(curr->E.Ea)+"\""+",";
    				  ret += "\"Type\":\""+curr->CouterType.Type+"\""+",";
    				  ret += "\"Number\":\""+curr->SerialN.SerialNumber+"\""+",";
    				  ret += "\"IsEnable\":"+toString(curr->IsEnable)+",";
    				  ret += "\"AckCnt\":"+toString((int)curr->AckCnt)+",";
    				  ret += "\"ErrCnt\":"+toString((int)curr->ErrCnt);
    				  ret+="},";
    			  }
        	  }
        	  return ret;
          }

          void Stop(void)
          {
        	  for(auto curr: Counters)
        	  {
        		 if(curr!=NULL)
        			 curr->cntPort->Close();
        	  }
          }

          TReturnValue GetReturnValue()
          {
			TReturnValue rval;
			ReturnValue = &rval;
			for(int ind = 0; ind < 60; ind++)//20
			{
				if( rval.IsComplite() )
				{
					break;
				}
				sleep(1);
			}
			ReturnValue = NULL;
			return rval;
          }

          string ApiAddress;
          long   ApiPort;
          string ObjectId;

        private:

          HTTPClient HttpClient;

          string ParseCmdString(string cmd){
        	  string CRLF = "[CRLF]";
        	  cmd = replaceAll(cmd, CRLF, "\r\n");
			return cmd;
          }


          TReturnValue 		*ReturnValue;
          DWord           	ModifyTime;

          Byte                NewFase;
          Byte                Fase;
          ICounter* GetSimpleCounter( Word _index );


          TEnergyCMD          		*CurrentECMD;

          TRtcTime            DateTime;
          Word                CounterIndex;
          vector<ICounter*>   Counters;   ////new
          TFifo<string> *fd;

          pthread_mutex_t 		sych;

          IPort *Port;
          Byte Buffer[500];
          Word DataLen;
          Byte SubFase;
          Word Period;

          bool needEcmd;
          bool busyEcmd;


        };

#endif /* ECOUNTER_H_ */

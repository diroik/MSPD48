/*
 * ModbusRTU.h
 *
 *  Created on: May 7, 2015
 *      Author: user
 */

#ifndef MODBUSRTU_H_
#define MODBUSRTU_H_

#include <Log.h>
#include <Classes.h>

#include <modbus-rtu.h>
#include <modbus-tcp.h>

using namespace std;
using namespace log4net;
using namespace MyClassTamlates;
//=============================================================================
//=============================================================================
class ModbusRegCmd
{
public:
	ModbusRegCmd(int a,int f, int r)
	{
		addr = a;
		reg  = r;
		fun  = f;
	}

	int addr;
	int fun;
	int reg;
};
//=============================================================================
class ModbusRegPar
{
public:
	ModbusRegPar(int a,int r,int f, int d)
	{
		addr = a;
		reg  = r;
		fun  = f;
		data = d;
	}

	int data;
	int addr;
	int reg;
	int fun;
};
//=============================================================================
class IModbusRegisterList
{
public:

	virtual ~IModbusRegisterList()
	{
		/*
		Log::INFO( "[IModbusRegisterList] destructor object: " +
				string("addr=") + toString(addr) + string("  ") +
				string("reg=")+toString(reg) + string("  ") + string("len=")+toString(len));*/
	}

	virtual bool IsFree()
	{
		return _ctx == NULL;
	}

	virtual bool DoCmd() = 0;

	virtual void* GetData() = 0;

	virtual ModbusRegPar* FindParameter(int regaddr) = 0;

	virtual string GetCfgString()
	{
		string ret = "[addr="+toString(addr)+" reg="+toString(reg)+" fun="+toString(fun)+" len="+toString(len)+"]";

		//Log::INFO( "[IModbusRegisterList] object is: [" +ret + "]");
		return ret;
	}

	virtual bool NeedDelete(){
		return needDelete;
	}

	int addr;
	int reg;
	int fun;
	int len;//val

	void        *outPtr;
	//bool needDelete;
	//bool doDelete;

protected:

	bool needDelete;
	modbus_t*	 	_ctx;
	CommSettings 	_sett;
	MODBUS_TYPE 	_type;
};

class ModbusHoldingRegistersList:public IModbusRegisterList
{
public:
	ModbusHoldingRegistersList(CommSettings sett, int a, int r, int l, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = false;
		_ctx 		= NULL;
		_sett	 	= sett;
		addr 		= a;
		reg  		= r;
		len 		= l;
		fun         = 3;
		_type		= type;
		outPtr = out;


		for(int i=0;i<len;i++)
			data.push_back(0);
	}

	ModbusHoldingRegistersList(int a, int r, int l, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = false;
		//doDelete   = false;
		_ctx 		= NULL;
		addr 		= a;
		reg  		= r;
		len 		= l;
		fun         = 3;
		_type		= type;
		outPtr = out;
		for(int i=0;i<len;i++)
			data.push_back(0);
	}

	bool DoCmd()
	{
		bool ret = false;
		uint16_t *tab_reg = new uint16_t[data.size()];
		try
		{
			//Log::DEBUG("ModbusDataRegisterList DoCmd start");

			if(_type == MODBUS_TCP)
			{
				//Log::DEBUG("ModbusDataRegisterList DoCmd MODBUS_TCP");
				_ctx = modbus_new_tcp(_sett.DeviceName.c_str(),_sett.BaudRate);//ip,port
			}
			else//RTU
			{
				_ctx = modbus_new_rtu(_sett.DeviceName.c_str(),_sett.BaudRate,_sett.Parity,_sett.DataBits,_sett.StopBit);//("/dev/ttyAPP2", 115200, 'N', 8, 1);
				modbus_rtu_set_serial_mode(_ctx, MODBUS_RTU_RS485);
				modbus_rtu_set_rts(_ctx, MODBUS_RTU_RTS_NONE);
			}
			modbus_set_slave(_ctx, addr);

			uint32_t sec = _sett.RecvTimeout/1000;
			uint32_t usec = (_sett.RecvTimeout%1000)*1000;
			modbus_set_response_timeout(_ctx, sec, usec);

			uint32_t bsec = _sett.SendTimeout/1000;
			uint32_t busec = (_sett.SendTimeout%1000)*1000;
			modbus_set_byte_timeout(_ctx, bsec, busec);

			_ctx->debug = 0;
			//Log::INFO( "[ModbusHoldingRegistersList] DoCmd: " + string("modbus_connect to ") + _sett.DeviceName +":"+ toString(_sett.BaudRate) );
			if (modbus_connect(_ctx) != -1)
			{
				//Log::DEBUG( "[ModbusDataRegisterList] modbus_read_registers addr=" + toString(addr) + " reg="+toString(reg) + " len="+toString(len) );
				int rc = modbus_read_registers(_ctx, reg, len, tab_reg);
				//Log::DEBUG( "[ModbusDataRegisterList] read data modbus_read_registers addr=" + toString(addr) + " reg="+toString(reg) + " size=" + toString(rc) );
				if (rc != -1)
				{
					for(int i=0;i<data.size();i++)
					{
						data[i]=tab_reg[i];
						//Log::DEBUG(  toString(reg+i) + string("=") + toString( (short)data[i] ));
					}
					//short sss = data[1];
					//Log::INFO("toString="+toString(sss));
					//Log::INFO("IntToString="+IntToString(sss));
					ret = true;
				}
			}
			modbus_close(_ctx);
		}
		catch(exception &e)
		{
			Log::ERROR( "[ModbusDataRegisterList] " + string( e.what() ) );
		}
		delete[] tab_reg;
		modbus_free(_ctx);
		_ctx = NULL;

		//Log::INFO( "[ModbusDataRegisterList] DoCmd end");
		return ret;
	}

	void* GetData()
	{
		return &data;
	}

	ModbusRegPar* FindParameter(int regaddr)
	{
		//Log::DEBUG("FindParameter for ["+GetCfgString()+string("] regaddr="+toString(regaddr)));
		ModbusRegPar* ret = NULL;
		if( regaddr >= reg && regaddr <= (reg+len) )
		{
			int regindex = regaddr - reg;
			//Log::DEBUG("FindParameter regindex="+toString(regindex) + string("data.size()=")+toString(data.size()));
			if(regindex < data.size())
			{
				int datavalue = data[regindex];
				ret = new ModbusRegPar(addr, regaddr, fun, datavalue);
			}
		}
		return ret;
	}

	vector<short> data;

private:
};

class ModbusInputRegistersList:public IModbusRegisterList
{
public:
	ModbusInputRegistersList(CommSettings sett, int a, int r, int l, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = false;
		//doDelete   = false;
		_ctx 		= NULL;
		_sett	 	= sett;
		addr 		= a;
		reg  		= r;
		len 		= l;
		fun         = 4;
		_type		= type;
		outPtr = out;
		for(int i=0;i<len;i++)
		{
			data.push_back(0);
		}
	}

	ModbusInputRegistersList(int a, int r, int l, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = false;
		//doDelete   = false;
		_ctx 		= NULL;
		addr 		= a;
		reg  		= r;
		len 		= l;
		fun         = 4;
		_type		= type;
		outPtr = out;
		for(int i=0;i<len;i++)
		{
			data.push_back(0);
		}
	}

	bool DoCmd()
	{
		bool ret=false;
		uint16_t *tab_reg = new uint16_t[data.size()];
		try
		{
			//Log::DEBUG("modbus_new_rtu");

			if(_type == MODBUS_TCP)
			{
				_ctx = modbus_new_tcp(_sett.DeviceName.c_str(),_sett.BaudRate);//ip,port
			}
			else
			{
				_ctx = modbus_new_rtu(	_sett.DeviceName.c_str(),
										_sett.BaudRate,
										_sett.Parity,
										_sett.DataBits,
										_sett.StopBit);//("/dev/ttyAPP2", 115200, 'N', 8, 1);

				modbus_rtu_set_serial_mode(_ctx, MODBUS_RTU_RS485);
				modbus_rtu_set_rts(_ctx, MODBUS_RTU_RTS_NONE);
			}
			modbus_set_slave(_ctx, addr);

			uint32_t sec = _sett.RecvTimeout/1000;
			uint32_t usec = (_sett.RecvTimeout%1000)*1000;
			modbus_set_response_timeout(_ctx, sec, usec);

			uint32_t bsec = _sett.SendTimeout/1000;
			uint32_t busec = (_sett.SendTimeout%1000)*1000;
			modbus_set_byte_timeout(_ctx, bsec, busec);
			//Log::INFO( "[ModbusInputRegistersList] " + string("modbus_connect") );
			if (modbus_connect(_ctx) != -1) {
				//Log::INFO( "[ModbusInputRegistersList] " + string("modbus_connect successfull") );
				int rc = modbus_read_input_registers(_ctx, reg, len, tab_reg);
				//Log::INFO( "[ModbusInputRegistersList] " + string("read data registers size=") + toString(rc) );
				if (rc != -1)
				{
					for(int i=0;i<data.size();i++)
					{
						data[i]=tab_reg[i];
						//Log::INFO(  toString(reg+i) + string("=") + toString( (short)data[i] ));
					}
					//short sss = data[1];
					//Log::INFO("toString="+toString(sss));
					//Log::INFO("IntToString="+IntToString(sss));
					//Log::INFO( "[ModbusDataRegisterList] " + string("modbus_read_registers sucecsfull") );
					ret = true;
				}
			}
			modbus_close(_ctx);
		}
		catch(exception &e)
		{
			Log::ERROR( "[ModbusDataRegisterList] " + string( e.what() ) );
		}
		delete[] tab_reg;
		modbus_free(_ctx);
		_ctx = NULL;
		return ret;
	}

	void* GetData()
	{
		return &data;
	}

	ModbusRegPar* FindParameter(int regaddr)
	{
		//Log::DEBUG("FindParameter for ["+GetCfgString()+string("] regaddr="+toString(regaddr)));
		ModbusRegPar* ret = NULL;
		if( regaddr >= reg && regaddr <= (reg+len) )
		{
			int regindex = regaddr - reg;
			//Log::DEBUG("FindParameter regindex="+toString(regindex) + string("data.size()=")+toString(data.size()));
			if(regindex < data.size())
			{
				int datavalue = data[regindex];
				ret = new ModbusRegPar(addr, regaddr, fun, datavalue);
			}
		}
		return ret;
	}

	vector<short> data;

private:
};

class ModbusCoilStatusRegistersList:public IModbusRegisterList
{
public:
	ModbusCoilStatusRegistersList(CommSettings sett, int a, int r, int l, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = false;
		//doDelete   = false;
		_ctx 		= NULL;
		_sett	 	= sett;
		addr 		= a;
		reg  		= r;
		len 		= l;
		fun         = 1;
		_type		= type;
		outPtr = out;
		for(int i=0;i<len;i++)
		{
			data.push_back(0);
		}
	}

	ModbusCoilStatusRegistersList(int a, int r, int l, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = false;
		//doDelete   = false;
		_ctx 		= NULL;
		addr 		= a;
		reg  		= r;
		len 		= l;
		fun         = 1;
		_type		= type;
		outPtr = out;
		for(int i=0;i<len;i++)
		{
			data.push_back(0);
		}
	}

	bool DoCmd()
	{
		bool ret=false;
		uint8_t *tab_reg = new uint8_t[data.size()];
		try
		{
			//Log::DEBUG("modbus_new_rtu");

			if(_type == MODBUS_TCP)
			{
				_ctx = modbus_new_tcp(_sett.DeviceName.c_str(),_sett.BaudRate);//ip,port
			}
			else
			{
				_ctx = modbus_new_rtu(	_sett.DeviceName.c_str(),
										_sett.BaudRate,
										_sett.Parity,
										_sett.DataBits,
										_sett.StopBit);//("/dev/ttyAPP2", 115200, 'N', 8, 1);
				modbus_rtu_set_serial_mode(_ctx, MODBUS_RTU_RS485);
				modbus_rtu_set_rts(_ctx, MODBUS_RTU_RTS_NONE);
			}
			modbus_set_slave(_ctx, addr);

			uint32_t sec = _sett.RecvTimeout/1000;
			uint32_t usec = (_sett.RecvTimeout%1000)*1000;
			modbus_set_response_timeout(_ctx, sec, usec);

			uint32_t bsec = _sett.SendTimeout/1000;
			uint32_t busec = (_sett.SendTimeout%1000)*1000;
			modbus_set_byte_timeout(_ctx, bsec, busec);

			if (modbus_connect(_ctx) != -1) {
				//Log::INFO( "[ModbusDataRegisterList] " + string("modbus_connect successfull") );
				int rc = modbus_read_bits(_ctx, reg, len, tab_reg);
				//Log::INFO( "[ModbusDataRegisterList] " + string("read data registers size=") + toString(rc) );
				if (rc != -1)
				{
					for(int i=0;i<data.size();i++)
					{
						data[i]=tab_reg[i];
						//Log::INFO(  toString(reg+i) + string("=") + toString( (short)data[i] ));
					}
					//short sss = data[1];
					//Log::INFO("toString="+toString(sss));
					//Log::INFO("IntToString="+IntToString(sss));
					//Log::INFO( "[ModbusDataRegisterList] " + string("modbus_read_registers sucecsfull") );
					ret = true;
				}
			}
			modbus_close(_ctx);
		}
		catch(exception &e)
		{
			Log::ERROR( "[ModbusCoilStatusRegistersList] " + string( e.what() ) );
		}
		delete[] tab_reg;
		modbus_free(_ctx);
		_ctx = NULL;
		return ret;
	}

	void* GetData()
	{
		return &data;
	}

	ModbusRegPar* FindParameter(int regaddr)
	{
		//Log::DEBUG("FindParameter for ["+GetCfgString()+string("] regaddr="+toString(regaddr)));
		ModbusRegPar* ret = NULL;
		if( regaddr >= reg && regaddr <= (reg+len) )
		{
			int regindex = regaddr - reg;
			//Log::DEBUG("FindParameter regindex="+toString(regindex) + string("data.size()=")+toString(data.size()));
			if(regindex < data.size())
			{
				int datavalue = data[regindex];
				ret = new ModbusRegPar(addr, regaddr, fun, datavalue);
			}
		}
		return ret;
	}

	vector<Byte> data;

private:
};

class ModbusDiscreteInputsList:public IModbusRegisterList
{
public:
	ModbusDiscreteInputsList(CommSettings sett, int a, int r, int l, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = false;
		_ctx 		= NULL;
		_sett	 	= sett;
		addr 		= a;
		reg  		= r;
		len 		= l;
		fun         = 2;
		_type		= type;
		outPtr = out;
		for(int i=0; i<len; i++)
			data.push_back(0);
	}

	ModbusDiscreteInputsList(int a, int r, int l, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = false;
		_ctx 		= NULL;
		addr 		= a;
		reg  		= r;
		len 		= l;
		fun         = 2;
		_type		= type;
		outPtr = out;
		for(int i=0; i<len; i++)
			data.push_back(0);
	}

	bool DoCmd()
	{
		bool ret=false;
		uint8_t *tab_reg = new uint8_t[data.size()];
		try
		{
			if(_type == MODBUS_TCP)
			{
				_ctx = modbus_new_tcp(_sett.DeviceName.c_str(),_sett.BaudRate);//ip,port
			}
			else{
				_ctx = modbus_new_rtu(	_sett.DeviceName.c_str(),
										_sett.BaudRate,
										_sett.Parity,
										_sett.DataBits,
										_sett.StopBit);//("/dev/ttyAPP2", 115200, 'N', 8, 1);

				modbus_rtu_set_serial_mode(_ctx, MODBUS_RTU_RS485);
				modbus_rtu_set_rts(_ctx, MODBUS_RTU_RTS_NONE);
			}

			modbus_set_slave(_ctx, addr);

			uint32_t sec = _sett.RecvTimeout/1000;
			uint32_t usec = (_sett.RecvTimeout%1000)*1000;
			modbus_set_response_timeout(_ctx, sec, usec);

			uint32_t bsec = _sett.SendTimeout/1000;
			uint32_t busec = (_sett.SendTimeout%1000)*1000;
			modbus_set_byte_timeout(_ctx, bsec, busec);

			if (modbus_connect(_ctx) != -1) {
				//Log::INFO( "[ModbusDiscreteInputsList] " + string("modbus_connect successful") );
				int rc = modbus_read_input_bits(_ctx, reg, len, tab_reg);
				//Log::INFO( "[ModbusDiscreteInputsList] " + string("modbus_read_input_bits size=") + toString(rc) );
				if (rc != -1)
				{
					for(int i=0;i<data.size();i++)
					{
						data[i]=tab_reg[i];
						//for(int j=0;j<8;j++) Log::DEBUG(toString((int)(j+1 + i*8))+string("=")+toString( (int)(tab_reg[i] & 1 << j) ));
						//Log::INFO(  toString(reg+i) + string(" = ") + toString((short)data[i]));
					}
					ret = true;
				}
			}
			modbus_close(_ctx);
		}
		catch(exception &e)
		{
			Log::ERROR( "[ModbusDiscreteInputsList] " + string( e.what() ) );
		}
		delete[] tab_reg;
		modbus_free(_ctx);
		_ctx = NULL;
		return ret;
	}

	void* GetData()
	{
		return &data;
	}

	ModbusRegPar* FindParameter(int regaddr)
	{
		ModbusRegPar* ret = NULL;
		if( regaddr >= reg && regaddr <= (reg+len) )
		{
			int regindex = regaddr - reg;
			if(regindex < data.size())
			{
				int datavalue = data[regindex];
				ret = new ModbusRegPar(addr, regaddr, fun, datavalue);
			}
		}
		return ret;
	}

	vector<Byte> data;

private:
};

class ModbusWriteSingleRegister:public IModbusRegisterList
{
public:
	ModbusWriteSingleRegister(CommSettings sett, int a, int r, short newData, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = true;
		_ctx 		= NULL;
		_sett	 	= sett;
		addr 		= a;
		reg  		= r;
		data 		= newData;
		fun         = 6;
		_type		= type;
		outPtr = out;
	}

	ModbusWriteSingleRegister( int a, int r, short newData, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = true;
		_ctx 		= NULL;
		addr 		= a;
		reg  		= r;
		data 		= newData;
		fun         = 6;
		_type		= type;
		outPtr = out;
	}

	bool DoCmd()
	{
		bool ret=false;
		try
		{
			if(_type == MODBUS_TCP)
			{
				_ctx = modbus_new_tcp(_sett.DeviceName.c_str(),_sett.BaudRate);//ip,port
			}
			else{
				_ctx = modbus_new_rtu(	_sett.DeviceName.c_str(),
										_sett.BaudRate,
										_sett.Parity,
										_sett.DataBits,
										_sett.StopBit);//("/dev/ttyAPP2", 115200, 'N', 8, 1);

				modbus_rtu_set_serial_mode(_ctx, MODBUS_RTU_RS485);
				modbus_rtu_set_rts(_ctx, MODBUS_RTU_RTS_NONE);
			}

			modbus_set_slave(_ctx, addr);

			uint32_t sec = _sett.RecvTimeout/1000;
			uint32_t usec = (_sett.RecvTimeout%1000)*1000;
			modbus_set_response_timeout(_ctx, sec, usec);

			uint32_t bsec = _sett.SendTimeout/1000;
			uint32_t busec = (_sett.SendTimeout%1000)*1000;
			modbus_set_byte_timeout(_ctx, bsec, busec);

			if (modbus_connect(_ctx) != -1) {
				//Log::INFO( "[ModbusDiscreteInputsList] " + string("modbus_connect successful") );
				int rc = modbus_write_register(_ctx, reg, data);
				Log::INFO( "[ModbusWriteSingleRegister] " +
						string("modbus_write_register data=") + toString(data)+ " reg="+ toString(reg) );
				if (rc != -1)
				{
					ret = true;
				}
			}
			modbus_close(_ctx);
		}
		catch(exception &e)
		{
			Log::ERROR( "[ModbusWriteSingleRegister] " + string( e.what() ) );
		}
		modbus_free(_ctx);
		_ctx = NULL;
		return ret;
	}

	void* GetData()
	{
		return &data;
	}

	ModbusRegPar* FindParameter(int regaddr)
	{
		ModbusRegPar* ret = NULL;
		return ret;
	}

	short data;

private:
};

class ModbusWriteSingleCoil:public IModbusRegisterList
{
public:
	ModbusWriteSingleCoil(CommSettings sett, int a, int r, short newData, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = true;
		_ctx 		= NULL;
		_sett	 	= sett;
		addr 		= a;
		reg  		= r;
		data 		= newData;
		fun         = 5;
		_type		= type;
		outPtr = out;
	}

	ModbusWriteSingleCoil( int a, int r, short newData, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = true;
		_ctx 		= NULL;
		addr 		= a;
		reg  		= r;
		data 		= newData;
		fun         = 5;
		_type		= type;
		outPtr = out;
	}

	bool DoCmd()
	{
		bool ret=false;
		try
		{
			if(_type == MODBUS_TCP)
			{
				_ctx = modbus_new_tcp(_sett.DeviceName.c_str(),_sett.BaudRate);//ip,port
			}
			else
			{
				_ctx = modbus_new_rtu(	_sett.DeviceName.c_str(),
										_sett.BaudRate,
										_sett.Parity,
										_sett.DataBits,
										_sett.StopBit);//("/dev/ttyAPP2", 115200, 'N', 8, 1);

				modbus_rtu_set_serial_mode(_ctx, MODBUS_RTU_RS485);
				modbus_rtu_set_rts(_ctx, MODBUS_RTU_RTS_NONE);
			}
			modbus_set_slave(_ctx, addr);

			uint32_t sec = _sett.RecvTimeout/1000;
			uint32_t usec = (_sett.RecvTimeout%1000)*1000;
			modbus_set_response_timeout(_ctx, sec, usec);

			uint32_t bsec = _sett.SendTimeout/1000;
			uint32_t busec = (_sett.SendTimeout%1000)*1000;
			modbus_set_byte_timeout(_ctx, bsec, busec);

			if (modbus_connect(_ctx) != -1) {
				//Log::INFO( "[ModbusDiscreteInputsList] " + string("modbus_connect successful") );
				int rc = modbus_write_bit(_ctx, reg, data);
				Log::INFO( "[ModbusWriteSingleCoil] " +
						string("modbus_write_bit data=") + toString(data)+ " reg="+ toString(reg) );
				if (rc != -1)
				{
					ret = true;
				}
			}
			modbus_close(_ctx);
		}
		catch(exception &e)
		{
			Log::ERROR( "[ModbusWriteSingleRegister] " + string( e.what() ) );
		}
		modbus_free(_ctx);
		_ctx = NULL;
		return ret;
	}

	void* GetData()
	{
		return &data;
	}

	ModbusRegPar* FindParameter(int regaddr)
	{
		ModbusRegPar* ret = NULL;
		return ret;
	}

	short data;

private:
};

class ModbusMultipleRegister:public IModbusRegisterList
{
public:
	ModbusMultipleRegister(CommSettings sett, int a, int r, int n, uint16_t *newData, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = true;
		_ctx 		= NULL;
		_sett	 	= sett;
		addr 		= a;
		reg  		= r;
		fun         = 16;
		_type		= type;
		outPtr = out;
		if(n < 127){
			nb=n;
			for(int i = 0;i < nb; i++){
				data[i] = newData[i];
			}
		}
	}

	ModbusMultipleRegister( int a, int r, int n, uint16_t *newData, void *out, MODBUS_TYPE type = MODBUS_RTU)
	{
		needDelete = true;
		_ctx 		= NULL;
		addr 		= a;
		reg  		= r;
		//data 		= newData;
		fun         = 16;
		_type		= type;
		outPtr = out;
		if(n < 127){
			nb=n;
			for(int i = 0;i < nb; i++){
				data[i] = newData[i];
			}
		}
	}

	bool DoCmd()
	{
		bool ret=false;
		try
		{
			Log::DEBUG("ModbusMultipleRegister DoCmd");
			if(_type == MODBUS_TCP)
			{
				_ctx = modbus_new_tcp(_sett.DeviceName.c_str(),_sett.BaudRate);//ip,port
			}
			else{
				_ctx = modbus_new_rtu(	_sett.DeviceName.c_str(),
										_sett.BaudRate,
										_sett.Parity,
										_sett.DataBits,
										_sett.StopBit);//("/dev/ttyAPP2", 115200, 'N', 8, 1);

				modbus_rtu_set_serial_mode(_ctx, MODBUS_RTU_RS485);
				modbus_rtu_set_rts(_ctx, MODBUS_RTU_RTS_NONE);
			}
			modbus_set_slave(_ctx, addr);

			uint32_t sec = _sett.RecvTimeout/1000;
			uint32_t usec = (_sett.RecvTimeout%1000)*1000;
			modbus_set_response_timeout(_ctx, sec, usec);

			uint32_t bsec = _sett.SendTimeout/1000;
			uint32_t busec = (_sett.SendTimeout%1000)*1000;
			modbus_set_byte_timeout(_ctx, bsec, busec);

			if (modbus_connect(_ctx) != -1) {
				Log::DEBUG( "[ModbusMultipleRegister] modbus_connect successful nb="+toString(nb) );
				int rc = modbus_write_registers(_ctx, reg, nb, data);
				Log::DEBUG( "[ModbusMultipleRegister] rc="+toString(rc) );
				string dt = "[";
				for(int k=0; k < nb; k++){
					dt+= toString(data[k])+" ";
				}
				dt+="]";
				Log::INFO( "[ModbusMultipleRegister] " +
						string("modbus_write_registers data=") + dt+ " reg="+ toString(reg) );

				if (rc != -1)
				{
					ret = true;
				}
			}
			modbus_close(_ctx);
		}
		catch(exception &e)
		{
			Log::ERROR( "[ModbusMultipleRegister] " + string( e.what() ) );
		}
		modbus_free(_ctx);
		_ctx = NULL;
		return ret;
	}

	void* GetData()
	{
		return data;
	}

	ModbusRegPar* FindParameter(int regaddr)
	{
		ModbusRegPar* ret = NULL;
		return ret;
	}

	uint16_t data[127];
	int nb;

private:
};


class TModbusManager:public TFastTimer
{
public:
	TModbusManager();

	virtual ~TModbusManager();

	void Init(string configFn)
	{
		_configFileName = configFn;
		//isNullOrWhiteSpace
		//ReloadConfig();
	}

	void Init(CommSettings sett)
	{
		_sett = sett;
		_myaddress = 254;
		//_myVersion = 0;
		_isMaster = true;
	}

	/*
	bool ReloadConfig()
	{
		bool ret = false;

		ClearDeviceList();

		string enable = uci::Get(_configFileName, "main", "enable");
		if(enable.size() > 0)
		{
			bool _enable = atoi(enable.c_str());
			if(_enable == false)
			{
				//Log::DEBUG("ModbusRTU:ReloadConfig modbus disabled");
				return false;
			}

			string myaddress = uci::Get(_configFileName, "main", "address");
			_myaddress = atoi(myaddress.c_str());
			_myVersion = uci::Get(_configFileName, "main", "version");
			_isMaster  = atoi(uci::Get(_configFileName, "main", "master").c_str());

			_sett.DeviceName 	= uci::Get(_configFileName, "comport", "device");
			_sett.BaudRate   	= atol(uci::Get(_configFileName, "comport", "baudrate").c_str());
			_sett.DataBits		= atoi(uci::Get(_configFileName, "comport", "databits").c_str());
			_sett.StopBit		= atoi(uci::Get(_configFileName, "comport", "stopbit").c_str());
			_sett.Parity		= uci::Get(_configFileName, "comport", "parity").c_str()[0];
			_sett.RecvTimeout 	= atol(uci::Get(_configFileName, "comport", "recvtimeout").c_str());
			_sett.SendTimeout 	= atol(uci::Get(_configFileName, "comport", "sendtimeout").c_str());

			string strdt = uci::Get(_configFileName, "configdata", "devices");
			//Log::DEBUG(strdt);
			vector<string> results;
			TBuffer::Split(strdt, " " , results);
			for(auto dev: results)
			{
				//Log::DEBUG(dev);
				vector<string> devstr;
				TBuffer::Split(dev, ":" , devstr);
				if(devstr.size() >= 4)
				{
					string addrstr	=	devstr[0];
					string funcstr	=	devstr[1];
					string regstr	=	devstr[2];
					string lenrstr	=	devstr[3];
					vector<string> parvect;
					TBuffer::Split(addrstr, "=" , parvect);
					if(parvect.size() >= 2)
					{
						int addr = atoi(parvect[1].c_str());
						parvect.clear();
						TBuffer::Split(funcstr, "=" , parvect);
						if(parvect.size() >= 2)
						{
							int func = atoi(parvect[1].c_str());
							parvect.clear();
							TBuffer::Split(regstr, "=" , parvect);
							if(parvect.size() >= 2)
							{
								int reg = atoi(parvect[1].c_str());
								parvect.clear();
								TBuffer::Split(lenrstr, "=" , parvect);
								if(parvect.size() >= 2)
								{
									int len = atoi(parvect[1].c_str());
									parvect.clear();
									if(func == 3)
										DeviceList.push_back(
												new ModbusHoldingRegistersList(_sett, addr, reg, len, NULL ));
									else if(func == 2)
										DeviceList.push_back(
												new ModbusDiscreteInputsList(_sett, addr, reg, len, NULL ));
								}
							}
						}
					}
				}
			}
			ret = true;
		}

		Log::DEBUG("ModbusRTU config file disabled");
		return ret;
	}*/

	void AddToDeviceList(int addr, int func, int reg, int len, void *out, long newData = 0, MODBUS_TYPE type = MODBUS_RTU)
	{

		if(func == 1)
		{
			DeviceList.push_back( new ModbusCoilStatusRegistersList(_sett, addr, reg, len, out, type ) );
		}
		else if(func == 2)
		{
			DeviceList.push_back( new ModbusDiscreteInputsList(		_sett, addr, reg, len, out, type ) );
		}
		else if(func == 3)
		{
			DeviceList.push_back( new ModbusHoldingRegistersList(	_sett, addr, reg, len, out, type ) );
		}
		else if(func == 4)
		{
			DeviceList.push_back( new ModbusInputRegistersList(		_sett, addr, reg, len, out, type ) );
		}
		else if(func == 5)
			DeviceList.push_back( new ModbusWriteSingleCoil(		_sett, addr, reg, (short)newData, out, type ) );
		else if(func == 6)
			DeviceList.push_back( new ModbusWriteSingleRegister(	_sett, addr, reg, (short)newData, out, type ) );
		else if(func == 16)
			DeviceList.push_back(new ModbusMultipleRegister(		_sett, addr, reg, len, (uint16_t *)newData, out, type ));

	}

	void SetNeedPopBack(bool val){
		_need_pop_back.SetData(val);
	}

	void ManageDeviceList(){
		if(_need_pop_back.GetData())
		{
			_need_pop_back.SetData(false);
			if(DeviceList.size() > 0)
			{
				bool flag = true;
				while(flag)
				{
					IModbusRegisterList*	del = DeviceList.back();
					if(del != NULL && del->NeedDelete() ){
						delete del;
						DeviceList.pop_back();
					}
					else{
						flag = false;
					}
				};
			}
		}
	}

	void ClearDeviceList()
	{
		for(int i=0; i<DeviceList.size(); i++){
			IModbusRegisterList *curr = DeviceList[i];
			if(curr != NULL) delete curr;
		}
		DeviceList.clear();
	}

	CommSettings GetCommSettings()
	{
		return _sett;
	}

	bool Stop()
	{
		Log::INFO( "ModbusRTUManager stopping.");
		IsStopped.SetData(true);
		int errorCNT = 0;
		bool exitFlg = false;
		do{
			exitFlg = true;
			for(int i=0; i<DeviceList.size(); i++){
				IModbusRegisterList *curr = DeviceList[i];
				if(curr != NULL){
					//Log::INFO( curr->GetCfgString()+string(" isFree=")+toString(curr->IsFree()));
					if(curr->IsFree() == false)
						exitFlg = false;
				}
			}
			if(exitFlg == false) //wait
			{	sleep(1);}
		}
		while(errorCNT++ <  20 && exitFlg == false);
		//Log::INFO( "errorCNT="+toString(errorCNT) );

		bool ret = errorCNT <= 20;
		//Log::INFO( "ModbusRTUManager stopped="+toString(ret));
		return ret;
	}

	void Start()
	{

		IsStopped.SetData(false);
		Log::INFO( "ModbusRTUManager Start.");
	}

	sync_deque<IModbusRegisterList *> DeviceList;

	//void        *outPtr;

	PFV2         OnCmdComplited;

	//sync_type<bool> NeedReloadConfig;

	sync_type<bool> IsStopped;

private:

	sync_type<bool>         _need_pop_back;
	int 		 _myaddress;
	bool         _isMaster;
	string    	 _configFileName;
	string       _myVersion;
	CommSettings _sett;

};

#endif /* MODBUSRTU_H_ */

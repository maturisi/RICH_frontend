/*
 *  TRich_ConnectTCP.h
 *  
 *
 *  Created by Matteo Turisini on 19/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef RICH_CONNECT_TCP_H
#define RICH_CONNECT_TCP_H

typedef struct
{
	int len;
	int type;
	int wrcnt;
	int addr;
	int flags;
	int vals[1];
} write_struct;

typedef struct
{
	int len;
	int type;
	int rdcnt;
	int addr;
	int flags;
} read_struct;

typedef struct
{
	int len;
	int type;
	int rdcnt;
	int data[1];
} read_rsp_struct;

class TRich_ConnectTCP
{

private:
	int fsockfd_reg;
	int fsockfd_event;
	
public:
	TRich_ConnectTCP();
	~TRich_ConnectTCP();
	
  bool OpenSocket_Event(const char * IPaddr=0,int port=0);
  bool OpenSocket_Reg(const char * IPaddr=0,int port=0);
	
	bool Close();

	bool CloseSocket_Event();
	bool CloseSocket_Reg();
	
	unsigned int Read(void *addr);
	void Write(void *addr, int val);
	
	int Receive(int * addr);
	
 private:
	int OpenSocket(const char * IPaddr,int port);
	bool TestEndianess();	
};
#endif

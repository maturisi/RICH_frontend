/*
 *  TRich_ConnectTCP.cpp
 *  
 *
 *  Created by Matteo Turisini on 19/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <string.h> // memset
#include <sys/socket.h> // close
#include <arpa/inet.h> // sockaddr_in,inet_pton  
#include <unistd.h> // usleep
#include <fcntl.h> /// fcntl manipulate file descriptor
#include <cerrno> // errno
#include "TRich_ConnectTCP.h"

#define READ_DELAY 5000 // microseconds pause in a Read operation between request and response    

TRich_ConnectTCP::TRich_ConnectTCP(){
	fsockfd_reg=0;
	fsockfd_event=0;
}

TRich_ConnectTCP::~TRich_ConnectTCP(){
}


int TRich_ConnectTCP::Receive(int * valp){

	int err;
  int val; 
  int n = recv(fsockfd_event, &val, sizeof(val), MSG_WAITALL);
	err = errno;
	//printf(" TCP Receive  %d bytes (%d expected) content is %ld (0x%X)\n",n,sizeof(val),val,val);
  *valp = val;
  //printf(" TCP Receive 0x%X  and pass 0x%X \n",val,*valp);

	if(n< 0){     
//	if ((err == EAGAIN) || (err == EWOULDBLOCK))
  	
	if (err == EWOULDBLOCK){
          printf("non-blocking operation returned EWOULDBLOCK\n");
					sleep(1);
       }
       else
       {
          printf("recv returned unrecoverable error(errno=%d)\n", err);
         // break;
       }
    }        

return n;
}


bool TRich_ConnectTCP::OpenSocket_Event(const char * IPaddr,int port){
  if(IPaddr==NULL){
    return false;
  }

  fsockfd_event = this->OpenSocket(IPaddr,port);  

  if(fsockfd_event<=0){
    printf("Error in %s: socketfd = %d\n",__FUNCTION__,fsockfd_event);
    return false;
  }else{
    //printf("TCP connection with %s Port %d on socket %d ready\n",IPaddr,port,fsockfd_event);
  }

	// NON BLOCKING
	int flags;
	if (-1 == (flags = fcntl(fsockfd_event, F_GETFL, 0))){        
		flags = 0;
	}
  int ret =  fcntl(fsockfd_event, F_SETFL, flags | O_NONBLOCK);
	
	// if fcntl returns no error, sock is now non-blocking

	if (ret<0){
		printf("ret fcntl %d \n",ret);
		return false;	
	}




  return true;
}
bool TRich_ConnectTCP::OpenSocket_Reg(const char * IPaddr,int port){
  if(IPaddr==NULL){
    return false;
  }

  fsockfd_reg = this->OpenSocket(IPaddr,port);  

  if(fsockfd_reg<=0){
    printf("Error in %s: socketfd = %d\n",__FUNCTION__,fsockfd_reg);
    return false;
  }else{
   // printf("TCP connection with %s Port %d on socket %d ready\n",IPaddr,port,fsockfd_reg);
  }

  this->TestEndianess();

  return true;
}

bool TRich_ConnectTCP::Close(){
	
  int ret = close(fsockfd_reg);
  printf("TCP %s %d\n",__FUNCTION__,ret);
  
  ret = close(fsockfd_event);
  printf("TCP %s %d\n",__FUNCTION__,ret);
  
  return true;
}



bool TRich_ConnectTCP::CloseSocket_Event(){
	int ret = close(fsockfd_event);
	bool success = (ret==0) ? true : false;
  	return success;
}
bool TRich_ConnectTCP::CloseSocket_Reg(){
	int ret = close(fsockfd_reg);
	bool success = (ret==0) ? true : false;
  	return success;
}
	



int TRich_ConnectTCP::OpenSocket(const char * IPaddr,int port){

	struct sockaddr_in lserv_addr;
	int sockfd =0;
       
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\nError : Could not create socket \n");
		return -1;
	} 
	
	memset(&lserv_addr, '0', sizeof(lserv_addr)); 
	
	lserv_addr.sin_family = AF_INET;
	lserv_addr.sin_port = htons(port); 
		
	if(inet_pton(AF_INET, IPaddr, &lserv_addr.sin_addr)<=0)
	{
		printf("\ninet_pton error occured\n");
		return -2;
	} 
	
	// connect
	if( connect(sockfd, (struct sockaddr *)&lserv_addr, sizeof(lserv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		/* add here a part to manage missed connection openings*/
		return -3;
	} 
	return sockfd;
}

unsigned int TRich_ConnectTCP::Read(void *addr){

	
	read_struct rs;
	read_rsp_struct rs_rsp;
	
	
	rs.len = 12;
	rs.type = 3;
	rs.rdcnt = 1;
	rs.addr = (int)((long)addr);
	rs.flags = 0;

	write(fsockfd_reg, &rs, sizeof(rs)); //
	
//	int byte_wr = write(fsockfd_reg, &rs, sizeof(rs)); //
	//printf("TCP %s  : ",__FUNCTION__);
//	printf("Request %d bytes of %d, ",byte_wr,sizeof(rs));

	//usleep(READ_DELAY);
	
	int byte_rd = read(fsockfd_reg, &rs_rsp, sizeof(rs_rsp));
//	printf("Read %d bytes of %d \n",byte_rd,sizeof(rs_rsp));

	if(byte_rd != sizeof(rs_rsp)){
		printf("Error in TCP %s: socket read failed...\n", __FUNCTION__);	
	}
	return rs_rsp.data[0];
}


void TRich_ConnectTCP::Write(void *addr, int val){



	write_struct ws;

	ws.len = 16;
	ws.type = 4;
	ws.wrcnt = 1;
	ws.addr = (int)((long)addr);
	ws.flags = 0;
	ws.vals[0] = val;
	write(fsockfd_reg, &ws, sizeof(ws));

//	int byte_ws = 	write(fsockfd_reg, &ws, sizeof(ws));
//	printf("TCP %s : ",__FUNCTION__);
//	printf("bytes %d of %d",byte_ws,sizeof(ws));
//	printf("  ADDR: 0x%08X VAL: 0x%08X (%d)\n",ws.addr,ws.vals[0],ws.vals[0]);
}

bool TRich_ConnectTCP::TestEndianess(){
	
	int val_w = 0x12345678;
	write(fsockfd_reg, &val_w, 4);
	
	int val_r = 0;
	int n = read(fsockfd_reg, &val_r, 4);
//	printf("n = %d, val = 0x%08X\n", n, val_r);
	
	//printf("TCP %s:",__FUNCTION__);
	if ((n!=4)||(val_r!=val_w)) {
		//printf("little\n");
		return false;
	}else {
		//printf("big\n");
		return true;
	}

	
}


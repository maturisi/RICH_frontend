/**
 * use makefile to produce executable 
 *
 * 	./rich run.frontend.asic_common.static_register.gain=64 run.frontend.asic_common.static_register.DAC0=349 run.title=PUPPE	
 *
 */

#include <iostream> // cerr, endl
#include <string> // string

#include "TRich_ConnectTCP.h"
#include "TRich_Config.h"
#include "TRich_Frontend.h"
#include "TRich_DAQ.h"

#define IPADDRESS "192.168.1.10"
#define PORT_REGISTERS 6102 
#define PORT_EVENTS 6103

#define CONFIG_FILE 	"../cfg/rich.cfg" 

int main(int argc, char *argv[]){
	
	printf("Welcome to RICH daq test\n");
	
	TRich_ConnectTCP * tcp = new TRich_ConnectTCP();	
	
	if(!tcp->OpenSocket_Reg(IPADDRESS,PORT_REGISTERS)){
	  printf("Problem connecting to FPGA BOARD: Register socket not opened...\n");return -1;
	}
	
	TRich_Config * cfg = new TRich_Config();
	cfg->ParseInputLine(argc, argv); // to check if the user want to change some parameters on the fly		
	if(!cfg->ParseFile(CONFIG_FILE)){
	  printf("error with Config File \n"); 
	  return -2;
	}//printf("Configuration data base ready\n");	
	cfg->Edit();	
	cfg->Read(); //printf("data imported ready for access\n");
	cfg->Init();
	cfg->Print();
	
	TRich_Frontend * fe = new TRich_Frontend();
	fe->SetTCP(tcp);
	fe->SetConfiguration(cfg);
	
	fe->ConfigureMAROC();
	fe->ConfigureFPGA();			

	if(!tcp->OpenSocket_Event(IPADDRESS,PORT_EVENTS)){
	  printf("Problem with connection: Event Socket not openend...\n");return -3;
	}
	
	TRich_DAQ * daq = new TRich_DAQ(fe,cfg);
	daq->PreEv(); 
	daq->DoEv(0);
	daq->PostEv();
	
	delete daq;
	
	delete fe;

	delete cfg;

	delete tcp;
	
}

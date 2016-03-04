/**
 * use makefile to produce executable 
 *
 * 	./rich run.frontend.asic_common.static_register.gain=64 run.frontend.asic_common.static_register.DAC0=349 run.title=PUPPE	
 *
 */

//02:00.0 Ethernet controller: VIA Technologies, Inc. VT6120/VT6121/VT6122 Gigabit Ethernet Adapter (rev 82)


#include "TRich_ConnectTCP.h"
#include "TRich_Config.h"
#include "TRich_Frontend.h"
#include "TRich_DAQ.h"
#include <unistd.h> // sleep

#define IPADDRESS "192.168.1.10"
#define PORT_REGISTERS 6102 
#define PORT_EVENTS 6103
#define CONFIG_FILE "../cfg/rich.cfg" 

bool Connect(TRich_ConnectTCP * tcp){

  return  tcp->OpenSocket_Reg(IPADDRESS,PORT_REGISTERS);

}

bool Disconnect(TRich_ConnectTCP * tcp){

  return tcp->CloseSocket_Reg();

}


int main(int argc, char *argv[]){

	printf("\n\nNew data acquisition\n");
  TRich_ConnectTCP tcp;

  bool success = Connect(&tcp);

  //success = tcp.OpenSocket_Reg(IPADDRESS,PORT_REGISTERS);
  if(!success){printf("Register socket not opened...exit\n");return -1;}

  TRich_Config cfg;

//  cfg.Create();

  cfg.ParseInputLine(argc, argv);
  success = cfg.ParseFile();
  if(!success){
		printf("Problem with configuration file...exit\n");
    return -2;
  }

  cfg.Edit();
  cfg.Read();
  cfg.Init();
  //cfg.Print();

  TRich_Frontend fe;
  fe.SetTCP(&tcp);
  fe.SetConfiguration(&cfg);
  fe.ConfigureMAROC();

  printf("Found %dASIC board\n",fe.GetNasic());

/*
  if(fe.GetNasic()==0){
	Disconnect(&tcp);
	return -6;
  }
*/
  fe.ConfigureFPGA();



  success = tcp.OpenSocket_Event(IPADDRESS,PORT_EVENTS);
  
  if(!success){\
		printf("Event Socket not openend...exit\n");
    return -3;
  }

	int charge = cfg.GetCTestAmp();
	int gain = cfg.GetGain(0,0);
	int threshold = cfg.GetThr(0);
	long long ret =0;


  TRich_DAQ daq;
  daq.SetFrontend(&fe); 
  daq.SetConfiguration(&cfg);
  daq.PreEv();

  ret = daq.DoEv(0);
	
	//printf("DAQ returned %lld\n",ret);

	daq.PostEv();

	//FILE * fout = fopen("./maroc3A_025_019_024.txt","a");

	FILE * fout = fopen("./pippo.txt","a");
	fprintf(fout,"%8d %8d %8d %10lld\n",charge,gain,threshold,ret);
	fclose(fout);
	
  success = tcp.CloseSocket_Event();
  if(!success){
		printf("Event Socket not closed...exit\n");
    return -4;
  }



//  success = tcp.CloseSocket_Reg();
success = Disconnect(&tcp);
if(!success){printf("Register Socket not closed...exit\n");return -5;}

return 0;
}

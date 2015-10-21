/**
 * use makefile to produce executable 
 *
 * 	./rich run.frontend.asic_common.static_register.gain=64 run.frontend.asic_common.static_register.DAC0=349 run.title=PUPPE	
 *
 */
#include "TRich_ConnectTCP.h"
#include "TRich_Config.h"
#include "TRich_Frontend.h"
#include "TRich_DAQ.h"

#define IPADDRESS "192.168.1.10"
#define PORT_REGISTERS 6102 
#define PORT_EVENTS 6103
#define CONFIG_FILE "../cfg/rich.cfg" 

int main(int argc, char *argv[]){
 

  TRich_ConnectTCP tcp;	

  bool success;
  success = tcp.OpenSocket_Reg(IPADDRESS,PORT_REGISTERS);
  if(!success){printf("Register socket not opened...exit\n");
    return -1;
  }
 
  TRich_Config cfg;

  //cfg.Create();

  cfg.ParseInputLine(argc, argv); 		
  success = cfg.ParseFile();
  if(!success){printf("Problem with configuration file...exit\n");
    return -2;
  }

  cfg.Edit();	
  cfg.Read(); 
  cfg.Init();
 // cfg.Print();
  
  TRich_Frontend fe;
  fe.SetTCP(&tcp);
  fe.SetConfiguration(&cfg);
  fe.ConfigureMAROC();
  fe.ConfigureFPGA();			

  success = tcp.OpenSocket_Event(IPADDRESS,PORT_EVENTS);
  
  if(!success){printf("Event Socket not openend...exit\n");
    return -3;
  }
  
  TRich_DAQ daq;
  daq.SetFrontend(&fe); 
  daq.SetConfiguration(&cfg);
  daq.PreEv();
  daq.DoEv(0);
  daq.PostEv();
 
  success = tcp.CloseSocket_Event();
  if(!success){printf("Event Socket not closed...exit\n");
    return -4;
  }

  success = tcp.CloseSocket_Reg();
  if(!success){printf("Register Socket not closed...exit\n");
    return -5;
  }
 return 0;	
}

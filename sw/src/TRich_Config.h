/*
 *  TRich_Config.h
 *  
 *  Created by Matteo Turisini on 23/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef RICH_CONFIG_H
#define RICH_CONFIG_H

#include <libconfig.h++>
#include "datatype.h"

#define DEFAULT_CONFIG_FILE "../cfg/rich.cfg"

using namespace std;
using namespace libconfig;

class TRich_Config {
  /*TRich_Config is the interface with the configuration file
    
    To use Getxxx methods and get the configuration file in the user space
    the file must be successfulluy parsed by Open() 
    data must be copied to private members by Read()
    
    GetLog prefix is used to read data added by daq program for logbooking purposes,
    please note that those data are accessible after daq (offline)
    
    Some methodds are for creating a new configuration from scratch
  */
  	
 private:
	
  Config fcfg; 	// typedef on libconfig.h++, is a tree of settings.	
  
  string *fargp;
  string *fargv;
  int	  fargc;

  string  frunprefix; 
  string  fnote;
  int	  fthreshold_default;
  int	  fgain_default;
  
  int	  fdaqmode;	
  int	  fEv_preset;
  int	  fTm_preset;
  int 	fSKA_duration;
  int 	fSKA_repetition;
  
	int		fChsel;

  int	  fCh0_31_Hold1;
  int	  fCh32_63_Hold1;
  int	  fCh0_31_Hold2;
  int	  fCh32_63_Hold2;
  
  RICH_fpga_t		ffp; 
  RICH_maroc_common_t	fmp;
  RICH_maroc_t		fmpa[3];
  
  int	  ftrig_delay;
  int	  fevtbuild_lookback;
  int	  fevtbuild_window;
  
 public: 
  TRich_Config(); 
  ~TRich_Config();
  
  
  
  bool	Create(const char * filename = DEFAULT_CONFIG_FILE);
  /*
   * Create new configuration file
   * Creates a new basic configuration file using hardcoded values
   * filename: name (and path) of the created file
   * return true in case of success, false if an I/O exception occours
   */
  
  bool	ParseFile(const char * filename = DEFAULT_CONFIG_FILE);	
  
  /*
   * Read a configuration file  
   * Parse a configuration file and creates a database of settings
   * filename: name (and path) of the file to be read
   * return true in case of success, false if an I/O exception or a Parse exception occours 
   */
  
  void	ParseInputLine(int argc,char *argv[]);
  
  /*
   * Process a set of c_style strings 	 
   * It is suitable for processing command line interface arguments.
   * It checks that arguments look properly formatted i.e. a dot-separated sequence of words (the path to the setting)
   * followed by a '=' followed by a value 
   * for more info about format please see libconfig documentation (http://www.hyperrealm.com/libconfig/)
   * argc: number of settings 
   * argv: array of string with path to the settings and their value
   * e.g ParseInputLine(2,./rich run.frontend.asic_common.static_register.DAC0=400);
   */
  
  void	Edit();
  /*
   * Replace Settings in the configuration
   * For all the arguments (see ParseInputLine) replace the setting value in the configuration
   */
  
  void	Read();
  /*
   * Set data members
   * Read from the configuration database (initializated by ParseFile) all the basic configuration settings
   * and set all the private data members for future use.
   */
  bool	Init();
  /*
   * Get individual channel/asic parameters and add them to the configuration
   */
  
  void	Print();
  /*
   * Print all the configuration settings
   */
  void 	Logbook (int nEv_Acq,float n_seconds);
  void  Logbook(std::string setting_name = "givemeanameINT",int ival=0);
  void  Logbook(std::string setting_name = "givemeanameFLOAT",float fval=0);
  void  Logbook(std::string setting_name = "givemeanameSTRING",std::string sval="nothing");
  void  Logbook(std::string setting_name = "givemeanameINT64",long long lval = 0);

  void	Export(string cfgFName);
  
  
  /* Getter from config file*/
  char	GetNasic();
  char	GetNpmt();
  
  /* access private data members*/
  string RunPrefix();
  int Threshold(int value = -1); // by default is a getter, with arg != -1 is a setter.
  int Gain(int value = -1); // by default is a getter, with arg != -1 is a setter.
  int Get_Ch00_31_Hold1();
  int Get_Ch32_63_Hold1();
  int Get_Ch00_31_Hold2();
  int Get_Ch32_63_Hold2();
  
  int Daq_Mode(int val =-1);
  int EventPreset(int val =-1);
  int TimePreset(int val =-1);

  int ScalerDuration(int val=-1);
  int ScalerRepetition(int val=-1);

  int GetGain(int idx=-1,int channel=-1);
  int GetThr(int idx=-1);

  int GetCTestAmp();


	
	int Ch_Sel(int val =-1);

  
  
  RICH_fpga_t *			Get_Settings_Fpga();		
  RICH_maroc_common_t *	Get_Settings_Maroc_Common();			
  RICH_maroc_t *			Get_Settings_Maroc(int idx = 0);	
  
  
  int	GetLogNEvents(); 
  float	GetLogNSeconds();
  void	Retrieve_Settings_MAROC(int idx=0);
  
  unsigned int GetTriggerDelay();
  unsigned int GetEventBuilderLookBack();
  unsigned int GetEventBuilderWindow();	
  
  void	PrintRUN();	
  void	PrintFPGA();
  void	PrintMAROC_StaticReg();
  void	PrintMAROC_DynamcReg();	
  void	PrintMAROC_adj();
  void	PrintEXTERNAL_Pulser();
  
 private:
  
  // Read Settings from the libconfig configuration and set private data members
  void	Read_Settings_FPGA();
  void	Read_Settings_MAROC_Common();
  void	Read_Settings_MAROC_Dynamic();	
  void	Read_Settings_External_Pulser();
  string Read_String(string path = NULL);
  
  // Read Settings from external files, set private data members and add them to the configuration
  void	Setup();	
  void	Gains();
  void	Thresholds();
  void	AddParam_MAROC3();
  void	AddParam_MAPMT();
  
  // to create a new configuration
  void	AddParam_RUN(Config * configuration);
  void	AddParam_FRONTEND(Config * configuration = NULL);
  void	AddParam_FPGA(Config * configuration);
  void	AddParam_MAROC3_com(Config * configuration);
  void	AddParam_EXTPULSER(Config * configuration);
  void	AddParam_EXT(Config * configuration);
  
  int Replace(const string value,string path);
};
#endif

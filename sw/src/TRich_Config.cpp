
/*
 *  TRich_Config.cpp
 *  
 *
 *  Created by Matteo Turisini on 23/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */


// # ./daq run.name=$prefix run.fe.mrc.sc.DAC0=$thr run.fe.mrc.sc.gain=$gain

#include <stdio.h> 
#include <string.h>// memset
#include <iostream> // cerr, endl
#include <fstream> 
#include <sstream> // istringstream
#include <stdlib.h> // exit,EXIT_FAILURE

#include "TRich_Config.h"

#define TABWIDTH 2

using namespace std;

TRich_Config::TRich_Config(){

  // init memory
  memset(&ffp, 0, 1 * sizeof(RICH_fpga_t));
  memset(&fmp, 0, 1 * sizeof(RICH_maroc_common_t));
  memset(&fmpa,0, 3 * sizeof(RICH_maroc_t));
  memset(&fext,0, 1 * sizeof(ExtPulser_t));
  
  fdaqmode = -1;	
  fEv_preset = -1;
  fTm_preset = -1;

  fSKA_duration=-1; 
  fSKA_repetition=-1;

  
  fthreshold_default =-1;
  fgain_default=-1;
  
	fChsel =-1;

  fargc = 0;
}

TRich_Config::~TRich_Config(){
  //printf("clean Configuration\n");
}
bool TRich_Config::Create(const char * output_file){
  
  printf("A new configuration will be created and saved to %s\n",output_file);
  
  Config lcfg;
  
  lcfg.setTabWidth(TABWIDTH);
  
  this->AddParam_RUN(&lcfg);
  this->AddParam_FRONTEND(&lcfg);
  this->AddParam_FPGA(&lcfg);
  this->AddParam_MAROC3_com(&lcfg);
  this->AddParam_EXT(&lcfg);

  try{
    lcfg.writeFile(output_file);
    cerr << "New configuration successfully written to: " << output_file<< endl;
    return true;
  }
  catch(const FileIOException &fioex){
    cerr << "I/O error while writing file: " << output_file << endl;
    return false;
  }
}

void	TRich_Config::AddParam_RUN(Config * configuration){
	  	
	Setting &root = configuration->getRoot(); 
  
  if(! root.exists("run")){root.add("run", Setting::TypeGroup);}
  Setting &run = root["run"];
  run.add("name"	, Setting::TypeString) = "run";
  run.add("note"	, Setting::TypeString) = "Test Bench INFN-Ferrara (Italy), 2015 August 14th";
  
	run.add("daq_mode"	, Setting::TypeInt) = 1; // 0 scaler; 1 TDC (obsolete); 2 TDC    
	
	run.add("tdc_event_preset"	, Setting::TypeInt) = 10000; // # of events
 	run.add("tdc_time_preset"	, Setting::TypeInt) = 2; // seconds 

	run.add("ska_duration"	, Setting::TypeInt) = 1;// seconds
  run.add("ska_iteration"	, Setting::TypeInt) = 1; //iteration 

  run.add("gain_maps", Setting::TypeInt) = 0;	 // 0 no; 1 yes
  run.add("thr_map", Setting::TypeInt) = 0;	 // 0 no; 1 yes

}

void	TRich_Config::AddParam_FRONTEND(Config * configuration){
  
  Setting &root  = configuration->getRoot();
  
  Setting &run =  root["run"];
  
  if(! run.exists("fe")){run.add("fe", Setting::TypeGroup);}
  //Setting &fe = run["fe"];
 
  
}

void	TRich_Config::AddParam_FPGA(Config * configuration){
  
  Setting &root  = configuration->getRoot();
  Setting &fe = root["run"]["fe"];
  
  if(! fe.exists("fpga")){
    fe.add("fpga", Setting::TypeGroup);
  }
  
  Setting &xil = fe["fpga"];
  
  xil.add("fpgaID", Setting::TypeInt) = 1;
  xil.add("firmwareVer", Setting::TypeInt) = 1;// 0 scaler; 1 = 0+tdc internal pulser
  
  xil.add("pulser_frequency", Setting::TypeInt) = 50000;
  xil.add("pulser_dutycycle", Setting::TypeFloat) = 0.05;
  xil.add("pulser_repetition", Setting::TypeInt) = 10000000;
  
  xil.add("out0", Setting::TypeInt) = 18; 
  xil.add("out1", Setting::TypeInt) = 14; 
  xil.add("out2", Setting::TypeInt) = 0; 

  xil.add("trig_source", Setting::TypeInt) = 18; 		

  xil.add("trig_delay", Setting::TypeInt) = 128; //128*8ns = 1024ns trigger delay 
  xil.add("evtb_lookback", Setting::TypeInt) = 1200/8; // lookback 1.2us, 
  xil.add("evtb_windowwidth", Setting::TypeInt) = 800/8; //capture 800ns

//  xil.add("evtb_blockNevent", Setting::TypeInt) = 1;




}

void	TRich_Config::AddParam_MAROC3_com(Config * configuration){
  Setting &root  = configuration->getRoot();
  Setting &fe = root["run"]["fe"];
  
  if(! fe.exists("mrc"))
    fe.add("mrc", Setting::TypeGroup);
  
  Setting &mrc = fe["mrc"];


	int ch_sel = 44; // [0..63], the same channel on all asics
  
	mrc.add("ch_sel", Setting::TypeInt)  = ch_sel;


/*  
  if(! mrc.exists("dyn"))
    mrc.add("dyn", Setting::TypeGroup);
  
  
  int Ch0_31_Hold1	= 0x80000000;
  int Ch32_63_Hold1	= 0x00000000;
  int Ch0_31_Hold2	= 0x00000000;
  int Ch32_63_Hold2	= 0x00000000;

  Setting &dyn = mrc["dyn"];
  
  dyn.add("Ch0_31_Hold1", Setting::TypeInt)  = Ch0_31_Hold1;
  dyn.add("Ch32_63_Hold1", Setting::TypeInt) = Ch32_63_Hold1;
  dyn.add("Ch0_31_Hold2", Setting::TypeInt)  = Ch0_31_Hold2;
  dyn.add("Ch32_63_Hold2", Setting::TypeInt) = Ch32_63_Hold2;
  */
  Setting &sc = mrc.add("sc", Setting::TypeGroup);
  
  sc.add("cmd_fsu", Setting::TypeInt) = 0;
  sc.add("cmd_ss", Setting::TypeInt) = 1;
  sc.add("cmd_fsb", Setting::TypeInt) = 1;
  sc.add("swb_buf_250f", Setting::TypeInt) = 0;
  sc.add("swb_buf_500f", Setting::TypeInt) = 0;
  sc.add("swb_buf_1p", Setting::TypeInt) = 0;
  sc.add("swb_buf_2p", Setting::TypeInt) = 0;
  sc.add("ONOFF_ss", Setting::TypeInt) = 1;
  sc.add("sw_ss_300f", Setting::TypeInt) = 1;
  sc.add("sw_ss_600f", Setting::TypeInt) = 1;
  sc.add("sw_ss_1200f", Setting::TypeInt) = 0;
  sc.add("EN_ADC", Setting::TypeInt) = 0;
  sc.add("H1H2_choice", Setting::TypeInt) = 0;
  sc.add("sw_fsu_20f", Setting::TypeInt) = 1;
  sc.add("sw_fsu_40f", Setting::TypeInt) = 1;
  sc.add("sw_fsu_25k", Setting::TypeInt) = 0;
  sc.add("sw_fsu_50k", Setting::TypeInt)= 0;
  sc.add("sw_fsu_100k", Setting::TypeInt) = 0;
  sc.add("sw_fsb1_50k", Setting::TypeInt) = 0;
  sc.add("sw_fsb1_100k", Setting::TypeInt) = 0;
  sc.add("sw_fsb1_100f", Setting::TypeInt) = 0;
  sc.add("sw_fsb1_50f", Setting::TypeInt) = 1;
  sc.add("cmd_fsb_fsu", Setting::TypeInt) = 0;
  sc.add("valid_dc_fs", Setting::TypeInt) = 1;
  sc.add("sw_fsb2_50k", Setting::TypeInt) = 0;
  sc.add("sw_fsb2_100k", Setting::TypeInt) = 0;
  sc.add("sw_fsb2_100f", Setting::TypeInt) = 0;
  sc.add("sw_fsb2_50f", Setting::TypeInt) = 1;
  sc.add("valid_dc_fsb2", Setting::TypeInt) = 0;
  sc.add("ENb_tristate", Setting::TypeInt)= 1;
  sc.add("polar_discri", Setting::TypeInt) = 0;
  sc.add("inv_discriADC", Setting::TypeInt) = 0;
  sc.add("d1_d2", Setting::TypeInt) = 0;
  sc.add("cmd_CK_mux", Setting::TypeInt) = 0;
  sc.add("ONOFF_otabg", Setting::TypeInt) = 0;
  sc.add("ONOFF_dac", Setting::TypeInt) = 0;
  sc.add("small_dac", Setting::TypeInt) = 0;
  sc.add("enb_outADC", Setting::TypeInt) = 0;
  sc.add("inv_startCmptGray", Setting::TypeInt) = 1;
  sc.add("ramp_8bit", Setting::TypeInt) = 0;
  sc.add("ramp_10bit", Setting::TypeInt) = 0;
  sc.add("DAC0", Setting::TypeInt) = 200; 
  sc.add("DAC1", Setting::TypeInt) = 0;
  sc.add("gain", Setting::TypeInt) = 64; 

}

void	TRich_Config::AddParam_MAROC3(){
	
	Setting &root  = fcfg.getRoot();
	Setting &fe = root["run"]["fe"];
	
	int n_asic = 3;
	//fe.lookupValue("n_asic", n_asic);
	
	if(! fe.exists("asic"))
		fe.add("asic", Setting::TypeList);
	
	Setting &asic = fe["asic"];
	
	for (int m=0; m<n_asic; m++) {
		
		Setting &maroc = asic.add(Setting::TypeGroup);
		
		maroc.add("marocID", Setting::TypeInt) = fmpa[m].did;
		maroc.add("threshold", Setting::TypeInt) = fmpa[m].thr;	
		
		Setting &gain = maroc.add("gain", Setting::TypeArray);
		
		for(int i = 0; i < 64; ++i){
			gain.add(Setting::TypeInt) =fmpa[m].gain[i];
		}
		
	}
}

void	TRich_Config::AddParam_MAPMT(){
	
	Setting &root  = fcfg.getRoot();
	Setting &fe = root["run"]["fe"];
	
	int n_mapmt =3;
	//fe.lookupValue("n_mapmt", n_mapmt);
	
	if(! fe.exists("mapmt"))
		fe.add("mapmt", Setting::TypeList);
	
	Setting &mapmt = fe["mapmt"];
	for (int t=0; t<n_mapmt; t++) {
		
		Setting &hama = mapmt.add(Setting::TypeGroup);
		hama.add("mapmtID", Setting::TypeString) = "ca555";
		hama.add("HV", Setting::TypeInt) = 1000;
	}
	
}

void TRich_Config::AddParam_EXT(Config * configuration){


  
  Setting &root  = configuration->getRoot();  
  Setting &run =  root["run"];
  if(! run.exists("ext")){run.add("ext", Setting::TypeGroup);}
  Setting &ext = run["ext"];

ext.add("hv", Setting::TypeInt)= -1;
ext.add("x", Setting::TypeInt)= -1;
ext.add("y", Setting::TypeInt)= -1;	
ext.add("w", Setting::TypeInt)= -1;


}


void	TRich_Config::AddParam_EXTPULSER(Config * configuration){
	
	Setting &root  = configuration->getRoot();
	Setting &fe = root["run"]["fe"];
	
	if(! fe.exists("external_pulser")){fe.add("external_pulser", Setting::TypeGroup);}
	
	Setting &extpulser = fe["external_pulser"];



//	Setting &lecroy = extpulser.add(Setting::TypeGroup);
	extpulser.add("LecroyModel", Setting::TypeString) = "Lecroy9210";
	extpulser.add("Vhigh_mV", Setting::TypeFloat) = 0.0; //mV
	extpulser.add("Vlow_mV", Setting::TypeFloat) = -50.0;//mV
	extpulser.add("Width_us", Setting::TypeFloat) = 50.00; //microsecond
	extpulser.add("Delay_us", Setting::TypeFloat) = 0.0;
	extpulser.add("Lead_ns", Setting::TypeFloat) = 0.90; // nanosecond
	extpulser.add("Trail_ns", Setting::TypeFloat) = 0.90; // nanosecond
	extpulser.add("Period_us", Setting::TypeFloat) = 100.0; // microsecond	

}

void	TRich_Config::ParseInputLine(int argc,char *argv[]){

	int i;
	char cdummy[1000];
	std::string sarg;
	
    // init
    fargp = new std::string[argc]; // max settings expected
    fargv = new std::string[argc];
    fargc = 0;
	
    for (i=1;i<argc; i++) {
		if (strcmp("-cnf", argv[i]) == 0) { 
			i++;
			sprintf(cdummy, "%s", argv[i]);
			continue;
		}
		
		if (strcmp("-fun", argv[i]) == 0) {
			printf("\n ------------------- WARNING ---------------------\n");
			printf(" This software requires a valid license\n");
			printf(" Any abuse will be persecuted by international laws\n");
			printf(" Licenses are released by the Author only\n");
			printf(" A single node license costs $ 1,000,000/y + VAT when applicable\n");
			printf(" ---------------------------------------------------\n");
			exit(0);
		}
		
		if (strcmp("-h", argv[i]) == 0) {
			i++;
			printf(" Command line: %s [-cnf prefix] [-h] [setting=value] ...\n", argv[0]);
			printf("  -cnf prefix    : config parameter file name\n");
			printf("  -h             : this help\n");
			printf("  setting=value  : setting is the path of a libconfig setting defined in the default config file, value is its value; can be repeated\n");
			printf(" Example:\n");
			printf("  %s run.fe.mrc.sc.gain=64 run.fe.mrc.sc.DAC0=400\n",argv[0]);
			//printf("  %s bus.[0].mpd.[1].adc.[0].clock_phase=3 run.mode=\"event\" run.comment=\"This is an example\"\n",argv[0]);
			exit(0);
		}
		
/*
		./rich run.fe.fpga.pulser_frequency=30
		./rich run.fe.fpga.pulser_frequency=11
		./rich run.fe.fpga.external_pulser.Vhigh_mV=300
		./rich run.fe.external_pulser.Vhigh_mV=300
		./rich run.fe.external_pulser.Vhigh_mV=300
		./rich run.fe.external_pulser.Vhigh_mV=50.0
		./rich run.fe.external_pulser.Vhigh_mV=0.0
		./rich run.fe.mrc.dynamic_register.Ch0_31_Hold2=0x3
		./rich run.fe.mrc.dyn.Ch0_31_Hold2=0x3
		./rich run.fe.mrc.dyn.Ch0_31_Hold2=0
		./rich run.fe.mrc.dyn.Ch0_31_Hold2=1
		./rich run.fe.mrc.dyn.Ch0_31_Hold2=4
		./rich run.fe.mrc.dyn.Ch0_31_Hold2=0x4000
		./rich run.fe.mrc.dyn.Ch0_31_Hold2=96
		./rich run.fe.mrc.dyn.Ch0_31_Hold2=0
		./rich run.fe.mrc.sc.DAC0=350
		./rich run.fe.mrc.sc.DAC0=400
		./rich run.fe.mrc.sc.gain=64
*/		
		sarg = argv[i];
		cout << __FUNCTION__ << " process inline argument " << sarg << endl;
		size_t found;
		
		found=sarg.find_first_of("=");      
		
		if (found< std::string::npos) {
			fargp[fargc]=sarg.substr(0,found);
			fargv[fargc]=sarg.substr(found+1,sarg.length());
			//cout << __FUNCTION__ << " " << i << " / " << fargc << " : " << fargp[fargc] << " = " << fargv[fargc] << endl;
			fargc++;
			
		} else {
			cout << __FUNCTION__ << " warning: >" << sarg << "< attribute looks not properly formed, skipped"<< endl;
		}
		
    }
	

}

bool	TRich_Config::ParseFile(const char * filename){
	
	try
	{
		fcfg.readFile(filename);
		return true;
	}
	catch(const FileIOException &fioex)
	{
		std::cerr<<std::endl;
		std::cerr << "I/O error while reading file " << filename<< ", please check the path" <<std::endl;		
		return false;
	}
	catch(const ParseException &pex)
	{
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
		<< " - " << pex.getError() << std::endl;
		return false;
	}
}

void	TRich_Config::Edit(){
	for (int i=0;i<fargc;i++) {
		this->Replace(fargv[i], fargp[i]);
    	}
}


// Convert a string into a numeric type
// from string to T ( see http://forums.codeguru.com/showthread.php?t=231054)
template <class T> 
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&)) {
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}
int TRich_Config::Replace(const string value,string path){

	//printf("value = %s ",value.c_str());
	//printf("path = %s ",path.c_str());
	//printf("\n");
	
	long int i64;
	int i32;
	float flo;
	std::string str;
	bool boo;
	
	if (fcfg.exists(path)) {
		cout << __FUNCTION__ << ": " << path << " will be modified" << endl;
		Setting &s1 = fcfg.lookup(path);
		Setting::Type tp = s1.getType();
		
		
		switch (tp) {
				
			case Setting::TypeInt64:
				
				if (s1.getFormat() == Setting::FormatHex) {
					from_string<long int>(i64, value, std::hex);
				} else {
					from_string<long int>(i64, value, std::dec);
				}
				s1 = i64;
				break;
				
			case Setting::TypeInt:
				
				if (s1.getFormat() == Setting::FormatHex) {
					from_string<int>(i32, value, std::hex);
				} else {
					from_string<int>(i32, value, std::dec);
				}
				s1 = i32;
				break;
				
			case Setting::TypeFloat:
				
				from_string<float>( flo, value, std::dec);
				s1 = flo;
				break;
				
			case Setting::TypeString:
				
				s1 = value;
				break;
				
			case Setting::TypeBoolean:
				
				from_string<bool>( boo, value, std::dec );
				s1 = boo;
				break;
				
			default:
				cout << __FUNCTION__ << ": " << path << " type not recognized, nothing replaced" << endl;
				return -1;
				break;
		}	
	} else {
		cout << __FUNCTION__ << ": " << path << " setting does not exist, nothing replaced" << endl;
		return -1;
    }
	return 0;
}



void TRich_Config::Read(){
	
	frunprefix	= Read_String("run.name");

	fnote		= Read_String("run.note");
	
     
	fdaqmode	= fcfg.lookup("run.daq_mode");			
	fEv_preset	= fcfg.lookup("run.tdc_event_preset");	
	fTm_preset	= fcfg.lookup("run.tdc_time_preset");

  fSKA_duration   = fcfg.lookup("run.ska_duration");
  fSKA_repetition = fcfg.lookup("run.ska_iteration");

	fChsel = fcfg.lookup("run.fe.mrc.ch_sel");

	this->Read_Settings_MAROC_Common();		
//	this->Read_Settings_MAROC_Dynamic();
	this->Read_Settings_FPGA();
}


bool	TRich_Config::Init(){
	
	this->Setup();
	this->Gains();
	this->Thresholds();
	this->AddParam_MAROC3();
	
	return true;
}

void	TRich_Config::Thresholds()
{	
	int lthr_map = fcfg.lookup("run.thr_map");
	//cout << "Threshold settings from ";
	if (lthr_map==1) {
		string filename = "./cfg/thr/thr.txt";
		ifstream ff;
		int lthr;
		ff.open(filename.c_str(),std::ifstream::in);
		if (ff.is_open()) {
			cout << "threshold map "<< filename <<endl;;
			for (int i=0; i<3; i++) {
				ff >> lthr;//printf("thr[%d] = %d\n",i,lthr);
				fmpa[i].thr = lthr;
			}
			ff.close();
		}else {
			cout << "Error opening threshold map file"<< filename <<endl;
		}
	}else {
		//cout << "default value (DAC0 = "<< fthreshold_default <<")"<<endl;
		for (int i=0; i<3; i++) {
			fmpa[i].thr = fthreshold_default;
		}
	} 
}



void	TRich_Config::Gains(){
	


	int lgain_maps = fcfg.lookup("run.gain_maps");

	if (lgain_maps!=0) {
		for (int idx=0; idx<3; idx++) {
			char filename[100];
			sprintf(filename,"./cfg/gain/gain_map_%d.txt",idx);
			cout << "Gain settings from gain map "<< filename <<endl;
			ifstream fin;
			int lgain;
			fin.open(filename,std::ifstream::in);
			if (fin.is_open()) {
				for (int i=0; i<64; i++) {
					fin >> lgain;
					//printf("gain[%d][%2d] = %d\n",idx,i,lgain);
					fmpa[idx].gain[i] = lgain;
				}
				fin.close();
			}else {
				cout << "Error opening gain map file"<<endl;
			}
		}
	}else {
		cout << "Gain settings from default value (gword = "<< fgain_default <<")"<<endl;
		for (int idx=0; idx<3; idx++) {
			for (int i=0; i<64; i++) {
				fmpa[idx].gain[i] = fgain_default;  
				//printf("asic %d channel %d gain %d\n",idx,i);
			}
		}
	}
}


void	TRich_Config::Setup(){

	fmpa[0].did = 710; 
	fmpa[1].did = -1; 
	fmpa[2].did = 716; 
	
}	


string	TRich_Config::Read_String(string path){
	
	string sret;
	try {
		string s1 = fcfg.lookup(path.c_str());
		sret = s1;
	}
	catch (const SettingNotFoundException &nfex) {
		cerr << "No '" << path <<"' setting in configuration file." << endl;
		sret.clear();
	}
	return sret;
}



char TRich_Config::GetNasic(){
	

// obsolete must be removed

	//char ret = 0;
	//int ln = fcfg.lookup("run.fe.n_asic");
	//ret = (char) ln;
//	printf("NUMERO DI ASIC  = %d\n", ret);

	return 3;
}
char TRich_Config::GetNpmt(){


// obsolete must be removed

//	char ret = 0;
//	int ln = fcfg.lookup("run.fe.n_mapmt");
//	ret = (char) ln;
//	printf("NUMERO DI MAPMT  = %d\n", ret);
	
	return 3;
}


void TRich_Config::Read_Settings_External_Pulser(){
	Setting & root = fcfg.getRoot();
	
	Setting & qinj = root["run"]["fe"]["external_pulser"];	
	
	
	string deviceModel;
	qinj.lookupValue("LecroyModel",deviceModel);

//	printf("Device Model %s\n",deviceModel.c_str());

	qinj.lookupValue("Vhigh_mV",fext.vhigh);
	qinj.lookupValue("Vlow_mV",	fext.vlow);
	qinj.lookupValue("Width_us",fext.width);
	qinj.lookupValue("Delay_us",fext.delay);
	qinj.lookupValue("Lead_ns", fext.lead);	
	qinj.lookupValue("Trail_ns",fext.trail);	
	qinj.lookupValue("Period_us",fext.period);	

	
}

void TRich_Config::Read_Settings_FPGA(){
		
	Setting & root = fcfg.getRoot();

	try {

     		Setting & fpga = root["run"]["fe"]["fpga"];
	
		fpga.lookupValue("fpgaID",	ffp.fpgaID);
		fpga.lookupValue("firmwareVer",		ffp.firmware_version);
		fpga.lookupValue("pulser_frequency",ffp.pulser_freq);
		fpga.lookupValue("pulser_dutycycle",ffp.pulser_dutycycle);
		fpga.lookupValue("pulser_repetition",ffp.pulser_repetition);	
		fpga.lookupValue("trig_source",ffp.trig_source);	
		fpga.lookupValue("trig_delay",		ffp.trig_delay);	
		fpga.lookupValue("evtb_lookback",	ffp.evtbuild_lookback);	
		fpga.lookupValue("evtb_windowwidth",ffp.evtbuild_window);	
		fpga.lookupValue("evtb_blockNevent",ffp.block_nevents);	

		fpga.lookupValue("out0",ffp.out[0]);	
		fpga.lookupValue("out1",ffp.out[1]);	
		fpga.lookupValue("out2",ffp.out[2]);	


		ftrig_delay = ffp.trig_delay;
		fevtbuild_lookback = ffp.evtbuild_lookback;
		fevtbuild_window = ffp.evtbuild_window;
	}
	catch (const SettingNotFoundException &nfex) {
		cerr << " Error while getting FPGA settings" << endl;
	}	
}



unsigned int TRich_Config::GetTriggerDelay(){
	
	return (unsigned int) ftrig_delay;
	
}
unsigned int TRich_Config::GetEventBuilderLookBack(){
	
	return (unsigned int) fevtbuild_lookback;
	
}
unsigned int TRich_Config::GetEventBuilderWindow(){
	
	return (unsigned int) fevtbuild_window;
	
}	



void TRich_Config::Read_Settings_MAROC_Common(){
	
	Setting & root = fcfg.getRoot();
	
	try {
		Setting & sc = root["run"]["fe"]["mrc"]["sc"];
		
		sc.lookupValue("cmd_fsu",		fmp.cmd_fsu);
		sc.lookupValue("cmd_ss",		fmp.cmd_ss);
		sc.lookupValue("cmd_fsb",		fmp.cmd_fsb);
		sc.lookupValue("swb_buf_250f",	fmp.swb_buf_250f);
		sc.lookupValue("swb_buf_500f",	fmp.swb_buf_500f );
		sc.lookupValue("swb_buf_1p",	fmp.swb_buf_1p );
		sc.lookupValue("swb_buf_2p",	fmp.swb_buf_2p);
		sc.lookupValue("ONOFF_ss",		fmp.ONOFF_ss );
		sc.lookupValue("sw_ss_300f",	fmp.sw_ss_300f);
		sc.lookupValue("sw_ss_600f",	fmp.sw_ss_600f);
		sc.lookupValue("sw_ss_1200f",	fmp.sw_ss_1200f );
		sc.lookupValue("EN_ADC",		fmp.EN_ADC);
		sc.lookupValue("H1H2_choice",	fmp.H1H2_choice );
		sc.lookupValue("sw_fsu_20f",	fmp.sw_fsu_20f);
		sc.lookupValue("sw_fsu_40f",	fmp.sw_fsu_40f);
		sc.lookupValue("sw_fsu_25k",	fmp.sw_fsu_25k );
		sc.lookupValue("sw_fsu_50k",	fmp.sw_fsu_50k);
		sc.lookupValue("sw_fsu_100k",	fmp.sw_fsu_100k );
		sc.lookupValue("sw_fsb1_50k",	fmp.sw_fsb1_50k );
		sc.lookupValue("sw_fsb1_100k",	fmp.sw_fsb1_100k);
		sc.lookupValue("sw_fsb1_100f",	fmp.sw_fsb1_100f );
		sc.lookupValue("sw_fsb1_50f",	fmp.sw_fsb1_50f );
		sc.lookupValue("cmd_fsb_fsu",	fmp.cmd_fsb_fsu);
		sc.lookupValue("valid_dc_fs",	fmp.valid_dc_fs );
		sc.lookupValue("sw_fsb2_50k",	fmp.sw_fsb2_50k);
		sc.lookupValue("sw_fsb2_100k",	fmp.sw_fsb2_100k );
		sc.lookupValue("sw_fsb2_100f",	fmp.sw_fsb2_100f );
		sc.lookupValue("sw_fsb2_50f",	fmp.sw_fsb2_50f);
		sc.lookupValue("valid_dc_fsb2",fmp.valid_dc_fsb2 );
		sc.lookupValue("ENb_tristate",	fmp.ENb_tristate );
		sc.lookupValue("polar_discri",	fmp.polar_discri );
		sc.lookupValue("inv_discriADC",fmp.inv_discriADC );
		sc.lookupValue("d1_d2",		fmp.d1_d2 );
		sc.lookupValue("cmd_CK_mux",	fmp.cmd_CK_mux );
		sc.lookupValue("ONOFF_otabg",	fmp.ONOFF_otabg );
		sc.lookupValue("ONOFF_dac",	fmp.ONOFF_dac );
		sc.lookupValue("small_dac",	fmp.small_dac );
		sc.lookupValue("enb_outADC",	fmp.enb_outADC );
		sc.lookupValue("inv_startCmptGray",fmp.inv_startCmptGray );
		sc.lookupValue("ramp_8bit",	fmp.ramp_8bit);
		sc.lookupValue("ramp_10bit",	fmp.ramp_10bit);
		sc.lookupValue("DAC0",		fmp.DAC0);
		sc.lookupValue("DAC1",		fmp.DAC1 );
		sc.lookupValue("gain",		fmp.gain );
		
		
		fgain_default = fmp.gain;
		fthreshold_default = fmp.DAC0;

	}
	catch (const SettingNotFoundException &nfex) {
		cerr << " Error while getting MAROC3 settings" << endl;
	}
}



void TRich_Config::Retrieve_Settings_MAROC(int idx){
	
	Setting & root = fcfg.getRoot();
	
	Setting &asic = root["run"]["fe"]["asic"][idx];
	
	asic.lookupValue("marocID", fmpa[idx].did);
	asic.lookupValue("threshold", fmpa[idx].thr);
	
	for (int i=0; i<64; i++) {
		fmpa[idx].gain[i] = root["run"]["fe"]["asic"][idx]["gain"][i];
	}	
}

void	TRich_Config::Print(){
	
	this->PrintRUN();
	this->PrintFPGA();
	this->PrintMAROC_StaticReg();
	this->PrintMAROC_DynamcReg();
	this->PrintMAROC_adj();
//	this->PrintEXTERNAL_Pulser();

}
void TRich_Config::PrintRUN(){
	
	printf("Settings Run\n");
	printf("\tPrefix: %s\n\tNotes:  %s\n",frunprefix.c_str(),fnote.c_str());

	printf("\tDAQ mode     : %d\n",fdaqmode);

	if(fdaqmode==0)
	{
		printf("\tScaler duration : %d\n",fSKA_duration);	
		printf("\tScaler repetition : %d\n",fSKA_repetition);	
	}else{
		printf("\tTDC event preset : %d\n",fEv_preset);	
		printf("\tTDC time preset : %d\n",fTm_preset);	

	}		
}
void TRich_Config::PrintFPGA(){
	
	printf("Settings FPGA\n");
	printf("\tFPGA ID: %d\n",ffp.fpgaID);
	printf("\tFirmware Version %d\n",ffp.firmware_version);
		
	printf("\tTRIGGER:\n");
	printf("\t\ttrig_source= %d ;\n",ffp.trig_source);
	printf("\t\ttrig_delay = %d [clock ticks 8ns]\n",ffp.trig_delay);
	
	printf("\tEVENT BUILDER:\n");		
	printf("\t\tlookback   = %d [clock ticks 8ns]\n",ffp.evtbuild_lookback);	
	printf("\t\twindow     = %d [clock ticks 8ns]\n",ffp.evtbuild_window);
	printf("\t\tblocksize  = %d [events/block]\n",ffp.block_nevents);

	printf("\tPULSER:\n");
	printf("\t\tfrequency  = %d [Hz]\n",ffp.pulser_freq);
	printf("\t\tdutycycle  = %.3f\n",ffp.pulser_dutycycle);	
	printf("\t\trepetition = %d [#]\n",ffp.pulser_repetition);	
}

void TRich_Config::PrintMAROC_StaticReg(){
	
	printf("Settings MAROC3 (common) \n");
	printf("\t\tcmd_fsu           = %d\n", fmp.cmd_fsu);
	printf("\t\tcmd_ss            = %d\n", fmp.cmd_ss);
	printf("\t\tcmd_fsb           = %d\n", fmp.cmd_fsb);
	printf("\t\tswb_buf_250f      = %d\n", fmp.swb_buf_250f);
	printf("\t\tswb_buf_500f      = %d\n", fmp.swb_buf_500f);
	printf("\t\tswb_buf_1p        = %d\n", fmp.swb_buf_1p);
	printf("\t\tswb_buf_2p        = %d\n", fmp.swb_buf_2p);
	printf("\t\tONOFF_ss          = %d\n", fmp.ONOFF_ss);
	printf("\t\tsw_ss_300f        = %d\n", fmp.sw_ss_300f);
	printf("\t\tsw_ss_600f        = %d\n", fmp.sw_ss_600f);
	printf("\t\tsw_ss_1200f       = %d\n", fmp.sw_ss_1200f);
	printf("\t\tEN_ADC            = %d\n", fmp.EN_ADC);
	printf("\t\tH1H2_choice       = %d\n", fmp.H1H2_choice);
	printf("\t\tsw_fsu_20f        = %d\n", fmp.sw_fsu_20f);
	printf("\t\tsw_fsu_40f        = %d\n", fmp.sw_fsu_40f);
	printf("\t\tsw_fsu_25k        = %d\n", fmp.sw_fsu_25k);
	printf("\t\tsw_fsu_50k        = %d\n", fmp.sw_fsu_50k);
	printf("\t\tsw_fsu_100k       = %d\n", fmp.sw_fsu_100k);
	printf("\t\tsw_fsb1_50k       = %d\n", fmp.sw_fsb1_50k);
	printf("\t\tsw_fsb1_100k      = %d\n", fmp.sw_fsb1_100k);
	printf("\t\tsw_fsb1_100f      = %d\n", fmp.sw_fsb1_100f);
	printf("\t\tsw_fsb1_50f       = %d\n", fmp.sw_fsb1_50f);
	printf("\t\tcmd_fsb_fsu       = %d\n", fmp.cmd_fsb_fsu);
	printf("\t\tvalid_dc_fs       = %d\n", fmp.valid_dc_fs);
	printf("\t\tsw_fsb2_50k       = %d\n", fmp.sw_fsb2_50k);
	printf("\t\tsw_fsb2_100k      = %d\n", fmp.sw_fsb2_100k);
	printf("\t\tsw_fsb2_100f      = %d\n", fmp.sw_fsb2_100f);
	printf("\t\tsw_fsb2_50f       = %d\n", fmp.sw_fsb2_50f);
	printf("\t\tvalid_dc_fsb2     = %d\n", fmp.valid_dc_fsb2);
	printf("\t\tENb_tristate      = %d\n", fmp.ENb_tristate);
	printf("\t\tpolar_discri      = %d\n", fmp.polar_discri);
	printf("\t\tinv_discriADC     = %d\n", fmp.inv_discriADC);
	
	
	printf("\t\td1_d2             = %d\n", fmp.d1_d2);
	printf("\t\tcmd_CK_mux        = %d\n", fmp.cmd_CK_mux);
	printf("\t\tONOFF_otabg       = %d\n", fmp.ONOFF_otabg);
	printf("\t\tONOFF_dac         = %d\n", fmp.ONOFF_dac);
	printf("\t\tsmall_dac         = %d\n", fmp.small_dac);
	printf("\t\tenb_outADC        = %d\n", fmp.enb_outADC);
	printf("\t\tinv_startCmptGray = %d\n", fmp.inv_startCmptGray);
	printf("\t\tramp_8bit         = %d\n", fmp.ramp_8bit);
	printf("\t\tramp_10bit        = %d\n", fmp.ramp_10bit);
	printf("\t\tDAC0              = %d\n", fmp.DAC0);
	printf("\t\tDAC1              = %d\n", fmp.DAC1);	
	
	printf("\t\tGAIN              = %d\n", fmp.gain);	
}


void TRich_Config::PrintMAROC_adj(){
	
	printf("Settings MAROC3 (single channel)  \n");
	
	for (int idx = 0; idx<3; idx++) {
		printf("\tID = %d\n",fmpa[idx].did);
		printf("\tThreshold = %d\n",fmpa[idx].thr);
		
		printf("\t\t");
		for (int i=0; i<64; i++) {
			printf("%3d  ",fmpa[idx].gain[i]);
			
			if ((i%8)==7) {
				printf("\n");
				printf("\t\t");
			}
		}
		printf("\n");
	}
}

void TRich_Config::PrintEXTERNAL_Pulser(){
	
	printf("Settings External Pulser:\n");
	
	printf("\tAMPLITUDE\n");	
	printf("\t\tHigh  = %6.1f [mV]\n",fext.vhigh);
	printf("\t\tLow   = %6.1f [mV]\n",fext.vlow);
	printf("\tTIME\n");		
	printf("\t\tWidth = %6.1f [us]\n",fext.width);
	printf("\t\tDelay = %6.1f [us]\n",fext.delay);
	printf("\t\tLead  = %6.1f [ns]\n",fext.lead);	
	printf("\t\tTrail = %6.1f [ns]\n",fext.trail);	
	printf("\t\tPeriod= %6.1f [us]\n",fext.period);	

}




RICH_fpga_t *			TRich_Config::Get_Settings_Fpga(){return &ffp;}
RICH_maroc_common_t *	TRich_Config::Get_Settings_Maroc_Common(){return &fmp;}			
RICH_maroc_t *			TRich_Config::Get_Settings_Maroc(int idx){return &fmpa[idx];}

int	TRich_Config::Get_Ch00_31_Hold1(){return fCh0_31_Hold1;}
int	TRich_Config::Get_Ch32_63_Hold1(){return fCh32_63_Hold1;}
int	TRich_Config::Get_Ch00_31_Hold2(){return fCh0_31_Hold2;}
int	TRich_Config::Get_Ch32_63_Hold2(){return fCh32_63_Hold2;}


void	TRich_Config::Read_Settings_MAROC_Dynamic(){
	


/*
	fCh0_31_Hold1	= fcfg.lookup("run.fe.mrc.dyn.Ch0_31_Hold1");
	fCh32_63_Hold1	= fcfg.lookup("run.fe.mrc.dyn.Ch32_63_Hold1");
	fCh0_31_Hold2	= fcfg.lookup("run.fe.mrc.dyn.Ch0_31_Hold2");
	fCh32_63_Hold2	= fcfg.lookup("run.fe.mrc.dyn.Ch32_63_Hold2");
	*/
}
void	TRich_Config::PrintMAROC_DynamcReg(){
/*
	printf("Settings MAROC3 (dynamic register) \n");
	

	printf("\t\tCh0_31_Hold1  = %u or HEXADECIMAL 0x%X\n",fCh0_31_Hold1, fCh0_31_Hold1);	
	printf("\t\tCh32_63_Hold1 = %u or HEXADECIMAL 0x%X\n",fCh32_63_Hold1,fCh32_63_Hold1);	
	printf("\t\tCh0_31_Hold2  = %u or HEXADECIMAL 0x%X\n",fCh0_31_Hold2, fCh0_31_Hold2);	
	printf("\t\tCh32_63_Hold2 = %u or HEXADECIMAL 0x%X\n",fCh32_63_Hold2,fCh32_63_Hold2);
*/
/*
	printf("\t\tCh0_31_Hold1  = 0x%X\n",fCh0_31_Hold1);	
	printf("\t\tCh32_63_Hold1 = 0x%X\n",fCh32_63_Hold1);	
	printf("\t\tCh0_31_Hold2  = 0x%X\n",fCh0_31_Hold2);	
	printf("\t\tCh32_63_Hold2 = 0x%X\n",fCh32_63_Hold2);
	*/
}

int		TRich_Config::Threshold(int value){
	if (value != -1) {
		fthreshold_default = value;
	}
	return fthreshold_default;
}

int		TRich_Config::Gain(int value){
	if (value != -1) {
		fgain_default = value;
	}
	return fgain_default;
}


int		TRich_Config::GetGain(int idx,int channel){

	if ((idx==0)||(idx==1)||(idx==2)) {
		return fmpa[idx].gain[channel];
	}else {
		printf("Error: This software is for 1 Tile setup\n");
		return -1;
	}
}
int		TRich_Config::GetThr(int idx){
	
	if ((idx==0)||(idx==1)||(idx==2)) {
		return fmpa[idx].thr;
	}else {
		printf("Error: This software is for 1 Tile setup\n");
		return -1;
	}
}


string	TRich_Config::RunPrefix(){return frunprefix;}


int	TRich_Config::Ch_Sel(int val){
	if (val!=-1) {
		fChsel = val;
	}
	return fChsel;
}



int	TRich_Config::Daq_Mode(int val){
	if (val!=-1) {
		fdaqmode = val;
	}
	return fdaqmode;
}
int	TRich_Config::EventPreset(int val){
	if (val!=-1) {
		fEv_preset = val;
	}
	return fEv_preset;
}
int	TRich_Config::TimePreset(int val){
	if (val!=-1) {
		fTm_preset = val;
	}
	return fTm_preset;
}

int	TRich_Config::ScalerDuration(int val){
	if (val!=-1) {
		fSKA_duration = val;
	}
	return fSKA_duration;
}

int	TRich_Config::ScalerRepetition(int val){
	if (val!=-1) {
		fSKA_repetition = val;
	}
	return fSKA_repetition;
}






void 	TRich_Config::Logbook(std::string setting_name,int ival){

	Setting & root = fcfg.getRoot();
	if(! root.exists("logbook")){root.add("logbook",Setting::TypeGroup);}
	Setting &logbook = root["logbook"];	
	
	logbook.add(setting_name, Setting::TypeInt)= ival;
}


void 	TRich_Config::Logbook(std::string setting_name,long long lval){

	Setting & root = fcfg.getRoot();
	if(! root.exists("logbook")){root.add("logbook",Setting::TypeGroup);}
	Setting &logbook = root["logbook"];	
	
	logbook.add(setting_name, Setting::TypeInt64)= lval;
}


void 	TRich_Config::Logbook(std::string setting_name,float fval){

	Setting & root = fcfg.getRoot();
	if(! root.exists("logbook")){root.add("logbook",Setting::TypeGroup);}
	Setting &logbook = root["logbook"];	
	
	logbook.add(setting_name, Setting::TypeFloat)= fval;

}
void 	TRich_Config::Logbook(std::string setting_name,std::string sval){
	
	Setting & root = fcfg.getRoot();
	if(! root.exists("logbook")){root.add("logbook",Setting::TypeGroup);}
	Setting &logbook = root["logbook"];	

	logbook.add(setting_name, Setting::TypeString)= sval.c_str();
}




void 	TRich_Config::Logbook (int nEv_Acq,float n_seconds){
	Setting & root = fcfg.getRoot();
	if(! root.exists("logbook")){root.add("logbook",Setting::TypeGroup);}
	Setting &logbook = root["logbook"];	
	
	logbook.add("Events", Setting::TypeInt)= nEv_Acq;
	logbook.add("Duration", Setting::TypeFloat)= n_seconds;		
}

void	TRich_Config::Export(string filename){
	
	string lfn = filename;
	fcfg.setTabWidth(TABWIDTH);
	fcfg.writeFile(lfn.c_str());
	printf("Logbook successfully written to %s\n",lfn.c_str());

}
int TRich_Config::GetLogNEvents(){
	int lnEv = 0;
	lnEv = fcfg.lookup("logbook.Events");
	return lnEv;
}

float TRich_Config::GetLogNSeconds(){
	float lnsec = 0;
	lnsec = fcfg.lookup("logbook.Duration");
	return lnsec;
}

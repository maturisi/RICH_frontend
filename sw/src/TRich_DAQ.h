/*
 *  TRich_DAQ.h
 *  
 *
 *  Created by Matteo Turisini on 28/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef RICH_DAQ_H
#define RICH_DAQ_H

#include <string>
#include <unistd.h> //usleep


#include "TRich_Frontend.h"
#include "TRich_Config.h"


#define MRC_RUN_NUMBER_FILE	 "../cfg/last.run"

#define OUT_PATH "../data/out/"

#define SUFFIX_RAW ".bin"
#define SUFFIX_TXT ".txt"
#define SUFFIX_LOG ".log"

#define SUFFIX_TDC "_TDC_"
#define SUFFIX_SCALER "_Scaler_"

#define MODE_SCALER	0
#define MODE_TDC	1


#define	GIGA	1000000000
#define PROGRESS_STEP	20


using namespace std; // e.g signal.h for interrupt from keybard

class TRich_DAQ{

private:
  string frunName; // binary output
  string flogName; // logbook output
  string ftxtName; // plain text output
	
  TRich_Config * fcfg;	
  TRich_Frontend * ffe;

  FILE * fraw;
  
  int fnevts;
  float fduration;
int fprogress_old;
  
  int ReadRunNum();
  int WriteRunNum(int rr);
  FILE * OpenOutFile(std::string file_prefix);
  
  string TimeString(); // date and hour
  
  timespec fstart;

	int nmeas;  
	
	void 	TimeStart(); 
  float 	TimeNow(); 
  void  ProgressBar();	

  
 public:
  TRich_DAQ();	
  ~TRich_DAQ();
  
  void SetFrontend(TRich_Frontend * fe=NULL); 
  void SetConfiguration(TRich_Config * cfg=NULL);
 
  void PreEv();
  long long DoEv(int verbosity=0);
  void PostEv();
  
  string GetRunName();
  string GetLogName();
};
#endif

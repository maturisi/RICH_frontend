/*
 *  TRich_DAQ.h
 *
 *  Created by Matteo Turisini on 28/03/15. 
 */

#ifndef RICH_DAQ_H
#define RICH_DAQ_H

#include <string>
#include "TRich_Frontend.h"
#include "TRich_Config.h"

class TRich_DAQ{

private:
	/*i/o files (configuration, data and logbook)*/

  std::string 		frunName;  
  std::string 		flogName;   
	FILE * 					fraw;			
  TRich_Config * 	fcfg;

	/* hardware */	
  TRich_Frontend *ffe;
  
	/* Statistics: event,time, duration */
  int 						fnevts;
  float 					fduration;
	int 						fprogress_old;
	int 						nmeas;  
	timespec 				fstart;

 public:
  TRich_DAQ();	
  ~TRich_DAQ();
  
  void SetFrontend(TRich_Frontend * fe=NULL); 
  void SetConfiguration(TRich_Config * cfg=NULL);
 
  void PreEv();
  long long DoEv(int verbosity=0);
  void PostEv();
  
  std::string GetRunName();
  std::string GetLogName();

private:
  int 						ReadRunNum();
  int 						WriteRunNum(int rr);
	FILE * 					OpenOutFile(std::string file_prefix);
  std::string 		TimeString(); 
	void 						TimeStart(); 
  float 					TimeNow(); 
  void  					ProgressBar();	

};
#endif

/*
 *  TRich_DAQ.cpp
 *  
 *
 *  Created by Matteo Turisini on 28/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h> // printf
#include <time.h>       /* time_t, struct tm, time, localtime */

#include <iostream> // cerr, endl
#include <signal.h>	// SIGINT, signal
#include <stdint.h>  
#include <unistd.h> //usleep


#include "TRich_DAQ.h"
#include "TRich_Scaler.h"

#define MRC_RUN_NUMBER_FILE	 	"../cfg/last.run"
#define OUT_PATH 							"../../data/out/"

#define MODE_SCALER						0
#define MODE_TDC							1
#define MODE_REGISTER_TEST		2
#define MODE_QSCAN						3
#define MODE_CONFIGUREONLY		4
#define MODE_ADC 							5
#define MODE_WAVEFORM					6

/* Signal Handling*/
static int sig_int=1;
void ctrl_c(int){ 
	sig_int =0;
	printf("\n CTRL-C pressed\n\r");
}


TRich_DAQ::TRich_DAQ()
{		
	fcfg = NULL;
	ffe = NULL ;
	nmeas =0;
	fprogress_old=0;
}

TRich_DAQ::~TRich_DAQ()
{

}

void TRich_DAQ::SetFrontend(TRich_Frontend * fe)
{
  if ( fe==NULL) cout << "Warning: NO front end"<<endl;
  
	ffe = fe ;   
}

void TRich_DAQ::SetConfiguration(TRich_Config * cfg)
{
  if (cfg==NULL) cout << "Warning: NO configuration"<<endl;
  
	fcfg = cfg;	
}


long TRich_DAQ::TimeEpoch()
{
	time_t sec;	
	
	time(&sec);

	//printf(" %ld seconds since epoch\n",(long)sec);
	
	return (long) sec;
}


std::string TRich_DAQ::TimeString()
{
	time_t	sec;	
	
	time(&sec);
	
	struct tm * ltim = localtime(&sec); 

	std::string dateStr =  asctime(ltim);

	return dateStr;
}


void 	TRich_DAQ::TimeStart()
{

#if __APPLE__	
  printf("I am on Mac OsX\n");
  //*pt = mach_absolute_time();// pt is a uint64_t
#else
	clock_gettime(CLOCK_REALTIME,&fstart);
#endif

}


float 	TRich_DAQ::TimeNow()
{
	float lduration = 0.0;
	nmeas++;
#if __APPLE__
	printf("I am on Mac OsX\n");
#else // linux

	timespec mark1;
	clock_gettime(CLOCK_REALTIME,&mark1);

	uint64_t t = mark1.tv_sec;
	uint64_t t_ns = mark1.tv_nsec;

  //printf("Time Now  : %12ld sec + %12ld nsecond ",t,t_ns);

	uint64_t duration;
	uint64_t duration_ns; 

	uint64_t giga = 10.0E9; 



	if(t_ns>(uint64_t)fstart.tv_nsec){
		duration = t - fstart.tv_sec;
		duration_ns =t_ns -fstart.tv_nsec ; 

	}else{
		duration = t - fstart.tv_sec-1;
		duration_ns = giga + t_ns -fstart.tv_nsec; 
	}

//	printf("Duration  : %12ld sec + %12ld nsecond\n",__FUNCTION__,duration,duration_ns);
//	printf("%s : %12ld sec + %12ld nsecond\n",__FUNCTION__,duration,duration_ns);

	lduration = (float)duration + ((float) duration_ns)/giga; 

	//printf("%6d %.9lf\n",nmeas,lduration); 
#endif
	return lduration;
	
}

void TRich_DAQ::PreEv()
{
	fraw = this->OpenOutFile(fcfg->RunPrefix());
	
	printf ("Date: %s", this->TimeString().c_str());

	this->TimeStart();
}




long long  TRich_DAQ::DoEv(int verbosity){
		
  signal(SIGINT,ctrl_c); // if receives ctrl-c from the keyboard execute ctrl_c

	// read DAQ settings
  int mode	   			= fcfg->Daq_Mode();
  int event_preset 	= fcfg->EventPreset() ;
  int time_preset  	= fcfg->TimePreset(); 
  int iteration    	= fcfg->ScalerRepetition();
  int duration    	= fcfg->ScalerDuration();
  

  //int n=0;
  long long nevents=0; 
  long long  nbytes = 0;
  long long dbytes = 0;;

  int val = 0;

	long sec;
	int err;

	int dac  = 0;

	TRich_Scaler * scaler;

  ffe->StartDAQ();
 

	int n;
	bool dbg = false; 
//	bool onlineparse=false;

  switch(mode){



  case MODE_TDC:
    
		printf("RICH - waiting for TDC event data...current run will end in %d seconds or press ctrl-c\n",time_preset );

    do{
   
	 		// Receive one word from from FPGA
 			n = ffe->Receive(&val);
		
			
			if(dbg){
				if((val==0)||(n==-1)){
				//	printf("Received nothing\n");
					continue;
				}else{
					printf("Received %d bytes, val = 0x%X\n",n,val);
				}
			}
	
/*		
			if(onlineparse){

 		
				if(((val & 0xF8000000) == 0x90000000))
				{						
					printf("HEADER found\n");		
				}
			}
*/
		 	// Write binary data on output file
			fwrite(&val, sizeof(val), 1, fraw);
    
			// Calculate duration
		 	fduration = this->TimeNow();
      
			// Calculate the amount of data in bytes
			nbytes+= sizeof(val); // total (since run began)
      dbytes+= sizeof(val); // partial (since last receivement)

			// Calculate the number of events by counting the number of headers found in raw data
      if((val & 0xF8000000) == 0x90000000)
			{
				nevents++;

				// print
				if(!(nevents % ((long long)(event_preset/10.))))
				{
	    		printf("[0x%08X] nevents = %8lld, nbytes = %8lld, dbytes = %8lld, duration %8lf \n", val, nevents, nbytes, dbytes,fduration);
	//	    		printf("[0x%08X] nevents = %8lld, Rate =%.1lf kevts/s, %.1lf MB/s\n", val, nevents, 0.001*nevents/fduration,0.000001*nbytes/fduration);
						fflush(stdout);
	   				dbytes = 0;     
	 		 	}
      }       
    }while(fduration<time_preset&&sig_int&&nevents<=event_preset);	  

    break;
    
  



	case MODE_QSCAN:

		while(1){
			
			ffe->AdjPulseAmplitude(dac);
			dac = dac + 2;
			if(dac > 1500) dac  = 0;
			usleep(10000);
			if(sig_int==0){break;} 
		}	
		break;


	case MODE_REGISTER_TEST:
	
		printf("TEST REGISTER: cycles %d [#], pause %d [s] \n",iteration,duration);

		for(int i=0;i<iteration;i++ )
		{
			nevents++;
			sec = this->TimeEpoch();		
			err = ffe->CheckMAROC();		

			printf("%lld) TIME EPOCH %ld MAROC ERRORS %d\n",nevents,sec,err);	
			if(err){fprintf(fraw, "%ld %d\n",sec,err);}

			if(i%10==5)
			{
				ffe->InjErrors();
			}
			if(sig_int==0){break;} 
			
			//usleep(duration*1000); // duration in milli seconds
			sleep(duration); // duration in seconds
		}

		break;

	case MODE_SCALER:
    
		printf("SCALER: Duration %d seconds, repetition %d \n",duration,iteration);
		ffe->DumpScalers(); // first measure skipped

		for(int i=0;i<iteration;i++) {
			
			sleep(duration);

			// old solution  Dump ritorna la somma dei conteggi
//			nevents += ffe->DumpScalers(fraw,true);

			// new solution

			scaler = ffe->DumpScaler();

			scaler->CalculateStatistics();

			if(1) scaler->Print(fraw); 	

			nevents = scaler->getActive(-1); // number of active channels is returned	
		
			delete scaler;

			if(sig_int==0){break;} 
		}
		break;


	    
  default: 
    printf("Unknown DAQ mode, please use 0 for scalers, 1 for TDC,2 for Slow Control Bit Flip Search\n");
    break;
  }
		 fduration = this->TimeNow();
  printf("\n");	
	fnevts= nevents;
	return nevents;
}


void TRich_DAQ::PostEv(){

	fclose(fraw);

	printf("DAQ Completed :\n");
	printf ("Date         : %s", this->TimeString().c_str());// time and date
  printf("Events[#]     : %6d \n",fnevts); // here is active channel in the case of MODE_SCALER:
	printf("Duration [s]  : %6.3lf\n",fduration);
	printf("Rate [kEvts/s] : %6.3lf\n",(fnevts/fduration)/1000.);
	printf("Data successfully written to %s \n",this->GetRunName().c_str());

	printf("LOG FILE %s\n",this->GetLogName().c_str());
  fcfg->Logbook("Events",fnevts);
  fcfg->Logbook("Duration",fduration);

	//fcfg->Print();

	fcfg->Export(this->GetLogName().c_str());
}





std::string TRich_DAQ::GetRunName(){return frunName;}
std::string TRich_DAQ::GetLogName(){return flogName;}


int TRich_DAQ::ReadRunNum(){

	FILE *ff;
	int rr;
	ff = fopen(MRC_RUN_NUMBER_FILE,"r");
	if (ff) {
		fscanf(ff,"%d",&rr);
		fclose(ff);
	} else {
		rr = -1;
	}
	return rr;
	
}
int TRich_DAQ::WriteRunNum(int rr){
	FILE *ff;
	ff = fopen(MRC_RUN_NUMBER_FILE,"w");
	if (ff) {
		fprintf(ff,"%d",rr);
		fclose(ff);
	} else {
		printf("WARNING: cannot update run file %s",MRC_RUN_NUMBER_FILE);
	}
	return 0;
}
FILE * TRich_DAQ::OpenOutFile(std::string file_prefix){

  int mode = fcfg->Daq_Mode();	

	std::string suffix = ".bin";
	if(mode!=MODE_TDC)
	{
		suffix = ".txt";  
	}


	FILE *fp;	
	int irun = this->ReadRunNum()+1;
	this->WriteRunNum(irun);
	
	char numstr[6]; // enough to hold all numbers up to 6 digits
	sprintf(numstr, "_%06d", irun);
	
	// binary output file 
	frunName.clear();
	frunName += OUT_PATH;
	frunName += file_prefix;	
	frunName += numstr;
	frunName += suffix;
//frunName += ".bin";
		

	// logbook output file 
	flogName.clear();
	flogName += OUT_PATH;
	flogName += file_prefix;	
	flogName += numstr;
	flogName += ".log";
	
	if ( (fp=fopen(frunName.c_str(), "wb")) == NULL ){
		printf("Cannot open output file %s\n",frunName.c_str());
		return NULL;
	}
	return fp;
}





// An idea for a ProgressBar
/*
void  	TRich_DAQ::ProgressBar(){

	int step = 20;

  int event_preset 	= fcfg->EventPreset() ;
  int time_preset  	= fcfg->TimePreset(); 

	//printf("Durata attuale %lf ",fduration);
	//printf("preset tempo %d ",time_preset);
	//printf("ratio  %lf\n",fduration/time_preset);
	//printf("ratio x step  %lf\n",(fduration/time_preset)*step);

	int progress_time = (fduration/time_preset)*step; 
	
	//printf("Eventi Attuali %d ",fnevts);
	//printf("preset eventi  %d ",event_preset);
	//printf("ratio evt %lf\n",((float)fnevts)/event_preset);
	//printf("ratio evt x step  %lf\n",(fnevts/event_preset)*step);

	int progress_evt = (((float) fnevts)/event_preset)*step;
	
	int progress = progress_time > progress_evt ? progress_time : progress_evt;

	progress = progress < step ? progress : step;

	//printf("progress time %d\n",progress_time);
	//printf("progress evt  %d\n",progress_evt);
	//printf("progress      %d\n",progress);
	
	for (int jprog = 0; jprog<(progress - fprogress_old); jprog++)
	{
		if (progress_time > progress_evt) 
		{
			printf("T");
		} else {
			printf("E");
		}
		fflush(stdout);
	}
	
	if (progress == step){} // 
	fprogress_old = progress;

}
*/



/*
 *  TRich_Scaler.h
 */

// TO DO: 
// add the time duration (ref) to data memebers export it in the ouput file and change the format accordingly
// Calculate more statistics if needed (RMS, Min, MAx)

#include <stdio.h>
#ifndef TRICH_SCALER_H
#define TRICH_SCALER_H

class TRich_Scaler {	
private:
	// data from FPGA board
	unsigned int fScaler[192];
	unsigned int fOr0[3];	
	unsigned int fOr1[3];
	unsigned int fInput[3];	
	unsigned int fOutput[3];	
	unsigned int fTrig;					
	unsigned int fGClk125;	
	unsigned int fSync;	
	unsigned int fBusy;	
	unsigned int fBusyCycles;	

	// statistics for single MAROC 64 channels

	
	double fmean[3];
	double frms[3]; 
 int factive[3];
	
	
	// number of active channels i.e. !=0

public:
	TRich_Scaler();
	~TRich_Scaler();
	
	void Reset();
	
	void Print( FILE * fout = NULL);

	void CalculateStatistics();
	
	void Write(const char * filename);

	double getMean(int asic);
	double getRMS(int asic);
	int getActive(int asic);	

	
	// get/set 
	// default argument equal to 0 is for getting the value
	// any other value of the argument is for setting the variable
	// set zero is not allowed, use Reset instead 
	// if you need to set zero a single variable please create new method like ResetTrig(); 

	unsigned int GClk125(unsigned int val=0);
	unsigned int Busy(unsigned int val=0);
	unsigned int BusyCycles(unsigned int val=0);
	unsigned int Sync(unsigned int val=0);
	unsigned int Trig(unsigned int val=0);	
	unsigned int Input(int idx = 0,unsigned int val=0);	
	unsigned int Output(int idx = 0,unsigned int val=0);	
	unsigned int Or0(int idx = 0,unsigned int val=0);	
	unsigned int Or1(int idx = 0,unsigned int val=0);	
	unsigned int Scaler(int idx = 0,unsigned int val=0);

};
#endif

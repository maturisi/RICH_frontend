/*
 *  TRich_Scaler.h
 *  
 *
 *  Created by Matteo Turisini on 04/04/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef TRICH_SCALER_H
#define TRICH_SCALER_H


//#include <TObject.h>

//class TRich_Scaler : public TObject {
class TRich_Scaler {	
private:
	
	unsigned int fGClk125;	
	unsigned int fBusy;	
	unsigned int fBusyCycles;	
	unsigned int fSync;	
	unsigned int fTrig;	
	unsigned int fInput[3];	
	unsigned int fOutput[3];	
	unsigned int fOr0[3];	
	unsigned int fOr1[3];
	unsigned int fScaler[192];
	
public:
	TRich_Scaler();
	~TRich_Scaler();
	
	void Reset();
	
	void Print();	
	
	// get/set (cannot set 0, use reset instead);
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
	
//	ClassDef (TRich_Scaler, 1) // scaler measurements
};

#endif
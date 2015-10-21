/*
 *  TRich_Scaler.cpp
 *  
 *
 *  Created by Matteo Turisini on 04/04/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>

#include "TRich_Scaler.h"


TRich_Scaler::TRich_Scaler(){
	
	Reset();
}

TRich_Scaler::~TRich_Scaler(){
		//printf("delete scaler\n");
};

void TRich_Scaler::Reset(){

	fGClk125=0;	
	fBusy=0;
	fBusyCycles=0;	
	fSync=0;
	fTrig=0;
	
	// clean 
	for (int i=0; i<192; i++) {
		fScaler[i]=0;
		if (i<3) {
			fInput[i]=0;	
			fOutput[i]=0;	
			fOr0[i]=0;	
			fOr1[i]=0;
			if (i<1) {
				fGClk125=0;	
				fSync=0;
				fTrig=0;
				fBusy=0;
				fBusyCycles=0;	
			}
		}
	}	
}


void TRich_Scaler::Print(){
	
	//printf("SCALER Measurements\n");
	printf("GClk125 = %10d\n",fGClk125);
	printf("BusyCyc = %10d\n",fBusyCycles);
	printf("Busy    = %10d\n",fBusy);
	printf("Sync	= %10d\n",fSync);
	printf("Trig	= %10d\n",fTrig);
	printf("Input	= %10d,%10d,%10d\n",fInput[0],fInput[1],fInput[2]);
	printf("Output	= %10d,%10d,%10d\n",fOutput[0],fOutput[1],fOutput[2]);
	printf("OR0	= %10d %10d %10d\n",fOr0[0],fOr0[1],fOr0[2]);	
	printf("OR1	= %10d %10d %10d\n",fOr1[0],fOr1[1],fOr1[2]);	
	
	for (int i=0; i<192; i++) {
		
		printf("%10d",fScaler[i]);
		
		if (i%8==7){			
			printf("\n");
		}
		if (i%64==63){			
			printf("\n");
		}
	}
}




// get/set (cannot set 0, use reset instead);

unsigned int TRich_Scaler::GClk125(unsigned int val){
	if (val!=0) {
		fGClk125 = val;
	}
	return fGClk125;
}	
unsigned int TRich_Scaler::Busy(unsigned int val){
	if (val!=0) {
		fBusy = val;
	}
	return fBusy;
}
unsigned int TRich_Scaler::BusyCycles(unsigned int val){
	if (val!=0) {
		fBusyCycles = val;
	}
	return fBusyCycles;
}	
unsigned int TRich_Scaler::Sync(unsigned int val){
	if (val!=0) {
		fSync = val;
	}
	return fSync;
}
unsigned int TRich_Scaler::Trig(unsigned int val){
	if (val!=0) {
		fTrig = val;
	}
	return fTrig;
}

unsigned int  TRich_Scaler::Input(int idx,unsigned int val){
	
	if (idx==0||idx==1||idx==2) {
		if (val!=0) {
			fInput[idx]=val;
		}
	}
	return fInput[idx];
}
unsigned int  TRich_Scaler::Output(int idx,unsigned int val){
	
	if (idx==0||idx==1||idx==2) {
		if (val!=0) {
			fOutput[idx]=val;
		}
	}
	return fOutput[idx];

}	
unsigned int TRich_Scaler::Or0(int idx,unsigned int val){
	if (idx==0||idx==1||idx==2) {
		if (val!=0) {
			fOr0[idx]=val;
		}
	}
	return fOr0[idx];

}
unsigned int TRich_Scaler::Or1(int idx,unsigned int val){
	if (idx==0||idx==1||idx==2) {
		if (val!=0) {
			fOr1[idx]=val;
		}
	}
	return fOr1[idx];

}	
unsigned int TRich_Scaler::Scaler(int idx,unsigned int val){
	
	if (idx>=0&&idx<=191) {
		if (val!=0) {
			fScaler[idx]=val;
		}
	}
	return fScaler[idx];
}



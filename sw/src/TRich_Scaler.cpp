/*
 *  TRich_Scaler.cpp
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
	
	for (int i=0; i<192; i++) {
		fScaler[i]=0;
		if (i<3) {
			fInput[i]=0;	
			fOutput[i]=0;	
			fOr0[i]=0;	
			fOr1[i]=0;
		}
	}	
	
	for (int i=0; i<3; i++) {
		fmean[i]=0.0;
		frms[i]=0.0;
		factive[i]=0;	
	}	
}


void TRich_Scaler::Print( FILE * fout){

	int mrc;	


	
	/****************/
	/* Print Screen */
	/****************/

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

		if (i%8==7)printf("\n");

		if (i%64==63){
			mrc = i/64;
			printf("\n");
			switch(mrc){
				case 0: printf("U3  ");break;
				case 1: printf("U26 ");break;
				case 2: printf("U27 ");break;
			}
			printf("MAROC[%d]: ",mrc);
			printf("Active channels %6d ",factive[mrc]);
			printf("Average counts %8.1lf ",fmean[mrc]);
			//printf("RMS %lf" frms[mrc]);
			//printf("Max %lf Min %lf",);
			printf("\n");
			printf("\n");

		}
	}

	printf("Total active channels %d\n",getActive(-1));
	printf("Global average counts = %lf\n ",getMean(-1));

	/******************/
	/* Export on file */
	/******************/


	if(fout!= NULL){

		fprintf(fout,"GClk125  = %d\n",fGClk125);
		fprintf(fout,"Sync  = %d\n",fSync );
		fprintf(fout,"Trig  = %d\n",fTrig );
		fprintf(fout,"Input0  = %d\n",fInput[0] );
		fprintf(fout,"Input1 = %d\n",fInput[1]);
		fprintf(fout,"Input2  = %d\n",fInput[2]);
		fprintf(fout,"Output0  = %d\n",fOutput[0]);
		fprintf(fout,"Output1  = %d\n",fOutput[1]);
		fprintf(fout,"Output2  = %d\n",fOutput[2]);
		fprintf(fout,"Or0_1  = %d\n",fOr0[0]);
		fprintf(fout,"Or0_2  = %d\n",fOr0[1]);
		fprintf(fout,"Or0_3  = %d\n",fOr0[2]);
		fprintf(fout,"Or1_1  = %d\n",fOr1[0]);
		fprintf(fout,"Or1_2  = %d\n",fOr1[1]);
		fprintf(fout,"Or1_3  = %d\n",fOr1[2]);
		fprintf(fout,"Busy  = %d\n",fBusy);
		fprintf(fout,"BusyCycles  = %d\n",fBusyCycles);

		for(int j = 0; j < 64; j++)	{ // loop on channels
			for(int i = 0; i < 3; i++){ // loop on ASICs
				fprintf(fout,"MAROC_%d_CH_%2d  = %.0lf\n",i,j,(double)fScaler[i]);
			}	
		}
	}










}


double TRich_Scaler::getMean(int asic){

	if(asic==0||asic==1||asic==2) return fmean[asic];
	
	return (fmean[0]+fmean[1]+fmean[2])/3;

}


double TRich_Scaler::getRMS(int asic){
	
	if(asic==0||asic==1||asic==2) return frms[asic];
	
	return  (frms[0]+frms[1]+frms[2])/3;
	
}

int TRich_Scaler::getActive(int asic){
	
	if(asic==0||asic==1||asic==2) return factive[asic];
	
	return factive[0]+factive[1]+factive[2];
}


void TRich_Scaler::CalculateStatistics(){

	int ch; // [0..191]
	
	for(int asic = 0; asic<3; asic++){
			
		for (int channel=0; channel<64; channel++) {
			
			ch = channel + asic*64;
			
			if(fScaler[ch]!=0){
				factive[asic]++;
				fmean[asic] += fScaler[ch];	
			}
		}
		
		if(factive[asic]){
			fmean[asic] = 	fmean[asic]/factive[asic] ;
		}
	}	
}
	
void TRich_Scaler::Write(const char * filename){

// qui ci va l'export che ora e' in TFrontEnd->DumpScalers

}


// get/set (cannot set 0, use reset instead);

unsigned int TRich_Scaler::GClk125(unsigned int val){
	
	if (val!=0) fGClk125 = val;
	
	return fGClk125;
}	

unsigned int TRich_Scaler::Busy(unsigned int val){
	
	if (val!=0)fBusy = val;
	
	return fBusy;
}

unsigned int TRich_Scaler::BusyCycles(unsigned int val){
	
	if (val!=0) fBusyCycles = val;
	
	return fBusyCycles;
}	
unsigned int TRich_Scaler::Sync(unsigned int val){
	
	if (val!=0) fSync = val;
	
	return fSync;
}
unsigned int TRich_Scaler::Trig(unsigned int val){ 
	
	if (val!=0) fTrig = val; return fTrig;
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



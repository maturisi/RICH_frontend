/*
 *  TRich_ADC.h
 *	header file for TRich_ADC class
 *
 *	DESCRIPTION:
 * 
 *	A class for ADC Event data 

 *	Matteo Turisini matteo.turisini@gmail.com
 *	INFN - Ferrara
 *
 */
 
 
#ifndef TRICH_ADC_H
#define TRICH_ADC_H

#define MAXCHANNEL 192

class TRich_ADC {	
	
private:
	int adc[MAXCHANNEL];
	int hold1_delay;
	int hold2_delay;
	int measID;


public:
				TRich_ADC();
				~TRich_ADC();

	void 	Reset();
	void 	Print();
	void 	Write(FILE * fout=NULL);
		
	int 	ADC(int channel=0,int val=-1);
 	int 	Hold1Delay(int val=-1);
	int 	Hold2Delay(int val=-1);
	int 	MeasID(int val=-1);
		
};

#endif

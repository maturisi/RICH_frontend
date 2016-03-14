#include <stdio.h>
#include "TRich_ADC.h"

	
TRich_ADC::TRich_ADC()
{
	Reset();
}

TRich_ADC::~TRich_ADC()
{

}

void 	TRich_ADC::Print()
{

	printf("Measurement %d\n",MeasID());
	printf("Hold 1 Delay = %d\n",Hold1Delay());
	printf("Hold 2 Delay = %d\n",Hold2Delay());

	for(int asic = 0;asic <3; asic++){
	printf("ASIC %d\n",asic);		
		for(int channel =0; channel <64; channel++){
			printf("%5d ",	ADC(channel+64*asic));
			if(channel%8==7) printf("\n");
		}
		printf("\n");
	}

}


void TRich_ADC::Write(FILE * f)
{
	if(f==NULL) return;

	fprintf(f,"%d\t",MeasID());
	fprintf(f,"%d\t",Hold1Delay());

	for(int asic = 0;asic <3; asic++){
		for(int channel =0; channel <64; channel++){
			fprintf(f,"%d\t",	ADC(channel+64*asic));
		}
	}
	fprintf(f, "\n");
}

void TRich_ADC::Reset()
{	

	Hold1Delay(0);
	Hold2Delay(0);
	MeasID(0);

	for(int ch = 0 ; ch<MAXCHANNEL ;ch++) ADC(ch,0);
}

int TRich_ADC::ADC(int channel,int val){
	
	if(channel>=MAXCHANNEL||channel<0) printf("Error in %s: Bad Channel %d\n",__FUNCTION__,channel);

	if (val >= 0) adc[channel] = val;

	return adc[channel];

}
int TRich_ADC::Hold1Delay(int val){

	if(val >= 0) hold1_delay = val;
	
	return hold1_delay;
}

int TRich_ADC::Hold2Delay(int val){

	if(val >= 0) hold2_delay = val;
	
	return hold2_delay;
}

int TRich_ADC::MeasID(int val){

	if(val >= 0) measID = val;
	
	return measID;

}



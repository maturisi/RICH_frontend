/*
 *  TRich_Adapter.h
 *  
 *
 *  Created by Matteo Turisini on 06/07/15.
 *
 *
 *
 *
 *	Mapping between anodes and electronic channels
 *  Preliminary version 
 *	to be expanded with 2 and 3 asic version, mapmts...
 *
 *
 */



#ifndef TRICH_ADAPTER_H
#define TRICH_ADAPTER_H

class TRich_Adapter {
	
private:
	unsigned char fAnode[192]; // array idx = maroc channel, array content = corresponding anode(e.g. channel 16 = anode 28)
public:
	TRich_Adapter();
	~TRich_Adapter();
	unsigned char GetAnode(unsigned char maroc_channel);/* takes ch-0..63 and returns pixel ID [0..63]=*/ 
private:
	void CreateMap(); // draft/preliminary map (64 channels only)
};

#endif

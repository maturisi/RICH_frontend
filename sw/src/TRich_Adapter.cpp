/*
 *  TRich_Adapter.cpp
 *  
 *
 *  Created by Matteo Turisini on 06/07/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include "TRich_Adapter.h"


TRich_Adapter::~TRich_Adapter(){

}

TRich_Adapter::TRich_Adapter(){

	for (int i =0 ; i<192; i++) {
		fAnode[i]=0;
	}
	this->CreateMap();
	
}

unsigned char TRich_Adapter::GetAnode(unsigned char maroc_channel){
	if (maroc_channel>63) {
		printf("Error in %s: multi asic version not yet implemented\n",__FUNCTION__);
		return 255;
	}
	return fAnode[maroc_channel];
}


void TRich_Adapter::CreateMap(){

	fAnode[0]=60;
	fAnode[1]=58;
	fAnode[2]=59;
	fAnode[3]=57;
	fAnode[4]=52;
	fAnode[5]=50;
	fAnode[6]=51;
	fAnode[7]=49;
	fAnode[8]=44;
	fAnode[9]=42;
	fAnode[10]=43;
	fAnode[11]=41;
	fAnode[12]=36;
	fAnode[13]=34;
	fAnode[14]=35;
	fAnode[15]=33;
	fAnode[16]=28;
	fAnode[17]=26;
	fAnode[18]=27;
	fAnode[19]=25;
	fAnode[20]=20;
	fAnode[21]=18;
	fAnode[22]=19;
	fAnode[23]=17;
	fAnode[24]=12;
	fAnode[25]=10;
	fAnode[26]=11;
	fAnode[27]=9;
	fAnode[28]=4;
	fAnode[29]=2;
	fAnode[30]=3;
	fAnode[31]=1;
	fAnode[32]=5;
	fAnode[33]=7;
	fAnode[34]=6;
	fAnode[35]=8;
	fAnode[36]=13;
	fAnode[37]=15;
	fAnode[38]=14;
	fAnode[39]=16;
	fAnode[40]=21;
	fAnode[41]=23;
	fAnode[42]=22;
	fAnode[43]=24;
	fAnode[44]=29;
	fAnode[45]=31;
	fAnode[46]=30;
	fAnode[47]=32;
	fAnode[48]=37;
	fAnode[49]=39;
	fAnode[50]=38;
	fAnode[51]=40;
	fAnode[52]=45;
	fAnode[53]=47;
	fAnode[54]=46;
	fAnode[55]=48;
	fAnode[56]=53;
	fAnode[57]=55;
	fAnode[58]=54;
	fAnode[59]=56;
	fAnode[60]=61;
	fAnode[61]=63;
	fAnode[62]=62;
	fAnode[63]=64;
}

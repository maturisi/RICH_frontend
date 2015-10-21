/*
 *  TRich_Frontend.h
 *  
 *
 *  Created by Matteo Turisini on 19/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef RICH_FRONTEND_H
#define RICH_FRONTEND_H

#include "datatype.h"
#include "TRich_ConnectTCP.h"
#include "TRich_Config.h"

#define MAROC_NUM 2


//#define MRC_RUN_NUMBER_FILE	 "cfg/last.run"
//#define DEFAULT_CONFIG_FNAME	"./maroc.cfg"

#define OUT_PATH "../data/out/"

#define SUFFIX_RAW_DATA ".txt"
#define SUFFIX_LOG ".log"

// Mux signal selection for SD->*Src registers
#define SD_SRC_SEL_0		0
#define SD_SRC_SEL_1		1
#define SD_SRC_SEL_MAROC_OR	2
#define SD_SRC_SEL_INPUT_1	5
#define SD_SRC_SEL_INPUT_2	6
#define SD_SRC_SEL_INPUT_3	7
#define SD_SRC_SEL_MAROC_OR1_0	10
#define SD_SRC_SEL_MAROC_OR1_1	11
#define SD_SRC_SEL_MAROC_OR2_0	12
#define SD_SRC_SEL_MAROC_OR2_1	13
#define SD_SRC_SEL_MAROC_OR3_0	14
#define SD_SRC_SEL_MAROC_OR3_1	15
#define SD_SRC_SEL_PULSER	18
#define SD_SRC_SEL_BUSY		19

class TRich_Frontend{

private:
	
	TRich_ConnectTCP *	ftcp;		// created outside and destroyed outside
	RICH_regs *		fRICH_regs;
	TRich_Config *		fConfigurator;	// created and destroyed with TRich_Frontend
	char			fN_asic;	// number of asic controlled by the fpga 
	char			fN_pmt;		// number of asic controlled by the fpga 

public:
	
	TRich_Frontend();
	~TRich_Frontend();

	void		SetConfiguration(TRich_Config *	configuration = NULL);	
	void		SetTCP(TRich_ConnectTCP * tcp = NULL); 

	void		ConfigureMAROC();		// static and dynamic registers
	void		ConfigureFPGA();			// Trigger, Event Builder, Internal Pulser
	
	void		StartDAQ(); 
	
	/* TDC */
	void		Fifo_Status();
	int		Fifo_nwords();
	void		Fifo_read(unsigned int *buf, int nwords);
	int		Process_Buf(unsigned int *buf, int nwords, FILE *f,FILE *outfile, int print);
	int		Receive(int * val);

	/* Scaler*/
	long		DumpScalers(FILE *outfile=NULL,	bool printscreen = false);

	void		PrintScaler(const char *name, unsigned int scaler, float ref);

	
private:	
	/* ASIC number*/	
	int AsicNumber();

	/* Static Register (Slow Control)*/
	void 		StaticReg(int n);
	void		ClearRegs();
	MAROC_Regs	Shift(MAROC_Regs regs);
	void		Print(MAROC_Regs regs);
	void		Init(MAROC_Regs *regs = NULL,int idx=0);
	
	/* Dynamic Register (for testing/probing)*/
	void 		DynReg(int n);
	void		ClearRegsDyn();
	void		ShiftDyn(MAROC_DyRegs wr, MAROC_DyRegs *rd1, MAROC_DyRegs *rd2, MAROC_DyRegs *rd3);
	void		PrintDyn(MAROC_DyRegs regs);
	void		InitDyn(MAROC_DyRegs *regs);
	
	/* Pulser */
	void		Set_Pulser(float freq = 10.0, float duty = 0.001, int count = 0x3FF);
	
	/* C_Test */	
	void		CTest(int val = 0);// Select a signal to entern the C_Test. By default no signal is sent to C_Test
	
	/* Soft Pulse Sync*/
	void		SoftPulseSync(); 
	
	/* Event Builder*/
	void		ConfigureEventBuilder(int trig_delay = 128);	
	
	/* TDC */
	void		Enable_TDC_Allchannels();
	void		Disable_TDC_Allchannels();
	void		Enable_TDC_channel(int ch);
	void		Disable_TDC_channel(int ch);
	void		Setup_Readout(int lookback, int window);
	void		Fifo_Reset();
	
	
	// Currently features does not exist in firmwarevoid		
	/*Set_TDC_DeadTime(int ticks_8ns);*/ 

	
	/* Trigger*/
	void		DisableTrigger();
	void		TriggerSource(int trigger_source = 18);
	void 		SetMask_FPGA_OR(int m0_0, int m0_1, int m1_0, int m1_1, int m2_0, int m2_1);

	
	/* Output Pulser to TTL output for probing/scope trigger*/
	void		ConfigureOutSrc( int out0,int out1, int out2);
	void 		PrintSource(int val);
	/* File Names*/
//	const char  * GetPrefix();	
};
#endif

/*
 *  TRich_Frontend.cpp
 *  
 *
 *  Created by Matteo Turisini on 19/03/15.
 *  Copyright 2015 __MyCompanyName__. All rights reserved.
 *
 */
#include <stdio.h> //printf
#include <string.h> // memset
#include <unistd.h> //usleep


#include "TRich_Frontend.h"
#include "TRich_Config.h"


TRich_Frontend::TRich_Frontend()
{
	ftcp 		= NULL;
	fRICH_regs 	= NULL;
	fConfigurator 	= NULL; 	
	fN_asic 	= 0; 
	fN_pmt		= 0;
}
TRich_Frontend::~TRich_Frontend()
{

}
void TRich_Frontend::SetTCP(TRich_ConnectTCP * tcp)
{
	ftcp =tcp;
}
void TRich_Frontend::SetConfiguration(TRich_Config * cfg)
{
	fConfigurator = cfg;
}

void TRich_Frontend::Fifo_read(unsigned int *buf, int nwords)
{
	while(nwords--)
		*buf++ = ftcp->Read(&fRICH_regs->EvtBuilder.FifoData);	
}

void TRich_Frontend::CTest( int src ) 
{

	int lsrc = src;

	lsrc = SD_SRC_SEL_PULSER; 	// pulser
//	lsrc = SD_SRC_SEL_0;		// nothing (useful in case of external laser driven by the fpga)
//	lsrc = SD_SRC_SEL_INPUT_10;     // external digital signal


	printf("%s: ",__FUNCTION__);
	PrintSource(lsrc);
	printf("\n");

	ftcp->Write(&fRICH_regs->Sd.CTestSrc, lsrc);	 // PULSER
}


void TRich_Frontend::DisableTrigger()
{
	ftcp->Write(&fRICH_regs->Sd.TrigSrc, 0); //Set trig source to disabled 
}

void TRich_Frontend::ConfigureEventBuilder(int tick_8ns)
{
	ftcp->Write(&fRICH_regs->EvtBuilder.TrigDelay, tick_8ns);	
}

void TRich_Frontend::TriggerSource(int trig_src)
{
  	int ltrig = trig_src; 
	
	printf("%s: ",__FUNCTION__);
	PrintSource(ltrig);
	printf("\n");

	ftcp->Write(&fRICH_regs->Sd.TrigSrc,ltrig);
}	

int	 TRich_Frontend::Fifo_nwords()
{
	return ftcp->Read(&fRICH_regs->EvtBuilder.FifoWordCnt);
}

void TRich_Frontend::Fifo_Reset()
{
	ftcp->Write(&fRICH_regs->Clk.Ctrl, 0x01); // assert soft reset
	ftcp->Write(&fRICH_regs->Clk.Ctrl, 0x00); // deassert soft reset	
}


void TRich_Frontend::Fifo_Status(){
	
	int wordCnt, eventCnt;
	wordCnt = ftcp->Read(&fRICH_regs->EvtBuilder.FifoWordCnt);
	eventCnt = ftcp->Read(&fRICH_regs->EvtBuilder.FifoEventCnt);
	//printf("FIFO Status: WCNT=%9d ECNT=%9d\n", wordCnt, eventCnt);
	printf("WCNT=%9d ECNT=%9d", wordCnt, eventCnt);
	//printf("ECNT=%9d", eventCnt);	
}
void TRich_Frontend::SoftPulseSync()
{
	ftcp->Write(&fRICH_regs->Sd.SyncSrc, 1);
	ftcp->Write(&fRICH_regs->Sd.SyncSrc, 0);
} 

void TRich_Frontend::PrintSource(int val){
	switch(val){
		case SD_SRC_SEL_0		: printf("ZERO");break; 
		case SD_SRC_SEL_1		: printf("ONE");break;
		case SD_SRC_SEL_INPUT_1		: printf("EXT1"); break;
		case SD_SRC_SEL_INPUT_2		: printf("EXT2"); break;
		case SD_SRC_SEL_INPUT_3		: printf("EXT3"); break;
		case SD_SRC_SEL_MAROC_OR	: printf("FPGA MASKED OR"); break;
		case SD_SRC_SEL_MAROC_OR1_0	: printf("MAR0C DAC0 asic1"); break;
		case SD_SRC_SEL_MAROC_OR1_1	: printf("MAROC DAC1 asic1"); break;
		case SD_SRC_SEL_MAROC_OR2_0	: printf("MAROC DAC0 asic2"); break;
		case SD_SRC_SEL_MAROC_OR2_1	: printf("MAROC DAC1 asic2"); break;
		case SD_SRC_SEL_MAROC_OR3_0	: printf("MAROC DAC0 asic3"); break;
		case SD_SRC_SEL_MAROC_OR3_1	: printf("MAROC DAC1 asic3"); break;
		case SD_SRC_SEL_PULSER		: printf("PULSER");break;
		case SD_SRC_SEL_BUSY		: printf("BUSY");break;
		default: printf("ERROR in %s : unknown outsource type",__FUNCTION__); break;
  }
}

void TRich_Frontend::ConfigureOutSrc(int out0,int out1,int out2)
{
	printf("FPGA outputs: "); 
	
	PrintSource(out0);

	printf(", ");

	PrintSource(out1);

	printf("\n");
	
	ftcp->Write(&fRICH_regs->Sd.OutSrc[0], out0);
 	ftcp->Write(&fRICH_regs->Sd.OutSrc[1], out1);
 	//ftcp->Write(&fRICH_regs->Sd.OutSrc[2], out2);
}


void TRich_Frontend::SetMask_FPGA_OR(int m0_0, int m0_1, int m1_0, int m1_1, int m2_0, int m2_1)
{
	ftcp->Write(&fRICH_regs->MAROC_Proc[0].HitOrMask0, m0_0);
	ftcp->Write(&fRICH_regs->MAROC_Proc[0].HitOrMask1, m0_1);
	ftcp->Write(&fRICH_regs->MAROC_Proc[1].HitOrMask0, m1_0);
	ftcp->Write(&fRICH_regs->MAROC_Proc[1].HitOrMask1, m1_1);
	ftcp->Write(&fRICH_regs->MAROC_Proc[2].HitOrMask0, m2_0);
	ftcp->Write(&fRICH_regs->MAROC_Proc[2].HitOrMask1, m2_1);
}

void TRich_Frontend::Enable_TDC_Allchannels()
{
	int i;
	for(i = 0; i < 3; i++)
	{
		ftcp->Write(&fRICH_regs->MAROC_Proc[i].DisableCh[0], 0x0000);
		ftcp->Write(&fRICH_regs->MAROC_Proc[i].DisableCh[1], 0x0000);
		ftcp->Write(&fRICH_regs->MAROC_Proc[i].DisableCh[2], 0x0000);
		ftcp->Write(&fRICH_regs->MAROC_Proc[i].DisableCh[3], 0x0000);
	}
}
void TRich_Frontend::Disable_TDC_Allchannels()
{
	int i;	
	for(i = 0; i < 3; i++)
	{
		ftcp->Write(&fRICH_regs->MAROC_Proc[i].DisableCh[0], 0xFFFF);
		ftcp->Write(&fRICH_regs->MAROC_Proc[i].DisableCh[1], 0xFFFF);
		ftcp->Write(&fRICH_regs->MAROC_Proc[i].DisableCh[2], 0xFFFF);
		ftcp->Write(&fRICH_regs->MAROC_Proc[i].DisableCh[3], 0xFFFF);
	}
}

void TRich_Frontend::Setup_Readout(int lookback, int window)
{
	ftcp->Write(&fRICH_regs->EvtBuilder.Lookback, lookback);// lookback*8ns from trigger time 
	ftcp->Write(&fRICH_regs->EvtBuilder.WindowWidth, window);// capture window*8ns hits after lookback time 
	ftcp->Write(&fRICH_regs->EvtBuilder.BlockCfg, 1);	// 1 event per block 
	ftcp->Write(&fRICH_regs->EvtBuilder.DeviceID, 0x01);	// setup a dummy device ID (5bit) written in each event for module id - needs to be expanded 
}

void TRich_Frontend::ClearRegs()
{	
	int val;	
	// set rst_sc low 
	val = ftcp->Read(&fRICH_regs->MAROC_Cfg.SerCtrl);
	val &= 0xFFFFFFFE;
	ftcp->Write(&fRICH_regs->MAROC_Cfg.SerCtrl, val);
	
	// set rst_sc high 
	val |= 0x00000001;
	ftcp->Write(&fRICH_regs->MAROC_Cfg.SerCtrl, val);

}

void TRich_Frontend::ClearRegsDyn(){

	int val;
	// set rst_r low 
	val = ftcp->Read(&fRICH_regs->MAROC_Cfg.SerCtrl);
	val &= 0xFFFFFFFB;
	ftcp->Write(&fRICH_regs->MAROC_Cfg.SerCtrl, val);
	
	// set rst_r high 
	val |= 0x00000004;
	ftcp->Write(&fRICH_regs->MAROC_Cfg.SerCtrl, val);

}

void TRich_Frontend::Set_Pulser(float freq, float duty, int count){
	
	if(freq <= 0.0){printf("ERROR in %s: freq invalid, setting to 1Hz\n", __FUNCTION__);freq = 1.0;}
	if((duty <= 0.0) || (duty >= 1.0)){printf("ERROR in %s: duty invalid, setting to 50%%\n", __FUNCTION__);duty = 0.5;}

	freq = 125000000 / freq;
	ftcp->Write(&fRICH_regs->Sd.PulserPeriod, (int)freq);
	
	duty = freq * (1.0 - duty);
	ftcp->Write(&fRICH_regs->Sd.PulserLowCycles, (int)duty);
	
	ftcp->Write(&fRICH_regs->Sd.PulserNCycles, count);
	ftcp->Write(&fRICH_regs->Sd.PulserStart, 0);
	
	/* enable pulser -> CTest output */

	// 
	//ftcp->Write(&fRICH_regs->Sd.CTestSrc, 18); // the pulser goes into C_TEST
	//ftcp->Write(&fRICH_regs->Sd.CTestSrc, 0); 


}



void TRich_Frontend::Enable_TDC_channel(int ch)
{
	int proc_idx, proc_reg, proc_bit;
	int val;	
	if((ch < 0) || (ch > 191)){printf("Error in %s: invalid channel %d\n", __FUNCTION__, ch);return;}
	proc_idx = ch / 64;
	proc_reg = (ch % 64) / 16;
	proc_bit = (ch % 16);
	val = ftcp->Read(&fRICH_regs->MAROC_Proc[proc_idx].DisableCh[proc_reg]);
	val = val & ~(1<<proc_bit);
	ftcp->Write(&fRICH_regs->MAROC_Proc[proc_idx].DisableCh[proc_reg], val);
}

void TRich_Frontend::Disable_TDC_channel(int ch)
{
	int proc_idx, proc_reg, proc_bit;
	int val;	
	if((ch < 0) || (ch > 191)){printf("Error in %s: invalid channel %d\n", __FUNCTION__, ch);return;}
	proc_idx = ch / 64;
	proc_reg = (ch % 64) / 16;
	proc_bit = (ch % 16);
	val = ftcp->Read(&fRICH_regs->MAROC_Proc[proc_idx].DisableCh[proc_reg]);
	val = val | (1<<proc_bit);
	ftcp->Write(&fRICH_regs->MAROC_Proc[proc_idx].DisableCh[proc_reg], val);
}
void TRich_Frontend::ConfigureFPGA()
{
  printf("%s...\n",__FUNCTION__);
  RICH_fpga_t *	A	= fConfigurator->Get_Settings_Fpga();			
 //fConfigurator->PrintFPGA();		
  

  // firmware update 
  // skipped for the momentConfigureFPGA
  //int lfpgaID	= A->fpgaID;
  //int lfirmware_version = A->firmware_version;
  //if (lfirmware_version) {
  //	this->UpdateFw();
  //}
  
  // multievent mode 
  //skipped for the moment
  //int lblock_nevents  = A->block_nevents;

if(A->pulser_repetition==0){printf("WARNING: Test Pulse is OFF\n");}
  this->Set_Pulser(A->pulser_freq, A->pulser_dutycycle, A->pulser_repetition);
  this->SetMask_FPGA_OR(0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF);
  //this->SetMask_FPGA_OR(0xFFFFFFFF,0xFFFFFFFF,0x0,0x0,0x0,0x0);
 // this->SetMask_FPGA_OR(0x0,0x0,0xFFFFFFFF,0xFFFFFFFF,0x0,0x0);
 // this->SetMask_FPGA_OR(0x0,0x0,0x0,0x0,0xFFFFFFFF,0xFFFFFFFF);
  

	this->ConfigureOutSrc(A->out[0],A->out[1],A->out[2]);
  this->ConfigureEventBuilder(A->trig_delay);	
  this->SoftPulseSync();
  this->Setup_Readout(A->evtbuild_lookback, A->evtbuild_window);
  this->DisableTrigger(); 
  this->Fifo_Reset();
  
  // hw is configured and trigger is disabled 	
}

void TRich_Frontend::StartDAQ(){// ask Ben about what to put here
	
	//printf("Starting DAQ ...");
	this->Disable_TDC_Allchannels();
	this->Enable_TDC_Allchannels();

  	RICH_fpga_t *	A	= fConfigurator->Get_Settings_Fpga();			
	
	// add the setting C_Test input

	this->CTest(); 

	this->TriggerSource(A->trig_source); //Set trig source to pulser when ready to take events 

}















MAROC_Regs TRich_Frontend::Shift(MAROC_Regs regs){
	
	MAROC_Regs result;
	int i, val;
	
	// write settings to FPGA shift register 
	ftcp->Write(&fRICH_regs->MAROC_Cfg.Regs.Global0.val, regs.Global0.val);
	ftcp->Write(&fRICH_regs->MAROC_Cfg.Regs.Global1.val, regs.Global1.val);
	ftcp->Write(&fRICH_regs->MAROC_Cfg.Regs.DAC.val, regs.DAC.val);
	
	for(i = 0; i < 32; i++){
		ftcp->Write(&fRICH_regs->MAROC_Cfg.Regs.CH[i].val, regs.CH[i].val);
	}
	
	// do shift register transfer 
	val = ftcp->Read(&fRICH_regs->MAROC_Cfg.SerCtrl);
	val |= 0x00000002;
	ftcp->Write(&fRICH_regs->MAROC_Cfg.SerCtrl, val);
	
	// check for shift register transfer completion 
	for(i = 10; i > 0; i--)
	{
		val = ftcp->Read(&fRICH_regs->MAROC_Cfg.SerStatus);
		if(!(val & 0x00000001))
			break;
		
		if(!i)
			printf("Error in %s: timeout on serial transfer...\n", __FUNCTION__);
		
		usleep(100);
	}
	
	// read back settings from FPGA shift register 
	result.Global0.val = ftcp->Read(&fRICH_regs->MAROC_Cfg.Regs.Global0.val);
	result.Global1.val = ftcp->Read(&fRICH_regs->MAROC_Cfg.Regs.Global1.val);
	result.DAC.val = ftcp->Read(&fRICH_regs->MAROC_Cfg.Regs.DAC.val);
	
	for(i = 0; i < 32; i++)
		result.CH[i].val = ftcp->Read(&fRICH_regs->MAROC_Cfg.Regs.CH[i].val);
	
	return result;
}
void TRich_Frontend::Print(MAROC_Regs regs){

	printf("Global0 = 0x%08X ", regs.Global0.val);

/*
	printf("   cmd_fsu           = %d\n", regs.Global0.bits.cmd_fsu);
	printf("   cmd_ss            = %d\n", regs.Global0.bits.cmd_ss);
	printf("   cmd_fsb           = %d\n", regs.Global0.bits.cmd_fsb);
	printf("   swb_buf_250f      = %d\n", regs.Global0.bits.swb_buf_250f);
	printf("   swb_buf_500f      = %d\n", regs.Global0.bits.swb_buf_500f);
	printf("   swb_buf_1p        = %d\n", regs.Global0.bits.swb_buf_1p);
	printf("   swb_buf_2p        = %d\n", regs.Global0.bits.swb_buf_2p);
	printf("   ONOFF_ss          = %d\n", regs.Global0.bits.ONOFF_ss);
	printf("   sw_ss_300f        = %d\n", regs.Global0.bits.sw_ss_300f);
	printf("   sw_ss_600f        = %d\n", regs.Global0.bits.sw_ss_600f);
	printf("   sw_ss_1200f       = %d\n", regs.Global0.bits.sw_ss_1200f);
	printf("   EN_ADC            = %d\n", regs.Global0.bits.EN_ADC);
	printf("   H1H2_choice       = %d\n", regs.Global0.bits.H1H2_choice);
	printf("   sw_fsu_20f        = %d\n", regs.Global0.bits.sw_fsu_20f);
	printf("   sw_fsu_40f        = %d\n", regs.Global0.bits.sw_fsu_40f);
	printf("   sw_fsu_25k        = %d\n", regs.Global0.bits.sw_fsu_25k);
	printf("   sw_fsu_50k        = %d\n", regs.Global0.bits.sw_fsu_50k);
	printf("   sw_fsu_100k       = %d\n", regs.Global0.bits.sw_fsu_100k);
	printf("   sw_fsb1_50k       = %d\n", regs.Global0.bits.sw_fsb1_50k);
	printf("   sw_fsb1_100k      = %d\n", regs.Global0.bits.sw_fsb1_100k);
	printf("   sw_fsb1_100f      = %d\n", regs.Global0.bits.sw_fsb1_100f);
	printf("   sw_fsb1_50f       = %d\n", regs.Global0.bits.sw_fsb1_50f);
	printf("   cmd_fsb_fsu       = %d\n", regs.Global0.bits.cmd_fsb_fsu);
	printf("   valid_dc_fs       = %d\n", regs.Global0.bits.valid_dc_fs);
	printf("   sw_fsb2_50k       = %d\n", regs.Global0.bits.sw_fsb2_50k);
	printf("   sw_fsb2_100k      = %d\n", regs.Global0.bits.sw_fsb2_100k);
	printf("   sw_fsb2_100f      = %d\n", regs.Global0.bits.sw_fsb2_100f);
	printf("   sw_fsb2_50f       = %d\n", regs.Global0.bits.sw_fsb2_50f);
	printf("   valid_dc_fsb2     = %d\n", regs.Global0.bits.valid_dc_fsb2);
	printf("   ENb_tristate      = %d\n", regs.Global0.bits.ENb_tristate);
	printf("   polar_discri      = %d\n", regs.Global0.bits.polar_discri);
	printf("   inv_discriADC     = %d\n", regs.Global0.bits.inv_discriADC);
	*/
	printf("Global1 = 0x%08X ", regs.Global1.val);
/*
	printf("   d1_d2             = %d\n", regs.Global1.bits.d1_d2);
	printf("   cmd_CK_mux        = %d\n", regs.Global1.bits.cmd_CK_mux);
	printf("   ONOFF_otabg       = %d\n", regs.Global1.bits.ONOFF_otabg);
	printf("   ONOFF_dac         = %d\n", regs.Global1.bits.ONOFF_dac);
	printf("   small_dac         = %d\n", regs.Global1.bits.small_dac);
	printf("   enb_outADC        = %d\n", regs.Global1.bits.enb_outADC);
	printf("   inv_startCmptGray = %d\n", regs.Global1.bits.inv_startCmptGray);
	printf("   ramp_8bit         = %d\n", regs.Global1.bits.ramp_8bit);
	printf("   ramp_10bit        = %d\n", regs.Global1.bits.ramp_10bit);
*/	
	printf("DAC = 0x%08X ", regs.DAC.val);

/*
	printf("   DAC0              = %d\n", regs.DAC.bits.DAC0);
	printf("   DAC1              = %d\n", regs.DAC.bits.DAC1);
*/	
/*
	printf("Channels:\n");
	printf("%7s%7s%7s%7s%7s\n", "CH", "Gain", "Sum", "CTest", "MaskOr");
	int i;
	for(i = 0; i < 64; i++)
	{
		if(i & 0x1)
			printf("%7d%7d%7d%7d%7d\n", i,
				   regs.CH[i>>1].bits.Gain0, regs.CH[i>>1].bits.Sum0,
				   regs.CH[i>>1].bits.CTest0, regs.CH[i>>1].bits.MaskOr0);
		else
			printf("%7d%7d%7d%7d%7d\n", i,
				   regs.CH[i>>1].bits.Gain1, regs.CH[i>>1].bits.Sum1,
				   regs.CH[i>>1].bits.CTest1, regs.CH[i>>1].bits.MaskOr1);
	}
	printf("\n");	
*/
}

void TRich_Frontend::Init(MAROC_Regs *regs,int idx){

  //printf("Initializing MAROC[%d] Slow Control...\n",idx);	
	int i;
	

	//fConfigurator->PrintMAROC_StaticReg();
	//fConfigurator->PrintMAROC_adj();
	

	RICH_maroc_common_t* 	B	= fConfigurator->Get_Settings_Maroc_Common();			
	RICH_maroc_t	*	C	= fConfigurator->Get_Settings_Maroc(idx); // idx	
	
	memset(regs, 0, sizeof(MAROC_Regs));

	regs->Global0.bits.cmd_fsu	= B->cmd_fsu;
	regs->Global0.bits.cmd_ss	= B->cmd_ss;
	regs->Global0.bits.cmd_fsb	= B->cmd_fsb;
	regs->Global0.bits.swb_buf_250f	= B->swb_buf_250f;
	regs->Global0.bits.swb_buf_500f	= B->swb_buf_500f;
	regs->Global0.bits.swb_buf_1p	= B->swb_buf_1p;
	regs->Global0.bits.swb_buf_2p	= B->swb_buf_2p;
	regs->Global0.bits.ONOFF_ss	= B->ONOFF_ss;
	regs->Global0.bits.sw_ss_300f 	= B->sw_ss_300f;
	regs->Global0.bits.sw_ss_600f 	= B->sw_ss_600f;
	regs->Global0.bits.sw_ss_1200f 	= B->sw_ss_1200f;
	regs->Global0.bits.EN_ADC	= B->EN_ADC;
	regs->Global0.bits.H1H2_choice 	= B->H1H2_choice;
	regs->Global0.bits.sw_fsu_20f 	= B->sw_fsu_20f;
	regs->Global0.bits.sw_fsu_40f 	= B->sw_fsu_40f;
	regs->Global0.bits.sw_fsu_25k 	= B->sw_fsu_25k;
	regs->Global0.bits.sw_fsu_50k 	= B->sw_fsu_50k;
	regs->Global0.bits.sw_fsu_100k 	= B->sw_fsu_100k;
	regs->Global0.bits.sw_fsb1_50k 	= B->sw_fsb1_50k;
	regs->Global0.bits.sw_fsb1_100k = B->sw_fsb1_100k;
	regs->Global0.bits.sw_fsb1_100f = B->sw_fsb1_100f;
	regs->Global0.bits.sw_fsb1_50f 	= B->sw_fsb1_50f;
	regs->Global0.bits.cmd_fsb_fsu 	= B->cmd_fsb_fsu;
	regs->Global0.bits.valid_dc_fs	= B->valid_dc_fs;
	regs->Global0.bits.sw_fsb2_50k 	= B->sw_fsb2_50k;
	regs->Global0.bits.sw_fsb2_100k = B->sw_fsb2_100k;
	regs->Global0.bits.sw_fsb2_100f = B->sw_fsb2_100f;
	regs->Global0.bits.sw_fsb2_50f 	= B->sw_fsb2_50f;
	regs->Global0.bits.valid_dc_fsb2= B->valid_dc_fsb2;
	regs->Global0.bits.ENb_tristate = B->ENb_tristate;
	regs->Global0.bits.polar_discri = B->polar_discri;
	regs->Global0.bits.inv_discriADC= B->inv_discriADC;
	regs->Global1.bits.d1_d2	= B->d1_d2;
	regs->Global1.bits.cmd_CK_mux 	= B->cmd_CK_mux;
	regs->Global1.bits.ONOFF_otabg 	= B->ONOFF_otabg;
	regs->Global1.bits.ONOFF_dac 	= B->ONOFF_dac;
	regs->Global1.bits.small_dac 	= B->small_dac; 
	regs->Global1.bits.enb_outADC 	= B->enb_outADC;
	regs->Global1.bits.inv_startCmptGray = 	B->inv_startCmptGray;
	regs->Global1.bits.ramp_8bit 	= B->ramp_8bit;
	regs->Global1.bits.ramp_10bit 	= B->ramp_10bit;
	

	regs->DAC.bits.DAC0 = fConfigurator->Threshold(); 
	//regs->DAC.bits.DAC0 = B->DAC0; 
	regs->DAC.bits.DAC1 = B->DAC1;

	int ctest=1;	
	//int gain=0;
	int maskOR = 0;

	int gain = fConfigurator->Gain();

	for(i = 0; i < 64; i++)
	{	

		gain = C->gain[i]; 		

/*
		if(i==0){printf("\n ASIC %d Threshold %d \n",idx,fConfigurator->Threshold());}
		printf("%4d",gain);
		if((i%8)==7){printf("\n");}
*/


		if(!(i & 0x1))
		{
			
			regs->CH[i>>1].bits.Gain0 = gain; 
			regs->CH[i>>1].bits.Sum0 = 0;
			regs->CH[i>>1].bits.CTest0 = ctest; 
			regs->CH[i>>1].bits.MaskOr0 = maskOR;
			gain=0;
		}
		else
		{
			regs->CH[i>>1].bits.Gain1 = gain; 
			regs->CH[i>>1].bits.Sum1 = 0;
			regs->CH[i>>1].bits.CTest1 = ctest;
			regs->CH[i>>1].bits.MaskOr1 = maskOR;
		}
	}
}




void TRich_Frontend::ShiftDyn(MAROC_DyRegs wr, MAROC_DyRegs *rd1, MAROC_DyRegs *rd2, MAROC_DyRegs *rd3){
	int i, val;
	
	/* write settings to FPGA shift register */
	ftcp->Write(&fRICH_regs->MAROC_Cfg.DyRegs_WrAll.Ch0_31_Hold1, wr.Ch0_31_Hold1);
	ftcp->Write(&fRICH_regs->MAROC_Cfg.DyRegs_WrAll.Ch32_63_Hold1, wr.Ch32_63_Hold1);
	ftcp->Write(&fRICH_regs->MAROC_Cfg.DyRegs_WrAll.Ch0_31_Hold2, wr.Ch0_31_Hold2);
	ftcp->Write(&fRICH_regs->MAROC_Cfg.DyRegs_WrAll.Ch32_63_Hold2, wr.Ch32_63_Hold2);
	
	/* do shift register transfer */
	val = ftcp->Read(&fRICH_regs->MAROC_Cfg.SerCtrl);
	val |= 0x00000008;
	ftcp->Write(&fRICH_regs->MAROC_Cfg.SerCtrl, val);
	
	/* check for shift register transfer completion */
	for(i = 10; i > 0; i--)
	{
		val = ftcp->Read(&fRICH_regs->MAROC_Cfg.SerStatus);
		if(!(val & 0x00000002))
			break;
		
		if(!i)
			printf("Error in %s: timeout on serial transfer...\n", __FUNCTION__);
		
		usleep(100);
	}
	
	/* read back settings from FPGA shift register */
	if(rd1)
	{
		rd1->Ch0_31_Hold1 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[0].Ch0_31_Hold1);
		rd1->Ch32_63_Hold1 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[0].Ch32_63_Hold1);
		rd1->Ch0_31_Hold2 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[0].Ch0_31_Hold2);
		rd1->Ch32_63_Hold2 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[0].Ch32_63_Hold2);
	}
	
	if(rd2)
	{
		rd2->Ch0_31_Hold1 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[1].Ch0_31_Hold1);
		rd2->Ch32_63_Hold1 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[1].Ch32_63_Hold1);
		rd2->Ch0_31_Hold2 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[1].Ch0_31_Hold2);
		rd2->Ch32_63_Hold2 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[1].Ch32_63_Hold2);
	}
	
	if(rd3)
	{
		rd3->Ch0_31_Hold1 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[2].Ch0_31_Hold1);
		rd3->Ch32_63_Hold1 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[2].Ch32_63_Hold1);
		rd3->Ch0_31_Hold2 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[2].Ch0_31_Hold2);
		rd3->Ch32_63_Hold2 = ftcp->Read(&fRICH_regs->MAROC_Cfg.DyRegs_Rd[2].Ch32_63_Hold2);
	}
	

}
void TRich_Frontend::PrintDyn(MAROC_DyRegs regs){
	int i;
	
	printf("Channels:\n");
	printf("%7s%7s%7s\n", "CH", "Hold1", "Hold2");
	for(i = 0; i < 64; i++)
	{
		if(i < 32)
			printf("%7d%7d%7d\n", i,
				   (regs.Ch0_31_Hold1>>i) & 0x1,
				   (regs.Ch0_31_Hold2>>i) & 0x1);
		else
			printf("%7d%7d%7d\n", i,
				   (regs.Ch32_63_Hold1>>(i-32)) & 0x1,
				   (regs.Ch32_63_Hold2>>(i-32)) & 0x1);
	}
	printf("\n");
	

}
void TRich_Frontend::InitDyn(MAROC_DyRegs *regs){
	
	memset(regs, 0, sizeof(MAROC_DyRegs));

	//fConfigurator->PrintMAROC_DynamcReg();
	
/*	
	regs->Ch0_31_Hold1	= fConfigurator->Get_Ch00_31_Hold1();
	regs->Ch32_63_Hold1 = fConfigurator->Get_Ch32_63_Hold1();
	regs->Ch0_31_Hold2	= fConfigurator->Get_Ch00_31_Hold2();
	regs->Ch32_63_Hold2 = fConfigurator->Get_Ch32_63_Hold2();	
*/
/*
	// reset everythying (redundant )
	regs->Ch0_31_Hold1 = 0x00000000;
	regs->Ch32_63_Hold1 = 0x00000000;
	regs->Ch0_31_Hold2 = 0x00000000;
	regs->Ch32_63_Hold2 = 0x00000000;

*/

	int channel = fConfigurator->Ch_Sel();
	if((channel>=64)||(channel<0)){
		printf("Warning in %s: bad channel selection ( ch_sel = %d), use default ch_sel = 0 \n",__FUNCTION__,channel);
		channel=0;
	}
	printf("Selected channel is %d  on all the chips\n",channel);				

	int wrd =1;
	if(channel<=31){
		wrd = wrd << channel;
		regs->Ch0_31_Hold1 = wrd;

	}else{
		channel = channel -32;
		wrd = wrd << channel;	
		regs->Ch32_63_Hold1 = wrd;	
	}
	//printf("word = 0x%X\n",wrd);
}
void TRich_Frontend::PrintScaler(const char *name, unsigned int scaler, float ref){

	printf("%-10s %9u %9fHz\n", name, scaler, (float)scaler / ref);
}

long TRich_Frontend::DumpScalers(FILE * outfile,bool printscreen){
	
	long ret=0;
	float ref;
	unsigned int val;
	int i, j;
	//char buf[100];
	
	/* halt scaler counting */
	ftcp->Write(&fRICH_regs->Sd.ScalerLatch, 0x1);
	
	/* read reference time */
	val = ftcp->Read(&fRICH_regs->Sd.Scaler_GClk125);
	if(!val)
	{
		printf("Error in %s: reference time invalid - scaler normalization incorrect\n", __FUNCTION__);
		val = 1;
	}
	ref = (float)val / 125.0E6;

	unsigned int lGClk125 	= ftcp->Read(&fRICH_regs->Sd.Scaler_GClk125);
	unsigned int lSync	= ftcp->Read(&fRICH_regs->Sd.Scaler_Sync);
	unsigned int lTrig	= ftcp->Read(&fRICH_regs->Sd.Scaler_Trig);
	unsigned int lInput0 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Input[0]);
	unsigned int lInput1 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Input[1]);
	unsigned int lInput2 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Input[2]);
	unsigned int lOutput0 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Output[0]);
	unsigned int lOutput1 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Output[1]);
	unsigned int lOutput2 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Output[2]);
	unsigned int lOr0_1 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Or1[0]);
	unsigned int lOr0_2 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Or2[0]);
	unsigned int lOr0_3 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Or3[0]);
	unsigned int lOr1_1 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Or1[1]);
	unsigned int lOr1_2 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Or2[1]);
	unsigned int lOr1_3 	= ftcp->Read(&fRICH_regs->Sd.Scaler_Or3[1]);
	unsigned int lBusy	= ftcp->Read(&fRICH_regs->Sd.Scaler_Busy);
	unsigned int lBusyCycles =ftcp->Read(&fRICH_regs->Sd.Scaler_BusyCycles);


	if(printscreen){
		printf("ref    = %10f\n",ref);
		printf("val    = %10d\n",val);
		//printf("ref  = %f\n",ref);//	printf("Frequency is scaler counts divided by ref e.g. %f Hz for Output0 \n",(float)lOutput0 / ref);
		printf("GClk125= %10d\n",lGClk125);
		printf("Trig   = %10d\n",lTrig );
		printf("Sync   = %10d\n",lSync );
		printf("Busy   = %10d\n",lBusy);
		printf("BsyCyc = %10d\n",lBusyCycles);

		printf("Input  = %10d %10d %10d \n",lInput0,lInput1,lInput2 );
		printf("Output = %10d %10d %10d \n",lOutput0,lOutput1,lOutput2);
	
		//printf("Input  = %10d %10d\n",lInput0,lInput1);
		//printf("Output = %10d %10d\n",lOutput0,lOutput1);
	
		printf("MRC0_OR= %10d %10d\n",lOr0_1,lOr1_1);
		printf("MRC1_OR= %10d %10d\n",lOr0_2,lOr1_2);
		printf("MRC2_OR= %10d %10d\n",lOr0_3,lOr1_3);
	}

	// add val e ref in the output file and change the parser accordingly
	if(outfile) fprintf(outfile,"GClk125  = %d\n",lGClk125);
	if(outfile) fprintf(outfile,"Sync  = %d\n",lSync );
	if(outfile) fprintf(outfile,"Trig  = %d\n",lTrig );
	if(outfile) fprintf(outfile,"Input0  = %d\n",lInput0 );
	if(outfile) fprintf(outfile,"Input1 = %d\n",lInput1);
	if(outfile) fprintf(outfile,"Input2  = %d\n",lInput2);
	if(outfile) fprintf(outfile,"Output0  = %d\n",lOutput0);
	if(outfile) fprintf(outfile,"Output1  = %d\n",lOutput1);
	if(outfile) fprintf(outfile,"Output2  = %d\n",lOutput2);
	if(outfile) fprintf(outfile,"Or0_1  = %d\n",lOr0_1);
	if(outfile) fprintf(outfile,"Or0_2  = %d\n",lOr0_2);
	if(outfile) fprintf(outfile,"Or0_3  = %d\n",lOr0_3);
	if(outfile) fprintf(outfile,"Or1_1  = %d\n",lOr1_1);
	if(outfile) fprintf(outfile,"Or1_2  = %d\n",lOr1_2);
	if(outfile) fprintf(outfile,"Or1_3  = %d\n",lOr1_3);
	if(outfile) fprintf(outfile,"Busy  = %d\n",lBusy);
	if(outfile) fprintf(outfile,"BusyCycles  = %d\n",lBusyCycles);

	for(j = 0; j < 64; j++)	{ // loop on channels
		for(i = 0; i < 3; i++){ // loop on ASICs
			val = ftcp->Read(&fRICH_regs->MAROC_Proc[i].Scalers[j]);
			if(outfile) fprintf(outfile,"MAROC_%d_CH_%2d  = %.0lf\n",i,j,1.*val/ref);
			ret += val; 
		}	
	}


	// print screen
	if(printscreen){
		for(i = 0; i < 3; i++){ // loop on ASICs
			for(j = 0; j < 64; j++)	{ // loop on channels
				if(j%8==0){
					printf("\n");
				}
				val = ftcp->Read(&fRICH_regs->MAROC_Proc[i].Scalers[j]);			
				printf("%8.0lf ", 1.*val/ref);						
			}
			printf("\n");		
		}
	}


	/* Reset Scalers*/
	ftcp->Write(&fRICH_regs->Sd.ScalerLatch, 0x2);
	
	/* enable scaler counting */
	ftcp->Write(&fRICH_regs->Sd.ScalerLatch, 0x0);

	// return total numbers of counts
	return ret;	
}


int TRich_Frontend::Receive(int * valp ){
 
  int n;
  n = ftcp->Receive(valp);
  return n;
}

int TRich_Frontend::Process_Buf(unsigned int *buf, int nwords, FILE *f,FILE *fraw, int print){

	static int tag = 15;
	static int tag_idx = 0;
	int word;

	int lnevents =0;
	
	while(nwords--)
	{
		word = *buf++;
		
		if(f) fprintf(f, "0x%08X", word);
		if(fraw) fprintf(fraw,"%08X\n",word); // raw data file
		
		if(print) printf("0x%08X", word);
		
		if(word & 0x80000000)
		{
			tag = (word>>27) & 0xF;
			tag_idx = 0;
		}
		else
			tag_idx++;
		
		switch(tag)
		{
			case 0:	// block header
				if(f) fprintf(f, " [BLOCKHEADER] SLOT=%d, BLOCKNUM=%d, BLOCKSIZE=%d\n", (word>>22)&0x1F, (word>>8)&0x3FF, (word>>0)&0xFF);
				if(print) printf(" [BLOCKHEADER] SLOT=%d, BLOCKNUM=%d, BLOCKSIZE=%d\n", (word>>22)&0x1F, (word>>8)&0x3FF, (word>>0)&0xFF);
				break;
				
			case 1:	// block trailer
				if(f) fprintf(f, " [BLOCKTRAILER] SLOT=%d, WORDCNT=%d\n", (word>>22)&0x1F, (word>>0)&0x3FFFFF);
				if(print) printf(" [BLOCKTRAILER] SLOT=%d, WORDCNT=%d\n", (word>>22)&0x1F, (word>>0)&0x3FFFFF);
				break;
				
			case 2:	// event header
				if(f) fprintf(f, " [EVENTHEADER] TRIGGERNUM=%d, DEVID=%d\n", (word>>0)&0x3FFFFF, (word>>22)&0x1F);
				if(print) printf(" [EVENTHEADER] TRIGGERNUM=%d, DEVID=%d\n", (word>>0)&0x3FFFFF, (word>>22)&0x1F);
				lnevents++;				
				break;
				
			case 3:	// trigger time
				if(tag_idx == 0)
				{
					if(f) fprintf(f, " [TIMESTAMP 0] TIME=%d\n", (word>>0)&0xFFFFFF);
					if(print) printf(" [TIMESTAMP 0] TIME=%d\n", (word>>0)&0xFFFFFF);
				}
				else if(tag_idx == 1)
				{
					if(f) fprintf(f, " [TIMESTAMP 1] TIME=%d\n", (word>>0)&0xFFFFFF);
					if(print) printf(" [TIMESTAMP 1] TIME=%d\n", (word>>0)&0xFFFFFF);
				}
				break;
				
			case 8:	// TDC hit
				if(f) fprintf(f, " [TDC HIT] EDGE=%d, CH=%d, TIME=%d\n", (word>>26)&0x1,(word>>16)&0xFF, (word>>0)&0xFFFF);
				if(print) printf(" [TDC HIT] EDGE=%d, CH=%d, TIME=%d\n", (word>>26)&0x1,(word>>16)&0xFF, (word>>0)&0xFFFF);
				break;
				
			case 14:	// data not valid
				if(f) fprintf(f, " [DNV]\n");
				if(print) printf(" [DNV]\n");
				break;
				
			case 15:	// filler word
				if(f) fprintf(f, " [FILLER]\n");
				if(print) printf(" [FILLER]\n");
				break;
				
			default:	// unknown
				if(f) fprintf(f, " [UNKNOWN]\n");
				if(print) printf(" [UNKNOWN]\n");
				break;
		}
	}
	return lnevents;
}

int TRich_Frontend::AsicNumber(){
	int n = MAROC_NUM;
	return n;
}

void TRich_Frontend::StaticReg(int n){

	MAROC_Regs wr_regs[n];
	MAROC_Regs rd_regs[n];

	int i;

	// init
	for(i = 0; i < n; i++)
	{
		printf("%s%d init: ",__FUNCTION__,i);
		memset(&wr_regs[i], 0, sizeof(MAROC_Regs));
		Init(&wr_regs[i],i);
		Print(wr_regs[i]);
		printf("\n");
	}
	
	// clear
	ClearRegs();
	
	// write
	for(i = n-1; i >= 0; i--)
	{
		//printf("%s%d write\n",__FUNCTION__,i);
		Shift(wr_regs[i]);
	}

	// read back
	for(i = 0; i < n; i++)
	{
		rd_regs[i] = Shift(wr_regs[i]);
		printf("%s%d read: ",__FUNCTION__,i);
		Print(rd_regs[i]);
		printf("\n");
	}


}
void TRich_Frontend::DynReg(int n){
	
	printf("%s...\n",__FUNCTION__);

	MAROC_DyRegs wr_dyn_regs;
	MAROC_DyRegs rd_dyn_regs[n];

	InitDyn(&wr_dyn_regs);

	ClearRegsDyn();

	ShiftDyn(wr_dyn_regs, NULL, NULL, NULL);

	if(MAROC_NUM == 2)
		ShiftDyn(wr_dyn_regs, &rd_dyn_regs[0], NULL, &rd_dyn_regs[1]);
	else if(MAROC_NUM == 3)
		ShiftDyn(wr_dyn_regs, &rd_dyn_regs[0], &rd_dyn_regs[1], &rd_dyn_regs[2]);
	
	for(int i = 0; i < MAROC_NUM; i++)
	{
		//printf("MAROC ID %d READ BACK DYN REG DUMP:\n", i);
		//PrintDyn(rd_dyn_regs[i]);
	}
}
void TRich_Frontend::ConfigureMAROC(){
  	
	printf("%s...\n",__FUNCTION__);

	int n = AsicNumber();

	StaticReg(n);

	DynReg(n);
}

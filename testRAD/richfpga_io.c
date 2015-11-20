#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include "richfpga_io.h"

RICH_regs *pRICH_regs = (RICH_regs *)0x0;

int sockfd_reg = 0;
int sockfd_event = 0;

typedef struct
{
	int len;
	int type;
	int wrcnt;
	int addr;
	int flags;
	int vals[1];
} write_struct;

typedef struct
{
	int len;
	int type;
	int rdcnt;
	int addr;
	int flags;
} read_struct;

typedef struct
{
	int len;
	int type;
	int rdcnt;
	int data[1];
} read_rsp_struct;

void rich_write32(void *addr, int val)
{
	write_struct ws;

	ws.len = 16;
	ws.type = 4;
	ws.wrcnt = 1;
	ws.addr = (int)((long)addr);
	ws.flags = 0;
	ws.vals[0] = val;
	write(sockfd_reg, &ws, sizeof(ws));
}

unsigned int rich_read32(void *addr)
{
	read_struct rs;
	read_rsp_struct rs_rsp;
	int len;
	
	rs.len = 12;
	rs.type = 3;
	rs.rdcnt = 1;
	rs.addr = (int)((long)addr);
	rs.flags = 0;
	write(sockfd_reg, &rs, sizeof(rs));
	
	len = read(sockfd_reg, &rs_rsp, sizeof(rs_rsp));
	if(len != sizeof(rs_rsp))
		printf("Error in %s: socket read failed...\n", __FUNCTION__);
	
	return rs_rsp.data[0];
}

void rich_read32_n(int n, void *addr, unsigned int *buf)
{
	read_struct rs;
	read_rsp_struct rs_rsp;
	int len, i;
	
	for(i = 0; i < n; i++)
	{
		rs.len = 12;
		rs.type = 3;
		rs.rdcnt = 1;
		rs.addr = (int)((long)addr);
		rs.flags = 0;
		write(sockfd_reg, &rs, sizeof(rs));
	}
	
	for(i = 0; i < n; i++)
	{
		len = read(sockfd_reg, &rs_rsp, sizeof(rs_rsp));
		if(len != sizeof(rs_rsp))
			printf("Error in %s: socket read failed...\n", __FUNCTION__);
		
		buf[i] = rs_rsp.data[0];
	}
}

/*****************************************************************/
/*          RICH Static Register Configuration Interface         */
/*****************************************************************/

void rich_clear_regs()
{
	int val;

	/* set rst_sc low */
	val = rich_read32(&pRICH_regs->MAROC_Cfg.SerCtrl);
	val &= 0xFFFFFFFE;
	rich_write32(&pRICH_regs->MAROC_Cfg.SerCtrl, val);

	/* set rst_sc high */
	val |= 0x00000001;
	rich_write32(&pRICH_regs->MAROC_Cfg.SerCtrl, val);
}

MAROC_Regs rich_shift_regs(MAROC_Regs regs)
{
	MAROC_Regs result;
	int i, val;
	
	/* write settings to FPGA shift register */
	rich_write32(&pRICH_regs->MAROC_Cfg.Regs.Global0.val, regs.Global0.val);
	rich_write32(&pRICH_regs->MAROC_Cfg.Regs.Global1.val, regs.Global1.val);
	rich_write32(&pRICH_regs->MAROC_Cfg.Regs.DAC.val, regs.DAC.val);
	
	for(i = 0; i < 32; i++)
		rich_write32(&pRICH_regs->MAROC_Cfg.Regs.CH[i].val, regs.CH[i].val);
	
	/* do shift register transfer */
	val = rich_read32(&pRICH_regs->MAROC_Cfg.SerCtrl);
	val |= 0x00000002;
	rich_write32(&pRICH_regs->MAROC_Cfg.SerCtrl, val);
	
	/* check for shift register transfer completion */
	for(i = 10; i > 0; i--)
	{
		val = rich_read32(&pRICH_regs->MAROC_Cfg.SerStatus);
		if(!(val & 0x00000001))
			break;
		
		if(!i)
			printf("Error in %s: timeout on serial transfer...\n", __FUNCTION__);

		usleep(100);
	}

	/* read back settings from FPGA shift register */
	result.Global0.val = rich_read32(&pRICH_regs->MAROC_Cfg.Regs.Global0.val);
	result.Global1.val = rich_read32(&pRICH_regs->MAROC_Cfg.Regs.Global1.val);
	result.DAC.val = rich_read32(&pRICH_regs->MAROC_Cfg.Regs.DAC.val);
	
	for(i = 0; i < 32; i++)
		result.CH[i].val = rich_read32(&pRICH_regs->MAROC_Cfg.Regs.CH[i].val);
	
	return result;
}

void rich_print_regs(MAROC_Regs regs)
{
	int i;
	
	printf("Global0 = 0x%08X\n", regs.Global0.val);
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

	printf("Global1 = 0x%08X\n", regs.Global1.val);
	printf("   d1_d2             = %d\n", regs.Global1.bits.d1_d2);
	printf("   cmd_CK_mux        = %d\n", regs.Global1.bits.cmd_CK_mux);
	printf("   ONOFF_otabg       = %d\n", regs.Global1.bits.ONOFF_otabg);
	printf("   ONOFF_dac         = %d\n", regs.Global1.bits.ONOFF_dac);
	printf("   small_dac         = %d\n", regs.Global1.bits.small_dac);
	printf("   enb_outADC        = %d\n", regs.Global1.bits.enb_outADC);
	printf("   inv_startCmptGray = %d\n", regs.Global1.bits.inv_startCmptGray);
	printf("   ramp_8bit         = %d\n", regs.Global1.bits.ramp_8bit);
	printf("   ramp_10bit        = %d\n", regs.Global1.bits.ramp_10bit);

	printf("DAC = 0x%08X\n", regs.DAC.val);
	printf("   DAC0              = %d\n", regs.DAC.bits.DAC0);
	printf("   DAC1              = %d\n", regs.DAC.bits.DAC1);

	printf("Channels:\n");
	printf("%7s%7s%7s%7s%7s\n", "CH", "Gain", "Sum", "CTest", "MaskOr");
	for(i = 0; i < 64; i++)
	{
		
	printf("%7d%7d%7d%7d%7d\n", i,regs.CH[i].bits.Gain, regs.CH[i].bits.Sum,regs.CH[i].bits.CTest, regs.CH[i].bits.MaskOr);
	
			
	}
	printf("\n");
}


void rich_init_regs(MAROC_Regs *regs)
{
	int i;

	memset(regs, 0, sizeof(MAROC_Regs));

	regs->Global0.bits.cmd_fsu = 1;
	regs->Global0.bits.cmd_ss = 1;
	regs->Global0.bits.cmd_fsb = 1;
	regs->Global0.bits.ONOFF_ss = 1;
	// RC Buffer
	regs->Global0.bits.swb_buf_250f = 0;
	regs->Global0.bits.swb_buf_500f = 1;
	regs->Global0.bits.swb_buf_1p = 1;
	regs->Global0.bits.swb_buf_2p = 1;
	// Slow Shaper
	regs->Global0.bits.sw_ss_300f = 1;
	regs->Global0.bits.sw_ss_600f = 0;
	regs->Global0.bits.sw_ss_1200f = 0;

	regs->Global0.bits.EN_ADC = 1;	// 1 enable ADC, 0 disable ADC
	regs->Global0.bits.H1H2_choice = 0;
	regs->Global0.bits.sw_fsu_20f = 1;
	regs->Global0.bits.sw_fsu_40f = 1;
	regs->Global0.bits.sw_fsu_25k = 0;
	regs->Global0.bits.sw_fsu_50k = 0;
	regs->Global0.bits.sw_fsu_100k = 0;
	regs->Global0.bits.sw_fsb1_50k = 0;
	regs->Global0.bits.sw_fsb1_100k = 0;
	regs->Global0.bits.sw_fsb1_100f = 1;
	regs->Global0.bits.sw_fsb1_50f = 1;
	regs->Global0.bits.cmd_fsb_fsu = 0;
	regs->Global0.bits.valid_dc_fs = 1;
	regs->Global0.bits.sw_fsb2_50k = 0;
	regs->Global0.bits.sw_fsb2_100k = 0;
	regs->Global0.bits.sw_fsb2_100f = 0;
	regs->Global0.bits.sw_fsb2_50f = 1;
	regs->Global0.bits.valid_dc_fsb2 = 0;
	regs->Global0.bits.ENb_tristate = 1;
	regs->Global0.bits.polar_discri = 0;
	regs->Global0.bits.inv_discriADC = 0;
	regs->Global1.bits.d1_d2 = 0;
	regs->Global1.bits.cmd_CK_mux = 0;
	regs->Global1.bits.ONOFF_otabg = 0;
	regs->Global1.bits.ONOFF_dac = 0;
	regs->Global1.bits.small_dac = 0; /* 0=2.3mV/DAC LSB, 1=1.1mV/DAC LSB */
	regs->Global1.bits.enb_outADC = 0;
//	regs->Global1.bits.enb_outADC = 1;
	regs->Global1.bits.inv_startCmptGray = 0;
	regs->Global1.bits.ramp_8bit = 0;
	regs->Global1.bits.ramp_10bit = 0;
//	regs->DAC.bits.DAC0 = 300; /* with small_dac = 0,  pedestal < ~200, signal ~200 to ~500, 500fC/pulse injected */
	regs->DAC.bits.DAC0 = 300; /* with small_dac = 0,  pedestal < ~200, signal ~200 to ~500, 500fC/pulse injected */
	regs->DAC.bits.DAC1 = 0;


	int lgain =64;   /* GAIN 64 = unity */
	int ctest =1;	
	for(i = 0; i < 64; i++)
	{
	
		//if(i == 0) ctest = 1;
		//else ctest = 0;
		
		regs->CH[i].bits.Gain = lgain;
		regs->CH[i].bits.Sum = 0;
		regs->CH[i].bits.CTest = ctest;
		regs->CH[i].bits.MaskOr = 0;
	}
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

/*****************************************************************/
/*          RICH Dynamic Register Configuration Interface        */
/*****************************************************************/

void rich_clear_dynregs()
{
	int val;

	/* set rst_r low */
	val = rich_read32(&pRICH_regs->MAROC_Cfg.SerCtrl);
	val &= 0xFFFFFFFB;
	rich_write32(&pRICH_regs->MAROC_Cfg.SerCtrl, val);

	/* set rst_r high */
	val |= 0x00000004;
	rich_write32(&pRICH_regs->MAROC_Cfg.SerCtrl, val);
}

void rich_shift_dynregs(MAROC_DyRegs wr, MAROC_DyRegs *rd1, MAROC_DyRegs *rd2, MAROC_DyRegs *rd3)
{
	int i, val;
	
	/* write settings to FPGA shift register */
	rich_write32(&pRICH_regs->MAROC_Cfg.DyRegs_WrAll.Ch0_31_Hold1, wr.Ch0_31_Hold1);
	rich_write32(&pRICH_regs->MAROC_Cfg.DyRegs_WrAll.Ch32_63_Hold1, wr.Ch32_63_Hold1);
	rich_write32(&pRICH_regs->MAROC_Cfg.DyRegs_WrAll.Ch0_31_Hold2, wr.Ch0_31_Hold2);
	rich_write32(&pRICH_regs->MAROC_Cfg.DyRegs_WrAll.Ch32_63_Hold2, wr.Ch32_63_Hold2);

	/* do shift register transfer */
	val = rich_read32(&pRICH_regs->MAROC_Cfg.SerCtrl);
	val |= 0x00000008;
	rich_write32(&pRICH_regs->MAROC_Cfg.SerCtrl, val);
	
	/* check for shift register transfer completion */
	for(i = 10; i > 0; i--)
	{
		val = rich_read32(&pRICH_regs->MAROC_Cfg.SerStatus);
		if(!(val & 0x00000002))
			break;
		
		if(!i)
			printf("Error in %s: timeout on serial transfer...\n", __FUNCTION__);

		usleep(100);
	}

	/* read back settings from FPGA shift register */
	if(rd1)
	{
		rd1->Ch0_31_Hold1 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[0].Ch0_31_Hold1);
		rd1->Ch32_63_Hold1 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[0].Ch32_63_Hold1);
		rd1->Ch0_31_Hold2 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[0].Ch0_31_Hold2);
		rd1->Ch32_63_Hold2 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[0].Ch32_63_Hold2);
	}

	if(rd2)
	{
		rd2->Ch0_31_Hold1 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[1].Ch0_31_Hold1);
		rd2->Ch32_63_Hold1 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[1].Ch32_63_Hold1);
		rd2->Ch0_31_Hold2 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[1].Ch0_31_Hold2);
		rd2->Ch32_63_Hold2 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[1].Ch32_63_Hold2);
	}

	if(rd3)
	{
		rd3->Ch0_31_Hold1 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[2].Ch0_31_Hold1);
		rd3->Ch32_63_Hold1 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[2].Ch32_63_Hold1);
		rd3->Ch0_31_Hold2 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[2].Ch0_31_Hold2);
		rd3->Ch32_63_Hold2 = rich_read32(&pRICH_regs->MAROC_Cfg.DyRegs_Rd[2].Ch32_63_Hold2);
	}
}


void rich_print_dynregs(MAROC_DyRegs regs)
{
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


void rich_init_dynregs(MAROC_DyRegs *regs)
{
	int i;

	memset(regs, 0, sizeof(MAROC_DyRegs));

	regs->Ch0_31_Hold1 = 0x00000001;
	regs->Ch32_63_Hold1 = 0x00000000;
	regs->Ch0_31_Hold2 = 0x00000000;
	regs->Ch32_63_Hold2 = 0x00000000;
}

void rich_print_scaler(char *name, unsigned int scaler, float ref)
{
	printf("%-10s %9u %9fHz\n", name, scaler, (float)scaler / ref);
}

void rich_dump_scalers()
{
	float ref, fval;
	unsigned int val;
	int i, j;
	char buf[100];

	/* halt scaler counting */
	rich_write32(&pRICH_regs->Sd.ScalerLatch, 0x1);

	/* read reference time */
	val = rich_read32(&pRICH_regs->Sd.Scaler_GClk125);
	if(!val)
	{
		printf("Error in %s: referenec time invalid - scaler normalization incorrect\n", __FUNCTION__);
		val = 1;
	}
	ref = (float)val / 125.0E6;

	/* read scalers */
	rich_print_scaler("GClk125", rich_read32(&pRICH_regs->Sd.Scaler_GClk125), ref);
	rich_print_scaler("Sync", rich_read32(&pRICH_regs->Sd.Scaler_Sync), ref);
	rich_print_scaler("Trig", rich_read32(&pRICH_regs->Sd.Scaler_Trig), ref);
	rich_print_scaler("Input0", rich_read32(&pRICH_regs->Sd.Scaler_Input[0]), ref);
	rich_print_scaler("Input1", rich_read32(&pRICH_regs->Sd.Scaler_Input[1]), ref);
	rich_print_scaler("Input2", rich_read32(&pRICH_regs->Sd.Scaler_Input[2]), ref);
	rich_print_scaler("Output0", rich_read32(&pRICH_regs->Sd.Scaler_Output[0]), ref);
	rich_print_scaler("Output1", rich_read32(&pRICH_regs->Sd.Scaler_Output[1]), ref);
	rich_print_scaler("Output2", rich_read32(&pRICH_regs->Sd.Scaler_Output[2]), ref);
	rich_print_scaler("Or0_1", rich_read32(&pRICH_regs->Sd.Scaler_Or0[0]), ref);
	rich_print_scaler("Or0_2", rich_read32(&pRICH_regs->Sd.Scaler_Or0[1]), ref);
	rich_print_scaler("Or0_3", rich_read32(&pRICH_regs->Sd.Scaler_Or0[2]), ref);
	rich_print_scaler("Or1_1", rich_read32(&pRICH_regs->Sd.Scaler_Or1[0]), ref);
	rich_print_scaler("Or1_2", rich_read32(&pRICH_regs->Sd.Scaler_Or1[1]), ref);
	rich_print_scaler("Or1_3", rich_read32(&pRICH_regs->Sd.Scaler_Or1[2]), ref);
	rich_print_scaler("Busy", rich_read32(&pRICH_regs->Sd.Scaler_Busy), ref);
	rich_print_scaler("BusyCycles", rich_read32(&pRICH_regs->Sd.Scaler_BusyCycles), ref);
/*
	for(j = 0; j < 64; j++)
	{
		printf("CH%2d", j);

		for(i = 0; i < 3; i++)
		{
			val = rich_read32(&pRICH_regs->MAROC_Proc[i].Scalers[j]);
			fval = (float)val / ref;
			printf(" [%10u,%13.3fHz]", val, fval);
		}
		printf("\n");
	}
*/
	/* resets scalers */
	rich_write32(&pRICH_regs->Sd.ScalerLatch, 0x2);

	/* enable scaler counting */
	rich_write32(&pRICH_regs->Sd.ScalerLatch, 0x0);
}

void rich_set_pulser(float freq, float duty, int count)
{
	if(freq <= 0.0)
	{
		printf("Error in %s: freq invalid, setting to 1Hz", __FUNCTION__);
		freq = 1.0;
	}

	if((duty <= 0.0) || (duty >= 1.0))
	{
		printf("Error in %s: duty invalid, setting to 50%", __FUNCTION__);
		duty = 0.5;
	}

	freq = 125000000 / freq;
	rich_write32(&pRICH_regs->Sd.PulserPeriod, (int)freq);

	duty = freq * (1.0 - duty);
	rich_write32(&pRICH_regs->Sd.PulserLowCycles, (int)duty);

	rich_write32(&pRICH_regs->Sd.PulserNCycles, count);
	rich_write32(&pRICH_regs->Sd.PulserStart, 0);
}

/* Currently feature does not exist in firmware
void rich_set_tdc_deadtime(int ticks_8ns)
{
	int i;
	
	for(i = 0; i < 3; i++)
		rich_write32(&pRICH_regs->MAROC_Proc[i].DeadCycles, ticks_8ns);
}
*/

void rich_enable_all_tdc_channels()
{
	int i;
	
	for(i = 0; i < 3; i++)
	{
		rich_write32(&pRICH_regs->MAROC_Proc[i].DisableCh[0], 0x0000);
		rich_write32(&pRICH_regs->MAROC_Proc[i].DisableCh[1], 0x0000);
		rich_write32(&pRICH_regs->MAROC_Proc[i].DisableCh[2], 0x0000);
		rich_write32(&pRICH_regs->MAROC_Proc[i].DisableCh[3], 0x0000);
	}
}

void rich_disable_all_tdc_channels()
{
	int i;
	
	for(i = 0; i < 3; i++)
	{
		rich_write32(&pRICH_regs->MAROC_Proc[i].DisableCh[0], 0xFFFF);
		rich_write32(&pRICH_regs->MAROC_Proc[i].DisableCh[1], 0xFFFF);
		rich_write32(&pRICH_regs->MAROC_Proc[i].DisableCh[2], 0xFFFF);
		rich_write32(&pRICH_regs->MAROC_Proc[i].DisableCh[3], 0xFFFF);
	}
}

void rich_enable_tdc_channel(int ch)
{
	int i;
	int proc_idx, proc_reg, proc_bit;
	int val;
	
	if((ch < 0) || (ch > 191))
	{
		printf("Error in %s: invalid channel %d\n", __FUNCTION__, ch);
		return;
	}

	proc_idx = ch / 64;
	proc_reg = (ch % 64) / 16;
	proc_bit = (ch % 16);

	val = rich_read32(&pRICH_regs->MAROC_Proc[proc_idx].DisableCh[proc_reg]);
	val = val & ~(1<<proc_bit);
	rich_write32(&pRICH_regs->MAROC_Proc[proc_idx].DisableCh[proc_reg], val);
}

void rich_disable_tdc_channel(int ch)
{
	int i;
	int proc_idx, proc_reg, proc_bit;
	int val;
	
	if((ch < 0) || (ch > 191))
	{
		printf("Error in %s: invalid channel %d\n", __FUNCTION__, ch);
		return;
	}

	proc_idx = ch / 64;
	proc_reg = (ch % 64) / 16;
	proc_bit = (ch % 16);

	val = rich_read32(&pRICH_regs->MAROC_Proc[proc_idx].DisableCh[proc_reg]);
	val = val | (1<<proc_bit);
	rich_write32(&pRICH_regs->MAROC_Proc[proc_idx].DisableCh[proc_reg], val);
}

void rich_setup_readout(int lookback, int window)
{
	rich_write32(&pRICH_regs->EvtBuilder.Lookback, lookback);		/* lookback*8ns from trigger time */
	rich_write32(&pRICH_regs->EvtBuilder.WindowWidth, window);	/* capture window*8ns hits after lookback time */
	rich_write32(&pRICH_regs->EvtBuilder.BlockCfg, 1);				/* 1 event per block */
	rich_write32(&pRICH_regs->EvtBuilder.DeviceID, 0x01);			/* setup a dummy device ID (5bit) written in each event for module id - needs to be expanded */
}

void rich_fifo_reset()
{
	rich_write32(&pRICH_regs->Clk.Ctrl, 0x01);						/* assert soft reset */
	rich_write32(&pRICH_regs->Clk.Ctrl, 0x00);						/* deassert soft reset */
}

void rich_fifo_status()
{
	int wordCnt, eventCnt;
	
	wordCnt = rich_read32(&pRICH_regs->EvtBuilder.FifoWordCnt);
	eventCnt = rich_read32(&pRICH_regs->EvtBuilder.FifoEventCnt);
	
	printf("FIFO Event Status: WCNT=%9d ECNT=%9d\n", wordCnt, eventCnt);
}

int rich_fifo_nwords()
{
	return rich_read32(&pRICH_regs->EvtBuilder.FifoWordCnt);
}

void rich_fifo_read(int *buf, int nwords)
{
	while(nwords--)
		*buf++ = rich_read32(&pRICH_regs->EvtBuilder.FifoData);
}

void rich_process_buf(int *buf, int nwords, FILE *f, int print)
{
	static int tag = 15;
	static int tag_idx = 0;
	int word;
	
	while(nwords--)
	{
		word = *buf++;
		
		if(f) fprintf(f, "0x%08X", word);
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
}

unsigned int grayToBinary(unsigned int num)
{
    unsigned int mask;
    for (mask = num >> 1; mask != 0; mask = mask >> 1)
    {
        num = num ^ mask;
    }
    return num;
}

void rich_check_adc()
{
	static FILE *f = NULL;
	static int hold_delay = 0, sample = 0;
	int ready = rich_read32(&pRICH_regs->MAROC_Adc.AdcStatus);
	int i, j, addr;
	unsigned int adc[3];
	
	if(!f) f = fopen("adc_RCBuf3500_SlowShap0300_Gain064.txt", "wt");
	
	printf("rich_check_adc() - ready=%X (hold=%3d): \n", ready, hold_delay);
	if((ready & 0x7) == 0x7)
	{
		printf("rich_check_adc() - valid (hold=%3d): ", hold_delay);
	
		fprintf(f, "%d\t%d\t", sample++, hold_delay);
		
		for(i = 0; i < 3; i++)
		{
			for(addr = 0; addr < 64; addr++)
			{
				rich_write32(&pRICH_regs->MAROC_Adc.AdcAddr, addr);
				adc[i] = rich_read32(&pRICH_regs->MAROC_Adc.AdcData[i]);
				fprintf(f, "%d\t", grayToBinary(adc[i]));
			}
		}
		fprintf(f, "\n");
// 		printf("Ch%2d   %4d [%d%d%d%d_%d%d%d%d_%d%d%d%d]   %4d [%d%d%d%d_%d%d%d%d_%d%d%d%d]   %4d [%d%d%d%d_%d%d%d%d_%d%d%d%d]\n", addr,
// 		adc[0], (adc[0]>>11)&0x1, (adc[0]>>10)&0x1, (adc[0]>>9)&0x1, (adc[0]>>8)&0x1, (adc[0]>>7)&0x1, (adc[0]>>6)&0x1, (adc[0]>>5)&0x1, (adc[0]>>4)&0x1, (adc[0]>>3)&0x1, (adc[0]>>2)&0x1, (adc[0]>>1)&0x1, (adc[0]>>0)&0x1,
// 		adc[1], (adc[1]>>11)&0x1, (adc[1]>>10)&0x1, (adc[1]>>9)&0x1, (adc[1]>>8)&0x1, (adc[1]>>7)&0x1, (adc[1]>>6)&0x1, (adc[1]>>5)&0x1, (adc[1]>>4)&0x1, (adc[1]>>3)&0x1, (adc[1]>>2)&0x1, (adc[1]>>1)&0x1, (adc[1]>>0)&0x1,
// 		adc[2], (adc[2]>>11)&0x1, (adc[2]>>10)&0x1, (adc[2]>>9)&0x1, (adc[2]>>8)&0x1, (adc[2]>>7)&0x1, (adc[2]>>6)&0x1, (adc[2]>>5)&0x1, (adc[2]>>4)&0x1, (adc[2]>>3)&0x1, (adc[2]>>2)&0x1, (adc[2]>>1)&0x1, (adc[2]>>0)&0x1
// 		);

		rich_write32(&pRICH_regs->MAROC_Adc.Hold1Delay, hold_delay);
		rich_write32(&pRICH_regs->MAROC_Adc.Hold2Delay, hold_delay);
 		hold_delay++;
 		if(hold_delay > 80) hold_delay = 0;

		rich_write32(&pRICH_regs->MAROC_Adc.AdcCtrl, (11<<4) | 0x1);		// Max bit=11 for 12bit, enable all 3 MAROC_ADC, clear ADC
	}
	rich_write32(&pRICH_regs->MAROC_Adc.AdcCtrl, (11<<4) | 0x1);		// Max bit=11 for 12bit, enable all 3 MAROC_ADC, clear ADC
}

void rich_setmask_fpga_or(int m0_0, int m0_1, int m1_0, int m1_1, int m2_0, int m2_1)
{
	rich_write32(&pRICH_regs->MAROC_Proc[0].HitOrMask0, m0_0);
	rich_write32(&pRICH_regs->MAROC_Proc[0].HitOrMask1, m0_1);
	rich_write32(&pRICH_regs->MAROC_Proc[1].HitOrMask0, m1_0);
	rich_write32(&pRICH_regs->MAROC_Proc[1].HitOrMask1, m1_1);
	rich_write32(&pRICH_regs->MAROC_Proc[2].HitOrMask0, m2_0);
	rich_write32(&pRICH_regs->MAROC_Proc[2].HitOrMask1, m2_1);
}

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

#define MAROC_NUM		3

void rich_test_regs()
{
	int i;
	
	MAROC_Regs wr_regs[MAROC_NUM];
	MAROC_Regs rd_regs[MAROC_NUM];
	MAROC_DyRegs wr_dyn_regs;
	MAROC_DyRegs rd_dyn_regs[MAROC_NUM];
	
	for(i = 0; i < MAROC_NUM; i++)
		rich_init_regs(&wr_regs[i]);

	rich_clear_regs();
	for(i = MAROC_NUM-1; i >= 0; i--)
		rich_shift_regs(wr_regs[i]);
	
	for(i = 0; i < MAROC_NUM; i++)
	{
		rd_regs[i] = rich_shift_regs(wr_regs[i]);
	
		printf("MAROC ID %d READ BACK REG DUMP:\n", i);
		rich_print_regs(rd_regs[i]);
	}

	/* Initialize dynamic registers */
	rich_init_dynregs(&wr_dyn_regs);

	rich_clear_dynregs();
	rich_shift_dynregs(wr_dyn_regs, NULL, NULL, NULL);
	if(MAROC_NUM == 2)
		rich_shift_dynregs(wr_dyn_regs, &rd_dyn_regs[0], NULL, &rd_dyn_regs[1]);
	else if(MAROC_NUM == 3)
		rich_shift_dynregs(wr_dyn_regs, &rd_dyn_regs[0], &rd_dyn_regs[1], &rd_dyn_regs[2]);

	for(i = 0; i < MAROC_NUM; i++)
	{
		printf("MAROC ID %d READ BACK DYN REG DUMP:\n", i);
		rich_print_dynregs(rd_dyn_regs[i]);
	}
}

void rich_SelectSpi(int sel)
{
	if(sel)
		rich_write32(&pRICH_regs->Clk.SpiCtrl, 0x200);
	else
		rich_write32(&pRICH_regs->Clk.SpiCtrl, 0x100);
}

unsigned char rich_TransferSpi(unsigned char data)
{
	int i;
	unsigned int val;

	rich_write32(&pRICH_regs->Clk.SpiCtrl, data | 0x400);

	for(i = 0; i < 1000; i++)
	{
		val = rich_read32(&pRICH_regs->Clk.SpiStatus);
		if(val & 0x800)
			break;
	}
	if(i == 1000)
		printf("%s: ERROR: Timeout!!!\n", __FUNCTION__);

	return val & 0xFF;
}


void rich_TransferSpi_n(int n, unsigned char *tx, unsigned char *rx)
{
	int i;
	unsigned int val;
	write_struct ws;
	read_struct rs;
	read_rsp_struct rs_rsp;
	int len;

	for(i = 0; i < n; i++)
	{
		ws.len = 16;
		ws.type = 4;
		ws.wrcnt = 1;
		ws.addr = (int)((long)&pRICH_regs->Clk.SpiCtrl);
		ws.flags = 0;
		ws.vals[0] = tx[i] | 0x400;
		write(sockfd_reg, &ws, sizeof(ws));

		rs.len = 12;
		rs.type = 3;
		rs.rdcnt = 1;
		rs.addr = (int)((long)&pRICH_regs->Clk.SpiStatus);
		rs.flags = 0;
		write(sockfd_reg, &rs, sizeof(rs));
	}
			
	for(i = 0; i < n; i++)
	{
		len = read(sockfd_reg, &rs_rsp, sizeof(rs_rsp));
		if(len != sizeof(rs_rsp))
			printf("Error in %s: socket read failed...\n", __FUNCTION__);
		if(!(rs_rsp.data[0] & 0x800)) printf("rich_TransferSpi_n() too fast!\n"); 
		rx[i] = rs_rsp.data[0] & 0xFF;
	}
}

void rich_FlashGetId(unsigned char *rsp)
{
	rich_SelectSpi(1);
	rich_TransferSpi(FLASH_CMD_GETID);
	rsp[0] = rich_TransferSpi(0xFF);
	rsp[1] = rich_TransferSpi(0xFF);
	rsp[2] = rich_TransferSpi(0xFF);
	rich_SelectSpi(0);
}

unsigned char rich_FlashGetStatus(unsigned char cmd)
{
	unsigned char rsp;
	
	rich_SelectSpi(1);
	rich_TransferSpi(cmd);
	rsp = rich_TransferSpi(0xFF);
	rich_SelectSpi(0);
	
	return rsp;
}

int rich_FirmwareUpdateVerify(const char *filename)
{
	int i, result;

	printf("Updating firmware...");
	result = rich_FirmwareUpdate(filename);
	if(result != OK)
	{
		printf("failed.\n");
		return result;
	}
	else
		printf("succeeded.\n");
	
	printf("\nVerifying...");
	result = rich_FirmwareVerify(filename);
	if(result != OK)
	{
		printf("failed.\n");
		return result;
	}
	else
		printf("ok.\n");

	printf("Warning: user must cycle power for firmware load to take effect. (Future update to driver will do this automatically).\n");
	usleep(120000);
		
	return OK;
}

int rich_FirmwareUpdate(const char *filename)
{
	FILE *f;
	int i, flashId = 0;
	unsigned int addr = 0, page = 0;
	unsigned char buf[1056], rspId[3];

	rich_SelectSpi(0);
	rich_FlashGetId(rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == FLASH_MFG_MICRON) &&
	    (rspId[1] == (FLASH_DEVID_N25Q256A>>8)) &&
	    (rspId[2] == (FLASH_DEVID_N25Q256A&0xFF)) )
	{
		f = fopen(filename, "rb");
		if(!f)
		{
			printf("%s: ERROR: dcFirmwareUpdate invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
	
		memset(buf, 0xff, 256);
		while(fread(buf, 1, 256, f) > 0)
		{
		  if(!(addr % 65536))		/* sector erase*/
			{
				rich_SelectSpi(1);
				rich_TransferSpi(FLASH_CMD_WREN);	/* write enable*/
				rich_SelectSpi(0);

				rich_SelectSpi(1);
				rich_TransferSpi(FLASH_CMD_ERASE64K);	/* 64k sector erase*/
				rich_TransferSpi((addr>>24)&0xFF);
				rich_TransferSpi((addr>>16)&0xFF);
				rich_TransferSpi((addr>>8)&0xFF);
				rich_TransferSpi((addr)&0xFF);
				rich_SelectSpi(0);

				printf(".");
				i = 0;
				while(1)
				{
					if(!(rich_FlashGetStatus(FLASH_CMD_GETSTATUS) & 0x1))
						break;
					usleep(16000);
					if(i == 60+6)	/* 1000ms maximum sector erase time*/
					{
						fclose(f);
						printf("%s: ERROR: failed to erase flash\n", __FUNCTION__);
						return ERROR;
					}
					i++;
				}
			}

			rich_SelectSpi(1);
			rich_TransferSpi(FLASH_CMD_WREN);	/* write enable*/
			rich_SelectSpi(0);

			rich_SelectSpi(1);
			rich_TransferSpi(FLASH_CMD_WRPAGE);	/* write page*/
			rich_TransferSpi((addr>>24)&0xFF);
			rich_TransferSpi((addr>>16)&0xFF);
			rich_TransferSpi((addr>>8)&0xFF);
			rich_TransferSpi((addr)&0xFF);
			for(i = 0; i < 256; i++)
				rich_TransferSpi(buf[i]);
			rich_SelectSpi(0);

			i = 0;
			while(1)
			{
			  if(!(rich_FlashGetStatus(FLASH_CMD_GETSTATUS) & 0x1))	/* no faster than 1us per call*/
					break;
			  if(i == 3000)	/* 3ms maximum page program time*/
				{
					fclose(f);
					printf("%s: ERROR: failed to program flash\n", __FUNCTION__);
					return ERROR;
				}
				i++;
			}
			memset(buf, 0xff, 256);
			addr+= 256;
		}
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: failed to identify flash id (or device not supported)\n", __FUNCTION__);
		return ERROR;
	}
	return OK;
}

int rich_FirmwareRead(const char *filename)
{
	FILE *f;
	int i,len, flashId = 0;
	unsigned int addr = 0;
	unsigned char buf[256];
	unsigned char rspId[3];

	rich_SelectSpi(0);
	rich_FlashGetId(rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == FLASH_MFG_MICRON) &&
	    (rspId[1] == (FLASH_DEVID_N25Q256A>>8)) &&
	    (rspId[2] == (FLASH_DEVID_N25Q256A&0xFF)) )
	{
		f = fopen(filename, "wb");
		if(!f)
		{
			printf("%s: ERROR: dcFirmwareRead invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
		
		rich_SelectSpi(1);
		rich_TransferSpi(FLASH_CMD_RD);	/* continuous array read */
		rich_TransferSpi((addr>>24)&0xFF);
		rich_TransferSpi((addr>>16)&0xFF);
		rich_TransferSpi((addr>>8)&0xFF);
		rich_TransferSpi((addr)&0xFF);
		
		for(i = 0; i < FLASH_BYTE_LENGTH; i++)
		{
			fputc(rich_TransferSpi(0xFF), f);
			if(!(i% 65536))
			{
				printf(".");
				usleep(16000);
			}
		}
			
		rich_SelectSpi(0);
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: failed to identify flash id 0x%02X\n", __FUNCTION__, flashId);
		return ERROR;
	}
	return OK;
}

int rich_FirmwareVerify(const char *filename)
{
	FILE *f;
	int i,len, flashId = 0;
	unsigned int addr = 0;
	unsigned char buf[256];
	unsigned char rspId[3], val;
	
	rich_SelectSpi(0);
	rich_FlashGetId(rspId);
	
	printf("Flash: Mfg=0x%02X, Type=0x%02X, Capacity=0x%02X\n", rspId[0], rspId[1], rspId[2]);

	if( (rspId[0] == FLASH_MFG_MICRON) &&
	    (rspId[1] == (FLASH_DEVID_N25Q256A>>8)) &&
	    (rspId[2] == (FLASH_DEVID_N25Q256A&0xFF)) )
	{
		f = fopen(filename, "rb");
		if(!f)
		{
			printf("%s: ERROR: dcFirmwareVerify invalid file %s\n", __FUNCTION__, filename);
			return ERROR;
		}
		
		rich_SelectSpi(1);
		rich_TransferSpi(FLASH_CMD_RD);	/* continuous array read */
		rich_TransferSpi((addr>>24)&0xFF);
		rich_TransferSpi((addr>>16)&0xFF);
		rich_TransferSpi((addr>>8)&0xFF);
		rich_TransferSpi((addr)&0xFF);

		while((len = fread(buf, 1, 256, f)) > 0)
		{
			for(i = 0; i < len; i++)
			{
				val = rich_TransferSpi(0xFF);
				if(buf[i] != val)
				{
					rich_SelectSpi(0);
					fclose(f);					
					printf("%s: ERROR: failed verify at addess 0x%08X[%02X,%02X]\n", __FUNCTION__, addr+i, buf[i], val);
					return ERROR;
				}
			}
			addr+=256;
			if(!(addr & 0xFFFF))
				printf(".");
		}
		rich_SelectSpi(0);
		fclose(f);
	}
	else
	{
		printf("%s: ERROR: failed to identify flash id 0x%02X\n", __FUNCTION__, flashId);
		return ERROR;
	}
	return OK;
}

int open_socket(int port)
{
	struct sockaddr_in serv_addr;
	int sockfd = 0;
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Error : Could not create socket \n");
		exit(1);
	}
	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, "192.168.1.10", &serv_addr.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		exit(1);
	} 

	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		exit(1);
	}
	return sockfd;
}

void open_register_socket()
{
	int n, val;
	
	sockfd_reg = open_socket(6102);

	/* Send endian test header */
	val = 0x12345678;
	write(sockfd_reg, &val, 4);
	
	val = 0;
	n = read(sockfd_reg, &val, 4);
	printf("n = %d, val = 0x%08X\n", n, val);
}

unsigned int gBuf[100000];

int main(int argc, char *argv[])
{
	int n, val = 0;
	long long nevents, nbytes, dbytes;
	FILE *f = fopen("rich_events.bin", "wb");

	open_register_socket();


	rich_RadTest_Init();
	
	while(1)
	{
		sleep(10);
		rich_RadTest_UpdateCounters();
	}


	rich_test_regs();
	
rich_set_pulser(1.0, 0.00001, 0xFFFFFFFF);					// freq, dutycycle, repetition (0= disable,0xffffffff = infinite)

	// Setup FPGA version of MAROC OR (note: this OR is formed in the FPGA from the MAROC hits and does not use the MAROC_OR signal)
	rich_setmask_fpga_or(0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000);

	rich_write32(&pRICH_regs->Sd.OutSrc[0], SD_SRC_SEL_PULSER);			// output pulser to TTL outputs for probing/scope trigger
	rich_write32(&pRICH_regs->Sd.OutSrc[1], SD_SRC_SEL_PULSER);
	rich_write32(&pRICH_regs->Sd.OutSrc[2], SD_SRC_SEL_MAROC_OR);		// output FPGA MAROC OR to TTL outputs for probing/scope trigger
	
	rich_write32(&pRICH_regs->Sd.AdcStartSrc, SD_SRC_SEL_PULSER);		// internal pulser fires test charge injection
//	rich_write32(&pRICH_regs->Sd.AdcStartSrc, 0);							// disable test charge injection
//	rich_write32(&pRICH_regs->Sd.AdcTrigSrc, 0);								// disable adc trigger
	rich_write32(&pRICH_regs->Sd.AdcTrigSrc, SD_SRC_SEL_PULSER);		// internal pulser fires adc trigger
	
//	rich_write32(&pRICH_regs->Sd.AdcTrigSrc, SD_SRC_SEL_MAROC_OR1_0);	// MAROC 1 OR fires adc trigger

	rich_write32(&pRICH_regs->MAROC_Adc.Hold1Delay, 0);
	rich_write32(&pRICH_regs->MAROC_Adc.Hold2Delay, 0);
	rich_write32(&pRICH_regs->MAROC_Adc.AdcCtrl, (11<<4) | 0x1);		// Max bit=11 for 12bit, enable all 3 MAROC_ADC, clear ADC
	
	/* Set trig source to disabled */
	rich_write32(&pRICH_regs->Sd.TrigSrc, 0);
	rich_write32(&pRICH_regs->EvtBuilder.TrigDelay, 128);				//128*8ns = 1024ns trigger delay

	/* Soft pulse sync */
	rich_write32(&pRICH_regs->Sd.SyncSrc, 1);
	rich_write32(&pRICH_regs->Sd.SyncSrc, 0);

	rich_fifo_reset();
	rich_fifo_status();
	rich_setup_readout(1200/8, 800/8);										// lookback 1.2us, capture 400ns
	
	rich_disable_all_tdc_channels();
	rich_enable_all_tdc_channels();
	
	sockfd_event = open_socket(6103);
	
	/* Set trig source to pulser when ready to take events */
	rich_write32(&pRICH_regs->Sd.TrigSrc, SD_SRC_SEL_PULSER);		// trigger source is pulser (delayed by TrigDelay)

	nevents = 0;
	nbytes = 0;
	dbytes = 0;
	printf("RICH - waiting for event data...\n");

	while(1)
	{
/*
		n = recv(sockfd_event, &val, sizeof(val), MSG_WAITALL);
		if(n != sizeof(val))
		{
			printf("Receive error (n=%d) - quitting...\n", n);
			break;
		}
		fwrite(&val, sizeof(val), 1, f);
		nbytes+= sizeof(val);
		dbytes+= sizeof(val);
		if((val & 0xF8000000) == 0x90000000)
		{
			nevents++;
			if(!(nevents % 10000))
			{
				printf("[0x%08X] nevents = %lld, nbytes = %lld, dbytes = %lld\n", val, nevents, nbytes, dbytes);
				fflush(stdout);
				
				dbytes = 0;
				rich_fifo_status();
			}
		}
*/
//		rich_dump_scalers();
//		rich_fifo_status();
		rich_check_adc();
//		usleep(100000);
		sleep(1);
//		printf(".");
//		fflush(stdout);
	}
	close(sockfd_reg);
	close(sockfd_event);
	fclose(f);
	
	return 0;
}

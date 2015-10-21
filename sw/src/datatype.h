
#ifndef RICHFPGA_IO_H
#define RICHFPGA_IO_H


typedef struct
{
	unsigned int	chan; // channel
	unsigned int	time; // [ns] delay from trigger timestamp 
	unsigned int	edge; // 0 =  rising 1 = falling 
	
}RICH_tdc_edge_t;

typedef struct
{
	int	did;
	int	thr;
	int	gain[64];
	
}RICH_maroc_t;


typedef struct
{
	//string deviceModel; // = "Lecroy9210";
	float vhigh; //= 0 [mV]
	float vlow;//= -50 [mV]
	float width; //= 50.0 [us]
	float delay; //= 0 [us?]
	float lead; //= 0.9 [ns]
	float trail;// = 0.9 [ns]
	float period;// = 100 [us]
	
}ExtPulser_t;

typedef struct
{
	int fpgaID;
	int firmware_version;
	int pulser_freq;
	float pulser_dutycycle;
	int pulser_repetition;

	int trig_delay;
	int trig_source;

	int evtbuild_lookback;
	int evtbuild_window;
	int block_nevents;

	int out[3];
	
}RICH_fpga_t;


typedef struct
{
	unsigned int cmd_fsu;
	unsigned int cmd_ss;
	unsigned int cmd_fsb;
	unsigned int swb_buf_250f;
	unsigned int swb_buf_500f;
	unsigned int swb_buf_1p;
	unsigned int swb_buf_2p;
	unsigned int ONOFF_ss;
	unsigned int sw_ss_300f;
	unsigned int sw_ss_600f;
	unsigned int sw_ss_1200f;
	unsigned int EN_ADC;
	unsigned int H1H2_choice;
	unsigned int sw_fsu_20f;
	unsigned int sw_fsu_40f;
	unsigned int sw_fsu_25k;
	unsigned int sw_fsu_50k;
	unsigned int sw_fsu_100k;
	unsigned int sw_fsb1_50k;

	unsigned int sw_fsb1_100k;
	unsigned int sw_fsb1_100f;
	unsigned int sw_fsb1_50f;
	unsigned int cmd_fsb_fsu;
	unsigned int valid_dc_fs;
	unsigned int sw_fsb2_50k;
	unsigned int sw_fsb2_100k;
	unsigned int sw_fsb2_100f;
	unsigned int sw_fsb2_50f;
	unsigned int valid_dc_fsb2;
	unsigned int ENb_tristate;
	unsigned int polar_discri;
	unsigned int inv_discriADC;
	unsigned int d1_d2;
	unsigned int cmd_CK_mux;
	unsigned int ONOFF_otabg;
	unsigned int ONOFF_dac;
	unsigned int small_dac;
	unsigned int enb_outADC;
	unsigned int inv_startCmptGray;
	unsigned int ramp_8bit;
	unsigned int ramp_10bit;
	
	unsigned int DAC0;
	unsigned int DAC1;	
	
	unsigned int gain;
	
}RICH_maroc_common_t;



typedef struct
{
	union
	{
		unsigned int val;
		struct
		{
			unsigned int cmd_fsu		: 1;
			unsigned int cmd_ss		: 1;
			unsigned int cmd_fsb		: 1;
			unsigned int swb_buf_250f	: 1;
			unsigned int swb_buf_500f	: 1;
			unsigned int swb_buf_1p		: 1;
			unsigned int swb_buf_2p		: 1;
			unsigned int ONOFF_ss		: 1;
			unsigned int sw_ss_300f		: 1;
			unsigned int sw_ss_600f		: 1;
			unsigned int sw_ss_1200f	: 1;
			unsigned int EN_ADC		: 1;
			unsigned int H1H2_choice	: 1;
			unsigned int sw_fsu_20f		: 1;
			unsigned int sw_fsu_40f		: 1;
			unsigned int sw_fsu_25k		: 1;
			unsigned int sw_fsu_50k		: 1;
			unsigned int sw_fsu_100k	: 1;
			unsigned int sw_fsb1_50k	: 1;
			unsigned int sw_fsb1_100k	: 1;
			unsigned int sw_fsb1_100f	: 1;
			unsigned int sw_fsb1_50f	: 1;
			unsigned int cmd_fsb_fsu	: 1;
			unsigned int valid_dc_fs	: 1;
			unsigned int sw_fsb2_50k	: 1;
			unsigned int sw_fsb2_100k	: 1;
			unsigned int sw_fsb2_100f	: 1;
			unsigned int sw_fsb2_50f	: 1;
			unsigned int valid_dc_fsb2	: 1;
			unsigned int ENb_tristate	: 1;
			unsigned int polar_discri	: 1;
			unsigned int inv_discriADC	: 1;
		} bits;
	} Global0;
	
	union
	{
		unsigned int val;
		struct
		{
			unsigned int d1_d2		: 1;
			unsigned int cmd_CK_mux		: 1;
			unsigned int ONOFF_otabg	: 1;
			unsigned int ONOFF_dac		: 1;
			unsigned int small_dac		: 1;
			unsigned int enb_outADC		: 1;
			unsigned int inv_startCmptGray	: 1;
			unsigned int ramp_8bit		: 1;
			unsigned int ramp_10bit		: 1;
			unsigned int Reserved0		: 23;
		} bits;
	} Global1;
	
	union
	{
		unsigned int val;
		struct
		{
			unsigned int DAC0		: 10;
			unsigned int Reserved0		: 6;
			unsigned int DAC1		: 10;
			unsigned int Reserved1		: 6;
		} bits;
	} DAC;
	
	unsigned int Reserved0;
	
	union
	{
		unsigned int val;
		struct
		{
			unsigned int Gain0		: 8;
			unsigned int Sum0		: 1;
			unsigned int CTest0		: 1;
			unsigned int MaskOr0		: 2;
			unsigned int Reserved0		: 4;
			unsigned int Gain1		: 8;
			unsigned int Sum1		: 1;
			unsigned int CTest1		: 1;
			unsigned int MaskOr1		: 2;
			unsigned int Reserved1		: 4;
		} bits;
	} CH[32];
	
} MAROC_Regs;

typedef struct
{
	unsigned int Ch0_31_Hold1;
	unsigned int Ch32_63_Hold1;
	unsigned int Ch0_31_Hold2;
	unsigned int Ch32_63_Hold2;
} MAROC_DyRegs;

typedef struct
{
	/* 0x0000-0x0003 */ unsigned int		Ctrl;
	/* 0x0004-0x0007 */ unsigned int		Reserved0[(0x0008-0x0004)/4];
	/* 0x0008-0x000B */ unsigned int		SpiCtrl;
	/* 0x000C-0x000F */ unsigned int		SpiStatus;
	/* 0x0010-0x00FF */ unsigned int		Reserved1[(0x0100-0x0010)/4];
} RICH_clk;

typedef struct
{
	/* 0x0000-0x0003 */ unsigned int		SerCtrl;
	/* 0x0004-0x0007 */ unsigned int		SerStatus;
	/* 0x0008-0x000F */ unsigned int		Reserved0[(0x0010-0x0008)/4];
	/* 0x0010-0x009F */ MAROC_Regs			Regs;
	/* 0x00A0-0x00AF */ MAROC_DyRegs		DyRegs_WrAll;
	/* 0x00B0-0x00DF */ MAROC_DyRegs		DyRegs_Rd[3];
	/* 0x00E0-0x00FF */ unsigned int		Reserved1[(0x0100-0x00E0)/4];
} RICH_MAROC_Cfg;



typedef struct
{
	/* 0x0000-0x000F */ unsigned int		DisableCh[4];
	/* 0x0010-0x0013 */ unsigned int		HitOrMask0;
	/* 0x0014-0x0017 */ unsigned int		HitOrMask1;
	/* 0x0018-0x00FF */ unsigned int		Reserved0[(0x0100-0x0018)/4];
	/* 0x0100-0x01FF */ unsigned int		Scalers[64];
} RICH_Maroc_Proc;



typedef struct
{
	/* 0x0000-0x0003 */ unsigned int		Lookback;
	/* 0x0004-0x0007 */ unsigned int		WindowWidth;
	/* 0x0008-0x000B */ unsigned int		BlockCfg;
	/* 0x000C-0x000F */ unsigned int		Reserved0[(0x0010-0x000C)/4];
	/* 0x0010-0x0013 */ unsigned int		DeviceID;
	/* 0x0014-0x0017 */ unsigned int		TrigDelay;
	/* 0x0018-0x0023 */ unsigned int		Reserved1[(0x0024-0x0018)/4];
	/* 0x0024-0x0027 */ unsigned int		FifoWordCnt;
	/* 0x0028-0x002B */ unsigned int		FifoEventCnt;
	/* 0x002C-0x007F */ unsigned int		Reserved2[(0x0080-0x002C)/4];
	/* 0x0080-0x0083 */ unsigned int		FifoData;
	/* 0x0084-0x00FF */ unsigned int		Reserved3[(0x0100-0x0084)/4];
} RICH_EvtBuilder;



typedef struct
{
	/* 0x0000-0x000B */ unsigned int		OutSrc[3];
	/* 0x000C-0x0037 */ unsigned int		Reserved0[(0x0038-0x000C)/4];
	/* 0x0038-0x003B */ unsigned int		CTestSrc;
	/* 0x003C-0x003F */ unsigned int		TrigSrc;
	/* 0x0040-0x0043 */ unsigned int		SyncSrc;
	/* 0x0044-0x005F */ unsigned int		Reserved1[(0x0060-0x0044)/4];
	/* 0x0060-0x0063 */ unsigned int		CTestDelay;
	/* 0x0044-0x007F */ unsigned int		Reserved2[(0x0080-0x0064)/4];
	/* 0x0080-0x0083 */ unsigned int		PulserPeriod;
	/* 0x0084-0x0087 */ unsigned int		PulserLowCycles;
	/* 0x0088-0x008B */ unsigned int		PulserNCycles;
	/* 0x008C-0x008F */ unsigned int		PulserStart;
	/* 0x0090-0x0093 */ unsigned int		PuslerStatus;
	/* 0x0094-0x00FF */ unsigned int		Reserved3[(0x0100-0x0094)/4];
	/* 0x0100-0x0103 */ unsigned int		ScalerLatch;
	/* 0x0104-0x0107 */ unsigned int		Reserved4[(0x0108-0x0104)/4];
	/* 0x0108-0x010B */ unsigned int		Scaler_GClk125;
	/* 0x010C-0x010F */ unsigned int		Scaler_Sync;
	/* 0x0110-0x0113 */ unsigned int		Scaler_Trig;
	/* 0x0114-0x011F */ unsigned int		Scaler_Input[3];
	/* 0x0120-0x012B */ unsigned int		Scaler_Output[3];
	/* 0x012C-0x0133 */ unsigned int		Scaler_Or1[2]; //chip[or]
	/* 0x0134-0x013B */ unsigned int		Scaler_Or2[2];
	/* 0x013C-0x0143 */ unsigned int		Scaler_Or3[2];
	/* 0x0144-0x0147 */ unsigned int		Scaler_Busy;
	/* 0x0148-0x014B */ unsigned int		Scaler_BusyCycles;
	/* 0x014C-0x01FF */ unsigned int		Reserved5[(0x0200-0x014C)/4];
} RICH_sd;


typedef struct
{
	/* 0x0000-0x00FF */ RICH_clk			Clk;
	/* 0x0100-0x01FF */ RICH_MAROC_Cfg		MAROC_Cfg;
	/* 0x0200-0x03FF */ RICH_sd			Sd;
	/* 0x0400-0x0FFF */ unsigned int		Reseverd0[(0x1000-0x0400)/4];
	/* 0x1000-0x15FF */ RICH_Maroc_Proc		MAROC_Proc[3];
	/* 0x1600-0x1FFF */ unsigned int		Reseverd1[(0x2000-0x1600)/4];
	/* 0x2000-0x20FF */ RICH_EvtBuilder		EvtBuilder;
} RICH_regs;
#endif

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include "richfpga_io.h"

#define DEBUG_PRINT_VALS	

extern RICH_regs *pRICH_regs;s

#define REG_TEST_LEN			1024
unsigned int PreviousRegArray[REG_TEST_LEN];
unsigned int CurrentRegArray[REG_TEST_LEN];

#define BRAM_TEST_LEN		65536
unsigned int PreviousBramArray[BRAM_TEST_LEN];
unsigned int CurrentBramArray[BRAM_TEST_LEN];

#define SPI_TEST_LEN			16*1024*1024
unsigned char PreviousSpiArray[SPI_TEST_LEN];
unsigned char CurrentSpiArray[SPI_TEST_LEN];



unsigned int RegError_Total;
unsigned int BramError_Total;
unsigned int SpiError_Total;
unsigned int HeartBeat_Total;
unsigned int Initialization_Total;
unsigned int Observation_Total;
unsigned int Correction_Total;
unsigned int Classifaction_Total;
unsigned int Injection_Total;
unsigned int Essential_Total;
unsigned int Uncorrectable_Total;

void rich_RadTest_ReadReg(unsigned int *buf)
{
	int i;
	
	printf("ReadReg() ");
	for(i = 0; i < REG_TEST_LEN; i++)
	{
		*buf++ = rich_read32(&pRICH_regs->Testing.RegData);
		if(!(i % 0xFF)) { printf("."); fflush(stdout); }
	}
	printf("\n");
	fflush(stdout);
}

void rich_RadTest_MonRd()
{
	int i;
	unsigned int val;
	
	for(i = 0; i < 100; i++)
	{
		val = rich_read32(&pRICH_regs->Testing.MonStatus);
		if(val & 0x2)
		{
			if(i) printf("\n");
			break;
		}
		val = rich_read32(&pRICH_regs->Testing.MonRd);
		if(!i) printf("Mon: ");
		printf("%c", val & 0xFF);
	}
}

void rich_RadTest_MonWr(char *s)
{
	int i = 0, len = strlen(s);
	unsigned int val;
	
	while(i < len)
	{
		val = rich_read32(&pRICH_regs->Testing.MonStatus);
		if(!(val & 0x1))
			rich_write32(&pRICH_regs->Testing.MonWr, s[i++]);
	}
}

void rich_RadTest_ReadBram(unsigned int *buf)
{
#define BRAM_BURST_SIZE	16
	int i, j;
	
	printf("ReadBram() ");
	
	rich_write32(&pRICH_regs->Testing.RamAddr, 0x80000000);	// reset bram address to 0x0000
	
	for(i = 0; i < BRAM_TEST_LEN/BRAM_BURST_SIZE; i++)
	{
		rich_read32_n(BRAM_BURST_SIZE, &pRICH_regs->Testing.RamRdData, buf);
		buf+=BRAM_BURST_SIZE;
		if(!(i % 0xFF)) { printf("."); fflush(stdout); }
	}
	printf("\n");
	fflush(stdout);
}

void rich_RadTest_ReadSpi(unsigned char *buf)
{
#define SPI_BURST_SIZE	64
	int i, j;
	unsigned char tx_buf[SPI_BURST_SIZE];
	unsigned char val;

	printf("ReadSpi() ");
	
	rich_SelectSpi(1);
	rich_TransferSpi(FLASH_CMD_RD);	/* continuous array read */
	rich_TransferSpi(0x0);				/* addr3 */
	rich_TransferSpi(0x0);				/* addr2 */
	rich_TransferSpi(0x0);				/* addr1 */
	rich_TransferSpi(0x0);				/* addr0 */
	
	memset(tx_buf, 0xFF, sizeof(tx_buf));
	for(i = 0; i < SPI_TEST_LEN/SPI_BURST_SIZE; i++)
	{
		rich_TransferSpi_n(SPI_BURST_SIZE, tx_buf, buf);
#if DEBUG_PRINT_VALS
		printf("\n0x%08X: ", i*SPI_BURST_SIZE);
		for(j = 0; j < SPI_BURST_SIZE; j++)
			printf("%02X", buf[j]);
#else
		if(!(i % 0xFF)) { printf("."); fflush(stdout); }
#endif
		buf+=SPI_BURST_SIZE;
	}
	rich_SelectSpi(0);
	printf("\n");
	fflush(stdout);
}

void rich_RadTest_InjectRegError()
{
	rich_write32(&pRICH_regs->Testing.RegCtrl, 0x1);	// flip some registers from default state
}

void rich_RadTest_InjectBramError()
{
	unsigned int val;
	
	rich_write32(&pRICH_regs->Testing.RamAddr, 0x80000000);	// reset bram address to 0x0000
	val = rich_read32(&pRICH_regs->Testing.RamRdData);
	rich_write32(&pRICH_regs->Testing.RamAddr, 0x80000000);	// reset bram address to 0x0000
	rich_write32(&pRICH_regs->Testing.RamWrData, val ^ 0x1);
}

void rich_RadTest_InjectCfgError()
{
	rich_RadTest_MonWr("I");
	sleep(1);
	rich_RadTest_MonRd();
	sleep(1);
	
	rich_write32(&pRICH_regs->Testing.ErrAddrL, 0x0);
	rich_write32(&pRICH_regs->Testing.ErrAddrH, 0xC0);
	rich_write32(&pRICH_regs->Testing.ErrCtrl, 0x2);
	
	sleep(1);
	rich_RadTest_MonWr("O");
	sleep(1);
	rich_RadTest_MonRd();
	sleep(1);
}

void rich_RadTest_XAdxRd()
{
	unsigned int val[32];
	int addr;
	double v;
	
	for(addr = 0; addr < 0x20; addr++)
	{
		rich_write32(&pRICH_regs->Testing.XAdcCtrl, 0x01000000 | (addr<<16));
		val[addr] = rich_read32(&pRICH_regs->Testing.XAdcStatus);
	}
	printf("FPGA TEMP      = %08X, %.3fC\n", val[0], (double)(val[0] & 0xFFFF) * 503.975 / 65536 - 273.15);
	printf("+1.0V VCCINT   = %08X, %.3fV\n", val[1], (double)(val[1] & 0xFFFF) * 3.0 / 65536.0);
	printf("+1.8V VCCAUX   = %08X, %.3fV\n", val[2], (double)(val[2] & 0xFFFF) * 3.0 / 65536.0);
	
	v = (double)(val[16] & 0xFFFF) * 1.0 / 65536.0;
	v = v + (v / 237.0) * 1240.0;
	printf("+5V            = %08X, %.3fV\n", val[16], v);
	
	v = (double)(val[24] & 0xFFFF) * 1.0 / 65536.0;
	v = v + (v / 249.0) * 825.0;
	printf("+3.3V          = %08X, %.3fV\n", val[24], v);
	
	v = (double)(val[17] & 0xFFFF) * 1.0 / 65536.0;
	v = v + (v / 1000.0) * 249.0;
	printf("+1.0V MGT      = %08X, %.3fV\n", val[17], v);
	
	v = (double)(val[22] & 0xFFFF) * 1.0 / 65536.0;
	v = v + (v / 604.0) * 301.0;
	printf("+1.2V MGT      = %08X, %.3fV\n", val[22], v);
	
	v = (double)(val[18] & 0xFFFF) * 1.0 / 65536.0;
	v = 27.0 + (0.598 - v) / 0.002;
	printf("LTM TEMP1      = %08X, %.3fC\n", val[18], v);
	
	v = (double)(val[26] & 0xFFFF) * 1.0 / 65536.0;
	v = 27.0 + (0.598 - v) / 0.002;
	printf("LTM TEMP2      = %08X, %.3fC\n", val[26], v);
}

void rich_RadTest_Init()
{
	RegError_Total				= 0;
	BramError_Total				= 0;
	SpiError_Total				= 0;
	HeartBeat_Total				= 0;
	Initialization_Total	= 0;
	Observation_Total			= 0;
	Correction_Total			= 0;
	Classifaction_Total		= 0;
	Injection_Total				= 0;
	Essential_Total				= 0;
	Uncorrectable_Total		= 0;
	
	// dummy spi transfer
	rich_SelectSpi(0);
	rich_TransferSpi(0x0);
	
	rich_write32(&pRICH_regs->Testing.RegCtrl, 0x2);	// initialize register array

	rich_RadTest_ReadReg(PreviousRegArray);
	rich_RadTest_ReadBram(PreviousBramArray);
	rich_RadTest_ReadSpi(PreviousSpiArray);
}


int count_setbits(unsigned int val)
{
	int i, cnt = 0;
	
	for(i = 0; i < 32; i++)
		if(val & (1<<i)) cnt++;
		
	return cnt;
}

void rich_RadTest_UpdateCounters()
{
	int i;
	unsigned int diff;
	time_t curtime;
	char *c_time_string;
	
	rich_RadTest_ReadReg(CurrentRegArray);
	rich_RadTest_ReadBram(CurrentBramArray);
	rich_RadTest_ReadSpi(CurrentSpiArray);
	
	// Register difference check
	for(i = 0; i < REG_TEST_LEN; i++)
	{
		diff = PreviousRegArray[i] ^ CurrentRegArray[i];
		RegError_Total+= count_setbits(diff);
	}
	memcpy(PreviousRegArray, CurrentRegArray, sizeof(CurrentRegArray));
	
	// Bram difference check
	for(i = 0; i < BRAM_TEST_LEN; i++)
	{
		diff = PreviousBramArray[i] ^ CurrentBramArray[i];
		BramError_Total+= count_setbits(diff);
	}
	memcpy(PreviousBramArray, CurrentBramArray, sizeof(CurrentBramArray));
	
	// Spi difference check
	for(i = 0; i < SPI_TEST_LEN; i++)
	{
		diff = (unsigned int)(PreviousSpiArray[i] ^ CurrentSpiArray[i]);
		SpiError_Total+= count_setbits(diff);
	}
	memcpy(PreviousSpiArray, CurrentSpiArray, sizeof(CurrentSpiArray));

	// FPGA config check
	HeartBeat_Total				= rich_read32(&pRICH_regs->Testing.HeartBeatCnt);
	Initialization_Total	= rich_read32(&pRICH_regs->Testing.InitializationCnt);
	Observation_Total			= rich_read32(&pRICH_regs->Testing.ObservationCnt);
	Correction_Total			= rich_read32(&pRICH_regs->Testing.CorrectionCnt);
	Classifaction_Total		= rich_read32(&pRICH_regs->Testing.ClassifactionCnt);
	Injection_Total				= rich_read32(&pRICH_regs->Testing.InjectionCnt);
	Essential_Total				= rich_read32(&pRICH_regs->Testing.EssentialCnt);
	Uncorrectable_Total		= rich_read32(&pRICH_regs->Testing.UncorrectableCnt);
	
	curtime = time(NULL);
	c_time_string = ctime(&curtime);
	
	printf("\n\n*************************************************************\n");
	printf("RICH FPGA RadTest Results @ %s", c_time_string);
	printf("*************************************************************\n");
	
	rich_RadTest_MonRd();
	rich_RadTest_XAdxRd();

	printf("RegError_Total = %u\n", RegError_Total);
	printf("BramError_Total = %u\n", BramError_Total);
	printf("SpiError_Total = %u\n", SpiError_Total);
	printf("HeartBeat_Total = %u\n", HeartBeat_Total);
	printf("Initialization_Total = %u\n", Initialization_Total);
	printf("Observation_Total = %u\n", Observation_Total);
	printf("Correction_Total = %u\n", Correction_Total);
	printf("Classifaction_Total = %u\n", Classifaction_Total);
	printf("Injection_Total = %u\n", Injection_Total);
	printf("Essential_Total = %u\n", Essential_Total);
	printf("Uncorrectable_Total = %u\n", Uncorrectable_Total);
	
//	rich_RadTest_InjectRegError();
//	rich_RadTest_InjectBramError();
//	rich_RadTest_InjectCfgError();
}

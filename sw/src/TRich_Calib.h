/*
 *  TRich_Calib.h
 *	header file for TRich_Calib class
 *
 *	DESCRIPTION:
 * 
 *	A class for managing the calibration data for one ASIC board
 *	Single NCHnel TDC Efficiency, delay and jitter are collected in a TTree  
 *	together with the MAROC slow control parameters Threshold (DAC0) and gain (preamp gain)
 *
 *  The class is intented to be an interface with TTree and related TFile
 *	both when write and for reading the TTree
 *
 *
 * TO DO:
 * Some check can be added to verify that maroc parameters are in the correct range (this is true alsp for other classes of the library..)
 * 
 *
 *	Matteo Turisini matteo.turisini@gmail.com
 *	INFN - Ferrara
 *
 */

#ifndef TRich_Calib_H
#define TRich_Calib_H

#include <TFile.h>
#include <TTree.h>

#define NASIC  (3)
#define NCH 	(NASIC*64)

#define CALTREENAME "CalibData" // name of the TTREE move everything in the general header


class TRich_Calib {	
	
private:

	int frunID;
	// number of trigggers (used as a normalization factor for efficiency calcualtion)	
	int fNtrig;

	/* MAROC PARAMETERS */
	int fPolarDiscri;
	int fThreshold[NASIC]; // one threshold per chip
	int fGain[NCH] ; // one gain per NCHnel

	/* 	TDC spectra (Rise and Falling Edges)*/	
	
	// Entries (Efficiency)
	double fNRise[NCH]; 
	double fNFall[NCH];

	// Mean value (Delay)
	double fTRise[NCH]; 
	double fTFall[NCH];
	
	// RMS (Jitter)
	double fJRise[NCH]; 	
	double fJFall[NCH];	

	/* Time over Threshold */
	double fNDura[NCH];
	double fTDura[NCH];
	double fJDura[NCH]; 


	/*  ROOT classes interfaces */
	TFile *	fFile;
	TTree *	fTree;
	
public:
								TRich_Calib(const char * filename=NULL);
								~TRich_Calib();
	void	Print();
	void 	Reset(); // Reset all variables associated with the TTree
	void  Fill(); // Fill a new entry in the provate TTree
	void  Write(); // Write the tree on disk (fFile)
	
	// Setters/Getters 
	// Example with Gain:
	// Gain (val, channel) SETTER MODE 0 Assign the value val to the specific channel, returns the assigned value
	// Gain (val,-1) 			 SETTER MODE 1 Assign the same value val to all channel, returns the assigned value
	// Gain (-1,channel) 	 GETTER        Returns the gain for that channel,

	int 	Gain(int val =-1, int chann = -1); 
	int 	Threshold(int val=-1,int  asic=-1); 
	int 	NTrigger(int val = -1);
	int 	RunID(int val = -1);


	// The first argument "what" is to distinguish between different edges type and time over threshold distributions
	// 0 = RISE EDGE
	// 1 = FALLING EDGE
	// 2 = DURATION
	// second and third argument are to access specific channel and to set/get the its value
	// the mode of operation are identical to the ones for gain and threshold (see above)

	// EXAMPLE:
	// Entries(0,-1,ch); Read the entries of the Rising edge spectrum for channel ch
	// RMS(1,-1,ch); Read the jitter of the Falling edge spectrum for channel ch
 
	double Entries(int what = -1, double val = -1.0, int ch = -1);
	double Mean		(int what = -1, double val = -1.0, int ch = -1);
	double RMS		(int what = -1, double val = -1.0, int ch = -1);


	// READING PART

	bool OpenFile(const char * filename =	NULL); // Open the file containing SCAN data in read mode return true if the file is correctly opened
	int GetTree(); // Retrieve the Tree of data and associates variables for reading, returns the number of entries
	bool GetEntry(int n); // Read an entry n


private:
	bool ValidAsic(int i);
	bool ValidChannel(int i);
};

#endif

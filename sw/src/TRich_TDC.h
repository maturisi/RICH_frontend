/*
 *  TRich_TDC.h
 *	header file for TRich_TDC class
 *
 *	DESCRIPTION:
 * 
 *	A class for ingesting TDC Event data 
 *	currently it manages the header and the hit list of one tile (192 channels)
 *  it is capable of decoding the timestamp
 *  and can record the time difference between consecutive triggers
 *  provides an output interface to ROOT structures 
 *
 *	Latest revision 2015, June 25
 *
 *	Matteo Turisini matteo.turisini@gmail.com
 *	INFN - Ferrara
 *
 */

#ifndef TRICH_TDC_H
#define TRICH_TDC_H

#include <vector>

#include <TFile.h>
#include <TTree.h>

#include "datatype.h"

#define MAX_EDGE 10000 /* per tile max number of edges for 1 tile*/

class TRich_TDC {	
	
private:
	
	/* fw version of May '15*/
	/* Device ID	*/unsigned int	fdid;
	/* Event ID		*/unsigned int	ftid;
	/* TimeStamp0	*/unsigned int	fts0; 
	/* TimeStamp1	*/unsigned int	fts1; 
	/* Edges List	*/std::vector<RICH_tdc_edge_t> fEdgeList; 

	int ftotEvents;
	
	/* analysis during parsing (for event tagging)*/	
	int			fposc; // positive edges counter
	int			fnegc; // negative edges counter
	
	double			fts_sec; // time interval between consecutive triggers
	
	/*  Root class handler */
	TFile *			ffile;
	TTree *			ftree;
	
	/* variables associated to the TTree */
	UInt_t			fnedge;
	UInt_t			fpolarity[MAX_EDGE]; // 0 rising; 1 falling
	UInt_t			fchannel[MAX_EDGE]; //[0..192 ]
	UInt_t			ftime[MAX_EDGE]; // delay from trigger (range depends on configuration)
		



public:
	TRich_TDC(const char * outname = NULL);
	~TRich_TDC();

	void			Reset();
	unsigned int	TrigNum	(int tid=-1);
	unsigned int	DevId	(int did=-1);
	unsigned int	Tstamp	(char tag,int time=-1);	

	void			Edge	(unsigned int chan = 0,unsigned int time = 0, unsigned int edge =0);

	int				Process();
	
private:
	double			DecodeTimeStamp();/* encode the 2 x 32 bit words (private members fts1,fts0) in a single timestamp in seconds*/
};

#endif

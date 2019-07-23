#ifndef __PMS1_h__
#define __PMS1_h__

int PMS1(int motifLen, int hammingDist, const CInputStringSet * inputStrs,
		 CCompactMotif * foundMotifs, int maxNumMotifsAllowed, 
		 int paramK, int sizeQTheshold, unsigned int maxQSize);

int PMS1Enhance(int motifLen, int hammingDist, const CInputStringSet * inputStrs,
		 CCompactMotif * foundMotifs, int maxNumMotifsAllowed, 
		 int paramK, int sizeQTheshold, unsigned int maxQSize);


int PMS1Enhance1(int motifLen, int hammingDist, const CInputStringSet * inputStrs,
		 CCompactMotif * foundMotifs, int maxNumMotifsAllowed, 
		 int paramK, int sizeQTheshold, unsigned int maxQSize);
#endif
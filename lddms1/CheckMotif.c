#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "SystemDefsAndFuncs.h"
#include "InputDefs.h"
#include "DNACommonFuncs.h"
#include "BitFuncs.h"
#include "UtilityFuncs.h"


//global input parameters
int motifLen = 11;
int hammingDist = 3;
int rateInDb = 5;
CInputStringSet inputStrs;
CCompactMotif foundMotifs[CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED];
CCompactMotif correctMotifs[CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED];
int numFoundMotifs = 0;
int numCorrectMotifs = 0;

int VerifyMotifs ( int moLen, int hmDist, int rate, const CInputStringSet * inptStrs,
				  const CCompactMotif * foMotifs, const CCompactMotif * correctMotifs, int nFoMotifs) {
	int i;
	int nCorrectMotifs = 0;
	int minMatches = ceil(rate*CONST_MAX_NUM_STRINGS/100);
	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH + 4];
	//printf("%s","go to the loop");
	for (i = 0; i < nFoMotifs; i++) {
		DecodeDNAString((char *)&foMotifs[i], (moLen + 3)/4, aMotif);
		fprintf(stdout, "\nTest: ");
		PrintDNAString(aMotif, moLen, 1, 0, stdout);
		//printf("%c", IsMotifInputStrSet(aMotif, moLen, hmDist, minMatches, inptStrs, 0, inptStrs->m_num - 1));
		if (IsMotifInputStrSet2(aMotif, moLen, hmDist, minMatches, inptStrs, 0, inptStrs->m_num - 1)){
		if (nCorrectMotifs < CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED) {
					memcpy(correctMotifs + nCorrectMotifs, foMotifs + i, sizeof(CCompactMotif));
					nCorrectMotifs++;
				}		
		} else {

				}
	}
	return nCorrectMotifs;
}


void PrintMotifsToFile(const char * fName, int moLen, CCompactMotif * foMotifs, int nMotifs) {
	int i;
	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH];
	FILE * f;
	f = fopen(fName, "w+");
	if (f == NULL) {
		fprintf(stdout, "\nUnable to create output file %s", fName);
		return;
	}
	//fprintf(stdout, "#Motifs=%d\n", nMotifs);
	for (i = 0; i < nMotifs; i++) {
		DecodeDNAString(foMotifs + i, moLen/4 + 1, aMotif);		
		PrintDNAString(aMotif, moLen, 1, 0, f);
		fprintf(f, "\n");
	}
	//fprintf(stdout, "#Motifs=%d", nMotifs);
	fclose(f);	
}

int checkMotif(char *inputFile, char *candidateFile, char *outputFile, int l, int d2) {

	double t1, t2;
	int i;
	char * fName; 


	// if (argc < 6) {
	// 	fprintf(stdout, "\nWrong parameters.\nCommand line: \ncheckMotif [input strFile] [input candidateFile] [output file] [motif length] [d1] [rateInDb]");
	// 	return;
	// }
	
	ComputeDNAByteTable();
    ComputeDNAByteTableInverse();

	motifLen = l;
	hammingDist = d2;
	rateInDb = 5;
	
	ReadInputFile(inputFile, &inputStrs);
	fName = candidateFile;
	fprintf(stdout, "\n\nInput Strings: %d strings l=%d d=%d", inputStrs.m_num, motifLen, hammingDist);
	// for (i = 0; i < inputStrs.m_num; i++) {	
	// 	fprintf(stdout, "\n\n%d\n", i + 1);
	// 	PrintInputString(&inputStrs.m_str[i], 0, stdout);
	// }
    
	numFoundMotifs = ReadOutputFile(fName, foundMotifs, CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED);
	numCorrectMotifs = VerifyMotifs(l, d2, rateInDb, &inputStrs, foundMotifs, correctMotifs, numFoundMotifs);

	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH];
	fprintf(stdout, "\n");
	for (i = 0; i < numCorrectMotifs; i++) {
		DecodeDNAString((char *)&correctMotifs[i], motifLen/4 + 1, aMotif);
		fprintf(stdout, "\n");
		PrintDNAString(aMotif, motifLen, 1, 0, stdout);
		}	

	PrintMotifsToFile(outputFile, motifLen, correctMotifs, numCorrectMotifs);

	fprintf(stdout, "\n\n#Motifs found: %d", numFoundMotifs);
	fprintf(stdout, "\nCorrect Motifs=%d\n", numCorrectMotifs);
	return numCorrectMotifs;
}





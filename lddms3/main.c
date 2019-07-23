#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "InputDefs.h"
#include "DNACommonFuncs.h"
#include "BitFuncs.h"
#include "SortFuncs.h"
#include "UtilityFuncs.h"
#include "PMSPrune.h"


//global input parameters
int motifLen = 11;
int hammingDist = 3;
int d2 = 0;
CInputStringSet inputStrs;
CCompactMotif foundMotifs[CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED];
int numFoundMotifs = 0;

int VerifyMotifs( int moLen, int hmDist, const CInputStringSet * inptStrs,
				  const CCompactMotif * foMotifs, int nFoMotifs) {
	int i;
	int nCorrectMotifs = 0;
	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH + 4];
	for (i = 0; i < nFoMotifs; i++) {
		DecodeDNAString((char *)&foMotifs[i], moLen/4 + 1, aMotif);
		//fprintf(stdout, "\nTest: ");
		//PrintDNAString(aMotif, moLen, 1, 0, stdout);
		if (IsMotifInputStrSet(aMotif, moLen, hmDist, inptStrs, 0, inptStrs->m_num - 1)) {
		//if (IsMotifInputStrSetWithBindingSite(aMotif, moLen, hmDist, inptStrs, 0, inptStrs->m_num - 1)) {
			nCorrectMotifs++;
		} else {

		}
	}
	return nCorrectMotifs;
}


void PrintMotitsToFile(const char * fName, int moLen, CCompactMotif * foMotifs, int nMotifs) {
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
	fprintf(stdout, "\n#Motifs = %d", nMotifs);
	fclose(f);	
}

void main(int argc, char *argv[]) {

	//int t1, t2;
	clock_t start, finish; 
	start = clock();  
    double  duration;
	int i, j;	
	char iFileName[500];
	char oFileName[500];

	ComputeDNAByteTable();
	ComputeDNAByteTableInverse();


	motifLen = 13;
	hammingDist = 4;
	sprintf(iFileName, "TestingInput\\input_l%d_d%d_0.txt", motifLen, hammingDist);
	sprintf(oFileName, "output_l%d_d%d_0.txt", motifLen, hammingDist);
		
	if (argc < 6) {
		fprintf(stdout, "\nWrong parameters.\nCommand line: \nPMSPrune [input file] [output file] [motif length] [d1] [d2] \n");
		return;
	} else {
		strcpy(iFileName, argv[1]);
		strcpy(oFileName, argv[2]);
		motifLen = atoi(argv[3]);
		hammingDist = atoi(argv[4]);
		d2 = atoi(argv[5]);	
	}

	ReadInputFile(iFileName, &inputStrs);

	fprintf(stdout, "\n\nInput Strings: %d strings l=%d d1=%d d2=%d", inputStrs.m_num, motifLen, hammingDist, d2);
	// for (i = 0; i < inputStrs.m_num; i++) {	
	// 	fprintf(stdout, "\n\n%d\n", i + 1);
	// 	PrintInputString(&inputStrs.m_str[i], 0, stdout);
	// }

	//t1 = time(NULL);
	
	numFoundMotifs = PMSPrune(motifLen, hammingDist, d2, &inputStrs, foundMotifs, CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED);
	//t2 = time(NULL);
	
	PrintMotitsToFile(oFileName, motifLen, foundMotifs, numFoundMotifs);
	finish = clock();  
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    fprintf(stdout, "\nTime = %.2f seconds\n", duration);
	return;
}





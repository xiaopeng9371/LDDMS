#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "SystemDefsAndFuncs.h"
#include "InputDefs.h"
#include "DNACommonFuncs.h"
#include "BitFuncs.h"
#include "SortFuncs.h"
#include "HashStrFuncs.h"
#include "UtilityFuncs.h"
#include "PMS1.h"


//global input parameters
int motifLen = 11;
int hammingDist = 3;
int rateInDb = 100;
CInputStringSet inputStrs;
CCompactMotif foundMotifs[CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED];
CCompactMotif correctMotifs[CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED];
int numFoundMotifs = 0;
int numCorrectMotifs = 0;

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

int VerifyMotifsWithRate( int moLen, int hmDist, int rate, const CInputStringSet * inptStrs,
				  const CCompactMotif * foMotifs, const CCompactMotif * correctMotifs, int nFoMotifs) {
	int i;
	int nCorrectMotifs = 0;
	int minMatches = ceil(rate*CONST_MAX_NUM_STRINGS/100);
	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH + 4];
	//printf("%s","go to the loop");
	for (i = 0; i < nFoMotifs; i++) {
		DecodeDNAString((char *)&foMotifs[i], (moLen + 3)/4, aMotif);
		//fprintf(stdout, "\nTest: ");
		//PrintDNAString(aMotif, moLen, 1, 0, stdout);
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

void checkMotif(char *inputFile, char *candidateFile, char *outputFile, int l, int d1) {

	double t1, t2;
	int i;
	char * fName; 


	// if (argc < 6) {
	// 	fprintf(stdout, "\nWrong parameters.\nCommand line: \ncheckMotif [input strFile] [input candidateFile] [output file] [motif length] [d1] [rateInDb]");
	// 	return;
	// }
	
	ComputeDNAByteTable();

	motifLen = l;
	hammingDist = d1;
	rateInDb = 100;
	
	ReadInputFile(inputFile, &inputStrs);
	fName = candidateFile;
	//fprintf(stdout, "\n\nInput Strings: %d strings l=%d d=%d", inputStrs.m_num, motifLen, hammingDist);
	// for (i = 0; i < inputStrs.m_num; i++) {	
	// 	fprintf(stdout, "\n\n%d\n", i + 1);
	// 	PrintInputString(&inputStrs.m_str[i], 0, stdout);
	// }

	numFoundMotifs = ReadOutputFile(fName, foundMotifs, CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED);
	numCorrectMotifs = VerifyMotifsWithRate(motifLen, hammingDist, rateInDb, &inputStrs, foundMotifs, correctMotifs, numFoundMotifs);

	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH];
	fprintf(stdout, "\n");
	// for (i = 0; i < numCorrectMotifs; i++) {
	// 	DecodeDNAString((char *)&correctMotifs[i], motifLen/4 + 1, aMotif);
	// 	fprintf(stdout, "\n");
	// 	PrintDNAString(aMotif, motifLen, 1, 0, stdout);
	// 	}	

	PrintMotifsToFile(outputFile, motifLen, correctMotifs, numCorrectMotifs);

	fprintf(stdout, "\n\n#Motifs found: %d", numFoundMotifs);
	fprintf(stdout, "\nCorrect Motifs=%d\n", numCorrectMotifs);
	
	return;
}

void main(int argc, char *argv[]) {

	clock_t beginTotal = clock();
	int t1, t2;
	int i, j;	
	char iFileName[1000];
	
	if (argc < 7) {
		fprintf(stdout, "Wrong parameters.\nCommand line: \nLDD2 [input file] [candidate file] [output file] [motif length] [d1] [d2] \n\n");
		return;
	}
	
	ComputeDNAByteTable();
	ComputeDNAByteTableInverse();

	motifLen = atoi(argv[4]);
	hammingDist = atoi(argv[6]);
	ReadInputFile(argv[1], &inputStrs);

	//fprintf(stdout, "\n\nInput Strings: %d strings l=%d d1=%d", inputStrs.m_num, motifLen, hammingDist);
	// for (i = 0; i < inputStrs.m_num; i++) {	
	// 	fprintf(stdout, "\n\n%d\n", i + 1);
	// 	PrintInputString(&inputStrs.m_str[i], 0, stdout);
	// }

	

	clock_t begin = clock();	
	numFoundMotifs = PMS1(motifLen, hammingDist, &inputStrs, foundMotifs, CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED, inputStrs.m_num - 1, 5000000, 40000000 * 1000 * 1000);
	clock_t end = clock();
	double time1= (double)(end - begin) / CLOCKS_PER_SEC;
	fprintf(stdout, "\nTime1 = %.2f second(s) \n", t1);
	VerifyMotifs(motifLen, hammingDist, &inputStrs, foundMotifs, numFoundMotifs);
	//fprintf(stdout, "\n#Correct Motifs=%d\n", VerifyMotifs(motifLen, hammingDist, &inputStrs, foundMotifs, numFoundMotifs));
	
	PrintMotifsToFile(argv[2], motifLen, foundMotifs, numFoundMotifs);

	hammingDist = atoi(argv[5]);

	begin = clock();
	checkMotif(argv[1], argv[2], argv[3], motifLen, hammingDist);
	end = clock();
	double time2= (double)(end - begin) / CLOCKS_PER_SEC;
	fprintf(stdout, "\nTime2 = %.2f seconds\n", time2);

	clock_t endTotal = clock();
	double timeTotal= (double)(endTotal - beginTotal) / CLOCKS_PER_SEC;
	fprintf(stdout, "\nTotal time = %.2f second(s) \n", timeTotal);
	return;
}





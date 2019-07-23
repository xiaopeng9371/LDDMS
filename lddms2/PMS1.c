#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "InputDefs.h"
#include "DNACommonFuncs.h"
#include "BitFuncs.h"
#include "HashStrFuncs.h"
#include "SortFuncs.h"
#include "UtilityFuncs.h"
#include "PMS1.h"


unsigned int PMS1GenDNeigh( int mLen, int hammingDist, 
							const char * rootStr, char * currentStr, 
							int pos, int depth, 
							int comMoLen, char * curDNeighs, char * maxDNeighs) {


	int i, j;
	char * bDNeighs = curDNeighs;

	if (curDNeighs < maxDNeighs) {
		EncodeDNAString(currentStr, mLen, curDNeighs);
		curDNeighs += comMoLen;
	} else {
		return 0;
	}
	//explore children
	if (depth < hammingDist) {
		for (i = pos + 1; i < mLen; i++) {		
			for (j = 0; j < 4; j++) {
				if ( !( (char)j == rootStr[i]) ) {
					currentStr[i] = j;					
					curDNeighs += comMoLen * PMS1GenDNeigh( mLen, hammingDist, 
															rootStr, currentStr, 
															i, depth + 1, 
															comMoLen, curDNeighs, maxDNeighs);
				}
			}
			currentStr[i] = rootStr[i];			
		}
	}
	return (curDNeighs - bDNeighs)/comMoLen;
}

void PMS1GenDNeighChecking( int mLen, int hammingDist, const char * rootStr, 
						    char * currentStr, char * encodedCurStr,
							int pos, int depth, CHashStrV2 * hashStr) {


	int i;
	char j;

	EncodeDNAString(currentStr, mLen, encodedCurStr);
	HashStrV2_FindAndMarkString(hashStr, encodedCurStr);

	//explore children
	if (depth < hammingDist) {
		for (i = pos + 1; i < mLen; i++) {		
			for (j = 0; j < 4; j++) {
				if ( !( (char)j == rootStr[i]) ) {
					currentStr[i] = j;					
					PMS1GenDNeighChecking(mLen, hammingDist, rootStr, currentStr, encodedCurStr, 
										  i, depth + 1, hashStr);
				}
			}
			currentStr[i] = rootStr[i];			
		}
	}
}

int PMS1(int motifLen, int hammingDist, const CInputStringSet * inputStrs,
		 CCompactMotif * foundMotifs, int maxNumMotifsAllowed, 
		 int paramK, int sizeQTheshold, unsigned int maxQSize) {

	int t1, t2;
	int i, k;
	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH];
	char * rootStr;
	char currentStr[CONST_MAX_MOTIF_STRING_LENGTH];
	int numFoundMotifs = 0;
	
	char * Q, * Q0, * Q1, *ptr;
	unsigned int sizeQ0, sizeQ1, sizeQ;
	int comMoLen = motifLen/4 + 1;
	if (motifLen % 4 == 0) {
		comMoLen = motifLen/4;
	}
	Q0 = malloc(maxQSize * comMoLen);
	Q1 = malloc(maxQSize * comMoLen);
	Q = malloc(maxQSize * comMoLen * (paramK + 1));
	if (Q0 == NULL || Q1 == NULL || Q == NULL) {
		if (Q0 != NULL) {
			free(Q0);
		}
		if (Q1 != NULL) {
			free(Q1);
		}
		if (Q != NULL) {
			free(Q);
		}
		fprintf(stdout, "\nUnable to allocate memory for Q0, Q1 and Q");
		return 0;
	}

	t1 = time(NULL);
	//fprintf(stdout, "\nGenerate Q0: ");	

	sizeQ0 = 0;
	for (i = 0; i < inputStrs->m_str[0].m_length - motifLen + 1; i++) {
		rootStr = inputStrs->m_str[0].m_data + i;
		memcpy(currentStr, rootStr, motifLen);
		sizeQ0 += PMS1GenDNeigh( motifLen, hammingDist, rootStr, currentStr, -1, 0, 
								comMoLen, Q0 + sizeQ0 * comMoLen, Q0 + maxQSize * comMoLen );
		sizeQ = sizeQ0;
		// if (sizeQ0 >= maxQSize) {
		// 	sizeQ0 = maxQSize;
		// 	fprintf(stdout, "\nSize of Q0 = %d during", sizeQ0);
		// 	//RadixSort(Q0, sizeQ0, comMoLen);
		// 	//sizeQ0 = RemoveSortedDuplicate(Q0, sizeQ0, comMoLen);
		// 	sizeQ0 += PMS1GenDNeigh( motifLen, hammingDist, rootStr, currentStr, -1, 0, 
		// 						comMoLen, Q0 + sizeQ0 * comMoLen, Q0 + maxQSize * comMoLen );
		// 	sizeQ = sizeQ0;
		// 	if (sizeQ0 >= maxQSize) {
		// 		break;
		// 	}			
		// }
	}
	// fprintf(stdout, "\nSize of Q0 = %d before", sizeQ0);
	// RadixSort(Q0, sizeQ0, comMoLen);
	// sizeQ0 = RemoveSortedDuplicate(Q0, sizeQ0, comMoLen);
	// fprintf(stdout, "\nSize of Q0 = %d after", sizeQ0);
	memcpy(Q, Q0, sizeQ0 * comMoLen);
	//Qaddress = sizeQ0; 
	for (k = 1; k <= paramK; k++) {
		sizeQ1 = 0;
		for (i = 0; i < inputStrs->m_str[k].m_length - motifLen + 1; i++) {
			rootStr = inputStrs->m_str[k].m_data + i;
			memcpy(currentStr, rootStr, motifLen);
			sizeQ1 += PMS1GenDNeigh( motifLen, hammingDist, rootStr, currentStr, -1, 0, 
									 comMoLen, Q1 + sizeQ1 * comMoLen, Q1 + maxQSize * comMoLen );
		}		
		memcpy(Q + sizeQ * comMoLen, Q1, sizeQ1 * comMoLen);
		sizeQ += sizeQ1;
		//Qaddress += sizeQ1;
		// fprintf(stdout, "\nSize of Q%d = %d before", k, sizeQ1);
		// RadixSort(Q1, sizeQ1, comMoLen);
		// sizeQ1 = RemoveSortedDuplicate(Q1, sizeQ1, comMoLen);
		// fprintf(stdout, "\nSize of Q%d = %d after", k, sizeQ1);		
		// if (sizeQ0 < sizeQTheshold ) {
		// 	break;
		// }
	}
	//fprintf(stdout, "\nSize of Q = %d before", sizeQ);

	RadixSort(Q, sizeQ, comMoLen);
	sizeQ = RemoveSortedDuplicate(Q, sizeQ, comMoLen);
	t2 = time(NULL);
	//fprintf(stdout, "\nT=%ds", t2 - t1);
	//fprintf(stdout, "\nSize of Q = %d after", sizeQ);
	//system("PAUSE");

	//RadixSort(Q0, sizeQ0, comMoLen);
	ptr = Q;
	for (i = 0; i < sizeQ; i++, ptr += comMoLen) {
		//DecodeDNAString(ptr, comMoLen, aMotif);
		//if (IsMotifInputStrSet(aMotif, motifLen, hammingDist, inputStrs, k, inputStrs->m_num - 1)) {
		//	fprintf(stdout, "\n");
		//	PrintDNAString(aMotif, motifLen, 1, 0, stdout);
			if (numFoundMotifs < maxNumMotifsAllowed) {
				memcpy(&foundMotifs[numFoundMotifs], ptr, comMoLen);
				numFoundMotifs++;
			}
		//}
	}

	//fprintf(stdout, "\n#Motifs=%d \n", numFoundMotifs);

	free(Q0);
	free(Q1);
	free(Q);

	return numFoundMotifs;
}


int PMS1Enhance(int motifLen, int hammingDist, const CInputStringSet * inputStrs,
		 CCompactMotif * foundMotifs, int maxNumMotifsAllowed, 
		 int paramK, int sizeQTheshold, unsigned int maxQSize) {

	int t1, t2;
	int i, j, k;
	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH];
	char * rootStr;
	char currentStr[CONST_MAX_MOTIF_STRING_LENGTH];
	char encondedStr[CONST_MAX_MOTIF_STRING_LENGTH];
	int numFoundMotifs = 0;
	
	char * ptr;
	int comMoLen = motifLen/4 + 1;
	int hashKeyLen = comMoLen - 1;
	CHashStrV2 hashStr;
	
	if (hashKeyLen < 0) {
		hashKeyLen = 1;
	}
	if (hashKeyLen > 3) {
		hashKeyLen = 3;
	}
	if (!HashStrV2_Create(&hashStr, hashKeyLen, 0, comMoLen, maxQSize)) {
		fprintf(stdout, "\nUnable to allocate memory for hash table");
		return 0;
	}

	t1 = time(NULL);
	//fprintf(stdout, "\nGenerate Q0: ");		
	for (i = 0; i < inputStrs->m_str[0].m_length - motifLen + 1; i++) {
		rootStr = inputStrs->m_str[0].m_data + i;
		memcpy(currentStr, rootStr, motifLen);

		hashStr.m_bucketTotalSize += PMS1GenDNeigh( motifLen, hammingDist, rootStr, currentStr, 
									-1, 0, comMoLen, 
									hashStr.m_bucketBuf + hashStr.m_bucketTotalSize * comMoLen, 
									hashStr.m_bucketBuf + maxQSize * comMoLen );

		if (hashStr.m_bucketTotalSize >= maxQSize) {
			hashStr.m_bucketTotalSize = maxQSize;
			//fprintf(stdout, "\nSize of Q0 = %d during", hashStr.m_bucketTotalSize);

			HashStrV2_SortAndRemoveDuplicateStrings(&hashStr);

			hashStr.m_bucketTotalSize += PMS1GenDNeigh( motifLen, hammingDist, rootStr, currentStr, 
									-1, 0, comMoLen, 
									hashStr.m_bucketBuf + hashStr.m_bucketTotalSize * comMoLen, 
									hashStr.m_bucketBuf + maxQSize * comMoLen );

			if (hashStr.m_bucketTotalSize >= maxQSize) {
				fprintf(stdout, "\nsome motits may be missed.");
				break;
			}			
		}
	}

	//fprintf(stdout, "\nSize of Q0 = %d before", hashStr.m_bucketTotalSize);
	HashStrV2_SortAndRemoveDuplicateStrings(&hashStr);
	HashStrV2_UpdateBucketPointers(&hashStr);
	//fprintf(stdout, "\nSize of Q0 = %d after", hashStr.m_bucketTotalSize);
	for (k = 1; k <= paramK; k++) {
		HashStrV2_ResetMarkBucket(&hashStr, 0);
		for (j = 0; j < inputStrs->m_str[k].m_length - motifLen + 1; j++) {
			rootStr = inputStrs->m_str[k].m_data + j;
			memcpy(currentStr, rootStr, motifLen);
			PMS1GenDNeighChecking(motifLen, hammingDist, rootStr, currentStr, encondedStr, 
									-1, 0, &hashStr);
		}
		//fprintf(stdout, "\nSize of Q%d = %d before", k, hashStr.m_bucketTotalSize);
		HashStrV2_RemoveUnmarkedStrings(&hashStr);
		HashStrV2_UpdateBucketPointers(&hashStr);
		//fprintf(stdout, "\nSize of Q%d = %d ", k, hashStr.m_bucketTotalSize);		
		if (hashStr.m_bucketTotalSize < sizeQTheshold ) {
			break;
		}
	}

	t2 = time(NULL);
	fprintf(stdout, "\nT=%ds", t2 - t1);
	//system("PAUSE");

	ptr = hashStr.m_bucketBuf;
	for (i = 0; i < hashStr.m_bucketTotalSize; i++, ptr += comMoLen) {
		DecodeDNAString(ptr, comMoLen, aMotif);
		if (IsMotifInputStrSet(aMotif, motifLen, hammingDist, inputStrs, k, inputStrs->m_num - 1)) {
			fprintf(stdout, "\n");
			PrintDNAString(aMotif, motifLen, 1, 0, stdout);
			if (numFoundMotifs < maxNumMotifsAllowed) {
				memcpy(&foundMotifs[numFoundMotifs], ptr, comMoLen);
				numFoundMotifs++;
			}
		}
	}
	//fprintf(stdout, "\n#Motifs=%d ", numFoundMotifs);

	HashStrV2_Delete(&hashStr);

	return numFoundMotifs;
}


int PMS1Enhance1(int motifLen, int hammingDist, const CInputStringSet * inputStrs,
		 CCompactMotif * foundMotifs, int maxNumMotifsAllowed, 
		 int paramK, int sizeQTheshold, unsigned int maxQSize) {

	int t1, t2;
	int i, j, k, loop;
	char aMotif[CONST_MAX_MOTIF_STRING_LENGTH];
	char * rootStr;
	char currentStr[CONST_MAX_MOTIF_STRING_LENGTH];
	char encodedStr[CONST_MAX_MOTIF_STRING_LENGTH];
	int numFoundMotifs = 0;
	
	char * ptr;
	int comMoLen = motifLen/4 + 1;
	int hashKeyLen = comMoLen - 1;
	CHashStrV2 hashStr;
	
	if (hashKeyLen < 0) {
		hashKeyLen = 1;
	}
	if (hashKeyLen > 3) {
		hashKeyLen = 3;
	}
	if (!HashStrV2_Create(&hashStr, hashKeyLen, 0, comMoLen, maxQSize)) {
		fprintf(stdout, "\nUnable to allocate memory for hash table");
		return 0;
	}

	memset(encodedStr, 0, sizeof(encodedStr));

	t1 = time(NULL);
	loop = 0;
	numFoundMotifs = 0;
	while (loop < inputStrs->m_str[0].m_length - motifLen + 1) {
		
		fprintf(stdout, "\nloop = %d Generate Q0: ", loop);
		
		HashStrV2_Reset(&hashStr);

		for (i = loop; i < inputStrs->m_str[0].m_length - motifLen + 1; i++) {
			rootStr = inputStrs->m_str[0].m_data + i;
			memcpy(currentStr, rootStr, motifLen);

			hashStr.m_bucketTotalSize += PMS1GenDNeigh( motifLen, hammingDist, rootStr, currentStr, 
										-1, 0, comMoLen, 
										hashStr.m_bucketBuf + hashStr.m_bucketTotalSize * comMoLen, 
										hashStr.m_bucketBuf + maxQSize * comMoLen );

			if (hashStr.m_bucketTotalSize >= maxQSize) {
				hashStr.m_bucketTotalSize = maxQSize;
				break;
			}
		}
		loop = i;

		//fprintf(stdout, "\nSize of Q0 = %d before, loop = %d", hashStr.m_bucketTotalSize, loop);
		HashStrV2_SortAndRemoveDuplicateStrings(&hashStr);
		HashStrV2_UpdateBucketPointers(&hashStr);
		//fprintf(stdout, "\nSize of Q0 = %d after", hashStr.m_bucketTotalSize);
		for (k = 1; k <= paramK; k++) {
			HashStrV2_ResetMarkBucket(&hashStr, 0);
			for (j = 0; j < inputStrs->m_str[k].m_length - motifLen + 1; j++) {
				rootStr = inputStrs->m_str[k].m_data + j;
				memcpy(currentStr, rootStr, motifLen);
				PMS1GenDNeighChecking(motifLen, hammingDist, rootStr, currentStr, encodedStr, 
										-1, 0, &hashStr);
			}
			HashStrV2_RemoveUnmarkedStrings(&hashStr);
			HashStrV2_UpdateBucketPointers(&hashStr);
			//fprintf(stdout, "\nSize of Q%d = %d ", k, hashStr.m_bucketTotalSize);		
			if (hashStr.m_bucketTotalSize < sizeQTheshold ) {
				break;
			}
		}

		ptr = hashStr.m_bucketBuf;
		for (i = 0; i < hashStr.m_bucketTotalSize; i++, ptr += comMoLen) {
			DecodeDNAString(ptr, comMoLen, aMotif);
			if (IsMotifInputStrSet(aMotif, motifLen, hammingDist, inputStrs, k, inputStrs->m_num - 1)) {
				if (numFoundMotifs < maxNumMotifsAllowed) {
					memcpy(&foundMotifs[numFoundMotifs], ptr, comMoLen);
					numFoundMotifs++;
				}
			}
		}

	}
	t2 = time(NULL);
	fprintf(stdout, "\nTime = %ds. Done.", t2 - t1);


	RadixSort(foundMotifs, numFoundMotifs, sizeof(CCompactMotif));
	numFoundMotifs = RemoveSortedDuplicate(foundMotifs, numFoundMotifs, sizeof(CCompactMotif));
/*	
	for (i = 0; i < numFoundMotifs; i++) {
		DecodeDNAString(foundMotifs + i, comMoLen, aMotif);
		fprintf(stdout, "\n");
		PrintDNAString(aMotif, motifLen, 1, 0, stdout);		
	}
	fprintf(stdout, "\n#Motifs=%d ", numFoundMotifs);
*/

	HashStrV2_Delete(&hashStr);

	return numFoundMotifs;
}

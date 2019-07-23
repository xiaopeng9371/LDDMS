#include "InputDefs.h";
#include "DNACommonFuncs.h";
#include "UtilityFuncs.h";


void PrintInputString(const CInputString * inputStr, const char isNumber, FILE * f) {	
	fprintf(f, "len=%d\n", inputStr->m_length);	
	PrintDNAString(inputStr->m_data, inputStr->m_length, 1, isNumber, f);
}

void ReadInputFile(const char * fName, CInputStringSet * inputStrs) {
	
	char str[1000000];
	FILE * f;

	inputStrs->m_num = 0;

	f = fopen(fName, "r");
	if (f == NULL) {
		return;
	}

	//read each line
	while ( fgets(str, sizeof(str), f) && inputStrs->m_num < CONST_MAX_NUM_STRINGS) {	
		if (str[0] != '>' && strlen(str) > 0) {			
			str[strlen(str) - 1] = '\0';
			inputStrs->m_str[inputStrs->m_num].m_length = CONST_MAX_INPUT_STRING_LENGTH;
			if ( inputStrs->m_str[inputStrs->m_num].m_length > strlen(str) ) {
				inputStrs->m_str[inputStrs->m_num].m_length = strlen(str);
			}
			memcpy(inputStrs->m_str[inputStrs->m_num].m_data, str, inputStrs->m_str[inputStrs->m_num].m_length);
			MapDNAStringFromLetterToNumber(inputStrs->m_str[inputStrs->m_num].m_data, inputStrs->m_str[inputStrs->m_num].m_length);
			inputStrs->m_num++;			
		}
	}
	fclose(f);
}

void PrintInputStrSet2File(FILE * f, const CInputStringSet * inputStrs, int startSeq, int endSeq) {
	int i;
	for (i = startSeq; i <= endSeq; i++ ) {
		fprintf(f, ">Sequence %d\n", i + 1);		
		PrintDNAString(inputStrs->m_str[i].m_data, inputStrs->m_str[i].m_length, 1, 0, f);
		fprintf(f, "\n");
	}
}


int CompareTwoCompactMotifs(const CCompactMotif * m1, const CCompactMotif * m2) {
	return memcmp((char *)m1, (char *)m2, sizeof(CCompactMotif));
}
unsigned int RemoveDupCompMotifsInOrderArray(CCompactMotif * motifs, 
											   const unsigned int numMotifs) {
	CCompactMotif * ptr1 = motifs; 
	CCompactMotif * ptr2 = motifs + 1;
	CCompactMotif * ptrE = motifs + numMotifs;
	if (numMotifs == 0) {
		return 0;
	}
	for (; ptr2 < ptrE; ptr2++) {
		if ( memcmp(ptr1, ptr2, sizeof(CCompactMotif)) != 0 ) {
			ptr1++;
			if (ptr1 < ptr2) {
				memcpy(ptr1, ptr2, sizeof(CCompactMotif));
			}

		}
	}
	return (ptr1 - motifs + 1);
}
int CompareTwoMotifs(const CMotif * m1, const CMotif * m2) {
	return memcmp((char *)m1, (char *)m2, sizeof(CMotif)) ;
}
unsigned int RemoveDupMotifsInOrderArray(CMotif * motifs, const unsigned int numMotifs) {
	CMotif * ptr1 = motifs; 
	CMotif * ptr2 = motifs + 1;
	CMotif * ptrE = motifs + numMotifs;
	if (numMotifs == 0) {
		return 0;
	}
	for (; ptr2 < ptrE; ptr2++) {
		if ( memcmp(ptr1, ptr2, sizeof(CMotif)) != 0 ) {
			ptr1++;
			if (ptr1 < ptr2) {
				memcpy(ptr1, ptr2, sizeof(CMotif));
			}

		}
	}
	return (ptr1 - motifs + 1);
}

int HammingDistStrStr(const char * str1, const char * str2, const  int len) {	
	int dist = 0;
	const char * ptrE = str1 + len;
	for (; str1 < ptrE; str1++) {
		if ( !(*str1 == *str2) ) {
			dist++;
		}
		str2++;
	}
	return dist;
}
int HammingDistStrInputStrAtPos(const char * str,  int strLen, 
										 const CInputString * inputStr, 
										 const  int pos) {
	int dist = 0;
	const char * ptrInput = inputStr->m_data + pos;
	const char * strE = str + strLen;
	for (; str < strE ; str++) {
		if ( !(*str == *ptrInput) ) {
			dist++;
		}
		ptrInput++;
	}
	return dist;
}
int HammingDistStrInputStr(const char * str,  int strLen,
									const CInputString * inputStr) {	
	 int i;
	 int tempDist, dist = strLen;
	 int numLoc = inputStr->m_length - strLen;
	for (i = 0; i < numLoc; i++) {
		tempDist = HammingDistStrInputStrAtPos(str, strLen, inputStr, i);		
		if ( dist > tempDist ) {
			dist = tempDist;
		}
	}
	return dist;
}

int AtPosMotifInputStr(const char * motif, 
					 const int mLen, const int hd,
					 const CInputString * inputStr) {	
	int i, len = inputStr->m_length - mLen;
	for (i = 0; i < len; i++) {				
		if ( HammingDistStrInputStrAtPos(motif, mLen, inputStr, i) <= hd ) {
			return i;
		}
	}
	return i;
}
char IsMotifInputStr(const char * motif, 
					 const int mLen, const int hd,
					 const CInputString * inputStr) {	
	int i, len = inputStr->m_length - mLen + 1;
	for (i = 0; i < len; i++) {				
		if ( HammingDistStrInputStrAtPos(motif, mLen, inputStr, i) <= hd ) {
			return 1;
		}
	}
	return 0;
}

char IsMotifInputStrSet(const char * motif, 
						const  int mLen, const  int hd,
					    const CInputStringSet * inputStrSet, 
						const int loStr, const int hiStr) {	
	int i;
	for (i = hiStr; i >= loStr; i--) {				
		if ( !IsMotifInputStr(motif, mLen, hd, &inputStrSet->m_str[i]) ) {
			return 0;
		}
	}
	return 1;
}

char IsMotifInputStrSet2(const char * motif, 
						const  int mLen, const  int hd, int minMatch,
					    const CInputStringSet * inputStrSet, 
						const int loStr, const int hiStr) {	
	int i;
	int match = 0;
	//printf(" minMatch = %d", minMatch);
	for (i = hiStr; i >= loStr; i--) {		
		if ( IsMotifInputStr(motif, mLen, hd, &inputStrSet->m_str[i]) ) {
			++match;
		}
	//printf(" match = %d", match);
	if (match >= minMatch){
			return 1;
			}		
	}
	return 0;
}

char IsMotifInputStrSetWithBindingSite(const char * motif, 
						const  int mLen, const  int hd,
					    const CInputStringSet * inputStrSet, 
						const int loStr, const int hiStr) {	
	int i;
	char isMotif = 1;
	for (i = hiStr; i >= loStr; i--) {				
		if ( !IsMotifInputStr(motif, mLen, hd, &inputStrSet->m_str[i]) ) {
			//return 0;
			isMotif = 0;
		}
	}
	fprintf(stdout, "\n");
	for (i = loStr; i <= hiStr; i++) {				
		fprintf(stdout, "%d ", AtPosMotifInputStr(motif, mLen, hd, &inputStrSet->m_str[i]));
	}
	return isMotif;
}

int MaxHammingDistFrom1StrToNStrs( const char * str, 											
								   const char ** strs, 
								   const int numStrs, 
								   const int strLen) {
	int tempDist, dist = 0;
	const char ** strsE = strs + numStrs;
	for (; strs < strsE; strs++) {
		tempDist = HammingDistStrStr(str, (*strs), strLen);
		if ( dist < tempDist ) {
			dist = tempDist;
		}		
	}
	return dist;
}

int ReadOutputFile(const char * fName, CCompactMotif * foundMotifs, int maxNumMotifsAllowed) {
	int numFoundMotifs = 0;
	int moLen;
	char str[1000];
	FILE * f;
	f = fopen(fName,"r");
	//printf("%s", "readfile");
	if (f == NULL) {
		return 0;
	}
	//printf("%s","before loop");
	while (numFoundMotifs < maxNumMotifsAllowed) {
		if (fscanf(f, "%s", str) == EOF) break;
		moLen = strlen(str);
		MapDNAStringFromLetterToNumber(str, moLen);		
		EncodeDNAString(str, moLen, (char *)(foundMotifs + numFoundMotifs));
		numFoundMotifs++;
		//printf("%d", numFoundMotifs);
		//if (fscanf(f, "%s", str) == EOF) break;
	}
	fclose(f);
	return numFoundMotifs;
}
#include <stdlib.h>
#include <stdio.h>
#include "SystemDefsAndFuncs.h"
#include "BitFuncs.h"
#include "SortFuncs.h"
#include "HashStrFuncs.h"

unsigned int HashStr_GetKey(const CHashStr * hash, const char * str) {
	unsigned int key = 0;
	char * charKeyPtr = (char *)&key;
	if (hash->m_isBigEndian) {
		memcpy(charKeyPtr + sizeof(unsigned int) - hash->m_keyLen, str, hash->m_keyLen);
	} else {		
		memcpy(charKeyPtr, str, hash->m_keyLen);
	}
	return key;
}

void HashStr_ConvertKeyToString(const CHashStr * hash, unsigned int key, char * strKey) {
	char * charKeyPtr = (char *)&key;
	if (hash->m_isBigEndian) {
		memcpy(strKey, charKeyPtr + sizeof(unsigned int) - hash->m_keyLen, hash->m_keyLen);
	} else {		
		memcpy(strKey, charKeyPtr, hash->m_keyLen);
	}
}

char HashStr_Create(CHashStr * hash, unsigned char keyLen, unsigned char strLen, 
				   unsigned int bucketMaxSize, unsigned int exBucketBufMaxSize) {

	unsigned int i;	

	hash->m_isBigEndian = SysGetEndian();

	hash->m_keyLen = keyLen;
	hash->m_strLen = strLen;
	hash->m_bkStoreLen = strLen - keyLen;
	hash->m_bucketMaxSize = bucketMaxSize;
	hash->m_exBucketBufMaxSize = exBucketBufMaxSize;
	hash->m_maxKey = 1;
	for (i = 0; i < hash->m_keyLen; i++) {
		hash->m_maxKey *= 256;
	}

	if (hash->m_maxKey == 0) {
		return 0;
	}
	if (hash->m_maxKey * hash->m_bucketMaxSize * hash->m_bkStoreLen == 0) {
		return 0;
	}
	if (hash->m_exBucketBufMaxSize * hash->m_strLen == 0) {
		return 0;
	}

	hash->m_sortAppearKey = malloc(256 * sizeof(unsigned char) * hash->m_strLen);
	hash->m_sortCountKeyTable = malloc(256 * sizeof(unsigned int) * hash->m_strLen);

	hash->m_bucket = (char **)malloc(hash->m_maxKey * sizeof(char *));
	hash->m_bucketSize = (unsigned int *)malloc(hash->m_maxKey * sizeof(unsigned int));
	hash->m_bucketMark = (char **)malloc(hash->m_maxKey * sizeof(char *));

	hash->m_exBucket = (char **)malloc(hash->m_maxKey * sizeof(char *));
	hash->m_exBucketSize = (unsigned int *)malloc(hash->m_maxKey * sizeof(unsigned int));
	hash->m_exBucketMark = (char **)malloc(hash->m_maxKey * sizeof(char *));

	hash->m_bucketBuf = (char *)malloc(hash->m_maxKey * hash->m_bucketMaxSize * hash->m_bkStoreLen);
	hash->m_bucketMarkBuf = (char *)malloc(hash->m_maxKey * (hash->m_bucketMaxSize/8 + 1));
	hash->m_exBucketBuf = (char *)malloc(hash->m_exBucketBufMaxSize * hash->m_strLen);
	hash->m_exBucketMarkBuf = (char *)malloc(hash->m_exBucketBufMaxSize/8 + hash->m_maxKey);

	if (hash->m_sortAppearKey == NULL || hash->m_sortCountKeyTable == NULL ||
		hash->m_bucket == NULL || hash->m_bucketSize == NULL || hash->m_bucketMark == NULL ||
		hash->m_exBucket == NULL || hash->m_exBucketSize == NULL || hash->m_exBucketMark == NULL ||
		hash->m_bucketBuf == NULL || hash->m_bucketMarkBuf == NULL ||
		hash->m_exBucketBuf == NULL || hash->m_exBucketMarkBuf == NULL) {

		if (hash->m_sortAppearKey != NULL) {
			free(hash->m_sortAppearKey);
		}
		if (hash->m_sortCountKeyTable != NULL) {
			free(hash->m_sortCountKeyTable);
		}
		if (hash->m_bucket != NULL) {
			free(hash->m_bucket);
		}
		if (hash->m_bucketSize != NULL) {
			free(hash->m_bucketSize);
		}
		if (hash->m_bucketMark  != NULL) {
			free(hash->m_bucketMark );
		}

		if (hash->m_exBucket != NULL) {
			free(hash->m_exBucket);
		}
		if (hash->m_exBucketSize != NULL) {
			free(hash->m_exBucketSize);
		}
		if (hash->m_exBucketMark != NULL) {
			free(hash->m_exBucketMark);
		}

		if (hash->m_bucketBuf != NULL) {
			free(hash->m_bucketBuf);
		}
		if (hash->m_bucketMarkBuf != NULL) {
			free(hash->m_bucketMarkBuf);
		}
		if (hash->m_exBucketBuf != NULL) {
			free(hash->m_exBucketBuf);
		}
		if (hash->m_exBucketMarkBuf != NULL) {
			free(hash->m_exBucketMarkBuf);
		}

		return 0;
	}
	
	for (i = 0; i < hash->m_maxKey; i++) {
		hash->m_bucket[i] = hash->m_bucketBuf + i * hash->m_bucketMaxSize * hash->m_bkStoreLen;
		hash->m_bucketMark[i] = hash->m_bucketMarkBuf + i * (hash->m_bucketMaxSize/8 + 1);
	}	
	
	HashStr_Reset(hash);

	return 1;
}


void HashStr_Delete(CHashStr * hash) {
	free(hash->m_sortAppearKey);
	free(hash->m_sortCountKeyTable);

	free(hash->m_bucket);
	free(hash->m_bucketSize);
	free(hash->m_bucketMark );

	free(hash->m_exBucket);
	free(hash->m_exBucketSize);
	free(hash->m_exBucketMark);

	free(hash->m_bucketBuf);
	free(hash->m_bucketMarkBuf);
	free(hash->m_exBucketBuf);
	free(hash->m_exBucketMarkBuf);
}

void HashStr_Reset(CHashStr * hash) {
	hash->m_exBucketTotalSize = 0;
	memset(hash->m_bucketSize, 0, hash->m_maxKey * sizeof(unsigned int));
	memset(hash->m_exBucketSize, 0, hash->m_maxKey * sizeof(unsigned int));
	HashStr_ResetEntireMarkBucket(hash, 0);
}

void HashStr_ResetEntireMarkBucket(CHashStr * hash, char val) {
	if (val) {
		memset(hash->m_bucketMarkBuf, 255, hash->m_maxKey * (hash->m_bucketMaxSize/8 + 1));
		memset(hash->m_exBucketMarkBuf, 255, hash->m_exBucketBufMaxSize/8 + hash->m_maxKey);
	} else {
		memset(hash->m_bucketMarkBuf, 0, hash->m_maxKey * (hash->m_bucketMaxSize/8 + 1));
		memset(hash->m_exBucketMarkBuf, 0, hash->m_exBucketBufMaxSize/8 + hash->m_maxKey);
	}
}

void HashStr_ResetMarkBucket(CHashStr * hash, char val) {
	unsigned int i;
	if (val) {
		for (i = 0; i < hash->m_maxKey; i++) {
			memset(hash->m_bucketMark[i], 255, hash->m_bucketSize[i]/8 + 1);
		}
	} else {
		for (i = 0; i < hash->m_maxKey; i++) {
			memset(hash->m_bucketMark[i], 0, hash->m_bucketSize[i]/8 + 1);
		}
	}
}

char HashStr_AddString(CHashStr * hash, const char * str) {
	unsigned int key = HashStr_GetKey(hash, str);
	if (hash->m_bucketSize[key] < hash->m_bucketMaxSize) {
		memcpy(hash->m_bucket[key] + hash->m_bucketSize[key] * hash->m_bkStoreLen, str + hash->m_keyLen, hash->m_bkStoreLen);
		hash->m_bucketSize[key]++;
		return 1;
	} else if (hash->m_exBucketTotalSize < hash->m_exBucketBufMaxSize) {
		memcpy(hash->m_exBucketBuf + hash->m_exBucketTotalSize * hash->m_strLen, str , hash->m_strLen);
		hash->m_exBucketTotalSize++;
		return 1;
	}	
	return 0;
}


char HashStr_DoesStringExist(CHashStr * hash, const char * str) {
	unsigned int key; 
	const char * foundStr; 
	key = HashStr_GetKey(hash, str);
	foundStr = BinarySearch(hash->m_bucket[key], hash->m_bucketSize[key], hash->m_bkStoreLen, str + hash->m_keyLen);
	if (foundStr == NULL) {
		return 0;
	} else {
		return 1;
	}
/*
	if (foundStr == NULL) {
		if (hash->m_bucketSize[key] >= hash->m_bucketMaxSize) {
			foundStr = BinarySearch(hash->m_exBucketBuf, hash->m_exBucketTotalSize, hash->m_strLen, str);
			if (foundStr == NULL) {
				return 0;
			} else {
				return 1;
			}
		} else {
			return 0;
		}
	}
	return 1;
*/
}

void HashStr_FindAndMarkString(CHashStr * hash, const char * str) {
	unsigned int key = HashStr_GetKey(hash, str);
	const char * foundStr = BinarySearch(hash->m_bucket[key], hash->m_bucketSize[key], hash->m_bkStoreLen, str + hash->m_keyLen);
	if (foundStr != NULL) {
		BitArrayWrite(hash->m_bucketMark[key], (foundStr - hash->m_bucket[key])/hash->m_bkStoreLen, 1);
	}
/*	
	if (foundStr == NULL) {
		if (hash->m_bucketSize[key] >= hash->m_bucketMaxSize) {
			foundStr = BinarySearch(hash->m_exBucketBuf, hash->m_exBucketTotalSize, hash->m_strLen, str);
			if (foundStr != NULL) {
				BitArrayWrite(hash->m_exBucketMarkBuf, (foundStr - hash->m_exBucketMarkBuf)/hash->m_strLen, 1);
			}
		} 		
	} else {
		BitArrayWrite(hash->m_bucketMark[key], (foundStr - hash->m_bucket[key])/hash->m_bkStoreLen, 1);
	}
*/
}

void HashStr_SortStrings(CHashStr * hash) {
	unsigned int i;
	for (i = 0; i < hash->m_maxKey; i++) {
		RadixSortEnhanced(hash->m_bucket[i], hash->m_bucketSize[i], hash->m_bkStoreLen,
						  hash->m_sortAppearKey, hash->m_sortCountKeyTable, 
						  hash->m_sortPosKeyTable, hash->m_sortEPosKeyTable);
		//RadixSort(hash->m_bucket[i], hash->m_bucketSize[i], hash->m_bkStoreLen);
		//QuickSort(hash->m_bucket[i], hash->m_bucketSize[i], hash->m_bkStoreLen, hash->m_bkStoreLen);
		//HeapSort(hash->m_bucket[i], hash->m_bucketSize[i], hash->m_bkStoreLen);
	}
	RadixSortEnhanced(hash->m_exBucketBuf, hash->m_exBucketTotalSize, hash->m_strLen,
					  hash->m_sortAppearKey, hash->m_sortCountKeyTable, 
					  hash->m_sortPosKeyTable, hash->m_sortEPosKeyTable);

}

void HashStr_RemoveDuplicateStrings(CHashStr * hash) {
	unsigned int i;	
	for (i = 0; i < hash->m_maxKey; i++) {
		hash->m_bucketSize[i] = RemoveSortedDuplicate(hash->m_bucket[i], hash->m_bucketSize[i], hash->m_bkStoreLen);
	}
	hash->m_exBucketTotalSize = RemoveSortedDuplicate(hash->m_exBucketBuf, hash->m_exBucketTotalSize, hash->m_strLen);
}

void HashStr_SortAndRemoveDuplicateStrings(CHashStr * hash) {
	unsigned int i;	
	for (i = 0; i < hash->m_maxKey; i++) {
		RadixSortEnhanced(hash->m_bucket[i], hash->m_bucketSize[i], hash->m_bkStoreLen,
						  hash->m_sortAppearKey, hash->m_sortCountKeyTable, 
						  hash->m_sortPosKeyTable, hash->m_sortEPosKeyTable);
		hash->m_bucketSize[i] = RemoveSortedDuplicate(hash->m_bucket[i], hash->m_bucketSize[i], hash->m_bkStoreLen);
	}
	RadixSortEnhanced(hash->m_exBucketBuf, hash->m_exBucketTotalSize, hash->m_strLen,
					  hash->m_sortAppearKey, hash->m_sortCountKeyTable, 
					  hash->m_sortPosKeyTable, hash->m_sortEPosKeyTable);
	hash->m_exBucketTotalSize = RemoveSortedDuplicate(hash->m_exBucketBuf, hash->m_exBucketTotalSize, hash->m_strLen);
}

void HashStr_RemoveUnmarkedStrings(CHashStr * hash) {	
	unsigned int i, j;
	const char * ptr, * ptr1;
	for (i = 0; i < hash->m_maxKey; i++) {		
		ptr = hash->m_bucket[i];
		ptr1 = ptr;
		for (j = 0; j < hash->m_bucketSize[i]; j++, ptr1 += hash->m_bkStoreLen) {
			if (BitArrayRead(hash->m_bucketMark[i], j)) {
				memcpy(ptr, ptr1, hash->m_bkStoreLen);
				ptr += hash->m_bkStoreLen;
			}
		}
		hash->m_bucketSize[i] = (ptr - hash->m_bucket[i])/hash->m_bkStoreLen;
	}
}

unsigned int HashStr_GetNumString(CHashStr * hash) {
	unsigned int numStr = 0;
	unsigned int i;
	for (i = 0; i < hash->m_maxKey; i++) {
		numStr += hash->m_bucketSize[i];
	}
	return numStr;
}

void HashStr_PrintStat(CHashStr * hash, FILE * f) {
	unsigned int i, numStr = 0, minSize, maxSize;
	minSize = hash->m_bucketSize[0];
	maxSize = hash->m_bucketSize[0];
	for (i = 0; i < hash->m_maxKey; i++) {
		numStr += hash->m_bucketSize[i];
		if (minSize > hash->m_bucketSize[i]) {
			minSize = hash->m_bucketSize[i];
		}
		if (maxSize < hash->m_bucketSize[i]) {
			maxSize = hash->m_bucketSize[i];
		}
	}
	fprintf(f, "\n%d %d exNum=%d num=%d min=%d max=%d\n", hash->m_bucketMaxSize, hash->m_exBucketBufMaxSize, hash->m_exBucketTotalSize, numStr, minSize, maxSize);
	for (i = 0; i < hash->m_maxKey; i++) {
		//fprintf(stdout, "[%d]=%d ", i, hash->m_bucketSize[i]);
	}
	//fprintf(f, "\nmaxKey=%d %d %d", hash->m_maxKey, hash->m_maxKey * hash->m_bucketMaxSize, hash->m_maxKey * hash->m_bucketMaxSize * hash->m_bkStoreLen);
}

void HashStr_PrintBucket(CHashStr * hash, unsigned int bucketID, FILE * f) {
	unsigned int i, j;
	for (i = 0; i < hash->m_bucketSize[bucketID]; i++) {
		fprintf(f, "\n%3d | ", i);
		for (j = 0; j < hash->m_bkStoreLen; j++) {
			fprintf(f, "%d ", (unsigned char)*(hash->m_bucket[bucketID] + i * hash->m_bkStoreLen + j));
		}
	}	
}


/*********************************************************************



*********************************************************************/


unsigned int HashStrV2_GetKey(const CHashStrV2 * hash, const char * str) {
	unsigned int key = 0;
	char * charKeyPtr = (char *)&key;
	char * charKeyPtr1;
	if (hash->m_isBigEndian) {
		charKeyPtr1 = charKeyPtr + sizeof(unsigned int) - hash->m_keyLen;
		memcpy(charKeyPtr1, str, hash->m_keyLen);
		if (hash->m_exKeyLenInBit > 0) {
			*(charKeyPtr1 - 1) = *(str + hash->m_keyLen);
			BitShift2RightZeroPadding(charKeyPtr1 - 1, 8 - hash->m_exKeyLenInBit);
		}
	} else {		
		memcpy(charKeyPtr, str, hash->m_keyLen);
		if (hash->m_exKeyLenInBit > 0) {
			*(charKeyPtr + hash->m_keyLen) = *(str + hash->m_keyLen);
			BitShift2RightZeroPadding(charKeyPtr + hash->m_keyLen, 8 - hash->m_exKeyLenInBit);
		}
	}
	return key;
}

// = 0 if the same, 1 if different
int HashStrV2_CompareKey(const CHashStrV2 * hash, const char * str1, const char * str2) {
	if (memcmp(str1, str2, hash->m_keyLen)) {
		return 1;
	} else {
		if (hash->m_exKeyLenInBit > 0) {
			return BitCompare(*(str1 + hash->m_keyLen), *(str2 + hash->m_keyLen), 0, hash->m_exKeyLenInBit);
		} else {		
			return 0;
		}
	}	
}

char HashStrV2_Create(CHashStrV2 * hash, unsigned char keyLen, unsigned char exKeyLenInBit,
					  unsigned char strLen, unsigned int bucketBufMaxSize) {

	unsigned int i;	

	hash->m_isBigEndian = SysGetEndian();

	hash->m_keyLen = keyLen;	
	hash->m_exKeyLenInBit = exKeyLenInBit;
	hash->m_strLen = strLen;
	hash->m_cmpKeyLen = hash->m_strLen - hash->m_keyLen;
	hash->m_strLenMinusOne = strLen - 1;
	if (strLen == 0) {
		return 0;
	}
	hash->m_bucketBufMaxSize = bucketBufMaxSize;
	hash->m_maxKey = 1;
	for (i = 0; i < hash->m_keyLen; i++) {
		hash->m_maxKey *= 256;
	}
	for (i = 0; i < hash->m_exKeyLenInBit; i++) {
		hash->m_maxKey *= 2;
	}
	if (hash->m_maxKey == 0) {
		return 0;
	}

	hash->m_sortAppearKey = malloc(256 * sizeof(unsigned char) * hash->m_strLen);
	hash->m_sortCountKeyTable = malloc(256 * sizeof(unsigned int) * hash->m_strLen);

	hash->m_bucket = (char **)malloc(hash->m_maxKey * sizeof(char *));
	hash->m_bucketSize = (TYPE_HASH_BUCKET_COUNT *)malloc(hash->m_maxKey * sizeof(TYPE_HASH_BUCKET_COUNT));
	hash->m_bucketBuf = (char *)malloc(hash->m_bucketBufMaxSize * hash->m_strLen);

	if (hash->m_sortAppearKey == NULL || hash->m_sortCountKeyTable == NULL ||
		hash->m_bucket == NULL || hash->m_bucketSize == NULL || hash->m_bucketBuf == NULL ) {

		if (hash->m_sortAppearKey != NULL) {
			free(hash->m_sortAppearKey);
		}
		if (hash->m_sortCountKeyTable != NULL) {
			free(hash->m_sortCountKeyTable);
		}
		if (hash->m_bucket != NULL) {
			free(hash->m_bucket);
		}
		if (hash->m_bucketSize != NULL) {
			free(hash->m_bucketSize);
		}
		if (hash->m_bucketBuf != NULL) {
			free(hash->m_bucketBuf);
		}

		return 0;
	}	
	
	
	HashStrV2_Reset(hash);

	return 1;
}


void HashStrV2_Delete(CHashStrV2 * hash) {
	free(hash->m_sortAppearKey);
	free(hash->m_sortCountKeyTable);

	free(hash->m_bucket);
	free(hash->m_bucketSize);
	free(hash->m_bucketBuf);
}

void HashStrV2_Reset(CHashStrV2 * hash) {
	hash->m_bucketTotalSize = 0;
	hash->m_lastStr = hash->m_bucketBuf;
	//memset(hash->m_bucket, NULL, hash->m_maxKey * sizeof(unsigned int *));
	//memset(hash->m_bucketSize, 0, hash->m_maxKey * sizeof(TYPE_HASH_BUCKET_COUNT));
	//memset(hash->m_bucketBuf, 0, hash->m_bucketBufMaxSize * hash->m_strLen);
}

void HashStrV2_ResetMarkBucket(CHashStrV2 * hash, char val) {
	char * ptr = hash->m_bucketBuf + hash->m_strLenMinusOne;
	char * ptrE = ptr + hash->m_strLen * hash->m_bucketTotalSize;
	for (; ptr < ptrE; ptr += hash->m_strLen) {
		BitWrite(ptr, 7, val);
	}
}

char HashStrV2_AddString(CHashStrV2 * hash, const char * str) {
	if (hash->m_bucketTotalSize < hash->m_bucketBufMaxSize) {
		memcpy(hash->m_lastStr, str, hash->m_strLen);
		hash->m_bucketTotalSize++;
		hash->m_lastStr += hash->m_strLen;
		return 1;
	} else {	
		return 0;
	}
}

char HashStrV2_AddStrings(CHashStrV2 * hash, const char * str, unsigned int nStr) {
	if (hash->m_bucketTotalSize + nStr < hash->m_bucketBufMaxSize) {
		memcpy(hash->m_lastStr, str, hash->m_strLen * nStr);
		hash->m_bucketTotalSize += nStr;
		hash->m_lastStr += (hash->m_strLen * nStr);
		return 1;
	} else {	
		return 0;
	}
}


char HashStrV2_DoesStringExist(CHashStrV2 * hash, const char * str) {
	unsigned int key; 
	const char * foundStr; 
	key = HashStrV2_GetKey(hash, str);
	if (hash->m_bucketSize[key] > 0) {
		foundStr = BinarySearch(hash->m_bucket[key], hash->m_bucketSize[key], hash->m_strLen, str);
		if (foundStr == NULL) {
			return 0;
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}

void HashStrV2_FindAndMarkString(CHashStrV2 * hash, const char * str) {
	unsigned int key; 
	char * foundStr;	
	key = HashStrV2_GetKey(hash, str);
	if (hash->m_bucketSize[key] > 0) {
		//foundStr = BinarySearch(hash->m_bucket[key], hash->m_bucketSize[key], hash->m_strLen, str);
		foundStr = BinarySearchEnhanced(hash->m_bucket[key], hash->m_bucketSize[key], 
										hash->m_strLen, hash->m_cmpKeyLen, 
										str + hash->m_keyLen);
		if (foundStr != NULL) {
			foundStr += hash->m_strLenMinusOne;
			BitWrite(foundStr, 7, 1);
		}
	} 
/*
	foundStr = BinarySearch(hash->m_bucketBuf, hash->m_bucketTotalSize, hash->m_strLen, str);
	if (foundStr != NULL) {
		foundStr += hash->m_strLenMinusOne;
		BitWrite(foundStr, 7, 1);
	}
*/
}

void HashStrV2_UpdateBucketPointers(CHashStrV2 * hash) {
	unsigned int key;
	char * ptr, *ptr1, * ptrE;
	//memset(hash->m_bucket, NULL, hash->m_maxKey * sizeof(unsigned int *));
	memset(hash->m_bucketSize, 0, hash->m_maxKey * sizeof(TYPE_HASH_BUCKET_COUNT));
	if (hash->m_bucketTotalSize == 0) {
		return;
	}
	ptrE = hash->m_bucketBuf + hash->m_bucketTotalSize * hash->m_strLen;
	ptr = hash->m_bucketBuf;
	ptr1 = hash->m_bucketBuf + hash->m_strLen; 	
	for (; ptr1 < ptrE; ptr1 += hash->m_strLen) {
		if (HashStrV2_CompareKey(hash, ptr, ptr1)) {
			key = HashStrV2_GetKey(hash, ptr);
			hash->m_bucket[key] = ptr;
			hash->m_bucketSize[key] = (ptr1 - ptr)/hash->m_strLen;
			ptr = ptr1;
		}
	}
	key = HashStrV2_GetKey(hash, ptr);
	hash->m_bucket[key] = ptr;
	hash->m_bucketSize[key] = (ptr1 - ptr)/hash->m_strLen;
}

void HashStrV2_SortStrings(CHashStrV2 * hash) {
	RadixSortEnhanced(hash->m_bucketBuf, hash->m_bucketTotalSize, hash->m_strLen,
					  hash->m_sortAppearKey, hash->m_sortCountKeyTable, 
					  hash->m_sortPosKeyTable, hash->m_sortEPosKeyTable);
	//RadixSort(hash->m_bucketBuf, hash->m_bucketTotalSize, hash->m_strLen);
	//HeapSort(hash->m_bucketBuf, hash->m_bucketTotalSize, hash->m_strLen);
	//QuickSort(hash->m_bucketBuf, hash->m_bucketTotalSize, hash->m_strLen);
}

void HashStrV2_RemoveDuplicateStrings(CHashStrV2 * hash) {
	hash->m_bucketTotalSize = RemoveSortedDuplicate(hash->m_bucketBuf, hash->m_bucketTotalSize, hash->m_strLen);
	hash->m_lastStr = hash->m_bucketBuf + hash->m_bucketTotalSize * hash->m_strLen;
}

void HashStrV2_SortAndRemoveDuplicateStrings(CHashStrV2 * hash) {
	HashStrV2_SortStrings(hash);
	HashStrV2_RemoveDuplicateStrings(hash);
}

void HashStrV2_RemoveUnmarkedStrings(CHashStrV2 * hash) {	
	const char * ptr, * ptr1, * ptrE;	
	ptr1 = hash->m_bucketBuf;
	ptrE = ptr1 + hash->m_bucketTotalSize * hash->m_strLen;
	for ( ; ptr1 < ptrE; ptr1 += hash->m_strLen ) {
		if (BitRead(*(ptr1 + hash->m_strLenMinusOne), 7) == 0) {
			break;
		}
	}
	ptr = ptr1;
	ptr1 += hash->m_strLen;
	for ( ; ptr1 < ptrE; ptr1 += hash->m_strLen ) {
		if (BitRead(*(ptr1 + hash->m_strLenMinusOne), 7)) {
			memcpy(ptr, ptr1, hash->m_strLen);
			ptr += hash->m_strLen;
		}
	}
	hash->m_bucketTotalSize = (ptr - hash->m_bucketBuf)/hash->m_strLen;
	hash->m_lastStr = ptr;
}

unsigned int HashStrV2_GetNumString(CHashStrV2 * hash) {
	return hash->m_bucketTotalSize;
}

void HashStrV2_PrintStat(CHashStrV2 * hash, FILE * f) {
	unsigned int i, minSize, maxSize;
	minSize = hash->m_bucketSize[0];
	maxSize = hash->m_bucketSize[0];
/*
	for (i = 0; i < hash->m_maxKey; i++) {
		if (minSize > hash->m_bucketSize[i]) {
			minSize = hash->m_bucketSize[i];
		}
		if (maxSize < hash->m_bucketSize[i]) {
			maxSize = hash->m_bucketSize[i];
		}
	}
*/
	fprintf(f, "\nmaxKey=%d maxNum=%d num=%d minBk=%d maxBk=%d", 
				hash->m_maxKey, hash->m_bucketBufMaxSize, hash->m_bucketTotalSize, 
				minSize, maxSize);
}

void HashStrV2_PrintBucket(CHashStrV2 * hash, unsigned int bucketID, FILE * f) {
	unsigned int i, j;
	for (i = 0; i < hash->m_bucketSize[bucketID]; i++) {
		fprintf(f, "\n%9d | ", i);
		for (j = 0; j < hash->m_strLen; j++) {
			fprintf(f, "%d ", (unsigned char)*(hash->m_bucket[bucketID] + i * hash->m_strLen + j));
		}
	}	
}

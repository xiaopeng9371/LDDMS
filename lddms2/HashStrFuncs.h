#ifndef __HashStrFuncs_h__
#define __HashStrFuncs_h__

struct CHashStr {
	char m_isBigEndian;

	unsigned char m_keyLen;
	unsigned int m_maxKey;
	unsigned char m_strLen;	
	unsigned char m_bkStoreLen;

	unsigned int m_bucketMaxSize;
	unsigned int * m_bucketSize;
	char ** m_bucket;
	char ** m_bucketMark;

	unsigned int m_exBucketBufMaxSize;
	unsigned int m_exBucketTotalSize;
	unsigned int * m_exBucketSize;
	char ** m_exBucket;	
	char ** m_exBucketMark;	
	
	char * m_bucketBuf;
	char * m_bucketMarkBuf;
	char * m_exBucketBuf;
	char * m_exBucketMarkBuf;
	

	unsigned char * m_sortAppearKey;
	unsigned int * m_sortCountKeyTable;
	unsigned char * m_sortPosKeyTable[256];
	unsigned char * m_sortEPosKeyTable[256];	

} typedef CHashStr;

unsigned int HashStr_GetKey(const CHashStr * hash, const char * str);
void HashStr_ConvertKeyToString(const CHashStr * hash, unsigned int key, char * strKey);

char HashStr_Create(CHashStr * hash, unsigned char keyLen, unsigned char strLen, 
				   unsigned int bucketBufMaxSize, unsigned int exBucketBufMaxSize);
void HashStr_Delete(CHashStr * hash);
void HashStr_Reset(CHashStr * hash);
void HashStr_ResetEntireMarkBucket(CHashStr * hash, char val);
void HashStr_ResetMarkBucket(CHashStr * hash, char val);

char HashStr_AddString(CHashStr * hash, const char * str);
char HashStr_DoesStringExist(CHashStr * hash, const char * str);
void HashStr_FindAndMarkString(CHashStr * hash, const char * str);

void HashStr_SortStrings(CHashStr * hash);
void HashStr_RemoveDuplicateStrings(CHashStr * hash);
void HashStr_SortAndRemoveDuplicateStrings(CHashStr * hash);
void HashStr_RemoveUnmarkedStrings(CHashStr * hash);

unsigned int HashStr_GetNumString(CHashStr * hash);

void HashStr_PrintStat(CHashStr * hash, FILE * f);
void HashStr_PrintBucket(CHashStr * hash, unsigned int bucketID, FILE * f);


typedef unsigned short int TYPE_HASH_BUCKET_COUNT;

struct CHashStrV2 {
	char m_isBigEndian;

	unsigned char m_keyLen;
	unsigned char m_exKeyLenInBit;
	unsigned char m_cmpKeyLen;
	unsigned int m_maxKey;
	unsigned char m_strLen;
	unsigned char m_strLenMinusOne;

	char * m_lastStr;

	unsigned int m_bucketBufMaxSize;
	unsigned int m_bucketTotalSize;	
	char ** m_bucket;
	TYPE_HASH_BUCKET_COUNT * m_bucketSize;
	char * m_bucketBuf;		

	unsigned char * m_sortAppearKey;
	unsigned int * m_sortCountKeyTable;
	unsigned char * m_sortPosKeyTable[256];
	unsigned char * m_sortEPosKeyTable[256];	

} typedef CHashStrV2;

unsigned int HashStrV2_GetKey(const CHashStrV2 * hash, const char * str);
int HashStrV2_CompareKey(const CHashStrV2 * hash, const char * str1, const char * str2);

char HashStrV2_Create(CHashStrV2 * hash, unsigned char keyLen, unsigned char exKeyLenInBit,
					  unsigned char strLen, unsigned int bucketBufMaxSize);
void HashStrV2_Delete(CHashStrV2 * hash);
void HashStrV2_Reset(CHashStrV2 * hash);
void HashStrV2_ResetMarkBucket(CHashStrV2 * hash, char val);

char HashStrV2_AddString(CHashStrV2 * hash, const char * str);
char HashStrV2_AddStrings(CHashStrV2 * hash, const char * str, unsigned int nStr);
char HashStrV2_DoesStringExist(CHashStrV2 * hash, const char * str);
void HashStrV2_FindAndMarkString(CHashStrV2 * hash, const char * str);

void HashStrV2_UpdateBucketPointers(CHashStrV2 * hash);
void HashStrV2_SortStrings(CHashStrV2 * hash);
void HashStrV2_RemoveDuplicateStrings(CHashStrV2 * hash);
void HashStrV2_SortAndRemoveDuplicateStrings(CHashStrV2 * hash);
void HashStrV2_RemoveUnmarkedStrings(CHashStrV2 * hash);

unsigned int HashStrV2_GetNumString(CHashStrV2 * hash);

void HashStrV2_PrintStat(CHashStrV2 * hash, FILE * f);
void HashStrV2_PrintBucket(CHashStrV2 * hash, unsigned int bucketID, FILE * f);

#endif
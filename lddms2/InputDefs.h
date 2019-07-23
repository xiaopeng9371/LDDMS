#ifndef __InputDefs_h__
#define __InputDefs_h__

#define CONST_MAX_NUM_STRINGS 20
#define CONST_MAX_INPUT_STRING_LENGTH 600
#define CONST_MAX_MOTIF_STRING_LENGTH 23
#define CONST_MAX_MOTIF_STRING_COMPACT_LENGTH 6
#define CONST_MAX_HAMMING_DIST 9

#define CONST_MAX_NUM_FOUND_MOTIFS_ALLOWED 200000000

struct CInputString {
	char m_data[CONST_MAX_INPUT_STRING_LENGTH];	
	int m_length;
} typedef CInputString;

struct CInputStringSet {
	CInputString m_str[CONST_MAX_NUM_STRINGS];	
	int m_num;
} typedef CInputStringSet;

typedef char CMotif[CONST_MAX_MOTIF_STRING_LENGTH];
typedef char CCompactMotif[CONST_MAX_MOTIF_STRING_COMPACT_LENGTH];

#endif

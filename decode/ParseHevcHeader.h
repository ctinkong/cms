#ifndef ParseHevcHeader_h
#define ParseHevcHeader_h

#include "parsesequenceheader.h"

class CParseHevcHeader : public CParseSequenceHeader
{
public:
	CParseHevcHeader(void);
	~CParseHevcHeader(void);

	virtual int DecodeExtradata(uint8_t *data, int data_size);

private:
	int DecodeNalUnit(uint8_t *data, int data_size);
	int DecodePtl();
	int ParsePtl(int max_num_sub_layers);
	int ParseSps();
};

#endif //ParseHevcHeader_h


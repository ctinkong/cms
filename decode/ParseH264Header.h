#ifndef ParseH264Header_h
#define ParseH264Header_h

#include "parsesequenceheader.h"

class CParseH264Header : public CParseSequenceHeader
{
public:
	CParseH264Header(void);
	~CParseH264Header(void);

	virtual int DecodeExtradata(uint8_t *data, int data_size);

private:
	int ParseSps();
	int DecodeNalUnit(uint8_t *data, int data_size);
};

#endif //ParseH264Header_h



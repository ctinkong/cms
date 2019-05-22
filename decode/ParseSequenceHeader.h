#ifndef ParseSequenceHeader_h
#define ParseSequenceHeader_h

#include "BitStream.h"

class CParseSequenceHeader : public CBitStream
{
public:
	CParseSequenceHeader(void);
	virtual ~CParseSequenceHeader(void);

	int GetWidth() { return m_width; };
	int GetHeight() { return m_height; };
	double GetFps() { return m_fps; };
	virtual int DecodeExtradata(uint8_t *data, int data_size) = 0;

protected:
	int H2645ExtractRbsp(const uint8_t *src, int length, uint8_t *dst);
	int SplitPacket(const uint8_t *buf, int buf_size);

	uint8_t *m_sps;
	int m_spsSize;
	int m_width;
	int m_height;
	double m_fps;
};

#endif //ParseSequenceHeader_h



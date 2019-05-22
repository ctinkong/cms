#ifndef BitStream_h
#define BitStream_h

typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

typedef union {
	uint32_t u32;
	uint16_t u16[2];
	uint8_t  u8[4];
	float    f32;
}alias32;

typedef union {
	uint16_t u16;
	uint8_t  u8[2];
}alias16;

#define FPS_RN32A(p) FPS_RNA(32, p)
#define FPS_RNA(s, p)    (((const alias##s*)(p))->u##s)

#define FPS_RN16(p) FPS_RN(16, p)
#define FPS_RN(s, p) (((const alias##s*)(p))->u##s)

#define FPS_RB16(p)    FPS_RB(16, p)
#define FPS_RB(s, p)   fps_bswap##s(FPS_RN##s(p))

static uint16_t fps_bswap16(uint16_t x)
{
	x = (x >> 8) | (x << 8);
	return x;
}

class CBitStream
{
public:
	CBitStream(void);
	~CBitStream(void);

	void InitBits(uint8_t *buf, int buf_size);
	uint32_t GetBits(int n);
	uint32_t PeekBits(int n);
	void SkipBits(int n);
	uint8_t *GetBuffer();
	uint32_t GetUe();
	int GetSe();
	int GetBitsLeft();
	int GetBitError() { return m_error; };

private:
	int m_index;
	int m_bufferSize;
	uint8_t *m_buffer;

	int m_error;
};

#endif //BitStream_h



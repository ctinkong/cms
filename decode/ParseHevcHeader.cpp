#include "ParseHevcHeader.h"

CParseHevcHeader::CParseHevcHeader(void)
{
}

CParseHevcHeader::~CParseHevcHeader(void)
{
}

int CParseHevcHeader::DecodePtl()
{
	if (GetBitsLeft() < 2 + 1 + 5 + 32 + 4 + 16 + 16 + 12)
		return -1;

	int profile_space = GetBits(2);
	int tier_flag = GetBits(1);
	int profile_idc = GetBits(5);
	SkipBits(32 + 4 + 44); // skip

	return 0;
}

int CParseHevcHeader::ParsePtl(int max_num_sub_layers)
{
	int idx = 0;
	const int MAX_SUB_LAYERS = 7;
	uint8_t sub_layer_profile_present_flag[MAX_SUB_LAYERS] = { 0 };
	uint8_t sub_layer_level_present_flag[MAX_SUB_LAYERS] = { 0 };

	//decode_profile_tier_level
	if (DecodePtl() < 0 || GetBitsLeft() < 8 + (8 * 2 * (max_num_sub_layers - 1 > 0)))
		return -1;

	int level_idc = GetBits(8);

	for (int i = 0; i < max_num_sub_layers - 1; i++) {
		sub_layer_profile_present_flag[i] = GetBits(1);
		sub_layer_level_present_flag[i] = GetBits(1);
	}

	if (max_num_sub_layers - 1 > 0)
		for (int i = max_num_sub_layers - 1; i < 8; i++)
			SkipBits(2); // reserved_zero_2bits[i]

	for (int i = 0; i < max_num_sub_layers - 1; i++) {
		if (sub_layer_profile_present_flag[i] && DecodePtl() < 0)
			return -1;

		if (GetBitsLeft() < 8)
			return -1;
		else
			SkipBits(8);
	}

	return 0;
}

int CParseHevcHeader::ParseSps()
{
	int ret = 0;
	const int MAX_VPS_COUNT = 16;
	const int MAX_SPS_COUNT = 32;
	int separate_colour_plane_flag = 0;

	// Coded parameters
	int vps_id = GetBits(4);
	if (vps_id >= MAX_VPS_COUNT) {
		return -1;
	}

	int max_sub_layers = GetBits(3) + 1;
	int temporal_id_nesting_flag = GetBits(1); // temporal_id_nesting_flag

	if ((ret = ParsePtl(max_sub_layers)) < 0)
		return ret;

	int sps_id = GetUe();
	if (sps_id >= MAX_SPS_COUNT) {
		return -1;
	}

	int chroma_format_idc = GetUe();
	if (chroma_format_idc == 3)
		separate_colour_plane_flag = GetBits(1);

	if (separate_colour_plane_flag)
		chroma_format_idc = 0;

	int width = GetUe();
	int height = GetUe();
	if (width < 0 || width > 8192 || height < 0 || height > 8192)
		return -1;

	int pic_conformance_flag = GetBits(1);
	if (pic_conformance_flag) { // pic_conformance_flag
		int vert_mult = 1 + (chroma_format_idc < 2);
		int horiz_mult = 1 + (chroma_format_idc < 3);

		int left_offset = GetUe() * horiz_mult;
		int right_offset = GetUe() * horiz_mult;
		int top_offset = GetUe() * vert_mult;
		int bottom_offset = GetUe() * vert_mult;
	}

	int bit_depth = GetUe() + 8;
	int bit_depth_chroma = GetUe() + 8;
	if (chroma_format_idc && bit_depth_chroma != bit_depth) {
		return -1;
	}

	m_width = width;
	m_height = height;

	return 0;
}

int CParseHevcHeader::DecodeNalUnit(uint8_t *data, int data_size)
{
	if (SplitPacket(data, data_size) > 0) {
		InitBits(m_sps, m_spsSize);
		if (GetBits(1) != 0)
			return -1;
		int nal_type = GetBits(6);
		int nuh_layer_id = GetBits(6);
		int temporal_id = GetBits(3) - 1;
		if (temporal_id < 0)
			return -1;

		return ParseSps();
	}

	return -1;
}

int CParseHevcHeader::DecodeExtradata(uint8_t *extradata, int extradata_size)
{
	int idx = 0;
	if (extradata_size > 3 &&
		(extradata[0] || extradata[1] || extradata[2] > 1)) {
		InitBits(extradata, extradata_size);
		SkipBits(21 * 8 + 6);

		int nal_len_size = GetBits(2) + 1;
		int num_arrays = GetBits(8);

		/* Decode nal units from hvcC. */
		for (int i = 0; i < num_arrays; i++) {
			SkipBits(2);
			int type = GetBits(6);
			int cnt = GetBits(16);

			for (int j = 0; j < cnt; j++) {
				// +2 for the nal size field
				uint32_t nalsize = PeekBits(16) + 2;

				if (GetBitsLeft() < (int)nalsize)
					return -1;

				//sps
				if (type == 33) {
					int ret = DecodeNalUnit(GetBuffer(), nalsize);

					return ret | GetBitError();
				}

				SkipBits(nalsize * 8);
			}
		}
	}

	return -1;
}



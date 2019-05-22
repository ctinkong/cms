#include "ParseH264Header.h"
#include "BitStream.h"
#include <stdlib.h>

CParseH264Header::CParseH264Header(void)
{
}

CParseH264Header::~CParseH264Header(void)
{
}

int CParseH264Header::ParseSps()
{
	int profile_idc = GetBits(8);
	int constraint_set0_flag = GetBits(1);
	int constraint_set1_flag = GetBits(1);
	int constraint_set2_flag = GetBits(1);
	int constraint_set3_flag = GetBits(1);
	int reserved_zero_4bits = GetBits(4);
	int level_idc = GetBits(8);
	int seq_parameter_set_id = GetUe();
	int chroma_format_idc = 1;

	if (profile_idc == 100 || profile_idc == 110 ||
		profile_idc == 122 || profile_idc == 144) {
		chroma_format_idc = GetUe();
		if (chroma_format_idc == 3) {
			int residual_colour_transform_flag = GetBits(1);
		}

		int bit_depth_luma_minus8 = GetUe();
		int bit_depth_chroma_minus8 = GetUe();
		int qpprime_y_zero_transform_bypass_flag = GetBits(1);
		int seq_scaling_matrix_present_flag = GetBits(1);

		int seq_scaling_list_present_flag[8];
		if (seq_scaling_matrix_present_flag) {
			for (int i = 0; i < 8; i++)
				seq_scaling_list_present_flag[i] = GetBits(1);
		}
	}

	int log2_max_frame_num_minus4 = GetUe();
	int pic_order_cnt_type = GetUe();
	if (pic_order_cnt_type == 0) {
		int log2_max_pic_order_cnt_lsb_minus4 = GetUe();
	}
	else if (pic_order_cnt_type == 1) {
		int delta_pic_order_always_zero_flag = GetBits(1);
		int offset_for_non_ref_pic = GetSe();
		int offset_for_top_to_bottom_field = GetSe();
		int num_ref_frames_in_pic_order_cnt_cycle = GetUe();

		int *offset_for_ref_frame = (int*)malloc(num_ref_frames_in_pic_order_cnt_cycle);
		for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
			offset_for_ref_frame[i] = GetSe();

		free(offset_for_ref_frame);
	}

	int num_ref_frames = GetUe();
	int gaps_in_frame_num_value_allowed_flag = GetBits(1);
	int pic_width_in_mbs_minus1 = GetUe();
	int pic_height_in_map_units_minus1 = GetUe();

	m_width = (pic_width_in_mbs_minus1 + 1) * 16;
	m_height = (pic_height_in_map_units_minus1 + 1) * 16;
	int frame_mbs_only_flag = GetBits(1);
	int mb_aff = 0;
	if (!frame_mbs_only_flag) {
		mb_aff = GetBits(1);
	}

	int direct_8x8_inference_flag = GetBits(1);
	int crop = GetBits(1);
	if (crop) {
		int crop_left = GetUe();
		int crop_right = GetUe();
		int crop_top = GetUe();
		int crop_bottom = GetUe();

		int vsub = (chroma_format_idc == 1) ? 1 : 0;
		int hsub = (chroma_format_idc == 1 ||
			chroma_format_idc == 2) ? 1 : 0;
		int step_x = 1 << hsub;
		int step_y = (2 - frame_mbs_only_flag) << vsub;

		m_width -= (crop_left + crop_right) * step_x;
		m_height -= (crop_top + crop_bottom) * step_y;
	}

	int vui_parameters_present_flag = GetBits(1);
	if (vui_parameters_present_flag) {
		unsigned int aspect_ratio_idc;
		int aspect_ratio_info_present_flag = GetBits(1);
		if (aspect_ratio_info_present_flag) {
			aspect_ratio_idc = GetBits(8);
			if (aspect_ratio_idc == 255) {
				int num = GetBits(16);
				int den = GetBits(16);
			}
		}

		if (GetBits(1))
			GetBits(1);

		int video_signal_type_present_flag = GetBits(1);
		if (video_signal_type_present_flag) {
			GetBits(3);    /* video_format */
			GetBits(1);		/* video_full_range_flag */

			int colour_description_present_flag = GetBits(1);
			if (colour_description_present_flag) {
				GetBits(8); /* colour_primaries */
				GetBits(8); /* transfer_characteristics */
				GetBits(8); /* matrix_coefficients */
			}
		}

		if (GetBits(1)) {
			int chroma_sample_location = GetUe() + 1;
			GetUe();
		}

		int timing_info_present_flag = GetBits(1);
		if (timing_info_present_flag) {
			int num_units_in_tick = GetBits(32);
			int time_scale = GetBits(32);
			int fixed_frame_rate_flag = GetBits(1);
			if (num_units_in_tick > 0 && time_scale > 0)
				m_fps = time_scale / double(num_units_in_tick) / 2;
		}
	}

	return 0;
}

int CParseH264Header::DecodeNalUnit(uint8_t *data, int data_size)
{
	if (SplitPacket(data, data_size) > 0) {
		InitBits(m_sps, m_spsSize);
		if (GetBits(1) != 0)
			return -1;

		int ref_idc = GetBits(2);
		int nal_type = GetBits(5);

		return ParseSps();
	}

	return -1;
}

int CParseH264Header::DecodeExtradata(uint8_t *buf, int size)
{
	if (!buf || size <= 0)
		return -1;

	if (buf[0] == 1) {
		int i, cnt, nalsize;
		uint8_t *p = buf;

		if (size < 7) {
			return 0;
		}
		// Decode sps from avcC
		cnt = *(p + 5) & 0x1f; // Number of sps
		p += 6;
		for (i = 0; i < cnt; i++) {
			nalsize = FPS_RB16(p) + 2;
			if (nalsize > size - (p - buf))
				return -1;

			int ret = DecodeNalUnit(p, nalsize);

			return ret | GetBitError();
		}
	}

	return -1;
}




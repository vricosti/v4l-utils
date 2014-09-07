/*
 * Copyright (c) 2011-2012 - Mauro Carvalho Chehab
 * Copyright (c) 2012-2014 - Andre Roth <neolynx@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file descriptors.h
 * @brief Provides a way to handle MPEG-TS descriptors found on Digital TV
 * 	streams.
 * @copyright GNU General Public License version 2 (GPLv2)
 * @author Mauro Carvalho Chehab
 * @author Andre Roth
 *
 * The descriptors are defined on:
 * - ISO/IEC 13818-1
 * - ETSI EN 300 468 V1.11.1 (2010-04)
 * - SCTE 35 2004
 * - http://www.etherguidesystems.com/Help/SDOs/ATSC/Semantics/Descriptors/Default.aspx
 * - http://www.coolstf.com/tsreader/descriptors.html
 * - ABNT NBR 15603-1 2007
 * - ATSC A/65:2009 spec
 *
 * Please submit bug report and patches to linux-media@vger.kernel.org
 */


#ifndef _DESCRIPTORS_H
#define _DESCRIPTORS_H

#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>

/** @brief Maximum size of a table session to be parsed */
#define DVB_MAX_PAYLOAD_PACKET_SIZE 4096

/** @brief number of bytes for the descriptor's CRC check */
#define DVB_CRC_SIZE 4


#ifndef _DOXYGEN
struct dvb_v5_fe_parms;
#endif

/** @brief Function prototype for a function that initializes the descriptors parsing */
typedef void (*dvb_table_init_func)(struct dvb_v5_fe_parms *parms, const uint8_t *buf, ssize_t buflen, void **table);

/** @brief Table with all possible descriptors */
extern const dvb_table_init_func dvb_table_initializers[256];

#ifndef _DOXYGEN
#define bswap16(b) do {\
	b = ntohs(b); \
} while (0)

#define bswap32(b) do {\
	b = ntohl(b); \
} while (0)

#define DVB_DESC_HEADER() \
	uint8_t type; \
	uint8_t length; \
	struct dvb_desc *next

#endif /* _DOXYGEN */

/**
 * @struct dvb_desc
 * @brief Linked list containing the several descriptors found on a
 * 	  MPEG-TS table
 *
 * @param type		Descriptor type
 * @param length	Length of the descriptor
 * @param next		pointer to the next descriptor
 * @param data		Descriptor data
 */

struct dvb_desc {
	DVB_DESC_HEADER();

	uint8_t data[];
} __attribute__((packed));

#ifndef _DOXYGEN

#define dvb_desc_foreach( _desc, _tbl ) \
	for( struct dvb_desc *_desc = _tbl->descriptor; _desc; _desc = _desc->next ) \

#define dvb_desc_find(_struct, _desc, _tbl, _type) \
	for( _struct *_desc = (_struct *) _tbl->descriptor; _desc; _desc = (_struct *) _desc->next ) \
		if(_desc->type == _type) \

#endif /* _DOXYGEN */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Converts from BCD to CPU integer internal representation
 *
 * @param bcd	value in BCD encoding
 */
uint32_t dvb_bcd(uint32_t bcd);

/**
 * @brief dumps data into the logs in hexadecimal format
 *
 * @param parms			Struct dvb_v5_fe_parms pointer
 * @param prefix	String to be printed before the dvb_hexdump
 * @param buf		Buffer to hex dump
 * @param len		Number of bytes to show
 */
void dvb_hexdump(struct dvb_v5_fe_parms *parms, const char *prefix,
		 const unsigned char *buf, int len);

/**
 * @brief parse MPEG-TS descriptors
 *
 * @param parms		Struct dvb_v5_fe_parms pointer
 * @param buf		Buffer with data to be parsed
 * @param buflen	Size of the buffer to be parsed
 * @param head_desc	pointer to the place to store the parsed data
 *
 * This function takes a buf as argument and parses it to find the
 * MPEG-TS descriptors inside it, creating a linked list.
 *
 * On success, head_desc will be allocated and filled with a linked list
 * with the descriptors found inside the buffer.
 *
 * This function is used by the several MPEG-TS table handlers to parse
 * the entire table that got read by dvb_read_sessions and other similar
 * functions.
 *
 * @return Returns 0 on success, a negative value otherwise.
 */
int  dvb_desc_parse(struct dvb_v5_fe_parms *parms, const uint8_t *buf,
		    uint16_t buflen, struct dvb_desc **head_desc);

/**
 * @brief frees a dvb_desc linked list
 *
 * @param list	struct dvb_desc pointer.
 */
void dvb_desc_free (struct dvb_desc **list);

/**
 * @brief prints the contents of a struct dvb_desc linked list
 *
 * @param parms		Struct dvb_v5_fe_parms pointer
 * @param desc	struct dvb_desc pointer.
 */
void dvb_desc_print(struct dvb_v5_fe_parms *parms, struct dvb_desc *desc);

#ifdef __cplusplus
}
#endif

/** @brief Function prototype for the descriptors parsing init code */
typedef int (*dvb_desc_init_func) (struct dvb_v5_fe_parms *parms, const uint8_t *buf, struct dvb_desc *desc);

/** @brief Function prototype for the descriptors parsing print code */
typedef void (*dvb_desc_print_func)(struct dvb_v5_fe_parms *parms, const struct dvb_desc *desc);

/** @brief Function prototype for the descriptors memory free code */
typedef void (*dvb_desc_free_func) (struct dvb_desc *desc);

/**
 * @struct dvb_descriptor
 * @brief Contains the parser information for the MPEG-TS parser code
 *
 * @param name		String containing the name of the descriptor
 * @param init		Pointer to a function to initialize the descriptor
 *			parser. This function fills the descriptor-specific
 *			internal structures
 * @param print		Prints the content of the descriptor
 * @param free		Frees all memory blocks allocated by the init function
 * @param size		Descriptor's size, in bytes.
 */
struct dvb_descriptor {
	const char *name;
	dvb_desc_init_func init;
	dvb_desc_print_func print;
	dvb_desc_free_func free;
	ssize_t size;
};

/**
 * @brief Contains the parsers for the several descriptors
 */
extern const struct dvb_descriptor dvb_descriptors[];

/**
 * @brief List containing all descriptors used by Digital TV MPEG-TS
 */
enum descriptors {
	/* ISO/IEC 13818-1 */
	video_stream_descriptor				= 0x02,
	audio_stream_descriptor				= 0x03,
	hierarchy_descriptor				= 0x04,
	registration_descriptor				= 0x05,
	ds_alignment_descriptor				= 0x06,
	target_background_grid_descriptor		= 0x07,
	video_window_descriptor				= 0x08,
	conditional_access_descriptor			= 0x09,
	iso639_language_descriptor			= 0x0a,
	system_clock_descriptor				= 0x0b,
	multiplex_buffer_utilization_descriptor		= 0x0c,
	copyright_descriptor				= 0x0d,
	maximum_bitrate_descriptor			= 0x0e,
	private_data_indicator_descriptor		= 0x0f,
	smoothing_buffer_descriptor			= 0x10,
	std_descriptor					= 0x11,
	ibp_descriptor					= 0x12,

	mpeg4_video_descriptor				= 0x1b,
	mpeg4_audio_descriptor				= 0x1c,
	iod_descriptor					= 0x1d,
	sl_descriptor					= 0x1e,
	fmc_descriptor					= 0x1f,
	external_es_id_descriptor			= 0x20,
	muxcode_descriptor				= 0x21,
	fmxbuffersize_descriptor			= 0x22,
	multiplexbuffer_descriptor			= 0x23,
	content_labeling_descriptor			= 0x24,
	metadata_pointer_descriptor			= 0x25,
	metadata_descriptor				= 0x26,
	metadata_std_descriptor				= 0x27,
	AVC_video_descriptor				= 0x28,
	ipmp_descriptor					= 0x29,
	AVC_timing_and_HRD_descriptor			= 0x2a,
	mpeg2_aac_audio_descriptor			= 0x2b,
	flexmux_timing_descriptor			= 0x2c,

	/* ETSI EN 300 468 V1.11.1 (2010-04) */

	network_name_descriptor				= 0x40,
	service_list_descriptor				= 0x41,
	stuffing_descriptor				= 0x42,
	satellite_delivery_system_descriptor		= 0x43,
	cable_delivery_system_descriptor		= 0x44,
	VBI_data_descriptor				= 0x45,
	VBI_teletext_descriptor				= 0x46,
	bouquet_name_descriptor				= 0x47,
	service_descriptor				= 0x48,
	country_availability_descriptor			= 0x49,
	linkage_descriptor				= 0x4a,
	NVOD_reference_descriptor			= 0x4b,
	time_shifted_service_descriptor			= 0x4c,
	short_event_descriptor				= 0x4d,
	extended_event_descriptor			= 0x4e,
	time_shifted_event_descriptor			= 0x4f,
	component_descriptor				= 0x50,
	mosaic_descriptor				= 0x51,
	stream_identifier_descriptor			= 0x52,
	CA_identifier_descriptor			= 0x53,
	content_descriptor				= 0x54,
	parental_rating_descriptor			= 0x55,
	teletext_descriptor				= 0x56,
	telephone_descriptor				= 0x57,
	local_time_offset_descriptor			= 0x58,
	subtitling_descriptor				= 0x59,
	terrestrial_delivery_system_descriptor		= 0x5a,
	multilingual_network_name_descriptor		= 0x5b,
	multilingual_bouquet_name_descriptor		= 0x5c,
	multilingual_service_name_descriptor		= 0x5d,
	multilingual_component_descriptor		= 0x5e,
	private_data_specifier_descriptor		= 0x5f,
	service_move_descriptor				= 0x60,
	short_smoothing_buffer_descriptor		= 0x61,
	frequency_list_descriptor			= 0x62,
	partial_transport_stream_descriptor		= 0x63,
	data_broadcast_descriptor			= 0x64,
	scrambling_descriptor				= 0x65,
	data_broadcast_id_descriptor			= 0x66,
	transport_stream_descriptor			= 0x67,
	DSNG_descriptor					= 0x68,
	PDC_descriptor					= 0x69,
	AC_3_descriptor					= 0x6a,
	ancillary_data_descriptor			= 0x6b,
	cell_list_descriptor				= 0x6c,
	cell_frequency_link_descriptor			= 0x6d,
	announcement_support_descriptor			= 0x6e,
	application_signalling_descriptor		= 0x6f,
	adaptation_field_data_descriptor		= 0x70,
	service_identifier_descriptor			= 0x71,
	service_availability_descriptor			= 0x72,
	default_authority_descriptor			= 0x73,
	related_content_descriptor			= 0x74,
	TVA_id_descriptor				= 0x75,
	content_identifier_descriptor			= 0x76,
	time_slice_fec_identifier_descriptor		= 0x77,
	ECM_repetition_rate_descriptor			= 0x78,
	S2_satellite_delivery_system_descriptor		= 0x79,
	enhanced_AC_3_descriptor			= 0x7a,
	DTS_descriptor					= 0x7b,
	AAC_descriptor					= 0x7c,
	XAIT_location_descriptor			= 0x7d,
	FTA_content_management_descriptor		= 0x7e,
	extension_descriptor				= 0x7f,

	/* SCTE 35 2004 */
	CUE_identifier_descriptor			= 0x8a,

	extended_channel_name				= 0xa0,
	service_location				= 0xa1,
	/* From http://www.etherguidesystems.com/Help/SDOs/ATSC/Semantics/Descriptors/Default.aspx */
	component_name_descriptor			= 0xa3,

	/* From http://www.coolstf.com/tsreader/descriptors.html */
	logical_channel_number_descriptor		= 0x83,

	/* ISDB Descriptors, as defined on ABNT NBR 15603-1 2007 */

	carousel_id_descriptor				= 0x13,
	association_tag_descriptor			= 0x14,
	deferred_association_tags_descriptor		= 0x15,

	hierarchical_transmission_descriptor		= 0xc0,
	digital_copy_control_descriptor			= 0xc1,
	network_identifier_descriptor			= 0xc2,
	partial_transport_stream_time_descriptor	= 0xc3,
	audio_component_descriptor			= 0xc4,
	hyperlink_descriptor				= 0xc5,
	target_area_descriptor				= 0xc6,
	data_contents_descriptor			= 0xc7,
	video_decode_control_descriptor			= 0xc8,
	download_content_descriptor			= 0xc9,
	CA_EMM_TS_descriptor				= 0xca,
	CA_contract_information_descriptor		= 0xcb,
	CA_service_descriptor				= 0xcc,
	TS_Information_descriptior			= 0xcd,
	extended_broadcaster_descriptor			= 0xce,
	logo_transmission_descriptor			= 0xcf,
	basic_local_event_descriptor			= 0xd0,
	reference_descriptor				= 0xd1,
	node_relation_descriptor			= 0xd2,
	short_node_information_descriptor		= 0xd3,
	STC_reference_descriptor			= 0xd4,
	series_descriptor				= 0xd5,
	event_group_descriptor				= 0xd6,
	SI_parameter_descriptor				= 0xd7,
	broadcaster_Name_Descriptor			= 0xd8,
	component_group_descriptor			= 0xd9,
	SI_prime_TS_descriptor				= 0xda,
	board_information_descriptor			= 0xdb,
	LDT_linkage_descriptor				= 0xdc,
	connected_transmission_descriptor		= 0xdd,
	content_availability_descriptor			= 0xde,
	service_group_descriptor			= 0xe0,
	carousel_compatible_composite_descriptor	= 0xf7,
	conditional_playback_descriptor			= 0xf8,
	ISDBT_delivery_system_descriptor		= 0xfa,
	partial_reception_descriptor			= 0xfb,
	emergency_information_descriptor		= 0xfc,
	data_component_descriptor			= 0xfd,
	system_management_descriptor			= 0xfe,

	/* ATSC descriptors - ATSC A/65:2009 spec */
	atsc_stuffing_descriptor			= 0x80,
	atsc_ac3_audio_descriptor			= 0x81,
	atsc_caption_service_descriptor			= 0x86,
	atsc_content_advisory_descriptor		= 0x87,
	atsc_extended_channel_descriptor		= 0xa0,
	atsc_service_location_descriptor		= 0xa1,
	atsc_time_shifted_service_descriptor		= 0xa2,
	atsc_component_name_descriptor			= 0xa3,
	atsc_DCC_departing_request_descriptor		= 0xa8,
	atsc_DCC_arriving_request_descriptor		= 0xa9,
	atsc_redistribution_control_descriptor		= 0xaa,
	atsc_ATSC_private_information_descriptor	= 0xad,
	atsc_genre_descriptor				= 0xab,
};

/* Please see desc_extension.h for extension_descriptor types */

#endif

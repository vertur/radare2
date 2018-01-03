/* radare - LGPL - Copyright 2009-2015 - pancake, nibble */
#include <stddef.h>

#include "r_cons.h"
#include "r_core.h"
#include "r_hash.h"
#include "r_types_base.h"

typedef void (*HashHandler)(const ut8 *block, int len);

static void handle_md4 (const ut8 *block, int len);
static void handle_md5 (const ut8 *block, int len);
static void handle_sha1 (const ut8 *block, int len);
static void handle_sha256 (const ut8 *block, int len);
static void handle_sha512 (const ut8 *block, int len);
static void handle_adler32 (const ut8 *block, int len);
static void handle_xor (const ut8 *block, int len);
static void handle_entropy (const ut8 *block, int len);
static void handle_hamdist (const ut8 *block, int len);
static void handle_parity (const ut8 *block, int len);
static void handle_pcprint (const ut8 *input, int len);
static void handle_mod255 (const ut8 *input, int len);
static void handle_luhn (const ut8 *input, int len);
static void handle_crc8_smbus (const ut8 *block, int len);
static void handle_crc8_cdma2000 (const ut8 *block, int len);
static void handle_crc8_darc (const ut8 *block, int len);
static void handle_crc8_dvb_s2 (const ut8 *block, int len);
static void handle_crc8_ebu (const ut8 *block, int len);
static void handle_crc8_icode (const ut8 *block, int len);
static void handle_crc8_itu (const ut8 *block, int len);
static void handle_crc8_maxim (const ut8 *block, int len);
static void handle_crc8_rohc (const ut8 *block, int len);
static void handle_crc8_wcdma (const ut8 *block, int len);
static void handle_crc15_can (const ut8 *block, int len);
static void handle_crc16 (const ut8 *block, int len);
static void handle_crc16_hdlc (const ut8 *block, int len);
static void handle_crc16_usb (const ut8 *block, int len);
static void handle_crc16_citt (const ut8 *block, int len);
static void handle_crc16_aug_ccitt (const ut8 *block, int len);
static void handle_crc16_buypass (const ut8 *block, int len);
static void handle_crc16_cdma2000 (const ut8 *block, int len);
static void handle_crc16_dds110 (const ut8 *block, int len);
static void handle_crc16_dect_r (const ut8 *block, int len);
static void handle_crc16_dect_x (const ut8 *block, int len);
static void handle_crc16_dnp (const ut8 *block, int len);
static void handle_crc16_en13757 (const ut8 *block, int len);
static void handle_crc16_genibus (const ut8 *block, int len);
static void handle_crc16_maxim (const ut8 *block, int len);
static void handle_crc16_mcrf4xx (const ut8 *block, int len);
static void handle_crc16_riello (const ut8 *block, int len);
static void handle_crc16_t10dif (const ut8 *block, int len);
static void handle_crc16_teledisk (const ut8 *block, int len);
static void handle_crc16_tms37157 (const ut8 *block, int len);
static void handle_crca (const ut8 *block, int len);
static void handle_crc16_kermit (const ut8 *block, int len);
static void handle_crc16_modbus (const ut8 *block, int len);
static void handle_crc16_x25 (const ut8 *block, int len);
static void handle_crc16_xmodem (const ut8 *block, int len);
static void handle_crc24 (const ut8 *block, int len);
static void handle_crc32 (const ut8 *block, int len);
static void handle_crc32c (const ut8 *block, int len);
static void handle_crc32_ecma_267 (const ut8 *block, int len);
static void handle_crc32_bzip2 (const ut8 * block, int len);
static void handle_crc32d (const ut8 * block, int len);
static void handle_crc32_mpeg2 (const ut8 * block, int len);
static void handle_crc32_posix (const ut8 * block, int len);
static void handle_crc32q (const ut8 * block, int len);
static void handle_crc32_jamcrc (const ut8 * block, int len);
static void handle_crc32_xfer (const ut8 * block, int len);

typedef struct {
	const char *name;
	HashHandler handler;
} RHashHashHandlers;

static RHashHashHandlers hash_handlers[] = {
	{"md4", handle_md4},
	{"md5", handle_md5},
	{"sha1", handle_sha1},
	{"sha256", handle_sha256},
	{"sha512", handle_sha512},
	{"adler32", handle_adler32},
	{"xor", handle_xor},
	{"entropy", handle_entropy},
	{"parity", handle_parity},
	{"hamdist", handle_hamdist},
	{"pcprint", handle_pcprint},
	{"mod255", handle_mod255},
	{"luhn", handle_luhn},
	{"crc8smbus", handle_crc8_smbus},
	{ /* CRC-8/CDMA2000     */ "crc8cdma2000", handle_crc8_cdma2000},
	{ /* CRC-8/DARC         */ "crc8darc", handle_crc8_darc},
	{ /* CRC-8/DVB-S2       */ "crc8dvbs2", handle_crc8_dvb_s2},
	{ /* CRC-8/EBU          */ "crc8ebu", handle_crc8_ebu},
	{ /* CRC-8/I-CODE       */ "crc8icode", handle_crc8_icode},
	{ /* CRC-8/ITU          */ "crc8itu", handle_crc8_itu},
	{ /* CRC-8/MAXIM        */ "crc8maxim", handle_crc8_maxim},
	{ /* CRC-8/ROHC         */ "crc8rohc", handle_crc8_rohc},
	{ /* CRC-8/WCDMA        */ "crc8wcdma", handle_crc8_wcdma},
	{"crc15can", handle_crc15_can},
	{"crc16", handle_crc16},
	{"crc16hdlc", handle_crc16_hdlc},
	{ /* CRC-16/USB         */ "crc16usb", handle_crc16_usb},
	{ /* CRC-16/CCITT-FALSE */ "crc16citt", handle_crc16_citt},
	{ /* CRC-16/AUG-CCITT   */ "crc16augccitt", handle_crc16_aug_ccitt },
	{ /* CRC-16/BUYPASS     */ "crc16buypass", handle_crc16_buypass },
	{ /* CRC-16/CDMA2000    */ "crc16cdma2000", handle_crc16_cdma2000 },
	{ /* CRC-16/DDS-110     */ "crc16dds110", handle_crc16_dds110 },
	{ /* CRC-16/RECT-R      */ "crc16dectr", handle_crc16_dect_r },
	{ /* CRC-16/RECT-X      */ "crc16dectx", handle_crc16_dect_x },
	{ /* CRC-16/DNP         */ "crc16dnp", handle_crc16_dnp },
	{ /* CRC-16/EN-13757    */ "crc16en13757", handle_crc16_en13757 },
	{ /* CRC-16/GENIBUS     */ "crc16genibus", handle_crc16_genibus },
	{ /* CRC-16/MAXIM       */ "crc16maxim", handle_crc16_maxim },
	{ /* CRC-16/MCRF4XX     */ "crc16mcrf4xx", handle_crc16_mcrf4xx },
	{ /* CRC-16/RIELLO      */ "crc16riello", handle_crc16_riello },
	{ /* CRC-16/T10-DIF     */ "crc16t10dif", handle_crc16_t10dif },
	{ /* CRC-16/TELEDISK    */ "crc16teledisk", handle_crc16_teledisk },
	{ /* CRC-16/TMS37157    */ "crc16tms37157", handle_crc16_tms37157 },
	{ /* CRC-A              */ "crca", handle_crca },
	{ /* CRC-16/KERMIT      */ "crc16kermit", handle_crc16_kermit },
	{ /* CRC-16/MODBUS      */ "crc16modbus", handle_crc16_modbus },
	{ /* CRC-16/X-25        */ "crc16x25", handle_crc16_x25 },
	{ /* CRC-16/XMODEM      */ "crc16xmodem", handle_crc16_xmodem },
	{"crc24", handle_crc24},
	{"crc32", handle_crc32},
	{"crc32c", handle_crc32c},
	{"crc32ecma267", handle_crc32_ecma_267},
	{ /* CRC-32/BZIP2       */ "crc32bzip2", handle_crc32_bzip2 },
	{ /* CRC-32D            */ "crc32d", handle_crc32d },
	{ /* CRC-32/MPEG-2      */ "crc32mpeg2", handle_crc32_mpeg2 },
	{ /* CRC-32/POSIX       */ "crc32posix", handle_crc32_posix },
	{ /* CRC-32Q            */ "crc32q", handle_crc32q },
	{ /* CRC-32/JAMCRC      */ "crc32jamcrc", handle_crc32_jamcrc },
	{ /* CRC-32/XFER        */ "crc32xfer", handle_crc32_xfer },
	{NULL, NULL},
};

static void handle_md4 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (true, R_HASH_MD4);
	const ut8 *c = r_hash_do_md4 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_MD4; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_md5 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (true, R_HASH_MD5);
	const ut8 *c = r_hash_do_md5 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_MD5; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_sha1 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (true, R_HASH_SHA1);
	const ut8 *c = r_hash_do_sha1 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_SHA1; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_sha256 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (true, R_HASH_SHA256);
	const ut8 *c = r_hash_do_sha256 (ctx, block, len);
	for (i=0; i<R_HASH_SIZE_SHA256; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_sha512 (const ut8 *block, int len) {
	int i = 0;
	RHash *ctx = r_hash_new (true, R_HASH_SHA512);
	const ut8 *c = r_hash_do_sha512 (ctx, block, len);
	for (i = 0; i < R_HASH_SIZE_SHA512; i++) r_cons_printf ("%02x", c[i]);
	r_cons_newline ();
	r_hash_free (ctx);
}

static void handle_adler32 (const ut8 *block, int len) {
	ut32 hn = r_hash_adler32 (block, len);
	ut8 *b = (ut8*)&hn;
	r_cons_printf ("%02x%02x%02x%02x\n", b[0], b[1], b[2], b[3]);
}

static void handle_xor (const ut8 *block, int len) {
	r_cons_printf ("%02x\n", r_hash_xor (block, len));
}

static void handle_entropy (const ut8 *block, int len) {
	r_cons_printf ("%f\n", r_hash_entropy (block, len));
}

static void handle_parity (const ut8 *block, int len) {
	r_cons_printf ("%d\n", r_hash_parity (block, len)?1:0);
}

static void handle_hamdist (const ut8 *block, int len) {
	r_cons_printf ("%02x\n", r_hash_hamdist (block, len));
}

static void handle_pcprint (const ut8 *block, int len) {
	r_cons_printf ("%d\n", r_hash_pcprint (block, len));
	//r_cons_printf ("%02x\n", r_hash_pcprint (block, len));
}

static void handle_mod255 (const ut8 *block, int len) {
	r_cons_printf ("%d\n", r_hash_mod255 (block, len));
	//r_cons_printf ("%02x\n", r_hash_pcprint (block, len));
}

static void handle_luhn (const ut8 *block, int len) {
	r_cons_printf ("%d\n", r_hash_luhn (block, len));
}

static void handle_crc8_smbus (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_8_SMBUS));
}

static void handle_crc8_cdma2000 (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_CDMA2000));
}

static void handle_crc8_darc (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_DARC));
}

static void handle_crc8_dvb_s2 (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_DVB_S2));
}

static void handle_crc8_ebu (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_EBU));
}

static void handle_crc8_icode (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_ICODE));
}

static void handle_crc8_itu (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_ITU));
}

static void handle_crc8_maxim (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_MAXIM));
}

static void handle_crc8_rohc (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_ROHC));
}

static void handle_crc8_wcdma (const ut8 *block, int len) {
	r_cons_printf ("%02" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC8_WCDMA));
}

static void handle_crc15_can (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_15_CAN));
}

static void handle_crc16 (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_16));
}

static void handle_crc16_hdlc (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_16_HDLC));
}

static void handle_crc16_usb (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_16_USB));
}

static void handle_crc16_citt (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_16_CITT));
}

static void handle_crc16_aug_ccitt (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_AUG_CCITT));
}

static void handle_crc16_buypass (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_BUYPASS));
}

static void handle_crc16_cdma2000 (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_CDMA2000));
}

static void handle_crc16_dds110 (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_DDS110));
}

static void handle_crc16_dect_r (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_DECT_R));
}

static void handle_crc16_dect_x (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_DECT_X));
}

static void handle_crc16_dnp (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_DNP));
}

static void handle_crc16_en13757 (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_EN13757));
}

static void handle_crc16_genibus (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_GENIBUS));
}

static void handle_crc16_maxim (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_MAXIM));
}

static void handle_crc16_mcrf4xx (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_MCRF4XX));
}

static void handle_crc16_riello (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_RIELLO));
}

static void handle_crc16_t10dif (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_T10_DIF));
}

static void handle_crc16_teledisk (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_TELEDISK));
}

static void handle_crc16_tms37157 (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_TMS37157));
}

static void handle_crca (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRCA));
}

static void handle_crc16_kermit (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_KERMIT));
}

static void handle_crc16_modbus (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_MODBUS));
}

static void handle_crc16_x25 (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_X25));
}

static void handle_crc16_xmodem (const ut8 *block, int len) {
	r_cons_printf ("%04" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC16_XMODEM));
}

static void handle_crc24 (const ut8 *block, int len) {
	r_cons_printf ("%06" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_24));
}

static void handle_crc32 (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_32));
}

static void handle_crc32c (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_32C));
}

static void handle_crc32_ecma_267 (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_32_ECMA_267));
}

static void handle_crc32_bzip2 (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC32_BZIP2));
}

static void handle_crc32d (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC32D));
}

static void handle_crc32_mpeg2 (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC32_MPEG2));
}

static void handle_crc32_posix (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC32_POSIX));
}

static void handle_crc32q (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC32Q));
}

static void handle_crc32_jamcrc (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC32_JAMCRC));
}

static void handle_crc32_xfer (const ut8 *block, int len) {
	r_cons_printf ("%08" PFMTCRCx "\n", r_hash_crc_preset (block, len, CRC_PRESET_CRC32_XFER));
}

static int cmd_hash_bang (RCore *core, const char *input) {
	char *p;
	const char *lang = input+1;
	if (r_sandbox_enable (0)) {
		eprintf ("hashbang disabled in sandbox mode\n");
		return false;
	}
	if (*lang=='/') {
		const char *ptr = lang+1;
		while (*lang) {
			if (*lang=='/')
				ptr = lang+1;
			lang++;
		}
		RLangPlugin *p = r_lang_get_by_extension (core->lang, ptr);
		if (p && p->name) lang = p->name;
	}
	if (*lang==' ') {
		RLangPlugin *p = r_lang_get_by_extension (core->lang, input+2);
		if (p && p->name) lang = p->name;
	} else if (input[1]=='?' || input[1]=='*' || input[1]=='\0') {
		r_lang_list (core->lang);
		return true;
	}
	p = strchr (input, ' ');
	bool doEval = false;
	if (p) {
		*p++ = 0;
		char *_e = strstr (p, "-e");
		if (_e) {
			doEval = true;
			p = _e + 2;
			p = r_str_chop (p);
		}
	}
	// TODO: set argv here
	if (r_lang_use (core->lang, lang)) {
		r_lang_setup (core->lang);
		if (p) {
			if (doEval) {
				r_lang_run_string (core->lang, p);
			} else {
				r_lang_run_file (core->lang, p);
			}
		} else {
			if (r_config_get_i (core->config, "scr.interactive")) {
				r_lang_prompt (core->lang);
			} else eprintf ("Error: scr.interactive required to run the rlang prompt\n");
		}
	} else {
		eprintf ("Invalid hashbang. See '#!' for help.\n");
	}
	return true;
}

static int cmd_hash(void *data, const char *input) {
	RCore *core = (RCore *)data;

	if (*input == '!') {
		return cmd_hash_bang (core, input);
	}
	if (*input == '?') {
		const char *helpmsg3[] = {
		"Usage #!interpreter [<args>] [<file] [<<eof]","","",
		" #", "", "comment - do nothing",
		" #!","","list all available interpreters",
		" #!python","","run python commandline",
		" #!python"," foo.py","run foo.py python script (same as '. foo.py')",
		//" #!python <<EOF        get python code until 'EOF' mark\n"
		" #!python"," arg0 a1 <<q","set arg0 and arg1 and read until 'q'",
		NULL};
		r_core_cmd_help (core, helpmsg3);
		return false;
	}
	/* this is a comment - captain obvious
	   should not be reached, see r_core_cmd_subst() */
	return 0;
}

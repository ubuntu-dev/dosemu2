#include "translate.h"

static const t_unicode cp852_c1_chars[] = {
0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x016f, 0x0107, 0x00e7, /* 0x80-0x87 */
0x0142, 0x00eb, 0x0150, 0x0151, 0x00ee, 0x0179, 0x00c4, 0x0106, /* 0x88-0x8F */
0x00c9, 0x0139, 0x013a, 0x00f4, 0x00f6, 0x013d, 0x013e, 0x015a, /* 0x90-0x97 */
0x015b, 0x00d6, 0x00dc, 0x0164, 0x0165, 0x0141, 0x00d7, 0x010d, /* 0x98-0x9F */
};
struct char_set cp852_c1 = {
	1,
	CHARS(cp852_c1_chars),
	0, "", 0, 32,
};

static const t_unicode cp852_g1_chars[] = {
0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x0104, 0x0105, 0x017d, 0x017e, /* 0xA0-0xA7 */
0x0118, 0x0119, 0x00ac, 0x017a, 0x010c, 0x015f, 0x00ab, 0x00bb, /* 0xA8-0xAF */
0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x00c1, 0x00c2, 0x011a, /* 0xB0-0xB7 */
0x015e, 0x2563, 0x2551, 0x2557, 0x255d, 0x017b, 0x017c, 0x2510, /* 0xB8-0xBF */
0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x0102, 0x0103, /* 0xC0-0xC7 */
0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x00a4, /* 0xC8-0xCF */
0x0111, 0x0110, 0x010e, 0x00cb, 0x010f, 0x0147, 0x00cd, 0x00ce, /* 0xD0-0xD7 */
0x011b, 0x2518, 0x250c, 0x2588, 0x2584, 0x0162, 0x016e, 0x2580, /* 0xD8-0xDF */
0x00d3, 0x00df, 0x00d4, 0x0143, 0x0144, 0x0148, 0x0160, 0x0161, /* 0xE0-0xE7 */
0x0154, 0x00da, 0x0155, 0x0170, 0x00fd, 0x00dd, 0x0163, 0x00b4, /* 0xE8-0xEF */
0x00ad, 0x02dd, 0x02db, 0x02c7, 0x02d8, 0x00a7, 0x00f7, 0x00b8, /* 0xF0-0xF7 */
0x00b0, 0x00a8, 0x02d9, 0x0171, 0x0158, 0x0159, 0x25a0, 0x00a0, /* 0xF8-0xFF */
};
struct char_set cp852_g1 = {
	1,
	CHARS(cp852_g1_chars),
	0, "", 0, 96,
};

struct char_set cp852 = {
	.c0 = &ibm_ascii_c0,
	.g0 = &ibm_ascii_g0,
	.c1 = &cp852_c1,
	.g1 = &cp852_g1,
	.names = { "cp852", 0 },
};

CONSTRUCTOR(static void init(void))
{
	register_charset(&cp852);
}

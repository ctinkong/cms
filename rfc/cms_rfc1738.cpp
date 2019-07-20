#include <rfc/cms_rfc1738.h>
#include <mem/cms_mf_mem.h>
#include <string.h>
#include <stddef.h>

static char rfc1738_unsafe_chars[] = {
	(char)0x3C,                            /* < */
	(char)0x3E,                            /* > */
	(char)0x22,                            /* " */
	(char)0x23,                            /* # */
	(char)0x7B,                            /* { */
	(char)0x7D,                            /* } */
	(char)0x7C,                            /* | */
	(char)0x5C,                            /* \ */
	(char)0x5E,                            /* ^ */
	(char)0x7E,                            /* ~ */
	(char)0x5B,                            /* [ */
	(char)0x5D,                            /* ] */
	(char)0x60,                            /* ` */
	(char)0x27,                            /* ' */
	(char)0x20                             /* space */
};

char* rfc1738_xcalloc_by_url(const char *url)
{
	char *buf = (char *)xcalloc(strlen(url) * 3 + 1, 1);
	return buf;
}

uint32 rfc1738_do_escape(const char *url, char *escaped_url)
{
	const char *p;
	char *q;
	unsigned int i, do_escape;
	uint32 len = 0;

	for (p = url, q = escaped_url; *p != '\0'; p++, q++)
	{
		do_escape = 0;

		do {

			/* RFC 1738 defines these chars as unsafe */
			for (i = 0; i < sizeof(rfc1738_unsafe_chars); i++) {
				if (*p == rfc1738_unsafe_chars[i]) {
					do_escape = 1;
					break;
				}
			}

			if (do_escape)
				break;

			/* RFC 1738 says any control chars (0x00-0x1F) are encoded */
			if ((unsigned char)*p <= (unsigned char)0x1F) {
				do_escape = 1;
				break;
			}
			/* RFC 1738 says 0x7f is encoded */
			if (*p == (char)0x7F) {
				do_escape = 1;
				break;
			}
			/* RFC 1738 says any non-US-ASCII are encoded */
			if (((unsigned char)*p >= (unsigned char)0x80)) {
				do_escape = 1;
				break;
			}

		} while (false);

		/* Do the triplet encoding, or just copy the char */
		/* note: we do not need snprintf here as q is appropriately
		 * allocated - KA */
		if (do_escape == 1)
		{
			snprintf(q, q - escaped_url, "%%%02X", (unsigned char)*p);
			q += sizeof(char) * 2;
			len += sizeof(char) * 2;
		}
		else
		{
			*q = *p;
			len += 1;
		}
	}
	*q = '\0';

	return len;
}

#define MAKECRCH
#define DYNAMIC_CRC_TABLE

#include <stdio.h>

#include "crc32.c"


static void write_table(FILE *out, const z_crc_t FAR *table)
{
    int n;

    for (n = 0; n < 256; n++)
        fprintf(out, "%s0x%08lxUL%s", n % 5 ? "" : "    ",
                (unsigned long)(table[n]),
                n == 255 ? "\n" : (n % 5 == 4 ? ",\n" : ", "));
}


#define filename "crc32.h"

int main(void)
{
	make_crc_table();

    /* write out CRC tables to crc32.h */
    {
        FILE *out;

        out = fopen(filename, "w");
        if (out == NULL)
        {
        	perror(filename);
        	return 1;
        }
        fprintf(out, "/* crc32.h -- tables for rapid CRC calculation\n");
        fprintf(out, " * Generated automatically by crc32.c\n */\n\n");
        fprintf(out, "local const z_crc_t FAR ");
        fprintf(out, "crc_table[TBLS][256] =\n{\n  {\n");
        write_table(out, crc_table[0]);
		{
			register int k;
	        fprintf(out, "#ifdef BYFOUR\n");
	        for (k = 1; k < 8; k++) {
	            fprintf(out, "  },\n  {\n");
	            write_table(out, crc_table[k]);
	        }
	        fprintf(out, "#endif\n");
        }
        fprintf(out, "  }\n};\n");
        fclose(out);
    }

	return 0;
}

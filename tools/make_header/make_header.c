#include <stdio.h>
#include <stdlib.h>

static int procCopyFile(FILE* objDst, FILE* objSrc)
{
	int iChar;

	while ( (iChar = fgetc(objSrc)) != EOF ) {
		if ( fputc(iChar, objDst) == EOF ) {
			return 0;
		}
	}
	return 1;
}

int main(void)
{
	FILE* objSrc;
	FILE* objDst;

	objSrc = fopen("src/api/public_decl.h", "rb");
	if ( objSrc == NULL ) {
		fprintf(stderr, "failed to open src/api/public_decl.h\n");
		return 1;
	}

	objDst = fopen("xpack.h", "wb");
	if ( objDst == NULL ) {
		fclose(objSrc);
		fprintf(stderr, "failed to create xpack.h\n");
		return 2;
	}

	fputs("/* Auto-generated from src/api/public_decl.h. */\n", objDst);
	fputs("#ifndef XPACK_H\n", objDst);
	fputs("#define XPACK_H\n\n", objDst);
	fputs("#include <stdint.h>\n", objDst);
	fputs("#include <xrt.h>\n\n", objDst);
	fputs("#ifdef __cplusplus\n", objDst);
	fputs("extern \"C\" {\n", objDst);
	fputs("#endif\n\n", objDst);

	if ( !procCopyFile(objDst, objSrc) ) {
		fclose(objSrc);
		fclose(objDst);
		fprintf(stderr, "failed to copy public declarations\n");
		return 3;
	}

	fputs("\n#ifdef __cplusplus\n", objDst);
	fputs("}\n", objDst);
	fputs("#endif\n\n", objDst);
	fputs("#endif\n", objDst);

	fclose(objSrc);
	fclose(objDst);
	return 0;
}

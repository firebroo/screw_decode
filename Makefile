all: decode

decode: screwdecode.c zencode.c
		gcc -o decode screwdecode.c zencode.c -lz


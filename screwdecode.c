#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "php_screw.h"

#define PM9SCREW        "\tPM9SCREW\t"
#define PM9SCREW_LEN     10

short pm9screw_mycryptkey[] = {
  11152, 368, 192, 1281, 62
};

void
decode_screw(char *filename)
{
	char	buf[PM9SCREW_LEN + 1];
	char    decode_filename[1024];

	FILE *fp = fopen(filename, "r");
	fread(buf, PM9SCREW_LEN, 1, fp);

	if (memcmp(buf, PM9SCREW, PM9SCREW_LEN) != 0) {
		fclose(fp);
		return;
	}

	struct	stat	stat_buf;
	char	*datap, *newdatap;
	int	datalen, newdatalen;
	int	cryptkey_len = sizeof pm9screw_mycryptkey / 2;
	int	i;

	fstat(fileno(fp), &stat_buf);
	datalen = stat_buf.st_size - PM9SCREW_LEN;
	datap = (char*)malloc(datalen);
	fread(datap, datalen, 1, fp);
	fclose(fp);

	for(i=0; i<datalen; i++) {
		datap[i] = (char)pm9screw_mycryptkey[(datalen - i) % cryptkey_len] ^ (~(datap[i]));
	}

	newdatap = zdecode(datap, datalen, &newdatalen);

	sprintf(decode_filename, "%s%s", filename, ".decode");
	FILE * fp2 = fopen(decode_filename, "w");
	fwrite(newdatap, newdatalen, 1, fp2);
	fclose(fp2);
}

void 
get_dir_all_file (char *path)
{
    DIR            *dp;
    struct stat     statbuf;
    struct dirent  *entry;

    stat(path, &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {
        if( (dp = opendir (path)) != NULL ) {
            chdir (path);         
            while( (entry = readdir(dp)) != NULL ) {
                lstat (entry->d_name, &statbuf);

                if ( S_ISDIR (statbuf.st_mode) ) {
                    if (strcmp (".", entry->d_name) == 0 || strcmp ("..", entry->d_name) == 0) {
                        continue;
                    }
                    get_dir_all_file (entry->d_name);
                } else {
                    decode_screw(entry->d_name);
                }
            }

            chdir ("..");
            closedir (dp);
        } else {
            fprintf (stderr,"cannot open directory: %s\n",path);
            return;
        }
    } else {
        char dir[1024] = {'\0'};
        char *filename = strrchr(path, '/') + 1;
        strncpy(dir, path, filename-path);
        chdir (dir);
        decode_screw(filename);       
    }
}



int
main (int argc, char *argv[])
{
	if (argc < 2) {
		fprintf (stderr,"请输入解密文件路劲\n");
		exit(0);
	}
    char *path = argv[1];
    get_dir_all_file(path);

    return 0;
}

#include <string.h>
#include <ext4.h>

int main(void) {

    ext4_file fd, fd2;
    ext4_fopen(&fd, "/squid-cache/test", "wb");

    printf("writing to file...");

#define BUF 256
    char buf[BUF] = "hello ext4!";
    return ext4_fwrite(&fd, "hello", strlen("hello"), NULL);

    printf("done.\n");
    ext4_fclose(&fd);
    memset(buf, 0, BUF);

    ext4_fopen(&fd2, "/squid-cache/test", "r");

    printf("buf should be empty: %s\n", buf);
    ext4_fread(&fd2, buf, BUF, NULL);
    printf("message in file: %s\n", buf);

    ext4_fclose(&fd2);


    return 0;
}

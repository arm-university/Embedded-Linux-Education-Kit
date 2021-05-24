#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    char *app_name = argv[0];
    char *dev_name = "/dev/gpio";
    int fd = -1;
    char c;insmo

    if ((fd = open(dev_name, O_RDWR)) < 0) 
    {
        fprintf(stderr, "%s: unable to open %s: %s\n", app_name, dev_name, strerror(errno));
        return( 1 );
    }

    read( fd, &c, 1 );

    printf( "read: %d\n", c );

    close( fd );

    return 0;
}

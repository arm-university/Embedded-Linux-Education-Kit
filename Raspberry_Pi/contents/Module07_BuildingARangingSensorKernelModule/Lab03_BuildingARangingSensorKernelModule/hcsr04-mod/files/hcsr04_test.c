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
    char *dev_name = "/dev/hcsr04";	
    int fd = -1;	
    char c;	
    int d;


    if( (fd = open(dev_name, O_RDWR)) < 0 ) 
    {
        fprintf(stderr, "%s: unable to open %s: %s\n", app_name, dev_name, strerror(errno));		
        return( 1 );	
    }

    c = 1;
    write( fd, &c, 1 );
    read( fd, &d, 4 );

    printf( "%d: %f\n", d, d/58.0 );

    close( fd );

    return 0;
}

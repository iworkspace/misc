#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#ifdef __FreeBSD__
#include <net/if.h>
#include <net/if_dl.h>
#else
#include <linux/if.h>
#endif
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


int sock_g = -1;


int usage(char *prog,int ret){
  printf("%s < -i interface > < -g group_address > < -s source_address > [ -h ] \n",prog);
  return ret;
}

void main_loop()
{
  
}

int
main(int argc, char *argv[])
{
    int opt
    char *interface = NULL;
    char *group_address = NULL;
    char *source_address = NULL;

    while ((opt = getopt(argc, argv, "i:g:s")) != -1) {
        switch (opt) {
        case 'i':
            interface = strdup(optarg);
            break;
        case 'g':
            group_address = strdup(optarg);
            break;
         case 's':
            source_address = strdup(optarg);
            break;
        case 'h':
            return usage(argv[0],0);
        default: /* '?' */
            return usage(argv[0],-1);
        }
    }

    if(!interface || !group_address || !source_address){
        return usage(argv[0],-1);
    }

    //create multicast socket
    
    //bind attr
    
    //socket pair set up

    //recv & echo reply.

    main_loop();
}

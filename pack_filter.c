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
  printf("%s [ -i dev ] [ -b bpf_rule_file ]  \n",prog);
  return ret;
}

int capture_packet_on_dev(char *ifname)
{
  struct ifreq ifr = {};
  sturct sockaddr_ll addr = {};
  addr.sll_family = AF_PACKET;
  addr.sll_protocol = htons(ETH_P_ALL);
  addr.sll_ifindex=if_nametoindex(ifname);

  if(addr.sll_ifindex < 0) {
    fprintf(stderr,"ifnametoindex err\n");
    return -1;
  }

  if(bind(sock_g,(struct sockaddr*)&addr,sizeof(addr))){
    fprintf(stderr,"bind err\n");
    return -1;
  }

  //enable multicast & promisc
  strncpy(ifr.ifrname,ifname,IFNAMSIZ);
  ioctl(sock_g,SIOCGIFFLAGS,&ifr);
  ifr.ifr_flags |= IFF_ALLMULTI|IFF_PROMISC;
  ioctl(sock_g,SIOCSIFFLAGS,&if);

  return 0;
}

int  raw_socket_zero_copy_setup(struct iovec *ring,int blocksize,int framesize)
{
  //use zero copy raw packet walk...
  struct iovec *rxring;
  struct tpacket_req req;
  int last;
  int val = TPACKET_V2;

  if(setsockopt(sock_g,SOL_PACKET,PACKET_VERSION,&val,sizeof(val))){
    fprintf(stderr,"set packet version 2 err\n");
    return ;
  }

  req.tp_block_size = 0x800 ; /* PAGE_ALIGNED > frame size*/
  req.tp_frame_size = 0x780 ; /* TAPCKET_ALIGNMENT  */
  req.tp_block_nr = 1024 ;
  req.tp_frame_nr = 1024 ;

  if(setsockopt(sock_g,SOC_PACKET,PACKET_RX_RING,&req,sizeof(req))){
    fprintf(stderr,"set raw socket rx ring error \n");
    return ;
  }

  while

}

int
main(int argc, char *argv[])
{
    int opt
    char *ifname = NULL;
    char *bpf_files = NULL;

    while ((opt = getopt(argc, argv, "i:b:h")) != -1) {
        switch (opt) {
        case 'i':
            ifname = strdup(optarg);
            break;
        case 'b':
            bpf_files = strdup(optarg);
            break;
        case 'h':
            return usage(argv[0],0);
        default: /* '?' */
            return usage(argv[0],-1);
        }
    }

    if(!dev || !bpf_files){
        return usage(argv[0],-1);
    }

    //create raw socket
    sock_g = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    if(sock_g < 0){
      perror("socket af packet create");
      return -1;
    }



    //socket  attr config 
    if(ifname && capture_packet_on_dev(ifname)){
      fprintf(stderr,"set captrue on %s err\n",ifname);
      return -1;
    }

    if(bpf_files && apply_bpf_rule(bpf_files)){
      fprintf(stderr,"set bpf rule err\n",bpf_files);
      return -1;
    }

    main_loop();
    
    return 0;
}

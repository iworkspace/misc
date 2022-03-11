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
int loglevel = 0;

int usage(char *prog,int ret){
  printf("%s [ -i dev ] [ -b bpf_rule_file ] [ -l loglevel ] \n",prog);
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

}

/**
 *
 *
[test]# tcpdump -ddd -nn -i any ip
4
40 0 0 14
21 0 1 2048
6 0 0 262144
6 0 0 0
[test]#
[test]# tcpdump -dd -nn -i any ip
{ 0x28, 0, 0, 0x0000000e },
{ 0x15, 0, 1, 0x00000800 },
{ 0x6, 0, 0, 0x00040000 },
{ 0x6, 0, 0, 0x00000000 },

 */

int apply_bpf_rule(char *bpf_file){
  struct sock_fprog fprog;
  struct sock_filter *filter = NULL;
  int ret = -1;
  FILE *bpf_f= fopen(bpf_file);
  if(!bpf_f){
    goto end;
  }

  if(1!=fscanf(bpf_f,"%d\n",&fprog.len)){
    goto end;
  }

  filter = malloc(sizeof(*filter)*(fprog.len+1));
  if(!filter){
    goto end;
  }

  int i;
  for(i=0;i<fprog.len;i++){
    if( 4 != fscanf(bpf_f,"%d %d %d %d\n",
          &filter[i].code,
          &filter[i].jt,
          &filter[i].jf,
          &filter[i].k)){
        goto end;
    }
  }
  fprog.filter = filter;

  if(setsockopt(sock_g,SO_LSOCKET,SO_ATTACH_FILTER,&fprog,sizeof(fprog))){
    goto end;
  }

  ret = 0;
end:
  if(filter){
      free(filter);
  }
  if(bpf_f){
    fclose(bpf_f);
  }
  return ret;
}

int main_loop()
{
  fd_set rset;
  struct iovec rx_rx
  if(raw_socket_zero_copy_setup(struct iovec *ring,int blocksize,int framesize)){
      return -1;
  }
  for(;;){
    FD_CLEAR(&rset);
    FD_SET(sock_g,&rset);
    if(1== select(sock_g+1,&rset,NULL,NULL,NULL)){
        walk_rx_ring();
    }else{
      break;
    }
  }
}

int
main(int argc, char *argv[])
{
    int opt
    char *ifname = NULL;
    char *bpf_files = NULL;

    while ((opt = getopt(argc, argv, "i:b:i:h")) != -1) {
        switch (opt) {
        case 'i':
            ifname = strdup(optarg);
            break;
        case 'b':
            bpf_files = strdup(optarg);
            break;
        case 'l':
            loglevel = atoi(optarg);
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

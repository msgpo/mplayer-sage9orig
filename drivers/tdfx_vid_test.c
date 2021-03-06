
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <inttypes.h>

#include "tdfx_vid.h"


static void print_tdfd_vid_cfg(tdfx_vid_config_t* cfg) {
     printf("tdfx_vid version %d\n"
	    "  Ram: %d\n"
	    "  Screen: %d x %d\n",
	    cfg->version,
	    cfg->ram_size,
	    cfg->screen_width, cfg->screen_height);	    
}
     

int main(int argc, char** argv) {
  int fd,i;
  unsigned char *mem,*ptr;
  tdfx_vid_agp_move_t move;
  tdfx_vid_config_t cfg;
  tdfx_vid_blit_t blit;

  fd = open("/dev/tdfx_vid", O_RDWR);

  if(fd <= 0) {
    printf("Can't open /dev/tdfx_vid\n");
    return 1;
  }

  if(ioctl(fd,TDFX_VID_GET_CONFIG,&cfg)) {
    printf("Ioctl GET_CONFIG error\n");
    close(fd);
    return 1;
  }
  
  print_tdfd_vid_cfg(&cfg);

  mem = mmap( NULL, 640*480*2, PROT_READ | PROT_WRITE, MAP_SHARED,
	      fd, 0);

  if(mem == MAP_FAILED) {
    printf("Memmap failed !!!!!\n");
    return 1;
  }

/*   for(ptr = mem, i = 0 ; i < 640*480 ; i++) { */
/*     ptr[0] = i & 0xFF; */
/*     ptr[1] = (i & 0xFF); */
/*     ptr += 2; */
/*   } */
    
  memset(mem,0xFF,640*480*2);
  
  memset(&move, 0, sizeof(tdfx_vid_agp_move_t));
  move.width = 640;
  move.height = 240;
  move.src_stride = 640;
  move.dst_stride = 640*2;

  if(ioctl(fd,TDFX_VID_AGP_MOVE,&move)) {
    printf("AGP Move failed !!!!\n");
    return 0;
  }
  
  printf("AGP Move ????\n");
  sleep(1);

  blit.src = 0;
  blit.src_stride = 640*2;
  blit.src_x = blit.src_y = 0;
  blit.src_w = 320;
  blit.src_h = 240;
  blit.src_format = cfg.screen_format;

  blit.dst = 240*640*2+320;
  blit.dst_stride = 640*2;
  blit.dst_x = blit.dst_y = 0;
  blit.dst_w = 320;
  blit.dst_h = 240;
  blit.dst_format = cfg.screen_format;

  if(ioctl(fd,TDFX_VID_BLIT,&blit)) {
    printf("Blit failed !!!!\n");
    return 0;
  }
  
  close(fd);
  return 1;
}

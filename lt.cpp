#include <stdio.h>
#include "E101.h"

int main()
{
  int err;
  printf(" Hello\n");
  err = init();

  set_motor(1,0);
  set_motor(2,0);
  sleep1(0,10000);

  set_motor(1,125);
  set_motor(2,125);
  sleep1(10,10000);
  set_motor(1,0);
  set_motor(2,0);
  sleep1(0,10000);

  int count = 0;
  int ain= 0;
  select_IO(0, 0);
  open_screen_stream();
  while(count<8)
	{
		write_digital(0,1);
		take_picture();
		update_screen();
		write_digital(0,0);
        ain =  read_analog(0);		
		count++;
		printf("count=%d ain=%d\n",count,ain);
		sleep1(1,200);

    }
    close_screen_stream();
	printf("After init() error=%d\n",err);
	printf("Again\n");
	
	return 0;
}

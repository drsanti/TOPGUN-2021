
#include <main.h>


void timer_callback(uint32_t ticks)
{
	if(ticks % 1000 == 0) {
		printf("ticks: %ld\n", ticks);
	}
	TGRHelpers_PerformCallback();
}

// key_down --> key_hold --> key_up

void key_down(uint32_t count) {
	printf("key_down: %ld\n", count);
}

void key_hold(uint32_t count) {
	printf("key_hold: %ld\n", count);
}

void key_up(uint32_t count) {
	printf("key_up: %ld\n", count);
}

void TGR_Main(void)
{
	Psw_SetPushedCallback(key_down);
	Psw_SetReleasedCallback(key_up);
	Psw_SetHoldingCallback(key_hold);
	Clk_SetTickCallback(timer_callback);
	while(1);
}

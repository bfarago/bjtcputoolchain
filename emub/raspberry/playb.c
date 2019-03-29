#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <math.h>
// #include <time.h>
#include <sys/time.h>


#define delay(ms) usleep( (ms)*1000)

#include "Std_Types.h"
#include "Pwm.h"
#include "VideoDrv.h"
#include "Keyboard.h"
#include "Sim.h"

uint8 g_KeepRunning = 1;
uint8 run = 0;

long long current_timestamp() {
	struct timeval te;
	gettimeofday(&te, NULL); // get current time
	long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // calculate milliseconds
//	printf("milliseconds: %lld\n", milliseconds);
	return milliseconds;
}

void sig_handler(int signo)
{
	if (signo == SIGINT) {
		printf("\nreceived SIGINT\n");
		g_KeepRunning = 0;
	}
}

int main(int argc, char** argv)
{
	Std_ReturnType ret;
	if (argc > 1) {
		FILE *f = fopen(argv[1], "rb");
		fread(Sim_Memory, 1 << 12, 1, f);
		fclose(f);
		run = 1;
	}
	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		printf("Signal handler init error\n");
	}
	//this is a try, on console the sigint is not enought.
	signal(SIGHUP, sig_handler);
	signal(SIGTERM, sig_handler);

	uint16 counter=0;
	if (VideoDrv_Enable_ScopeOut) {
		printf("Raspberry Pi PWM\n");
		ret = Pwm_Init(0);
		if (ret) {
			printf("PWM init error\n");
			return 1;
		}
	}
	ret= VideoDrv_Init(0);
	if (ret){
		printf("VideoDrv_bjtcpu init error\n");
		return 1;
	}
	ret = Keyboard_Init();
	for(; g_KeepRunning;){
		if ((counter%5000) ==0)
		{
			VideoDrv_MainFunctionDiagnostics();
			counter=0;
		}
		
		if ((counter%5) ==0){
			VideoDrv_MainFunctionTest();
		}
		
		VideoDrv_MainFunction();
		counter++;
		//delay(1);
		Keyboard_ReadScan();
		if (Keyboard_IsEscapePressed()) {
			g_KeepRunning = 0;
		}
		if (run) {
			static long long next_run = 0;
			long long now = current_timestamp();
			if (now > next_run) {
				next_run = now + 10;
				Sim_OnKeyDown();
				for (int i = 0; i < 5000; i++)
					Sim_Step();
			}
		}
	}

	if (VideoDrv_Enable_ScopeOut) {
		Pwm_DeInit();
	}
	Keyboard_DeInit();
	VideoDrv_DeInit();
	
  return 0 ;
}
#include "Keyboard.h"
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include "linux/kd.h"
#include "fcntl.h"
#include "sys/ioctl.h"

static struct termios Keyboard_tty_attr_old;
static int Keyboard_old_mode;
static uint8 Keyboard_Escape=0;

typedef enum {
	KM_Standard,
	KM_Scan, // on console, but not perfect :)
	KM_Ev  //not implemented
} Keyboard_Mode_e;
Keyboard_Mode_e Keyboard_Mode = KM_Scan; // KM_Standard;


uint8 Key_Pressed[Key_Last];

Std_ReturnType Keyboard_Check(Keyboard_scan_e scan) {
	Std_ReturnType ret = E_NOT_OK;
	if (Key_Pressed[scan]) {
		ret = E_OK;
		Key_Pressed[scan] = 0;
	}
	return ret;
}

Std_ReturnType Keyboard_Init(void) {
	if (KM_Standard == Keyboard_Mode) {
		//nothing
	}
	{
		struct termios tty_attr;
		int flags;
		int r;
		int ttyfd = STDIN_FILENO;

		if (isatty(ttyfd)) {
			printf("keyboard: stdin is a tty\n");
		}

		// make stdin non-blocking
		flags = fcntl(0, F_GETFL);
		flags |= O_NONBLOCK;
		r=fcntl(ttyfd, F_SETFL, flags);
		if (r < 0) {
			printf("keyboard fctl nonblock error:%d %d %s\n", r, errno, strerror(errno));
		}

		if (KM_Standard != Keyboard_Mode) {
			//lets try scan code / console mode
			// save old keyboard mode
			int r = ioctl(ttyfd, KDGKBMODE, &Keyboard_old_mode);
			if (r < 0) {
				printf("keyboard mode read error: %d %d %s\nfallback to standard mode\n", r, errno, strerror(errno));
				Keyboard_Mode = KM_Standard; //Will use normal escape sequence
			}
			printf("%x", Keyboard_old_mode);
		}

		r=tcgetattr(ttyfd, &Keyboard_tty_attr_old);
		if (r < 0) {
			printf("keyboard attr read error: %d %d %s\n", r, errno, strerror(errno));
		}
		else {
			printf("attr read ok\n");
		}
		
		memcpy(&tty_attr, &Keyboard_tty_attr_old, sizeof(tty_attr)); // tty_attr = Keyboard_tty_attr_old;
		//goal: turn off buffering, echo and key processing
		//Ok, I don't know how the ptty should setup for this, but lets try :)
		tty_attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ICRNL);
		
		/*
		tty_attr.c_lflag &= ~(ICANON | ECHO | ISIG);
		tty_attr.c_iflag &= ~(ISTRIP | INLCR | ICRNL | IGNCR | IXON | IXOFF);
		*/

		/*
		tty_attr.c_lflag &= ~(ECHO | ICANON);
		tty_attr.c_cc[VTIME] = 0;
		tty_attr.c_cc[VMIN] = 0;
		*/
		
		/*
		tty_attr.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
		tty_attr.c_oflag &= ~OPOST;
		tty_attr.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		tty_attr.c_cflag &= ~(CSIZE | PARENB);
		tty_attr.c_cflag |= CS8;
		*/
		r=tcsetattr(ttyfd, TCSANOW, &tty_attr);
		if (r < 0) {
			printf("keyboard attr set error: %d %d %s\n", r, errno, strerror(errno));
			return 0;
		}
		else {
			printf("keyboard attr set.\n");
		}
		if (KM_Scan == Keyboard_Mode) {
			//so, this is a console, the mode was saved as well
			r = ioctl(ttyfd, KDSKBMODE, K_RAW);
			if (r < 0) {
				printf("keyboard mode raw set error: %d %d %s\n", r, errno, strerror(errno));
				return 0;
			}
		}
	}
	Keyboard_Escape = 0;
	return E_OK;
}

Std_ReturnType Keyboard_DeInit(void) {
	int ttyfd = STDIN_FILENO;
	int r = 0;
	if (KM_Scan== Keyboard_Mode) {
		//consol was available, scan codes was used.
		if (Keyboard_old_mode) {
			r = ioctl(ttyfd, KDSKBMODE, Keyboard_old_mode);
			if (r < 0) {
				printf("keyboard mode old set error: %d %d %s\n", r, errno, strerror(errno));
			}
		}
		else {
			r = -1;
		}
		if (r < 0) {
			r = ioctl(ttyfd, KDSKBMODE, K_UNICODE);
			if (r < 0) {
				printf("keyboard mode unicode set error: %d %d %s\n", r, errno, strerror(errno));
			}
		}

	}
	r=tcsetattr(ttyfd, TCSANOW, &Keyboard_tty_attr_old);
	if (r < 0) {
		printf("keyboard attr set error: %d %d %s\n", r, errno, strerror(errno));
	}
	return E_OK;
}

void Keyboard_ReadScan(void)
{
	char buf[1];
	int res;
	if (KM_Standard ==Keyboard_Mode)
	{
		//ONLY standard mode is possible
		int c = getchar();
		//printf("%02x ", c);
		switch (c) {
		case 32:
			Key_Pressed[Key_Space] = 1;
			break;
		case 27:
			c = getchar();
			printf("%02x ", c);
			switch (c) {
			case 0x5b:
				c = getchar();
				printf("%02x ", c);
				switch (c) {
				case 0x41:
					Key_Pressed[Key_Up] = 1;
					break;
				case 0x42:
					Key_Pressed[Key_Dn] = 1;
					break;
				case 0x43:
					Key_Pressed[Key_Right] = 1;
					break;
				case 0x44:
					Key_Pressed[Key_Left] = 1;
					break;
				}
				break;
			default:
				
				Key_Pressed[Key_Esc] = 1;
				Keyboard_Escape = 1;
				break;
			}
			break;
		}
	}
	else
	{
		// read scan code is possible
		res = read(0, &buf[0], 1);
		// keep reading til there's no more
		while (res >= 0) {
			//printf("%02x ", buf[0]);
			switch (buf[0]) {
			case 0x01:
				// escape was pressed
				Keyboard_Escape = 1;
				break;
			case 0x81:
				// escape was released
				Keyboard_Escape = 0;
				break;
			case 0x9c:
				// 9c 67 e7
				// 6c ec

				res = read(0, &buf[0], 1);
				switch (buf[0]) {
				case 0x67:
					res = read(0, &buf[0], 1);
					switch (buf[0]) {
					case 0xe7:
						Key_Pressed[Key_Up] = 1;
						break;
					}
					break;
				}
				break;
			}
			res = read(0, &buf[0], 1);
		}
	}
}

uint8 Keyboard_IsEscapePressed(void)
{
	return Keyboard_Escape;
}

#include "Keyboard.h"

#include "unistd.h"
#include "linux/kd.h"
#include "termios.h"
#include "fcntl.h"
#include "sys/ioctl.h"

static struct termios Keyboard_tty_attr_old;
static int Keyboard_old_mode;
static uint8 Keyboard_Escape=0;

typedef enum {
	KM_Standard,
	KM_Scan, //not working :)
	KM_Test, //test
	KM_Ev  //not implemented
} Keyboard_Mode_e;
Keyboard_Mode_e Keyboard_Mode = KM_Standard; // KM_Scan; // KM_Standard;

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
	}else if (KM_Test == Keyboard_Mode){
		int flags;
		/* make stdin non-blocking */
		flags = fcntl(0, F_GETFL);
		flags |= O_NONBLOCK;
		fcntl(0, F_SETFL, flags);
		/* save old keyboard mode */
		if (ioctl(0, KDGKBMODE, &Keyboard_old_mode) < 0) {
			return 0;
		}
		ioctl(0, KDSKBMODE, K_RAW);
	}
	else
	{
		struct termios tty_attr;
		int flags;

		/* make stdin non-blocking */
		flags = fcntl(0, F_GETFL);
		flags |= O_NONBLOCK;
		fcntl(0, F_SETFL, flags);

		/* save old keyboard mode */
		if (ioctl(0, KDGKBMODE, &Keyboard_old_mode) < 0) {
			return 0;
		}
		printf("%x", Keyboard_old_mode)

		tcgetattr(0, &Keyboard_tty_attr_old);

		/* turn off buffering, echo and key processing */
		memcpy(&tty_attr, &Keyboard_tty_attr_old, sizeof(tty_attr)); // tty_attr = Keyboard_tty_attr_old;
		//tty_attr.c_lflag &= ~(ICANON | ECHO | ISIG);
		//tty_attr.c_iflag &= ~(ISTRIP | INLCR | ICRNL | IGNCR | IXON | IXOFF);
		tty_attr.c_lflag &= ~(ECHO | ICANON);
		tty_attr.c_cc[VTIME] = 0;
		tty_attr.c_cc[VMIN] = 0;
		//
		// tty_attr.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);

		/*
		tty_attr.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
		tty_attr.c_oflag &= ~OPOST;
		tty_attr.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		tty_attr.c_cflag &= ~(CSIZE | PARENB);
		tty_attr.c_cflag |= CS8;
		*/
		tcsetattr(0, TCSANOW, &tty_attr);

		
		ioctl(0, KDSKBMODE, K_RAW);

	}
	Keyboard_Escape = 0;
	return E_OK;
}

Std_ReturnType Keyboard_DeInit(void) {
	if (KM_Standard == Keyboard_Mode) {
	}
	else if (KM_Test == Keyboard_Mode) {
		ioctl(0, KDSKBMODE, Keyboard_old_mode);
	}
	else
	{
		//NOT WORKING on RASPBERRY !?
		//tcsetattr(0, TCSAFLUSH, &Keyboard_tty_attr_old);
		ioctl(0, KDSKBMODE, Keyboard_old_mode);
		tcsetattr(0, TCSANOW, &Keyboard_tty_attr_old);
	}
	return E_OK;
}

void Keyboard_ReadScan(void)
{
	char buf[1];
	int res;
	if (KM_Standard ==Keyboard_Mode)
	{
		int c = getchar();
		switch (c) {
		case 27:
			c = getchar();
			switch (c) {
			case 91:
				c = getchar();
				switch (c) {
				case 65:
					Key_Pressed[Key_Up] = 1;
					break;
				case 66:
					Key_Pressed[Key_Dn] = 1;
					break;
				}

				break;
			default:
				Key_Pressed[Key_Esc] = 1;
				break;
			}
			break;
		}
	}
	else
	{
		/* read scan code from stdin */
		res = read(0, &buf[0], 1);
		/* keep reading til there's no more*/
		while (res >= 0) {
			printf("%02x ", buf[0]);
			switch (buf[0]) {
			case 0x01:
				/* escape was pressed */
				Keyboard_Escape = 1;
				break;
			case 0x81:
				/* escape was released */
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

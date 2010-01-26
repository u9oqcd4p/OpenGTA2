#ifndef INPUT_KEYS_H
#define INPUT_KEYS_H

//Keyboard key definitions: 8-bit ISO-8859-1 (Latin 1) encoding is used
//for printable keys (such as A-Z, 0-9 etc), and values above 256
//represent special (non-printable) keys (e.g. F1, Page Up etc).
//Based on GLFW input stuff
#define KBKEY_UNKNOWN      -1
#define KBKEY_SPACE        32
#define KBKEY_SPECIAL      256
#define KBKEY_ESC          (KBKEY_SPECIAL+1)
#define KBKEY_F1           (KBKEY_SPECIAL+2)
#define KBKEY_F2           (KBKEY_SPECIAL+3)
#define KBKEY_F3           (KBKEY_SPECIAL+4)
#define KBKEY_F4           (KBKEY_SPECIAL+5)
#define KBKEY_F5           (KBKEY_SPECIAL+6)
#define KBKEY_F6           (KBKEY_SPECIAL+7)
#define KBKEY_F7           (KBKEY_SPECIAL+8)
#define KBKEY_F8           (KBKEY_SPECIAL+9)
#define KBKEY_F9           (KBKEY_SPECIAL+10)
#define KBKEY_F10          (KBKEY_SPECIAL+11)
#define KBKEY_F11          (KBKEY_SPECIAL+12)
#define KBKEY_F12          (KBKEY_SPECIAL+13)
#define KBKEY_F13          (KBKEY_SPECIAL+14)
#define KBKEY_F14          (KBKEY_SPECIAL+15)
#define KBKEY_F15          (KBKEY_SPECIAL+16)
#define KBKEY_F16          (KBKEY_SPECIAL+17)
#define KBKEY_F17          (KBKEY_SPECIAL+18)
#define KBKEY_F18          (KBKEY_SPECIAL+19)
#define KBKEY_F19          (KBKEY_SPECIAL+20)
#define KBKEY_F20          (KBKEY_SPECIAL+21)
#define KBKEY_F21          (KBKEY_SPECIAL+22)
#define KBKEY_F22          (KBKEY_SPECIAL+23)
#define KBKEY_F23          (KBKEY_SPECIAL+24)
#define KBKEY_F24          (KBKEY_SPECIAL+25)
#define KBKEY_F25          (KBKEY_SPECIAL+26)
#define KBKEY_UP           (KBKEY_SPECIAL+27)
#define KBKEY_DOWN         (KBKEY_SPECIAL+28)
#define KBKEY_LEFT         (KBKEY_SPECIAL+29)
#define KBKEY_RIGHT        (KBKEY_SPECIAL+30)
#define KBKEY_LSHIFT       (KBKEY_SPECIAL+31)
#define KBKEY_RSHIFT       (KBKEY_SPECIAL+32)
#define KBKEY_LCTRL        (KBKEY_SPECIAL+33)
#define KBKEY_RCTRL        (KBKEY_SPECIAL+34)
#define KBKEY_LALT         (KBKEY_SPECIAL+35)
#define KBKEY_RALT         (KBKEY_SPECIAL+36)
#define KBKEY_TAB          (KBKEY_SPECIAL+37)
#define KBKEY_ENTER        (KBKEY_SPECIAL+38)
#define KBKEY_BACKSPACE    (KBKEY_SPECIAL+39)
#define KBKEY_INSERT       (KBKEY_SPECIAL+40)
#define KBKEY_DEL          (KBKEY_SPECIAL+41)
#define KBKEY_PAGEUP       (KBKEY_SPECIAL+42)
#define KBKEY_PAGEDOWN     (KBKEY_SPECIAL+43)
#define KBKEY_HOME         (KBKEY_SPECIAL+44)
#define KBKEY_END          (KBKEY_SPECIAL+45)
#define KBKEY_KP_0         (KBKEY_SPECIAL+46)
#define KBKEY_KP_1         (KBKEY_SPECIAL+47)
#define KBKEY_KP_2         (KBKEY_SPECIAL+48)
#define KBKEY_KP_3         (KBKEY_SPECIAL+49)
#define KBKEY_KP_4         (KBKEY_SPECIAL+50)
#define KBKEY_KP_5         (KBKEY_SPECIAL+51)
#define KBKEY_KP_6         (KBKEY_SPECIAL+52)
#define KBKEY_KP_7         (KBKEY_SPECIAL+53)
#define KBKEY_KP_8         (KBKEY_SPECIAL+54)
#define KBKEY_KP_9         (KBKEY_SPECIAL+55)
#define KBKEY_KP_DIVIDE    (KBKEY_SPECIAL+56)
#define KBKEY_KP_MULTIPLY  (KBKEY_SPECIAL+57)
#define KBKEY_KP_SUBTRACT  (KBKEY_SPECIAL+58)
#define KBKEY_KP_ADD       (KBKEY_SPECIAL+59)
#define KBKEY_KP_DECIMAL   (KBKEY_SPECIAL+60)
#define KBKEY_KP_EQUAL     (KBKEY_SPECIAL+61)
#define KBKEY_KP_ENTER     (KBKEY_SPECIAL+62)
#define KBKEY_LAST         KBKEY_KP_ENTER


#define KEY_UP		0
#define KEY_DOWN	1
#define KEY_LEFT	2
#define KEY_RIGHT	3
#define KEY_FIRE	4
#define KEY_JUMP	5
#define KEY_ENTER	6

#define NUM_KEYS 8

#endif
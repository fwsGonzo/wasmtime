#include <emscripten.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
extern bool is_transparent(int32_t);
extern float api_sin(float);

#define GET_ID(x)  ((x) & 65535)

__attribute__((used, retain, visibility("default")))
extern uint16_t stdCactusVisibility(uint32_t src, uint32_t dst, uint16_t facing)
{
	if (GET_ID(src) == GET_ID(dst))
		facing &= (1 | 2 | 16 | 32);
	else if (facing == 8) {
		is_transparent(dst);
		is_transparent(dst);
		is_transparent(dst);
		is_transparent(dst);
		is_transparent(dst);
		facing = 0; // Remove bottom
	}
	return facing;
}

__attribute__((used, retain, visibility("default")))
extern uint16_t stdBenchVisibility2(uint32_t src, uint32_t dst, uint16_t facing)
{
	if (GET_ID(src) == GET_ID(dst))
		facing &= (1 | 2 | 16 | 32);
	else if (facing == 8 && !is_transparent(dst))
		facing = 0; // Remove bottom
	return facing;
}

__attribute__((used, retain, visibility("default")))
uint32_t stdRainbowBlockColor(uint32_t, int x, int z)
{
	static const float period = 0.5f;
	// Rainbow block color
//#define USE_MATH
#ifdef USE_MATH
	const int r = sinf(x * period) * 127 + 128;
	const int g = sinf(z * period) * 127 + 128;
	const int b = sinf((x + z) * period) * 127 + 128;
#else
	const int r = api_sin(x * period) * 127 + 128;
	const int g = api_sin(z * period) * 127 + 128;
	const int b = api_sin((x + z) * period) * 127 + 128;
#endif
	return 255 << 24 | r << 16 | g << 8 | b;
}

__attribute__((used, retain, visibility("default")))
uint32_t stdRainbowBlockColor2(uint32_t, int x, int z)
{
	static const float period = 0.5f;
	// Rainbow block color
	const int r = api_sin(x * period) * 127 + 128;
	const int g = api_sin(z * period) * 127 + 128;
	const int b = api_sin((x + z) * period) * 127 + 128;
	return 255 << 24 | r << 16 | g << 8 | b;
}

__attribute__((used, retain, visibility("default")))
extern uint32_t benchAllocFree(uint32_t, int xx, int zz)
{
	char* x = (char *)malloc(1024);
	__asm__("" :: "m"(*(char(*)[1024]) x) : "memory");
	char v = x[1023];
	memset(x, 0, 1024);
	__asm__("" :: "m"(*(char(*)[1024]) x) : "memory");
	free((void*)x);
	__asm__("" :: "m"(*(char(*)[1024]) x) : "memory");
	return v;
}

EMSCRIPTEN_KEEPALIVE extern
void call_tester_0() {  }
EMSCRIPTEN_KEEPALIVE extern
void call_tester_1(int arg0) {  }
EMSCRIPTEN_KEEPALIVE extern
void call_tester_2(int arg0, int arg1) {  }
EMSCRIPTEN_KEEPALIVE extern
void call_tester_3(int arg0, int arg1, int arg2) {  }
EMSCRIPTEN_KEEPALIVE extern
void call_tester_4(int arg0, int arg1, int arg2, int arg3) {  }
EMSCRIPTEN_KEEPALIVE extern
void call_tester_5(int arg0, int arg1, int arg2, int arg3, int arg4) {  }
EMSCRIPTEN_KEEPALIVE extern
void call_tester_6(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) {  }
EMSCRIPTEN_KEEPALIVE extern
void call_tester_7(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6) {  }
EMSCRIPTEN_KEEPALIVE extern
void call_tester_8(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7) {  }


extern void syscall_handler_0();
extern void syscall_handler_1(int arg0);
extern void syscall_handler_2(int arg0, int arg1);
extern void syscall_handler_3(int arg0, int arg1, int arg2);
extern void syscall_handler_4(int arg0, int arg1, int arg2, int arg3);
extern void syscall_handler_5(int arg0, int arg1, int arg2, int arg3, int arg4);
extern void syscall_handler_6(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5);
extern void syscall_handler_7(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
EMSCRIPTEN_KEEPALIVE extern
void syscall_tester_0() { syscall_handler_0(); }
EMSCRIPTEN_KEEPALIVE extern
void syscall_tester_1(int arg0) { syscall_handler_1(arg0); }
EMSCRIPTEN_KEEPALIVE extern
void syscall_tester_2(int arg0, int arg1) { syscall_handler_2(arg0, arg1); }
EMSCRIPTEN_KEEPALIVE extern
void syscall_tester_3(int arg0, int arg1, int arg2) { syscall_handler_3(arg0, arg1, arg2); }
EMSCRIPTEN_KEEPALIVE extern
void syscall_tester_4(int arg0, int arg1, int arg2, int arg3) { syscall_handler_4(arg0, arg1, arg2, arg3); }
EMSCRIPTEN_KEEPALIVE extern
void syscall_tester_5(int arg0, int arg1, int arg2, int arg3, int arg4) { syscall_handler_5(arg0, arg1, arg2, arg3, arg4); }
EMSCRIPTEN_KEEPALIVE extern
void syscall_tester_6(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) { syscall_handler_6(arg0, arg1, arg2, arg3, arg4, arg5); }
EMSCRIPTEN_KEEPALIVE extern
void syscall_tester_7(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6) { syscall_handler_7(arg0, arg1, arg2, arg3, arg4, arg5, arg6); }

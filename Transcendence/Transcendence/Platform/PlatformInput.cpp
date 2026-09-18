//	PlatformInput.cpp
//	macOS virtual-key mapping
//
//	Implements the Win32 virtual-key surface that the engine uses for direct
//	input polling (GetAsyncKeyState/GetKeyState). The mapping table is the
//	inverse of the SDL-scancode -> VK table used when synthesizing key
//	messages, and covers every key referenced by DefaultKeyMappings.h.

#include "PlatformInput.h"

SDL_Scancode PlatformVKToScancode(int vk)
	{
	//	Letters and digits

	if (vk >= 'A' && vk <= 'Z')
		return (SDL_Scancode)(SDL_SCANCODE_A + (vk - 'A'));

	if (vk >= '0' && vk <= '9')
		return (SDL_Scancode)(SDL_SCANCODE_0 + (vk - '0'));

	switch (vk)
		{
		case VK_BACK:			return SDL_SCANCODE_BACKSPACE;
		case VK_TAB:			return SDL_SCANCODE_TAB;
		case VK_CLEAR:			return SDL_SCANCODE_CLEAR;
		case VK_RETURN:			return SDL_SCANCODE_RETURN;
		case VK_SHIFT:			return SDL_SCANCODE_LSHIFT;
		case VK_CONTROL:		return SDL_SCANCODE_LCTRL;
		case VK_MENU:			return SDL_SCANCODE_LALT;
		case VK_PAUSE:			return SDL_SCANCODE_PAUSE;
		case VK_CAPITAL:		return SDL_SCANCODE_CAPSLOCK;
		case VK_ESCAPE:			return SDL_SCANCODE_ESCAPE;
		case VK_SPACE:			return SDL_SCANCODE_SPACE;
		case VK_PRIOR:			return SDL_SCANCODE_PAGEUP;
		case VK_NEXT:			return SDL_SCANCODE_PAGEDOWN;
		case VK_END:			return SDL_SCANCODE_END;
		case VK_HOME:			return SDL_SCANCODE_HOME;
		case VK_LEFT:			return SDL_SCANCODE_LEFT;
		case VK_UP:				return SDL_SCANCODE_UP;
		case VK_RIGHT:			return SDL_SCANCODE_RIGHT;
		case VK_DOWN:			return SDL_SCANCODE_DOWN;
		case VK_INSERT:			return SDL_SCANCODE_INSERT;
		case VK_DELETE:			return SDL_SCANCODE_DELETE;
		case VK_LWIN:			return SDL_SCANCODE_LGUI;
		case VK_RWIN:			return SDL_SCANCODE_RGUI;
		case VK_NUMPAD0:		return SDL_SCANCODE_KP_0;
		case VK_NUMPAD1:		return SDL_SCANCODE_KP_1;
		case VK_NUMPAD2:		return SDL_SCANCODE_KP_2;
		case VK_NUMPAD3:		return SDL_SCANCODE_KP_3;
		case VK_NUMPAD4:		return SDL_SCANCODE_KP_4;
		case VK_NUMPAD5:		return SDL_SCANCODE_KP_5;
		case VK_NUMPAD6:		return SDL_SCANCODE_KP_6;
		case VK_NUMPAD7:		return SDL_SCANCODE_KP_7;
		case VK_NUMPAD8:		return SDL_SCANCODE_KP_8;
		case VK_NUMPAD9:		return SDL_SCANCODE_KP_9;
		case VK_MULTIPLY:		return SDL_SCANCODE_KP_MULTIPLY;
		case VK_ADD:			return SDL_SCANCODE_KP_PLUS;
		case VK_SUBTRACT:		return SDL_SCANCODE_KP_MINUS;
		case VK_DECIMAL:		return SDL_SCANCODE_KP_PERIOD;
		case VK_DIVIDE:			return SDL_SCANCODE_KP_DIVIDE;
		case VK_F1:				return SDL_SCANCODE_F1;
		case VK_F2:				return SDL_SCANCODE_F2;
		case VK_F3:				return SDL_SCANCODE_F3;
		case VK_F4:				return SDL_SCANCODE_F4;
		case VK_F5:				return SDL_SCANCODE_F5;
		case VK_F6:				return SDL_SCANCODE_F6;
		case VK_F7:				return SDL_SCANCODE_F7;
		case VK_F8:				return SDL_SCANCODE_F8;
		case VK_F9:				return SDL_SCANCODE_F9;
		case VK_F10:			return SDL_SCANCODE_F10;
		case VK_F11:			return SDL_SCANCODE_F11;
		case VK_F12:			return SDL_SCANCODE_F12;
		case VK_NUMLOCK:		return SDL_SCANCODE_NUMLOCKCLEAR;
		case VK_SCROLL:			return SDL_SCANCODE_SCROLLLOCK;
		case VK_OEM_1:			return SDL_SCANCODE_SEMICOLON;
		case VK_OEM_PLUS:		return SDL_SCANCODE_EQUALS;
		case VK_OEM_COMMA:		return SDL_SCANCODE_COMMA;
		case VK_OEM_MINUS:		return SDL_SCANCODE_MINUS;
		case VK_OEM_PERIOD:		return SDL_SCANCODE_PERIOD;
		case VK_OEM_2:			return SDL_SCANCODE_SLASH;
		case VK_OEM_3:			return SDL_SCANCODE_GRAVE;
		case VK_OEM_4:			return SDL_SCANCODE_LEFTBRACKET;
		case VK_OEM_5:			return SDL_SCANCODE_BACKSLASH;
		case VK_OEM_6:			return SDL_SCANCODE_RIGHTBRACKET;
		case VK_OEM_7:			return SDL_SCANCODE_APOSTROPHE;
		}

	//	Mouse buttons have no scancode; they are polled through the mouse
	//	button state instead.

	return SDL_SCANCODE_UNKNOWN;
	}

SHORT PlatformAsyncKeyStateForState(int vk, const Uint8 *pKeyState, Uint32 dwMouseButtons, SDL_Keymod mod)
	{
	switch (vk)
		{
		case VK_LBUTTON:
			return (dwMouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) ? (SHORT)0x8000 : 0;

		case VK_RBUTTON:
			return (dwMouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? (SHORT)0x8000 : 0;

		case VK_MBUTTON:
			return (dwMouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) ? (SHORT)0x8000 : 0;
		}

	//	Modifier keys. The modifier state already accounts for either physical
	//	key, so prefer it; fall through to the scancode state below so that a
	//	caller which has not pumped SDL events yet still observes the key.

	if (vk == VK_SHIFT && (mod & KMOD_SHIFT))
		return (SHORT)0x8000;

	if (vk == VK_CONTROL && (mod & KMOD_CTRL))
		return (SHORT)0x8000;

	if (vk == VK_MENU && (mod & KMOD_ALT))
		return (SHORT)0x8000;

	if (vk == VK_NUMLOCK && (mod & KMOD_NUM))
		return (SHORT)0x8000;

	if (!pKeyState)
		return 0;

	//	Win32 reports VK_SHIFT/VK_CONTROL/VK_MENU when either physical key is
	//	held, so accept both scancodes.

	switch (vk)
		{
		case VK_SHIFT:
			return (pKeyState[SDL_SCANCODE_LSHIFT] || pKeyState[SDL_SCANCODE_RSHIFT]) ? (SHORT)0x8000 : 0;

		case VK_CONTROL:
			return (pKeyState[SDL_SCANCODE_LCTRL] || pKeyState[SDL_SCANCODE_RCTRL]) ? (SHORT)0x8000 : 0;

		case VK_MENU:
			return (pKeyState[SDL_SCANCODE_LALT] || pKeyState[SDL_SCANCODE_RALT]) ? (SHORT)0x8000 : 0;
		}

	SDL_Scancode sc = PlatformVKToScancode(vk);
	if (sc != SDL_SCANCODE_UNKNOWN && pKeyState[sc])
		return (SHORT)0x8000;

	return 0;
	}

SHORT PlatformGetAsyncKeyState(int vk)
	{
	return PlatformAsyncKeyStateForState(vk, SDL_GetKeyboardState(NULL), SDL_GetMouseState(NULL, NULL), SDL_GetModState());
	}

SHORT PlatformGetKeyState(int vk)
	{
	SDL_Keymod mod = SDL_GetModState();
	SHORT result = PlatformGetAsyncKeyState(vk);
	if (vk == VK_NUMLOCK && (mod & KMOD_NUM))
		result |= 0x0001;

	return result;
	}

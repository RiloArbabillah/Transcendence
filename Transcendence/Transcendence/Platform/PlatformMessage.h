//	PlatformMessage.h
//	macOS message payload utilities.
//
//	The macOS shell delivers Win32 messages (WM_KEYDOWN, WM_MOUSEMOVE, ...) to
//	the game UI through a queue. Win32 packs mouse coordinates and wheel deltas
//	into the message payload as pairs of signed 16-bit values, and unpacks them
//	with the sign-extending GET_X_LPARAM/GET_Y_LPARAM macros. The helpers here
//	are the single place where the macOS shell packs and unpacks those payloads,
//	so that the packing and the unpacking cannot drift apart.
//
//	They are pure functions over their arguments and live in their own
//	translation unit, so they can be unit tested without a window, an event
//	loop, or an initialized SDL video subsystem.
//
//	The queue entry points themselves (PlatformPostMessage,
//	PlatformPeekMessage, PlatformSendMessage, ...) are declared in Kernel.h,
//	next to the Win32 shims that call them.
//
//	See docs/migrate_to_osx/platform-event-utilities.md.

#pragma once

#include "Kernel.h"

//	Packs a point the way Win32 packs one into an LPARAM: the low word holds x
//	and the high word holds y, each as a signed 16-bit value.
//
//	Values outside [-32768, 32767] are clamped to the nearest bound. Win32
//	wraps instead, which turns a coordinate on a monitor to the left of the
//	origin into a coordinate on the right; clamping keeps the value on the same
//	side of the desktop. Window-client coordinates are always in range; only
//	the screen coordinates carried by WM_MOUSEWHEEL can leave it.

DWORD PlatformPackPoint (int x, int y);

//	Unpacks a point packed by PlatformPackPoint, sign-extending both halves the
//	way GET_X_LPARAM/GET_Y_LPARAM do. Either output pointer may be null.

void PlatformUnpackPoint (DWORD dwPoint, int *px, int *py);

//	Packs a WM_MOUSEWHEEL wParam the way Win32 does: the low word holds the
//	MK_* key flags and the high word holds the wheel delta.

DWORD PlatformPackMouseWheel (WORD wKeyFlags, int iDelta);

//	The WM_MOUSEWHEEL accessors that match PlatformPackMouseWheel.

int PlatformUnpackMouseWheelDelta (DWORD dwWParam);
WORD PlatformUnpackMouseWheelFlags (DWORD dwWParam);

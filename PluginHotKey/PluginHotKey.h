/*
Copyright (C) 2014 Brian Ferguson

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __PLUGIN_HOTKEY_H__
#define __PLUGIN_HOTKEY_H__

#include "Stdafx.h"

struct KeyInfo
{
	WCHAR* name;
	short number;
};

const KeyInfo g_VirtualKeys[] =
{
	{ L"LBUTTON", VK_LBUTTON },
	{ L"RBUTTON", VK_RBUTTON },
	{ L"MBUTTON", VK_MBUTTON },
	{ L"XBUTTON1", VK_XBUTTON1 },
	{ L"XBUTTON2", VK_XBUTTON2 },
	{ L"BACKSPACE", VK_BACK },
	{ L"TAB", VK_TAB },
	{ L"ENTER", VK_RETURN },
	{ L"SHIFT", VK_SHIFT },
	{ L"CTRL", VK_CONTROL },
	{ L"ALT", VK_MENU },
	{ L"PAUSE", VK_PAUSE },
	{ L"CAPSLOCK", VK_CAPITAL },
	{ L"ESCAPE", VK_ESCAPE },
	{ L"SPACE", VK_SPACE },
	{ L"PAGEUP", VK_PRIOR },
	{ L"PAGEDOWN", VK_NEXT },
	{ L"END", VK_END },
	{ L"HOME", VK_HOME },
	{ L"LEFT", VK_LEFT },
	{ L"UP", VK_UP },
	{ L"RIGHT", VK_RIGHT },
	{ L"DOWN", VK_DOWN },
	{ L"PRINTSCREEN", VK_SNAPSHOT },
	{ L"INSERT", VK_INSERT },
	{ L"DELETE", VK_DELETE },
	{ L"LWIN", VK_LWIN },
	{ L"RWIN", VK_RWIN },
	{ L"MENU", VK_APPS },
	{ L"NUM0", VK_NUMPAD0 },
	{ L"NUM1", VK_NUMPAD1 },
	{ L"NUM2", VK_NUMPAD2 },
	{ L"NUM3", VK_NUMPAD3 },
	{ L"NUM4", VK_NUMPAD4 },
	{ L"NUM5", VK_NUMPAD5 },
	{ L"NUM6", VK_NUMPAD6 },
	{ L"NUM7", VK_NUMPAD7 },
	{ L"NUM8", VK_NUMPAD8 },
	{ L"NUM9", VK_NUMPAD9 },
	{ L"MULT", VK_MULTIPLY },
	{ L"ADD", VK_ADD },
	{ L"SUBTRACT", VK_SUBTRACT },
	{ L"DECIMAL", VK_DECIMAL },
	{ L"DIVIDE", VK_DIVIDE },
	{ L"F1", VK_F1 },
	{ L"F2", VK_F2 },
	{ L"F3", VK_F3 },
	{ L"F4", VK_F4 },
	{ L"F5", VK_F5 },
	{ L"F6", VK_F6 },
	{ L"F7", VK_F7 },
	{ L"F8", VK_F8 },
	{ L"F9", VK_F9 },
	{ L"F10", VK_F10 },
	{ L"F11", VK_F11 },
	{ L"F12", VK_F12 },
	{ L"F13", VK_F13 },
	{ L"F14", VK_F14 },
	{ L"F15", VK_F15 },
	{ L"F16", VK_F16 },
	{ L"F17", VK_F17 },
	{ L"F18", VK_F18 },
	{ L"F19", VK_F19 },
	{ L"F20", VK_F20 },
	{ L"F21", VK_F21 },
	{ L"F22", VK_F22 },
	{ L"F23", VK_F23 },
	{ L"F24", VK_F24 },
	{ L"NUMLOCK", VK_NUMLOCK },
	{ L"SCROLLLOCK", VK_SCROLL },
	{ L"LSHIFT", VK_LSHIFT },
	{ L"RSHIFT", VK_RSHIFT },
	{ L"LCTRL", VK_LCONTROL },
	{ L"RCTRL", VK_RCONTROL },
	{ L"LALT", VK_LMENU },
	{ L"RALT", VK_RMENU },
	{ L"COLON", VK_OEM_1 },					// :;
	{ L"PLUS", VK_OEM_PLUS },				// +=
	{ L"COMMA", VK_OEM_COMMA },				// ,<
	{ L"MINUS", VK_OEM_MINUS },				// -_
	{ L"PERIOD", VK_OEM_PERIOD },			// .>
	{ L"FORWARDSLASH", VK_OEM_2 },			// /?
	{ L"BACKTICK", VK_OEM_3 },				// `~
	{ L"LBRACKET", VK_OEM_4 },				// [{
	{ L"BACKSLASH", VK_OEM_5 },				// \|
	{ L"RBRACKET", VK_OEM_6 },				// ]}
	{ L"QUOTE", VK_OEM_7 }					// '"
};

struct Measure
{
	std::wstring action;
	std::wstring keys;

	std::vector<short> virtualKeys;

	bool toggle;							// Toggle key state
	bool hasToggle;							// Key is either CapsLock, NumLock, or ScrollLock
	bool hasShift;							// Keys contain SHIFT, but not LSHIFT or RSHIFT
	bool hasCTRL;							// Keys contain CTRL, but not LCTRL or RCTRL
	bool hasALT;							// Keys contain ALT, but not LALT or RALT

	void* skin;
	void* rm;

	Measure() :
		action(),
		keys(),
		virtualKeys(),
		toggle(false),
		hasToggle(false),
		hasShift(false),
		hasCTRL(false),
		hasALT(false),
		skin(),
		rm()
	{ }
};

/*
** From Rainmeter\ConfigParser.cpp
**
** Splits the string from the delimiters.
** Now trims empty element in vector and white-space in each string.
**
** Modified from http://www.digitalpeer.com/id/simple
*/
static std::vector<std::wstring> Tokenize(const std::wstring& str, const std::wstring delimiters = L" ")
{
	std::vector<std::wstring> tokens;

	size_t lastPos, pos = 0;
	do
	{
		lastPos = str.find_first_not_of(delimiters, pos);

		if (lastPos == std::wstring::npos)
			break;

		pos = str.find_first_of(delimiters, lastPos + 1);
		std::wstring token = str.substr(lastPos, pos - lastPos);	// len = (pos != std::wstring::npos) ? pos - lastPos : pos

		size_t pos2 = token.find_first_not_of(L" \t\r\n");
		if (pos2 != std::wstring::npos)
		{
			size_t lastPos2 = token.find_last_not_of(L" \t\r\n");

			if (pos2 != 0 || lastPos2 != (token.size() - 1))
			{
				token.assign(token, pos2, lastPos2 - pos2 + 1);		// Trim white-space
			}

			tokens.push_back(token);
		}

		if (pos == std::wstring::npos)
			break;

		++pos;

	} while (true);

	return tokens;
}

#endif

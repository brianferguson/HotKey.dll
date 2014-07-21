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

#include "PluginHotKey.h"

static std::vector<Measure*> g_Measures;
HINSTANCE g_Instance = nullptr;
HHOOK g_Hook;

void RemoveMeasure(Measure* measure);
void ParseKeys(Measure* measure);
LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void RemoveVariations(std::vector<short>& keys, const bool hasShift, const bool hasCTRL, const bool hasALT);

LPCWSTR g_ErrRange = L"Invalid HotKey: %s";
LPCWSTR g_ErrEmpty = L"Missing \"Keys\" option.";
LPCWSTR g_ErrHook =  L"Could not start the keyboard hook.";

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_Instance = hinstDLL;

		// Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH notification calls
		DisableThreadLibraryCalls(hinstDLL);
		break;
	}

	return TRUE;
}

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	Measure* measure = new Measure;
	*data = measure;

	measure->skin = RmGetSkin(rm);
	measure->rm = rm;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure* measure = (Measure*)data;

	measure->action = RmReadString(rm, L"Action", L"");

	std::wstring keys = RmReadString(rm, L"HotKey", L"");
	if (keys.empty())
	{
		RmLog(rm, LOG_WARNING, g_ErrEmpty);
		RemoveMeasure(measure);
		return;
	}

	// Test whether if keys has changed (or first run)
	if (keys != measure->keys)
	{
		measure->keys = keys;
		measure->virtualKeys.clear();
		measure->hasShift = measure->hasCTRL = measure->hasALT = false;

		short status = 0;
		if (_wcsicmp(keys.c_str(), L"CAPSLOCK STATUS") == 0)
		{
			status = VK_CAPITAL;
			measure->hasToggle = true;
		}
		else if (_wcsicmp(keys.c_str(), L"NUMLOCK STATUS") == 0)
		{
			status = VK_NUMLOCK;
			measure->hasToggle = true;
		}
		else if (_wcsicmp(keys.c_str(), L"SCROLLLOCK STATUS") == 0)
		{
			status = VK_SCROLL;
			measure->hasToggle = true;
		}
		else
		{
			measure->hasToggle = measure->toggle = false;
		}

		if (measure->hasToggle)
		{
			measure->toggle = LOWORD(GetKeyState(status)) ? true : false;
			if (!measure->action.empty())
			{
				RmExecute(measure->skin, measure->action.c_str());
			}

			measure->virtualKeys.push_back(status);
		}
		else
		{
			ParseKeys(measure);
		}

		if (std::find(g_Measures.begin(), g_Measures.end(), measure) == g_Measures.end())
		{
			g_Measures.push_back(measure);

			// Start the keyboard hook
			if (g_Measures.size() == 1)
			{
				if (!(g_Hook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, NULL, 0)))
				{
					RmLog(rm, LOG_WARNING, g_ErrHook);
					RemoveMeasure(measure);
				}
			}
		}
	}
}

PLUGIN_EXPORT double Update(void* data)
{
	Measure* measure = (Measure*)data;
	return measure->toggle ? 1.0 : 0.0;
}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure* measure = (Measure*)data;
	RemoveMeasure(measure);
	delete measure;
}

void RemoveMeasure(Measure* measure)
{
	std::vector<Measure*>::iterator found = std::find(g_Measures.begin(), g_Measures.end(), measure);
	if (found != g_Measures.end())
	{
		g_Measures.erase(found);
	}

	if (g_Measures.empty())
	{
		UnhookWindowsHookEx(g_Hook);
	}
}

void ParseKeys(Measure* measure)
{
	std::vector<std::wstring> tokens = Tokenize(measure->keys);
	for (auto& key : tokens)
	{
		long number = 0;
		size_t keySize = key.size();
		bool found = false;

		if (keySize == 1 && std::iswprint(key[0]))					// Convert single character
		{
			number = LOBYTE(VkKeyScanEx(key[0], GetKeyboardLayout(0)));
			found = true;
		}
		else if (keySize > 1 && key[0] == L'0')						// Convert hex, oct, binary to decimal
		{
			switch (key[1])
			{
			case L'x':
				number = wcstol(key.c_str(), nullptr, 16);
				found = true;
				break;

			case L'o':
				number = wcstol(key.c_str() + 2, nullptr, 8);
				found = true;
				break;

			case L'b':
				number = wcstol(key.c_str() + 2, nullptr, 2);
				found = true;
				break;

			default:
				break;
			}
		}
		else if (keySize > 1)										// Convert string
		{
			for (const auto& iter : g_VirtualKeys)
			{
				if (_wcsicmp(key.c_str(), iter.name) == 0)
				{
					number = iter.number;
					found = true;
					break;
				}
			}
		}

		if (!found)													// Assume key is in decimal form already
		{
			number = wcstol(key.c_str(), nullptr, 10);
		}

		// Check range, should be between VK_LBUTTON(0x01, 1) and VK_OEM_CLEAR(0xFE, 254)
		//	per http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
		if (number < VK_LBUTTON || number > VK_OEM_CLEAR)
		{
			RmLogF(measure->rm, LOG_ERROR, g_ErrRange, key.c_str());
			RemoveMeasure(measure);
			return;
		}

		measure->virtualKeys.push_back((short)number);

		// When L/R control keys are used, the system also sends the generic control key as well
		if (number == VK_LSHIFT || number == VK_RSHIFT) measure->virtualKeys.push_back(VK_SHIFT);
		else if (number == VK_LCONTROL || number == VK_RCONTROL) measure->virtualKeys.push_back(VK_CONTROL);
		else if (number == VK_LMENU || number == VK_RMENU) measure->virtualKeys.push_back(VK_MENU);
		else if (number == VK_SHIFT) measure->hasShift = true;
		else if (number == VK_CONTROL) measure->hasCTRL = true;
		else if (number == VK_MENU) measure->hasALT = true;
	}

	// Sort, remove duplicates, and remove L and R variations (if needed)
	std::sort(measure->virtualKeys.begin(), measure->virtualKeys.end());
	measure->virtualKeys.erase(std::unique(measure->virtualKeys.begin(), measure->virtualKeys.end()), measure->virtualKeys.end());
	RemoveVariations(measure->virtualKeys, measure->hasShift, measure->hasCTRL, measure->hasALT);
}

LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		switch (wParam)
		{
			case WM_SYSKEYUP:
			case WM_KEYUP:
			{
				std::vector<short> current;
				bool hasToggle = false;
				short keyCode = 0;
				for (short i = 0; i < 256; ++i)
				{
					if (GetAsyncKeyState(i) & 0x8000)
					{
						current.push_back(i);

						// In case of the measure is a toggle key
						if (i == VK_CAPITAL || i == VK_SCROLL || i == VK_NUMLOCK)
						{
							hasToggle = true;
							keyCode = i;
						}
					}
				}

				for (auto& measure : g_Measures)
				{
					std::vector<short> temp = current;
					RemoveVariations(temp, measure->hasShift, measure->hasCTRL, measure->hasALT);

					// Compare all the keys, or just the toggle keys
					if (temp == measure->virtualKeys ||
						(hasToggle && measure->hasToggle && keyCode == measure->virtualKeys[0]))
					{
						if (measure->hasToggle)
						{
							measure->toggle = !measure->toggle;
						}

						if (!measure->action.empty())
						{
							RmExecute(measure->skin, measure->action.c_str());
						}
					}
				}
			}
			break;
		}
	}

	return CallNextHookEx(g_Hook, nCode, wParam, lParam);
}

inline void RemoveVariations(std::vector<short>& keys, const bool hasShift, const bool hasCTRL, const bool hasALT)
{
	auto remove = [&](const bool modifier, const short key) -> void
	{
		if (modifier) keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());
	};

	remove(hasShift, VK_LSHIFT);
	remove(hasShift, VK_RSHIFT);
	remove(hasCTRL, VK_LCONTROL);
	remove(hasCTRL, VK_RCONTROL);
	remove(hasALT, VK_LMENU);
	remove(hasALT, VK_RMENU);

	keys.shrink_to_fit();
}

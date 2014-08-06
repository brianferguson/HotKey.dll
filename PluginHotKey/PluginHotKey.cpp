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

static std::vector<Measure*> g_UpMeasures;
static std::vector<Measure*> g_DownMeasures;
static HINSTANCE g_Instance = nullptr;
static HHOOK g_Hook = nullptr;
static bool g_IsHookActive = false;

void RemoveMeasure(Measure* measure, const bool isUp = true, const bool isDown = true);
void ParseKeys(Measure* measure);
LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

LPCWSTR g_ErrRange = L"Invalid HotKey: %s";
LPCWSTR g_ErrEmpty = L"Missing \"Keys\" option.";
LPCWSTR g_ErrHook = L"Could not %s the keyboard hook.";
LPCWSTR g_ErrCommand = L"Invalid command: %s";

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

	std::wstring keys = RmReadString(rm, L"HotKey", L"");
	if (keys.empty())
	{
		RmLog(rm, LOG_WARNING, g_ErrEmpty);
		RemoveMeasure(measure);
		return;
	}

	measure->upAction = RmReadString(rm, L"KeyUpAction", L"", FALSE);
	measure->downAction = RmReadString(rm, L"KeyDownAction", L"", FALSE);
	measure->showAllKeys = RmReadInt(rm, L"ShowAllKeys", 0) != 0;

	// Only update if the "HotKey" option was changed
	if (keys != measure->keys)
	{
		measure->keys = keys;
		measure->virtualKeys.clear();

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
			if (!measure->downAction.empty())
			{
				RmExecute(measure->skin, measure->downAction.c_str());
			}

			measure->virtualKeys.push_back(status);
		}
		else
		{
			ParseKeys(measure);
		}

		// If there is an "Up" action, push the measure to the global "Up" list (if it doesn't exist).
		// Else if there isn't an "Up" action AND the measure is in the global list, remove it.
		if (std::find(g_UpMeasures.begin(), g_UpMeasures.end(), measure) == g_UpMeasures.end())
		{
			if (!measure->upAction.empty())
			{
				g_UpMeasures.push_back(measure);
			}
		}
		else if (measure->upAction.empty())
		{
			RemoveMeasure(measure, true, false);
		}

		// Add measure to global "Down" list (if it doesn't exist). Also add any "Toggle" measures to
		// make sure they are updated.
		if (std::find(g_DownMeasures.begin(), g_DownMeasures.end(), measure) == g_DownMeasures.end())
		{
			if (!measure->downAction.empty() || measure->hasToggle || measure->showAllKeys)
			{
				g_DownMeasures.push_back(measure);
			}
		}
		else if (measure->downAction.empty())
		{
			RemoveMeasure(measure, false, true);
		}

		// Start the keyboard hook
		if (!g_IsHookActive &&
			((!measure->upAction.empty() || !measure->downAction.empty() || measure->hasToggle || measure->showAllKeys) &&
			(g_UpMeasures.size() + g_DownMeasures.size()) >= 1))
		{
			g_Hook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, NULL, 0);
			if (g_Hook)
			{
				g_IsHookActive = true;
			}
			else
			{
				RmLogF(rm, LOG_ERROR, g_ErrHook, L"start");
				RemoveMeasure(measure);
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

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	Measure* measure = (Measure*)data;

	if (_wcsicmp(args, L"Start") == 0)
	{
		measure->isActive = true;
	}
	else if (_wcsicmp(args, L"Stop") == 0)
	{
		measure->isActive = false;
	}
	else if (_wcsicmp(args, L"Toggle") == 0)
	{
		measure->isActive = !measure->isActive;
	}
	else
	{
		RmLogF(measure->rm, LOG_WARNING, g_ErrCommand, args);
	}
}

void RemoveMeasure(Measure* measure, const bool isUp, const bool isDown)
{
	auto remove = [&](const bool& state, std::vector<Measure*>& gMeasures) -> void
	{
		if (state)
		{
			std::vector<Measure*>::iterator found = std::find(gMeasures.begin(), gMeasures.end(), measure);
			if (found != gMeasures.end())
			{
				gMeasures.erase(found);
			}
		}
	};

	remove(isUp, g_UpMeasures);
	remove(isDown, g_DownMeasures);

	if (g_IsHookActive && g_UpMeasures.empty() && g_DownMeasures.empty())
	{
		while (g_Hook && UnhookWindowsHookEx(g_Hook) == FALSE)
		{
			RmLogF(measure->rm, LOG_ERROR, g_ErrHook, L"stop");
		}

		g_Hook = nullptr;
		g_IsHookActive = false;
	}
}

void ParseKeys(Measure* measure)
{
	bool hasAlt = false, hasCtrl = false, hasShift = false;

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
		// per http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
		if (number < VK_LBUTTON || number > VK_OEM_CLEAR)
		{
			RmLogF(measure->rm, LOG_ERROR, g_ErrRange, key.c_str());
			RemoveMeasure(measure);
			return;
		}

		measure->virtualKeys.push_back((short)number);

		if (number == VK_SHIFT) hasShift = true;
		else if (number == VK_CONTROL) hasCtrl = true;
		else if (number == VK_MENU) hasAlt = true;
	}

	// Sort lowest to highest
	std::sort(measure->virtualKeys.begin(), measure->virtualKeys.end());

	// Remove duplicates
	measure->virtualKeys.erase(std::unique(measure->virtualKeys.begin(), measure->virtualKeys.end()), measure->virtualKeys.end());

	// Remove any L/R variations (only if the HotKey has the generic modifier)
	// ie. SHIFT overrides LSHIFT
	auto remove = [&](const bool modifier, const short key) -> void
	{
		if (modifier)
		{
			measure->virtualKeys.erase(
				std::remove(measure->virtualKeys.begin(), measure->virtualKeys.end(), key),
				measure->virtualKeys.end());
		}
	};

	remove(hasShift, VK_LSHIFT);
	remove(hasShift, VK_RSHIFT);
	remove(hasCtrl, VK_LCONTROL);
	remove(hasCtrl, VK_RCONTROL);
	remove(hasAlt, VK_LMENU);
	remove(hasAlt, VK_RMENU);

	measure->virtualKeys.shrink_to_fit();
}

LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

		auto doAction = [&](const bool isUpMeasure) -> void
		{
			for (auto& measure : (isUpMeasure ? g_UpMeasures : g_DownMeasures))
			{
				// Log keystoke if needed
				if (measure->showAllKeys)
				{
					WCHAR text[32];
					DWORD dwCode = MapVirtualKey(kbdStruct->vkCode, 0) << 16;
					if (GetKeyNameText(dwCode, text, sizeof(text) / sizeof(WCHAR)) == 0)
					{
						dwCode |= (1 << 24);
						if (GetKeyNameText(dwCode, text, sizeof(text) / sizeof(WCHAR)) == 0)
						{
							wcsncpy_s(text, L"Unknown Key", 32);
						}
					}
					RmLogF(measure->rm, LOG_NOTICE, L"Key: %s, Hex: 0x%X (%i), Scan Code: 0x%X (%i), State: %s, Time: %i",
						text, kbdStruct->vkCode, kbdStruct->vkCode, kbdStruct->scanCode, kbdStruct->scanCode,
						isUpMeasure ? L"Up" : L"Down", kbdStruct->time);
				}

				// Only execute if the measure is active
				if (measure->isActive)
				{
					if (std::find(measure->virtualKeys.begin(), measure->virtualKeys.end(), (short)kbdStruct->vkCode) != measure->virtualKeys.end())
					{
						bool executeAction = true;
						for (const auto& key : measure->virtualKeys)
						{
							if (key != (short)kbdStruct->vkCode && (!(GetAsyncKeyState(key) & 0x8000)))
							{
								executeAction = false;
								break;
							}
						}

						// Handle toggle keys.
						// MSDN states that the low-order bit of the return value of GetKeyState will indicate
						// if the toggle is "on" or not, however after some testing, it seems it more complicated
						// when the toggle key is held down, in which the low-order bit seems to be reversed.
						// Instead of testing the low-order bit, just test for 0 and -127. This may need to be
						// updated in the future.
						if (measure->hasToggle)
						{
							const short state = GetKeyState(measure->virtualKeys[0]);
							measure->toggle = state == 0 || state == -127 ? true : false;

						}

						// Since toggle keys are added to the "Down" measures no matter what,
						// make sure there is a down "Action" before executing.
						if (executeAction && !measure->downAction.empty())
						{
							RmExecute(measure->skin, isUpMeasure ?
								measure->upAction.c_str() :
								measure->downAction.c_str());
						}
					}
				}
			}
		};

		switch (wParam)
		{
		case WM_SYSKEYUP:
		case WM_KEYUP:
			doAction(true);
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			doAction(false);
			break;
		}
	}

	return CallNextHookEx(g_Hook, nCode, wParam, lParam);
}

#pragma once
#include <Windows.h>
#include <unordered_map>
#include <WinUser.h>

enum class KeyState
{
	KeyStateUp = 0,
	KeyStateDown = 1,
};

struct Input
{
	std::string mName;
	int mVirtKey;
};

class InputState
{
public:
	InputState();
	void PollKeyboard();
	void PollMouse();
	void OnKeyStateChange(std::string keyCode, KeyState newState);
	bool IsKeyDown(std::string keyCode);

private:
	std::unordered_map<std::string, int> mKeyCodes;
	std::unordered_map<int, KeyState> mKeyStates;

	std::vector<Input> mInputs =
	{
		{"LEFTMOUSE", VK_LBUTTON},
		{"RIGHTMOUSE", VK_RBUTTON},
		{"MIDDLEMOUSE", VK_MBUTTON},
		{"BACKSPACE", VK_BACK},
		{"TAB", VK_TAB},
		{"ENTER", VK_RETURN},
		{"SHIFT", VK_SHIFT},
		{"CONTROL", VK_CONTROL},
		{"ALT", VK_MENU},
		{"ESC", VK_ESCAPE},
		{"SPACE", VK_SPACE},
		{"A",	0x41},
		{"B",	0x42},
		{"C",	0x43},
		{"D",	0x44},
		{"E",	0x45},
		{"F",	0x46},
		{"G",	0x47},
		{"H",	0x48},
		{"I",	0x49},
		{"J",	0x4A},
		{"K",	0x4B},
		{"L",	0x4C},
		{"M",	0x4D},
		{"N",	0x4E},
		{"O",	0x4F},
		{"P",	0x50},
		{"Q",	0x51},
		{"R",	0x52},
		{"S",	0x53},
		{"T",	0x54},
		{"U",	0x55},
		{"V",	0x56},
		{"W",	0x57},
		{"X",	0x58},
		{"Y",	0x59},
		{"Z",	0x5A},
		{"~",   VK_OEM_3},
	};
};
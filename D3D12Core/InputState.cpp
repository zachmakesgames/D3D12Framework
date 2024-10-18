#include "InputState.h"

InputState::InputState()
{
	for (Input inp : mInputs)
	{
		mKeyCodes[inp.mName] = inp.mVirtKey;
		mKeyStates[inp.mVirtKey] = KeyState::KeyStateUp;
	}
}

void InputState::PollKeyboard()
{
	for (auto& iter : mKeyCodes)
	{
		std::string keyCode = iter.first;
		int virtKey = iter.second;

		KeyState currentState = mKeyStates[virtKey];
		std::string msg = "Key state for key " + keyCode + " has changed to";

		if (GetKeyState(virtKey) & 0x8000)
		{
			if (currentState != KeyState::KeyStateDown)
			{
				msg += " down\r\n";
				OutputDebugStringA(msg.c_str());
				mKeyStates[virtKey] = KeyState::KeyStateDown;
			}
		}
		else
		{
			if (currentState != KeyState::KeyStateUp)
			{
				msg += " up\r\n";
				OutputDebugStringA(msg.c_str());
				mKeyStates[virtKey] = KeyState::KeyStateUp;
			}
		}
	}
}

void InputState::PollMouse()
{

}

void InputState::OnKeyStateChange(std::string keyCode, KeyState newState)
{

}

bool InputState::IsKeyDown(std::string keyCode)
{
	if (mKeyCodes.find(keyCode) != mKeyCodes.end())
	{
		int virtKey = mKeyCodes[keyCode];
		return (mKeyStates[virtKey] == KeyState::KeyStateDown);
	}
}
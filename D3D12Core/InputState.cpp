#include "InputState.h"

InputState::InputState()
{
	for (Input inp : mInputs)
	{
		mKeyCodes[inp.mName] = inp.mVirtKey;
		mKeyStates[inp.mVirtKey] = KeyState::KeyStateUp;
	}

	// Init stuff for DirectInput, if I ever decide to us it
	//HINSTANCE appInst = GetModuleHandle(NULL);
	//HRESULT hr = DirectInput8Create(appInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&mDirectInput, NULL);
	//if (FAILED(hr))
	//{
	//	OutputDebugStringA("Failed to init direct input!\r\n");
	//}

	//hr = mDirectInput->CreateDevice(GUID_SysMouse, &mDIMouse, NULL);
	//if (FAILED(hr))
	//{
	//	OutputDebugStringA("Failed to init direct input mouse device!\r\n");
	//}

	//hr = mDIMouse->SetDataFormat(&c_dfDIMouse);
	//if (FAILED(hr))
	//{
	//	OutputDebugStringA("Failed to set mouse data format!\r\n");
	//}

	//// May need to set cooperation level, but we need the window handle
	//mDIMouse->Acquire();
}

InputState::~InputState()
{
	//mDIMouse->Unacquire();
	//mDirectInput->Release();
}

void InputState::PollKeyboard()
{
	for (auto& iter : mKeyCodes)
	{
		std::string keyCode = iter.first;
		int virtKey = iter.second;

		KeyState currentState = mKeyStates[virtKey];

		if (GetKeyState(virtKey) & 0x8000)
		{
			if (currentState != KeyState::KeyStateDown)
			{
				mKeyStates[virtKey] = KeyState::KeyStateDown;
			}
		}
		else
		{
			if (currentState != KeyState::KeyStateUp)
			{
				mKeyStates[virtKey] = KeyState::KeyStateUp;
			}
		}
	}
}

void InputState::PollMouse(HWND window)
{
	POINT newPosition = {};
	GetCursorPos(&newPosition);

	//DIMOUSESTATE newState;
	//mDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &newState);

	// Can use this to transform the mouse position to client space instead
	// of to screen space
	/*if (window != NULL)
	{
		ScreenToClient(window, &newPosition);
	}*/
	mMouseDeltaPosition = { newPosition.x - mMousePosition.x, newPosition.y - mMousePosition.y };
	mMousePosition = newPosition;
	
	//mMouseDeltaPosition = { newState.lX - mMousePosition.x, newState.lY - mMousePosition.y };
	//mMousePosition = { newState.lX, newState.lY };
}

POINT InputState::GetMousePosition()
{
	return mMousePosition;
}

POINT InputState::GetMouseChange()
{
	return mMouseDeltaPosition;
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
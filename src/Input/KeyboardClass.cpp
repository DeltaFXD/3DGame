#include "KeyboardClass.h"

KeyboardClass::KeyboardClass()
{
	for (int i = 0; i < 256; i++)
	{
		this->keyStates[i] = false; //Initialze all key states to off (false)
		this->keyToggleStates[i] = false; //Initialize all key toggle states to off (false);
	}
}

bool KeyboardClass::KeyIsPressed(const unsigned char keycode)
{
	return this->keyStates[keycode];
}

bool KeyboardClass::KeyIsToggled(const unsigned char keycode)
{
	return this->keyToggleStates[keycode];
}

bool KeyboardClass::KeyBufferIsEmpty()
{
	return this->keyBuffer.empty();
}

bool KeyboardClass::CharBufferIsEmpty()
{
	return this->charBuffer.empty();
}

KeyboardEvent KeyboardClass::ReadKey()
{
	if (this->keyBuffer.empty()) //If no keys to be read
	{ 
		return KeyboardEvent(); //Return empty keyboard event
	}
	else
	{
		KeyboardEvent e = this->keyBuffer.front(); //Get first Keyboard Event from queue
		this->keyBuffer.pop(); //Remove first item from queue
		return e; //Return Keyboard Event
	}
}

unsigned char KeyboardClass::ReadChar()
{
	if (this->charBuffer.empty()) //If no keys to be read
	{ 
		return 0u; //Return 0 (NULL char)
	}
	else
	{
		unsigned char c = this->charBuffer.front(); //Get first char from queue
		this->charBuffer.pop(); //Remove first item from queue
		return c; //Return char
	}
}

void KeyboardClass::OnKeyPressed(const unsigned char key)
{
	if (!this->keyStates[key])
	{
		this->keyToggleStates[key] = !this->keyToggleStates[key];
	}
	this->keyStates[key] = true;
	this->keyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Press, key));
}

void KeyboardClass::OnKeyReleased(const unsigned char key)
{
	this->keyStates[key] = false;
	this->keyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Release, key));
}

void KeyboardClass::OnChar(const unsigned char key)
{
	this->charBuffer.push(key);
}

void KeyboardClass::EnableAutoRepeatKeys()
{
	this->autoRepeatKeys = true;
}

void KeyboardClass::DisableAutoRepeatKeys()
{
	this->autoRepeatKeys = false;
}

void KeyboardClass::EnableAutoRepeatChars()
{
	this->autoRepeatChars = true;
}

void KeyboardClass::DisableAutoRepeatChars()
{
	this->autoRepeatChars = false;
}

bool KeyboardClass::IsKeyAutoRepeat()
{
	return this->autoRepeatKeys;
}

bool KeyboardClass::IsCharsAutoRepeat()
{
	return this->autoRepeatChars;
}
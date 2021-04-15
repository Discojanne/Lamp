#pragma once

#include "InputListener.h"
#include "Point.h"
#include <map>

class InputSystem
{
public:
	InputSystem();
	~InputSystem();

	void update();
	void addListener(InputListener* listener);
	void removeListener(InputListener* listener);

	static InputSystem* get();

private:
	std::map<InputListener*, InputListener*> m_mapListeners;

	unsigned char m_keys_state[256] = {};
	unsigned char m_old_keys_state[256] = {};

	Point m_old_mouse_pos;
	bool m_first_time = true;
};


#include "InputSystem.h"
#include <Windows.h>

InputSystem::InputSystem()
{
}

InputSystem::~InputSystem()
{
}

void InputSystem::update()
{
    POINT current_mouse_pos = {};
    ::GetCursorPos(&current_mouse_pos);

    if (m_first_time)
    {
        m_old_mouse_pos = Point(current_mouse_pos.x, current_mouse_pos.y);
        m_first_time = false;
    }

    if (current_mouse_pos.x != m_old_mouse_pos.m_x || current_mouse_pos.y != m_old_mouse_pos.m_y)
    {
        std::map<InputListener*, InputListener*>::iterator it = m_mapListeners.begin();

        while (it != m_mapListeners.end())
        {
            it->second->onMouseMove(Point(current_mouse_pos.x - m_old_mouse_pos.m_x, current_mouse_pos.y - m_old_mouse_pos.m_y));
            ++it;
        }
    }
    m_old_mouse_pos = Point(current_mouse_pos.x, current_mouse_pos.y);



    if (::GetKeyboardState(m_keys_state)) 
    {
        for (unsigned int i = 0; i < 256; i++)
        {
            // Key is down
            if (m_keys_state[i] & 0x80)
            {
                std::map<InputListener*, InputListener*>::iterator it = m_mapListeners.begin();

                while (it != m_mapListeners.end())
                {
                    if (i == VK_LBUTTON)
                    {
                        if (m_keys_state[i] != m_old_keys_state[i])
                        {
                            it->second->onLeftMouseDown(Point(current_mouse_pos.x, current_mouse_pos.y));
                        }
                    }
                    else if (VK_RBUTTON)
                    {
                        if (m_keys_state[i] != m_old_keys_state[i])
                        {
                            it->second->onRightMouseDown(Point(current_mouse_pos.x, current_mouse_pos.y));
                        }
                    }

                    it->second->onKeyDown(i);
                    ++it;
                }
            }
            else // key is up
            {
                if (m_keys_state[i] != m_old_keys_state[i])
                {
                    std::map<InputListener*, InputListener*>::iterator it = m_mapListeners.begin();

                    while (it != m_mapListeners.end())
                    {
                        if (i == VK_LBUTTON)
                            it->second->onLeftMouseUp(Point(current_mouse_pos.x, current_mouse_pos.y));
                        else if (VK_RBUTTON)
                            it->second->onRightMouseUp(Point(current_mouse_pos.x, current_mouse_pos.y));
                        
                        it->second->onKeyUp(i);

                        ++it;
                    }
                }
            }
        }
        // store current key state to old keys state buffer
        ::memcpy(m_old_keys_state, m_keys_state, sizeof(unsigned char) * 256);
    }
}

void InputSystem::addListener(InputListener* listener)
{
    m_mapListeners.insert(std::make_pair<InputListener*, InputListener*>
        (std::forward<InputListener*>(listener), std::forward<InputListener*>(listener)));
}

void InputSystem::removeListener(InputListener* listener)
{
    std::map<InputListener*, InputListener*>::iterator it = m_mapListeners.find(listener);

    if (it != m_mapListeners.end())
    {
        m_mapListeners.erase(it);
    }
}

InputSystem* InputSystem::get()
{
    static InputSystem is;
    return &is;
}

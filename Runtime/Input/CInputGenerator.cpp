#include "CInputGenerator.hpp"
#include "../CArchitectureMessage.hpp"
#include "../CArchitectureQueue.hpp"

namespace Retro
{

void CInputGenerator::Update(float dt, CArchitectureQueue& queue)
{
    /* Keyboard/Mouse first */
    CFinalInput kbInput = m_windowCb.getFinalInput(0, dt);
    bool kbUsed = false;

    /* Dolphin controllers next */
    for (int i=0 ; i<4 ; ++i)
    {
        bool connected;
        EStatusChange change = m_dolphinCb.getStatusChange(i, connected);
        if (change)
            queue.Push(std::move(MakeMsg::CreateControllerStatus(TargetGame, i, connected)));
        if (connected)
        {
            CFinalInput input = m_dolphinCb.getFinalInput(i, dt, m_leftDiv, m_rightDiv);
            if (i == 0) /* Merge KB input with first controller */
            {
                input |= kbInput;
                kbUsed = true;
            }
            queue.Push(std::move(MakeMsg::CreateUserInput(TargetGame, input)));
        }
    }

    /* Send straight keyboard input if no first controller present */
    if (!kbUsed)
        queue.Push(std::move(MakeMsg::CreateUserInput(TargetGame, kbInput)));
}

}

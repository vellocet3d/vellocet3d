#pragma once

namespace vel
{
    struct InputState
    {
        bool	keyW = false;
        bool	keyA = false;
        bool	keyS = false;
        bool	keyD = false;
        bool	keyEsc = false;
        bool	keySpace = false;
		bool	keyUp = false;
		bool	keyDown = false;
		bool	keyRight = false;
		bool	keyLeft = false;
        float	mouseXPos = 0.0f;
        float	mouseYPos = 0.0f;
        double	scrollX = 0;
        double	scrollY = 0;
    };
}


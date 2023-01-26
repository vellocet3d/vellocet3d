#pragma once

#include <string>

#include "vel/FontBitmap.h"
#include "vel/Actor.h"

namespace vel 
{
	struct TextActor 
	{
		std::string		name;
		std::string 	text;
		FontBitmap*		fontBitmap;
		Actor*			actor = nullptr;
		bool			requiresUpdate = false;

		void			updateText(std::string updatedText)
		{
			this->text = updatedText;
			this->requiresUpdate = true;
		}
	};
}
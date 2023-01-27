#pragma once

#include <string>

#include "vel/FontBitmap.h"
#include "vel/Actor.h"

namespace vel 
{
	enum TextActorAlignment {
		LEFT_ALIGN,
		CENTER_ALIGN,
		RIGHT_ALIGN
	};

	struct TextActor 
	{
		std::string				name;
		std::string 			text;
		FontBitmap*				fontBitmap;
		TextActorAlignment		alignment;
		Actor*					actor = nullptr;
		bool					requiresUpdate = false;

		void					updateText(std::string updatedText)
		{
			this->text = updatedText;
			this->requiresUpdate = true;
		}
	};
}
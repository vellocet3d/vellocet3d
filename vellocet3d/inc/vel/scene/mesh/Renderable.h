#pragma once


namespace vel
{
	typedef int GLsizei;
	struct Renderable
	{
		unsigned int	VAO;
		unsigned int	VBO;
		unsigned int    EBO;
		GLsizei			indiceCount;
	};    
}
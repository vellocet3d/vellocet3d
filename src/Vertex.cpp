#include "vel/Vertex.h"

namespace vel
{
    bool Vertex::operator==(const Vertex &v) const
    {
        return position == v.position && normal == v.normal && textureCoordinates == v.textureCoordinates;
    }
}
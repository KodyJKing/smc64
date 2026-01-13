#pragma once
#include "halo1/Halo1.hpp"

namespace Rewind {
    void rewind( Halo1::EntityRecord* rec, float timescale, float globalTimescale, Halo1::Entity& snap );
}
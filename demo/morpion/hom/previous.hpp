

#pragma once

#include "DDD.h"
#include "Hom.h"
#include "hom/const.hpp"


GHom
	previous(int cell,game_status_type status);

DDD
	previous(DDD& succ, Hom pred);

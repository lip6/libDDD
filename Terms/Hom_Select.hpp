#ifndef __HOM_SELECT_H_
#define __HOM_SELECT_H_

#include "SDD.h"
#include "SHom.h"

GShom select_hom(int type_condition,
	     const DataSet* condition,
		 const GShom& next = GShom::id) ;

GShom  select_deephom(int type_condition,
		 const GShom& condition,
		      const GShom& next) ;

GShom extract_value (int trigger, const GShom & extractor = GShom::id) ;


#endif // __HOM_SELECT_H_

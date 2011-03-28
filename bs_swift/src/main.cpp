/** 
 * @file main.cpp
 * @brief 
 * @author 
 * @version 
 * @date 2010-03-03
 */
//#include "amg_stdafx.h"
#include "pmis_coarse.h"
#include "pmis2_coarse.h"
#include "cljp_coarse.h"
#include "cljp2_coarse.h"
#include "ruge_coarse.h"

#include "simple_smbuilder.h"

#include "direct_pbuild.h"
#include "standart_pbuild.h"
#include "standart2_pbuild.h"
#include "standart3_pbuild.h"

//#include "amg_solver.h"

#include "coarse_tools.h"

#include "py_coarse_iface.h"
#include "py_pbuild_iface.h"

using namespace blue_sky;
using namespace blue_sky::python;
using namespace boost::python;

#define REG_TYPE(S)                     \
    res &= BS_KERNEL.register_type(*bs_init.pd_, S::bs_type()); BS_ASSERT (res);


namespace blue_sky {
  BLUE_SKY_PLUGIN_DESCRIPTOR_EXT ("swift", "1.0.0", "AMG linear solver", "Algebraic multi grid linear solver", "swift")

  BLUE_SKY_REGISTER_PLUGIN_FUN
  {
    //const plugin_descriptor &pd = *bs_init.pd_;

    bool res = true;
   
    REG_TYPE (pmis_coarse)
    REG_TYPE (pmis2_coarse)
    REG_TYPE (cljp_coarse)
    REG_TYPE (cljp2_coarse)
    REG_TYPE (ruge_coarse)
    
    REG_TYPE (simple_smbuilder)
    
    REG_TYPE (coarse_tools)

    REG_TYPE (direct_pbuild)
    REG_TYPE (standart_pbuild)
    REG_TYPE (standart2_pbuild)
    REG_TYPE (standart3_pbuild)

    //REG_TYPE (amg_solver)
    
    return res;
  }
}
#ifdef BSPY_EXPORTING_PLUGIN
BLUE_SKY_INIT_PY_FUN
{
  using namespace boost::python;

  python::py_export_coarse ();
  python::py_export_pbuild ();
  
}
#endif

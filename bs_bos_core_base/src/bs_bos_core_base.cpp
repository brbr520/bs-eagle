/**
 * \file bs_bos_core_base.cpp
 * \brief
 * \author Sergey Miryanov
 * \date 24.03.2009
 * */
#include "bs_bos_core_base_stdafx.h"

#include "py_bs_assert.h"
#include "named_pbase_access.h"
#include "data_dimens.h"
#include "py_strategies.h"
#include "auto_value.h"

using namespace blue_sky;

#ifdef BSPY_EXPORTING_PLUGIN
using namespace blue_sky::python;
using namespace boost::python;
#endif

namespace blue_sky {
  BLUE_SKY_PLUGIN_DESCRIPTOR_EXT ("bs_bos_core_base", "1.0.0", "BS_BOS_CORE_BASE", "BS_BOS_CORE_BASE", "bs_bos_core_base")

  BLUE_SKY_REGISTER_PLUGIN_FUN
  {
    init_bos_logs();
   
    const plugin_descriptor &pd = *bs_init.pd_;

    bool res = true;
    res &= BS_KERNEL.register_type (pd, property_base::bs_type ());
    res &= BS_KERNEL.register_type (pd, named_pbase::bs_type ());

    return res;
  }
}

#ifdef BSPY_EXPORTING_PLUGIN
template <typename T>
struct auto_value_to_python
{
  static PyObject *
  convert (auto_value <T> const &t)
  {
    return incref (object (t.data ()).ptr ());
  }

  static void
  make_known ()
  {
    to_python_converter <auto_value <T>, auto_value_to_python <T> > ();
  }
};

BLUE_SKY_INIT_PY_FUN
{
  auto_value_to_python <long>::make_known ();
  auto_value_to_python <unsigned long>::make_known ();
  auto_value_to_python <int>::make_known ();
  auto_value_to_python <unsigned int>::make_known ();
  auto_value_to_python <float>::make_known ();
  auto_value_to_python <double>::make_known ();
  auto_value_to_python <char>::make_known ();


  //python::py_export_assert ();
  python::py_export_named_pbase ("named_pbase");
  python::export_data_dimens ();
}
#endif 

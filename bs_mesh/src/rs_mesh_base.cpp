/*!
	\file rs_mesh_base.cpp
  \brief This file implements base class for reservoir simulation meshes
  \author Mark Khait
  \date 2009-07-20
  */
  
#include "bs_mesh_stdafx.h"
#include "rs_mesh_base.h"

using namespace blue_sky;


rs_mesh_base ::rs_mesh_base ()
{
  depths = give_kernel::Instance().create_object(bs_array<t_float>::bs_type());
  actnum_array = 0; 
  poro_array = 0;
  ntg_array = 0;
  multpv_array = 0;
  
}


void
rs_mesh_base ::init_props (const sp_hdm_t hdm)
{
  minpv = hdm->get_prop ()->get_f ("minimal_pore_volume");
  minsv = hdm->get_prop ()->get_f ("minimal_splice_volume");
  max_thickness = hdm->get_prop ()->get_f ("maximum_splice_thickness");
  darcy_constant = hdm->get_darcy_constant ();
  
  spv_float data_array;
  spv_int sp_actnum_array;
  
  sp_actnum_array = hdm->get_pool ()->get_i_data("ACTNUM");
  if (sp_actnum_array->size()) actnum_array = &(*sp_actnum_array)[0];
  
  n_elements = static_cast <t_long> (sp_actnum_array->size ());
  n_active_elements =  std::accumulate(sp_actnum_array->begin(), sp_actnum_array->end(),0);
  
  data_array = hdm->get_pool ()->get_fp_data("PORO");
  if (data_array->size()) poro_array = &(*data_array)[0];
  
  data_array = hdm->get_pool ()->get_fp_data("NTG");
  if (data_array && data_array->size()) ntg_array = &(*data_array)[0];
  
  data_array = hdm->get_pool ()->get_fp_data("MULTPV");
  if (data_array && data_array->size()) multpv_array = &(*data_array)[0];
}



int rs_mesh_base::init_int_to_ext()
{
  t_long n_ext = t_long(ext_to_int->size());
  if (n_ext == 0)
    return -1;
    
  int_to_ext->init (n_active_elements, 0);
  
  t_long *int_to_ext_data = int_to_ext->data ();
  t_long *ext_to_int_data = ext_to_int->data ();
  
  for (t_long i = 0; i < n_ext; i++)
    {
      if (ext_to_int_data[i] != -1)
        {
          if (ext_to_int_data[i] >= n_active_elements)
            {
              bs_throw_exception (boost::format ("ext_to_int[%d] >= %d") % i % n_active_elements);
            }
        
          int_to_ext_data[ext_to_int_data[i]] = i;
        }
    }
  return 0;  
}


void rs_mesh_base::check_data() const
{
  base_t::check_data ();
  
  if (minpv < 0)
    bs_throw_exception (boost::format ("minpv = %d is out of range")% minpv);
  if (minsv < 0)
    bs_throw_exception (boost::format ("minsv = %d is out of range")% minsv);
  if (max_thickness < 0)
    bs_throw_exception (boost::format ("max_thickness = %d is out of range")% max_thickness);
    
  if (!actnum_array)
    bs_throw_exception ("ACTNUM array is not initialized");
  if (!poro_array)
    bs_throw_exception ("PORO array is not initialized");
  if (!depths->size ())
    bs_throw_exception ("depths array is not initialized");
}


//BS_INST_STRAT(rs_mesh_base);

#ifndef WELL_PROCESS_COMPLETIONS_H
#define WELL_PROCESS_COMPLETIONS_H

#include "wpi_iface.h"
#include "rs_mesh_iface.h"

namespace blue_sky
{
  class well;

  namespace wells
  {
    class connection;
    struct completion_coords 
    {
      auto_value <bool, false> use_CCF_flag; 
      auto_value <t_float, 0> completion_length;  //!< length of current complition
      t_float x1[3];   //!< start point of completion 
      t_float x2[3];   //!< end point of completion 
      t_float compute_length ()
        {
          return (completion_length = sqrt ((x2[0] - x1[0]) * (x2[0] - x1[0]) + (x2[1] - x1[1]) * (x2[1] - x1[1]) + (x2[2] - x1[2]) * (x2[2] - x1[2])));
        }
    };

    struct completion 
      {
        typedef v_float::iterator               vf_iterator;
        typedef v_float::const_iterator         vf_const_iterator;

        typedef well                            well_t;                             //!< base type for wells 
        typedef wells::connection               connection_t;
        typedef t_double                        item_t;
        typedef t_long                          index_t;
        typedef spv_double                      item_array_t;
        typedef smart_ptr <fi_params, true>     sp_params_t;

        typedef rs_mesh_iface                   mesh_iface_t;
        typedef smart_ptr <mesh_iface_t, true>  sp_mesh_iface_t;
        typedef smart_ptr <connection_t, true>  sp_connection_t;
        typedef smart_ptr <well_t, true>        sp_well_t;                          //!< smart_ptr to well type
        typedef wpi::well_hit_cell_3d           well_hit_cell_3d; 
        typedef completion_coords               completion_coords_t;
        
        static void
        process_completion (const physical_constants &internal_constants, 
                            const sp_params_t &params, 
                            const sp_mesh_iface_t &mesh, 
                            const stdv_float &perm, 
                            const stdv_float &ntg,
                            sp_well_t &well, 
                            std::vector <wpi::well_hit_cell_3d> &well_path_segs,
                            t_uint con_branch, 
                            t_float con_md, 
                            t_float con_len);
      };                                                    

  } // namespace wells
} // namespace blue_sky



#endif // WELLS_PROCESS_COMPLETIONS_H

/** 
 * @file cljp_coarse.h
 * @brief implementation of Cleary-Luby-Jones-Plassmann grid coasering method
 * @author 
 * @version 
 * @date 2010-03-10
 */
#ifndef __CLJP_COARSE_H
#define __CLJP_COARSE_H

#include "amg_coarse_iface.h"

namespace blue_sky
{
  /** 
   * @brief interface class for block CSR matrix storage and manipulation
   */
  class BS_API_PLUGIN cljp_coarse: public amg_coarse_iface
    {
      //! bcsr matrix
      typedef bcsr_amg_matrix_iface                             bcsr_t;
      typedef smart_ptr<bcsr_t, true>                           sp_bcsr_t;
      //! this_t
      typedef cljp_coarse                                       this_t;

      BLUE_SKY_TYPE_DECL(cljp_coarse);
    //-----------------------------------------
    //  METHODS
    //-----------------------------------------
    public:
      /** 
       * @brief build CF markers and Strength matrix
       * 
       * @param a_matrix        -- <INPUT> A matrix
       * @param wksp            -- <INPUT/OUTPUT> workspace 
       * @param cf_markers      -- <OUTPUT> CF markers 
       * @param s_markers       -- <OUTPUT> non zero elements of Strength matrix
       * 
       * @return number of coarse points (n_coarse_size) or < 0 if error occur
       */
      virtual t_long build (sp_bcsr_t a_matrix, 
                              spv_double wksp, 
                              spv_long cf_markers,
                              spv_long s_markers);
      //! destructor
      virtual ~cljp_coarse ()
        {}

#ifdef BSPY_EXPORTING_PLUGIN
      virtual std::string py_str () const
        {
          return std::string ("CLJP grid coasering builder.");
        }
#endif //BSPY_EXPORTING_PLUGIN

    //-----------------------------------------
    //  VARIABLES
    //-----------------------------------------
    protected:
      spv_long sp_graph;
      
    };
}

#endif //__CLJP_COARSE_H


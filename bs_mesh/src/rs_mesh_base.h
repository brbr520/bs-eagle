#ifndef RS_MESH_BASE_H
#define RS_MESH_BASE_H
/*!
	\file rs_mesh_base.h
  \brief This file declares base class for reservoir simulation meshes
  \author Mark Khait
  \date 2009-07-20
 */

#include "flux_connections_iface.h"
#include "mesh_base.h"

using namespace blue_sky;

template<class strategy_t>
class BS_API_PLUGIN rs_mesh_base : public mesh_base<strategy_t>
  {

  //-----------------------------------------
  // TYPES
  //-----------------------------------------

  public:
    ///////////////////////
    // BASE TYPES
    ///////////////////////
    typedef mesh_base <strategy_t>                      base_t;

    typedef typename base_t::i_type_t                   i_type_t;
    typedef typename base_t::fp_storage_type_t          fp_storage_type_t;

    typedef typename base_t::index_array_t              index_array_t;
    typedef typename base_t::item_array_t               item_array_t;
    
    typedef typename base_t::sp_fp_array_t              sp_fp_array_t;
    typedef typename base_t::sp_i_array_t               sp_i_array_t;
    typedef typename base_t::sp_fp_storage_array_t      sp_fp_storage_array_t;
    typedef typename base_t::sp_bcsr_t                  sp_bcsr_t;
    typedef typename base_t::sp_idata_t                 sp_idata_t;
    
    ///////////////////////
    // OWN TYPES
    ///////////////////////

    typedef typename strategy_t::fp_type_t                 fp_type_t;
    
    typedef flux_connections_iface<strategy_t>          flux_conn_iface_t;
    typedef smart_ptr <flux_conn_iface_t, true>         sp_flux_conn_iface_t;
    
    
  //-----------------------------------------
  //  METHODS
  //-----------------------------------------

  public:

    ///////////////////////
    // INIT
    ///////////////////////
        
    //! default constructor
    rs_mesh_base ();

    //! default destructor
    virtual ~rs_mesh_base ()	{};
    
    //! init mesh from pool
    void init_props (const sp_idata_t &idata);
    
    //! init mesh from arrays
    void init_props (array_uint8_t   actnum_array,
                     array_float16_t poro_array,
                     array_float16_t ntg_array,
                     array_float16_t multpv_array);
                     
    
    //! initialize int_to_ext indexation
    int init_int_to_ext();
    
    //! check mesh data
    void check_data () const;

    ///////////////////////
    // ACCESS
    ///////////////////////
    
    //! set darcy constant for correct transmissibility calculation 
    void set_darcy (double darcy_constant_)
    {
      darcy_constant = darcy_constant_;
    }
    
    //! return depths of cell centers (length n_active_elements)
    const sp_fp_array_t get_depths () const
    {
      return depths;
    }
    
    ///////////////////////
    // WORK
    ///////////////////////

    //! allocate jacobian 
    virtual int build_jacobian_and_flux_connections (const sp_bcsr_t /*jacobian*/, const sp_flux_conn_iface_t/*flux_conn*/, 
                                                     sp_i_array_t /*boundary_array*/) = 0;

  //-----------------------------------------
  //  VARIABLES
  //-----------------------------------------

  protected:
    
    fp_type_t minpv;	  //!< minimum pore volume
    fp_type_t minsv;	  //!< minimum volume for splicing
    fp_type_t max_thickness;	  //!< maximum thickness between blocks for splicing
    
    physical_constants ph_const; //!< default physical constants
    fp_type_t darcy_constant; //Darcy coefficient

    //smart_ptr on properties
    i_type_t *actnum_array;	//!< smart_ptr on actnum array
    fp_storage_type_t *poro_array;		//!< smart_ptr on poro array
    fp_storage_type_t *ntg_array; 		//!< smart_ptr on ntg array
    fp_storage_type_t *multpv_array;	//!< smart_ptr on multpv_array array

    sp_fp_array_t depths; //!< depths of elements center points

  };
#endif // RS_MESH_BASE_H

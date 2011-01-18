#ifndef BS_DATA_CLASS_H
#define BS_DATA_CLASS_H
/*!
  \file data_class.h
  \brief initial data holder
	\author Nikonov Maxim
*/

#include "bs_array_map.h"
#include "rocktab_table.h"
#include "prvd_table.h"
#include "read_class.h"
#include "arrays.h"
#include "arrays_tables.h"
#include "convert_units.h"

#include "data_dimens.h"

#include "strategies.h"
#include "bs_tree.h"

namespace blue_sky {

//!< values of geometry definition type
#define GEOM_FLAG_NOT_INITIALIZED  0
#define GEOM_FLAG_DX_DY_DZ         1
#define GEOM_FLAG_ZCORN_COORD      2

#define BOUNDARY_CONDITION_TYPE_I  1
#define BOUNDARY_CONDITION_TYPE_II 2

#define DEFAULT_BOUNDARY_CONDITION 2

//! Maximal number of time steps in one TSTEP expression
#define MAX_TIME_STEPS_DEF      1000

  /*!
  \brief convert  -- convert 'carray' to UfaSolver format and write it to 'array'
  \param msh    -- mesh
  \param array  -- pointer to destination array
  \param carray -- pointer to source array
  */
  template <class index_t, class sp_index_array_t, class array_t, class carray_t>
  void
  convert_arrays (index_t cells_count, const sp_index_array_t index_map, array_t &array, const carray_t &carray)
  {
    index_t *index_map_data = &(*index_map)[0];
    carray_t::pointed_t::value_type *carray_data = &(*carray)[0];
    
    for (typename index_t i = 0; i < cells_count; ++i)
      array[i] = carray_data[index_map_data[i]];
  }

  /*!
  \class data
  \ingroup KeywordLanguage
  \brief Main class for read, save and store input information
  */
  template <class strategy_t>
  class BS_API_PLUGIN idata : public bs_node
    {
    private:
      struct idata_traits;

    public:
      //! typedefs
      typedef idata<strategy_t>                           this_t;
      typedef smart_ptr<this_t , true>                    sp_this_t;
      
      typedef typename strategy_t::i_type_t               i_type_t;
      typedef typename strategy_t::fp_storage_type_t      fp_type_t;

      typedef bs_array_map <i_type_t, i_type_t>           amap_i;
      typedef bs_array_map <i_type_t, fp_type_t>          amap_fp;

      typedef smart_ptr<amap_i>                           sp_amap_i;
      typedef smart_ptr<amap_fp>                          sp_amap_fp;

      typedef typename std::vector <val_vs_depth>         vval_vs_depth;
      
      typedef bs_array<i_type_t>                          arr_i;
      typedef bs_array<fp_type_t>                         arr_fp;

      typedef smart_ptr<arr_i, true>                      sp_arr_i;
      typedef smart_ptr<arr_fp, true>                     sp_arr_fp;

      typedef smart_ptr <FRead, true>										  sp_reader_t;

      struct pvt_info
        {
          sp_arr_fp							      main_data_;
          auto_value <bool, false>		has_density_;
          auto_value <fp_type_t>      density_;
          auto_value <fp_type_t>      molar_density_;
        };

      typedef std::vector <pvt_info>									  pvt_vector;

			struct BS_API_PLUGIN arrays_helper {
				typedef std::map < std::string, int > names_map_t;

				names_map_t names_map_i;
				names_map_t names_map_fp;

				i_type_t    *dummy_array_i;
				fp_type_t   *dummy_array_fp;

				arrays_helper ();
				~arrays_helper ();

				void init_names_maps ();

				void add_correspondence_i (const std::string &name, int index);
				void add_correspondence_fp (const std::string &name, int index);

				int get_idx_i (const std::string &name) const;
				int get_idx_fp (const std::string &name) const;
			};

		public:

      ~idata ();

      //! \brief init idata members method
      void init();

      //! \brief Assignment operator
      this_t &operator=(const this_t&);

      void set_defaults_in_pool();
      void set_region (int r_pvt, int r_sat, int r_eql, int r_fip);

      void set_density (sp_arr_fp density);
      void set_density_internal (const fp_type_t *density);

      //! return prvd vector
      vval_vs_depth &get_prvd();
      //! return rsvd vector
      vval_vs_depth &get_rsvd();
      //! return pbvd vector
      vval_vs_depth &get_pbvd();

      sp_arr_fp
      get_rock()
      {
        return rock;
      }

      sp_arr_fp
      get_p_ref()
      {
        return p_ref;
      }

      //int no_blanks (char *buf) const;
      //void build_argument_list (const sp_reader_t &reader);
			//void build_operator_list ();
			//void output_argument_list ();
			//void clear_argument_list ();
      //void read_left (const sp_reader_t &reader,
      //                char *s, char **right, std::string &name, int *i1,
      //                int *i2, int *j1, int *j2, int *k1, int *k2, const char *keyword);
      //void algorithm_read_and_done (const sp_reader_t &reader, char *buf, const char *keyword);
      //void calculate (const sp_reader_t &reader, ar_args_t &l, char *right, const char *keyword);
      //int read_numbers (ar_stack <ar_args_t> &st, char **buf);
      //int read_operator (const sp_reader_t &reader, ar_args_t &res, ar_stack <ar_args_t> &ar, ar_stack <ar_operat_t> &op,
      //                    char **start_ptr, int *flag_brek, const char *keyword,
      //                    const char *buf, int *Arguments_count);
      //int do_calculate (const sp_reader_t &reader, ar_args_t &res, ar_stack <ar_args_t> &ar, ar_stack <ar_operat_t> &op, const char *keyword);
      //int prior_calculate (const sp_reader_t &reader, ar_args_t &res, ar_stack <ar_args_t> &ar, ar_stack <ar_operat_t> &op, int priority, const char *keyword);
      //int read_arg_func (const sp_reader_t &reader, ar_stack <ar_args_t> &ar, ar_stack <ar_operat_t> &op, char **start_ptr, const char *keyword, int *arguments_count, int flag_brek);
      //int test_token (int prev, int cur);

      sp_arr_i    get_int_non_empty_array (int array_index);
      sp_arr_fp   get_fp_non_empty_array (int array_index);
      
      sp_arr_i    get_int_array (const std::string &array_name);
      sp_arr_fp   get_fp_array (const std::string &array_name);
      
      sp_arr_i    get_int_non_empty_array (const std::string &array_name);
      sp_arr_fp   get_fp_non_empty_array (const std::string &array_name);
      
      bool 
      contain (const std::string &array_name) const;
      
    public:
			arrays_helper ahelper;
      int rpo_model;                   //!< 3-ph oil relative permeability model: flag 0, 1 or 2 (stone model)

      fp_type_t minimal_pore_volume;      //!< Minimal pore volume allowed for active cells
      fp_type_t minimal_splice_volume;    //!< Minimal pore volume allowed for active cells to splice with other cells
      fp_type_t maximum_splice_thickness; //!< Default maximum thickness allowed between active cells to be coupled

      data_dimens dimens;              //!< dimension description

      i_type_t pvt_region;               //!< Number of PVT regions in simulation
      i_type_t sat_region;               //!< Number of saturation regions in simulation
      i_type_t eql_region;               //!< Number of equilibrium regions in simulation
      i_type_t fip_region;               //!< Number of FIP regions in simulation

      i_type_t fi_n_phase;               //!< number of phases if full implicit simulation
      i_type_t fi_phases;                //!< sizeof (int) bit fields (1 -- phase present, 0 -- do not present)

      i_type_t rock_region;              //!< Number of ROCK regions

      sp_arr_i equil_regions;

      //! \brief class's public data area
      sp_amap_i   i_map;
      sp_amap_fp  fp_map;


      auto_value <int> init_section;             //!< flag indicating whether we have init section
      //!< if init_section == 0, cdata::check_sat will not go.

      convert_units input_units_converter;          //!< Input units converter
      convert_units output_units_converter;         //!< Output units converter

      std::vector<rocktab_table <base_strategy_fif> > rocktab;    //!< rocktab tables for all rock regions

      //TITLE
      std::string title;

      pvt_vector pvto, pvtdo, pvtg, pvtw;

      sp_arr_fp rock;            //!< Array (pvt_region) - compressibility of rock for each region
      sp_arr_fp equil;
      sp_arr_fp p_ref;           //!< Array (pvt_region) - reference pressure for compressibility of rock for each region

      //! pressure points at reference depth used for PRVD keyword content,
      //! and initial pressure initialization
      //! for all pvt regions
      //! array (eql_region)
      vval_vs_depth prvd;

      //! RS points at reference depth used for RSVD keyword content,
      //! and initial RS initialization
      //! for all pvt regions
      //! array (eql_region)
      vval_vs_depth rsvd;

      //! PBUB points at reference depth used for PBVD keyword content,
      //! and initial RS initialization
      //! for all pvt regions
      //! array (eql_region)
      vval_vs_depth pbvd;

      BLUE_SKY_TYPE_DECL_T(idata)
    };
}

#endif // BS_DATA_CLASS_H

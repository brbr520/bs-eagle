/*!
 * \file two_stage_prec.cpp
 * \brief implementation of tow stage preconditioner
 * \date 2006-08-04
 */
#include "bs_kernel.h"
#include "bs_assert.h"

#include BS_FORCE_PLUGIN_IMPORT ()
#include "save_seq_vector.h"
#include "strategies.h"
#include "matrix_iface.h"
#include BS_STOP_PLUGIN_IMPORT ()

//#include "stdafx.h"
#include "two_stage_prec.h"
//#include "solve_helper.h"

namespace blue_sky
  {
    //////////////////////////////////////////////////////////////////////////
    //  two_stage_prec

    //! constructor
    template <class strat_t>
    two_stage_prec<strat_t>::two_stage_prec (bs_type_ctor_param /*param*/)
      : two_stage_prec_iface<strat_t> ()
    {
      prop = BS_KERNEL.create_object ("prop");
      if (!prop)
        {
          bs_throw_exception ("Type (prop) not registered");
        }
      init_prop ();
      sp_r = BS_KERNEL.create_object (fp_array_t::bs_type ());
      sp_w = BS_KERNEL.create_object (fp_array_t::bs_type ());
    }

    //! copy constructor
    template <class strat_t>
    two_stage_prec<strat_t>::two_stage_prec(const two_stage_prec &solver)
      : bs_refcounter (), two_stage_prec_iface<strat_t> ()
    {
      if (&solver != this)
        *this = solver;
    }

    //! destructor
    template <class strat_t>
    two_stage_prec<strat_t>::~two_stage_prec ()
    {}

    //! set solver's properties
    template <class strat_t>
    void two_stage_prec<strat_t>::set_prop (sp_prop_t prop_)
    {
      prop = prop_;

      init_prop ();
    }

    template <class strat_t> void
    two_stage_prec<strat_t>::init_prop ()
      {
        success_idx = prop->get_index_b (std::string ("is_success"));
        if (success_idx < 0)
          success_idx = prop->add_property_b (false, std::string ("is_success"), std::string ("True if solver successfully convergent"));
        
        if (success_idx < 0)
          {
            bs_throw_exception ("Can not regidter some properties");
          }
      }
    template <class strat_t>
    int two_stage_prec<strat_t>::solve (sp_matrix_t matrix, 
                                        sp_fp_array_t sp_rhs, 
                                        sp_fp_array_t sp_sol)
    {
      BS_ERROR (matrix, "two_stage_prec_solve");
      BS_ERROR (sp_rhs->size (), "two_stage_prec_solve");
      BS_ERROR (sp_sol->size (), "two_stage_prec_solve");
      BS_ERROR (prop, "two_stage_prec_solve");

      BS_ASSERT (prec);
      BS_ASSERT (prec_2);

      i_type_t n = matrix->get_n_rows () * matrix->get_n_block_size ();
      BS_ASSERT ((size_t)n == sp_rhs->size ()) (n) (sp_rhs->size ());

      if (prec->solve (matrix, sp_rhs, sp_sol))
        {
          bs_throw_exception ("TWO_STAGE: PREC 1 failed");
        }

      sp_r->resize (n);
      sp_w->resize (n);
      sp_r->assign (0);
      sp_w->assign (0);

      fp_type_t *sol = &(*sp_sol)[0];
      fp_type_t *w = &(*sp_w)[0];

      //r_array = rhs - sol - new rhs - step 5 in Olegs book
      matrix->calc_lin_comb (-1.0, 1.0, sp_sol, sp_rhs, sp_r);

      // solve system with ILU - step 6
      if (prec_2->solve (matrix, sp_r, sp_w))
        {
          bs_throw_exception ("TWO_STAGE: PREC 2 failed");
        }

      // Then we make a correction of our solution x = x + w - step 7
      i_type_t i = 0;
      i_type_t n2 = n - (n % 4);
      for (; i < n2; i+=4)
        {
          sol[i]        += w[i];
          sol[i + 1]    += w[i + 1];
          sol[i + 2]    += w[i + 2];
          sol[i + 3]    += w[i + 3];
        }

      for (; i < n; i++)
        {
          sol[i] += w[i];
        }

      return 0;
    }

    template <class strat_t>
    int two_stage_prec<strat_t>::solve_prec (sp_matrix_t matrix, 
                                             sp_fp_array_t sp_rhs, 
                                             sp_fp_array_t sp_sol)
    {
      return solve (matrix, sp_rhs, sp_sol);
    }

    /**
    * @brief setup for CGS
    *
    * @param matrix -- input matrix
    *
    * @return 0 if success
    */
    template <class strat_t> int
    two_stage_prec<strat_t>::setup (sp_matrix_t matrix)
    {
      if (!matrix)
        {
          bs_throw_exception ("CGS: Passed matrix is null");
        }

      BS_ASSERT (prec);
      BS_ASSERT (prec_2);
      BS_ASSERT (prop);
      prec->setup (matrix);
      prec_2->setup (matrix);
      
      return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    BLUE_SKY_TYPE_STD_CREATE_T_DEF(two_stage_prec, (class));
    BLUE_SKY_TYPE_STD_COPY_T_DEF(two_stage_prec, (class));

    BLUE_SKY_TYPE_IMPL_T_EXT(1, (two_stage_prec<base_strategy_fif>),    1, (two_stage_prec_iface<base_strategy_fif>),    "two_stage_prec_fif",  "Two stage preconditioner", "Two stage preconditioner", false);
    BLUE_SKY_TYPE_IMPL_T_EXT(1, (two_stage_prec<base_strategy_did>),    1, (two_stage_prec_iface<base_strategy_did>),    "two_stage_prec_did",  "Two stage preconditioner", "Two stage preconditioner", false);
    BLUE_SKY_TYPE_IMPL_T_EXT(1, (two_stage_prec<base_strategy_dif>),    1, (two_stage_prec_iface<base_strategy_dif>),    "two_stage_prec_dif",  "Two stage preconditioner", "Two stage preconditioner", false);

    BLUE_SKY_TYPE_IMPL_T_EXT(1, (two_stage_prec<base_strategy_flf>),    1, (two_stage_prec_iface<base_strategy_flf>),    "two_stage_prec_flf",  "Two stage preconditioner", "Two stage preconditioner", false);
    BLUE_SKY_TYPE_IMPL_T_EXT(1, (two_stage_prec<base_strategy_dld>),    1, (two_stage_prec_iface<base_strategy_dld>),    "two_stage_prec_dld",  "Two stage preconditioner", "Two stage preconditioner", false);
    BLUE_SKY_TYPE_IMPL_T_EXT(1, (two_stage_prec<base_strategy_dlf>),    1, (two_stage_prec_iface<base_strategy_dlf>),    "two_stage_prec_dlf",  "Two stage preconditioner", "Two stage preconditioner", false);

} // namespace blue_sky
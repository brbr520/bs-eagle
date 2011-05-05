/**
 *       \file  equil_keywords.cpp
 *      \brief  Keywords for EQUIL model
 *     \author  Sergey Miryanov (sergey-miryanov), sergey.miryanov@gmail.com
 *       \date  29.04.2011
 *  \copyright  This source code is released under the terms of 
 *              the BSD License. See LICENSE for more details.
 * */

#include "equil_keywords.hpp"
#include "keyword_manager_iface.h"
#include "init_model_iface.hpp"
#include "read_class.h"
#include "data_class.h"
#include "constants.h"

namespace blue_sky 
{
  equil_keywords::equil_keywords (bs_type_ctor_param)
  {
  }

  equil_keywords::equil_keywords (equil_keywords const &src)
  : bs_refcounter (src), keyword_info_base (src)
  {
  }

  namespace 
  {
    // FIXME: subject to refactoring
    void
    EQUIL (std::string const &keyword, keyword_params &params)
    {
      BS_SP (FRead) reader = params.hdm->get_reader ();
      BS_SP (idata) idata = params.hdm->get_data ();
      if (!idata->pvto.size())
        {
          bs_throw_exception (boost::format ("Error in %s: PVT table for oil has not been initialized yet (keyword: %s")
            % reader->get_prefix () % keyword);
        }
      if (!idata->pvtw.size())
        {
          bs_throw_exception (boost::format ("Error in %s: PVT table for water has not been initialized yet (keyword: %s")
            % reader->get_prefix () % keyword);
        }
      if (!idata->pvtg.size())
        {
          bs_throw_exception (boost::format ("Error in %s: PVT table for gas has not been initialized yet (keyword: %s)")
            % reader->get_prefix () % keyword);
        }
      if (!idata->rock->size())
        {
          bs_throw_exception (boost::format ("Error in %s: rock properties have not been initialized yet (keyword: %s)")
            % reader->get_prefix () % keyword);
        }

      t_long eql_region = idata->props->get_i ("eql_region");
      if (eql_region == 0)
        {
          bs_throw_exception (boost::format ("Error in %s: eql_region == 0 (keyword: %s)")
            % reader->get_prefix () % keyword);
        }
      if (idata->equil->size () != eql_region * EQUIL_TOTAL)
        {
          bs_throw_exception (boost::format ("Error in %s: EQUIL table size mismatch (keyword: %s), %d == %d")
            % reader->get_prefix () % keyword % idata->equil->size () % (eql_region * EQUIL_TOTAL));
        }


      t_float *equil = idata->equil->data ();
      for (t_long i = 0; i < eql_region; ++i)
        {
          char buf[CHAR_BUF_LEN] = {0};
          reader->read_line (buf, CHAR_BUF_LEN);
          if (buf[0] == '/')
            {
              bs_throw_exception (boost::format ("Error in %s: not enough valid arguments for keyword %s")
                % reader->get_prefix () % keyword);
            }

          // Values by default
          boost::array <double, EQUIL_TOTAL> dbuf;
          dbuf[EQUIL_DAT_DEPTH] = -1.0; //Ddat
          dbuf[EQUIL_DAT_PRESS] = 0.0;  //Pdat
          dbuf[EQUIL_WOC_DEPTH] = -1.0; //Dwoc
          dbuf[EQUIL_WOC_PRESS] = 0.0;  //Pwoc
          dbuf[EQUIL_GOC_DEPTH] = 0.0;  // Dgoc
          dbuf[EQUIL_GOC_PRESS] = 0.0;  // Pgoc
          dbuf[EQUIL_RS_TYPE]   = 0;    // RS type initialization
          dbuf[EQUIL_RV_TYPE]   = 0;    // RV type initialization
          dbuf[EQUIL_NUM_SEC]   = 100;  // number of points
          dbuf[EQUIL_COMP_TYPE] = 0;    // composition type initialization
          dbuf[EQUIL_COMP_ARG]  = 0;    // composition argument initialization

          char *end_ptr = 0;
          unwrap (buf);
          scanf_d (buf,     &end_ptr, &dbuf[EQUIL_DAT_DEPTH], 0, 0, "Can't read EQUIL_DEPTH");
          scanf_d (end_ptr, &end_ptr, &dbuf[EQUIL_DAT_PRESS], 0, 0, "Can't read EQUIL_DEPTH_PRESSURE");
          scanf_d (end_ptr, &end_ptr, &dbuf[EQUIL_WOC_DEPTH], 0, 0, "Can't read EQUIL_WOC_DEPTH");
          scanf_d (end_ptr, &end_ptr, &dbuf[EQUIL_WOC_PRESS], 0, 0, "Can't read EQUIL_WOC_DEPTH_PRESSURE");
          scanf_d (end_ptr, &end_ptr, &dbuf[EQUIL_GOC_DEPTH], 0, 0, "Can't read EQUIL_GOC_DEPTH");
          scanf_d (end_ptr, &end_ptr, &dbuf[EQUIL_GOC_PRESS], 0, 0, "Can't read EQUIL_GOC_DEPTH_PRESSURE");
          scanf_d (end_ptr, &end_ptr, &dbuf[EQUIL_RS_TYPE],   0, 0, "Can't read RS initialization type at GOC depth");

          if (dbuf[EQUIL_DAT_DEPTH] < 0) // Ddat
            {
              bs_throw_exception (boost::format ("Error in %s: incorrect value of EQUIL_DEPTH %f")
                % reader->get_prefix () % dbuf[EQUIL_DAT_DEPTH]);
            }

          if (dbuf[EQUIL_DAT_PRESS] < 0) //Pdat
            {
              bs_throw_exception (boost::format ("Error in %s: incorrect pressure value at datum depth %f")
                % reader->get_prefix () % dbuf[EQUIL_DAT_PRESS]);
            }

          if (dbuf[EQUIL_WOC_DEPTH] < 0)
            {
              bs_throw_exception (boost::format ("Error in %s: incorrect value of WOC depth %f")
                % reader->get_prefix () % dbuf[EQUIL_WOC_DEPTH]);
            }

          if (dbuf[EQUIL_GOC_DEPTH] < 0)
            {
              //                rep->print (LOG_READ_SECTION, LOG_ERR,
              //                           GET_TEXT ("Error in %s: incorrect value of GOC depth %lf\n"), r.get_prefix (), dbuf[4]);
              //                  return -8;
              dbuf[EQUIL_GOC_DEPTH] = 0;
            }

          for (t_long ii = 0; ii < EQUIL_TOTAL; ++ii)
            equil[EQUIL_TOTAL * i + ii] = dbuf[ii];
        }

      // FIXME: 
      //if (idata->prvd.size())
      //  idata->prvd.resize(0);
      if (idata->rsvd.size())
        idata->rsvd.resize(0);
      if (idata->pbvd.size())
        idata->pbvd.resize(0);

      // flag indicating init section
      idata->props->set_i ("init_section", 1);

      BOSOUT (section::read_data, level::medium) <<  keyword << bs_end;


    }
    void
    RSVD (std::string const &keyword, keyword_params &params)
    {
      BS_SP (FRead) reader = params.hdm->get_reader ();
      BS_SP (idata) idata = params.hdm->get_data ();
      
      t_long eql_region = idata->props->get_i ("eql_region");
      if (eql_region == 0)
        {
          bs_throw_exception (boost::format ("Error in %s: eql_region == 0 (keyword: %s)")
            % reader->get_prefix () % keyword);
        }

      idata->rsvd.resize (eql_region);

      // Read table for each of region
      std::vector <double> dbuf;
      for (t_long i = 0; i < eql_region; ++i)
        {
          t_long len = reader->read_table (keyword, dbuf, 2);
          if (len < 1)
            {
              bs_throw_exception (boost::format ("Error in %s: not enough valid arguments for keyword %s")
                % reader->get_prefix () % keyword);
            }

          idata->rsvd[i].set_table_len (len);

          std::vector <double> &dpt = idata->rsvd[i].tdepth();
          std::vector <double> &prs = idata->rsvd[i].tvalues();

          dpt.assign (len, 0);
          prs.assign (len, 0);
          for (t_long j = 0; j < len; ++j)
            {
              dpt[j] = dbuf[j * 2 + 0];
              prs[j] = dbuf[j * 2 + 1];
            }

          BOSOUT (section::read_data, level::medium) << "len=" << len << " i=" << i << bs_end;
        }
      BOSOUT (section::read_data, level::medium) <<  keyword << bs_end;
    }
    void
    PBVD (std::string const &keyword, keyword_params &params)
    {
      BS_SP (FRead) reader = params.hdm->get_reader ();
      BS_SP (idata) idata = params.hdm->get_data ();
      
      t_long eql_region = idata->props->get_i ("eql_region");
      if (eql_region == 0)
        {
          bs_throw_exception (boost::format ("Error in %s: eql_region == 0 (keyword: %s)")
            % reader->get_prefix () % keyword);
        }

      idata->pbvd.resize (eql_region);

      // Read table for each of region
      std::vector <double> dbuf;
      for (t_long i = 0; i < eql_region; ++i)
        {
          t_long len = reader->read_table (keyword, dbuf, 2);
          if (len < 1)
            {
              bs_throw_exception (boost::format ("Error in %s: not enough valid arguments for keyword %s")
                % reader->get_prefix () % keyword);
            }

          idata->pbvd[i].set_table_len (len);

          std::vector <double> &dpt = idata->pbvd[i].tdepth();
          std::vector <double> &prs = idata->pbvd[i].tvalues();

          dpt.assign (len, 0);
          prs.assign (len, 0);
          for (t_long j = 0; j < len; ++j)
            {
              dpt[j] = dbuf[j * 2 + 0];
              prs[j] = dbuf[j * 2 + 1];
            }

          BOSOUT (section::read_data, level::medium) << "len=" << len << " i=" << i << bs_end;
        }
      BOSOUT (section::read_data, level::medium) <<  keyword << bs_end;
    }

    void
    activate (std::string const &, keyword_params &params)
    {
      BS_SP (keyword_manager_iface) keyword_manager = params.hdm->get_keyword_manager ();
      BS_ASSERT (keyword_manager);

      BS_SP (init_model_iface) init_model (BS_KERNEL.create_object ("equil_init_model"), bs_dynamic_cast ());
      params.hdm->set_init_model (init_model);

      keyword_manager->register_keyword ("EQUIL", keyword_manager_iface::keyword_handler (EQUIL, 0));
      keyword_manager->register_keyword ("RSVD", keyword_manager_iface::keyword_handler (RSVD, 0));
      keyword_manager->register_keyword ("PBVD", keyword_manager_iface::keyword_handler (PBVD, 0));
    }
  }

  void
  equil_keywords::register_keywords (sp_objbase &km, std::string provider) const
  {
    BS_SP (keyword_manager_iface) keyword_manager (km, bs_dynamic_cast ());
    BS_ASSERT (keyword_manager);

    if (provider == "")
      {
        provider = "EQUIL_MODEL";
        keyword_manager->register_keyword ("EQUIL_MODEL", keyword_handler (0, activate));
      }

    keyword_manager->register_supported_keyword ("EQUIL", provider);
    keyword_manager->register_supported_keyword ("RSVD", provider);
    keyword_manager->register_supported_keyword ("PBVD", provider);
  }

  BLUE_SKY_TYPE_STD_CREATE (equil_keywords);
  BLUE_SKY_TYPE_STD_COPY (equil_keywords);
  BLUE_SKY_TYPE_IMPL (equil_keywords, keyword_info_base, "Keywords for EQUIL model", "EQUIL_MODEL", "EQUIL_MODEL");
}

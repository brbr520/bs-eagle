/// @file hdm_serialize_action.cpp
/// @brief Implementation of hdm save & load functions called by clients
/// @author uentity
/// @version 
/// @date 30.01.2012
/// @copyright This source code is released under the terms of
///            the BSD License. See LICENSE for more details.

#include "bs_bos_core_data_storage_stdafx.h"

#include "hdm_serialize.h"
#include "bs_misc.h"
#include <fstream>
#include <sstream>

// save to and load from text archives
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/python.hpp>

// path separator
#ifdef UNIX
#define PATHSEP '/'
#else
#define PATHSEP '\\'
#endif

// extension of file with hdm dump
#define HDM_DUMP_EXT ".hdm"

namespace boarch = boost::archive;
namespace boser = boost::serialization;
namespace bp = boost::python;

namespace blue_sky {

namespace  {

template< class dst_stream >
dst_stream& hdm_serialize_save_impl(
	dst_stream& f,
	smart_ptr< hdm > t,
	const std::string& prj_path,
	const std::string& prj_name,
	const std::string& deep_copy_suffix
){
	// save project path for serialization code
	kernel::idx_dt_ptr p_dt = BS_KERNEL.pert_idx_dt(hdm::bs_type());
	//std::string well_pool_filename = prj_name + "_well_pool.db";
	p_dt->insert< std::string >(prj_path);
	// and project name
	p_dt->insert< std::string >(prj_name);
	// DEBUG
	//BSERR << "hdm_serialize_save invoked with" << bs_end;
	//BSERR << "project path: " << prj_path << bs_end;
	//BSERR << "model path: " << prj_name << bs_end;
	// inform to explicitly copy storage files
	if(deep_copy_suffix.size()) {
		p_dt->insert< std::string >(deep_copy_suffix);
		//BSERR << "!model name suffix!: " << deep_copy_suffix << bs_end;
	}

	// make archive
	try {
		boarch::polymorphic_text_oarchive oa(f);
		oa << t;
	}
	catch(const bs_exception& e) {
		BSERR << std::string("Error during hdm_serialize_save: ") + e.what() << bs_end;
	}
	catch(const std::exception& e) {
		BSERR << std::string("Error during hdm_serialize_save: ") + e.what() << bs_end;
	}
	catch(...) {
		BSERR << "Unknown error during hdm_serialize_save!" << bs_end;
		// clear kernel table to not confuse with further saves
		p_dt->clear< std::string >();
		throw;
	}

	// clear kernel table to not confuse with further saves
	p_dt->clear< std::string >();
	return f;
}

template< class src_stream >
smart_ptr< hdm > hdm_serialize_load_impl(
	src_stream& f,
	const std::string& prj_path,
	const std::string& prj_name
){
	// save project path for serialization code
	kernel::idx_dt_ptr p_dt = BS_KERNEL.pert_idx_dt(hdm::bs_type());
	//std::string well_pool_filename = prj_name + "_well_pool.db";
	p_dt->insert< std::string >(prj_path);
	// and project name
	p_dt->insert< std::string >(prj_name);

	// load archive
	// actually need to rethrow all exceptions
	smart_ptr< hdm > t;
	try {
		boarch::polymorphic_text_iarchive ia(f);
		ia >> t;
	}
	catch(const bs_exception& e) {
		BSERR << std::string("Error during hdm_serialize_load: ") + e.what() << bs_end;
		// clear kernel table
		p_dt->clear< std::string >();
		throw;
		//t = BS_KERNEL.create_object(hdm::bs_type());
		//t.release();
	}
	catch(const std::exception& e) {
		BSERR << std::string("Error during hdm_serialize_load: ") + e.what() << bs_end;
		// clear kernel table
		p_dt->clear< std::string >();
		throw;
		//t = BS_KERNEL.create_object(hdm::bs_type());
		//t.release();
	}
	catch(...) {
		BSERR << "Unknown error during hdm_serialize_save!" << bs_end;
		// clear kernel table
		p_dt->clear< std::string >();
		throw;
	}

	// clear kernel table
	p_dt->clear< std::string >();
	// clear dangling refs of loaded in kernel
	//BS_KERNEL.tree_gc();

	return t;
}

} /* hidden namespace */

/*-----------------------------------------------------------------
 * save hdm
 *----------------------------------------------------------------*/
void hdm_serialize_save(
	smart_ptr< hdm > t,
	const std::string& prj_path,
	const std::string& prj_name,
	const std::string& deep_copy_suffix
){
	std::string fname = prj_path + PATHSEP + prj_name + HDM_DUMP_EXT;
	std::ofstream f(fname.c_str());
	hdm_serialize_save_impl(f, t, prj_path, prj_name, deep_copy_suffix);
}

std::string hdm_serialize_to_str(
	smart_ptr< hdm > t,
	const std::string& prj_path,
	const std::string& prj_name,
	const std::string& deep_copy_suffix
){
	std::ostringstream f;
	return hdm_serialize_save_impl(f, t, prj_path, prj_name, deep_copy_suffix).str();
}

/*-----------------------------------------------------------------
 * load hdm
 *----------------------------------------------------------------*/
smart_ptr< hdm > hdm_serialize_load(
	const std::string& prj_path,
	const std::string& prj_name
){
	std::ifstream f((prj_path + PATHSEP + prj_name + HDM_DUMP_EXT).c_str());
	return hdm_serialize_load_impl(f, prj_path, prj_name);
}

smart_ptr< hdm > hdm_serialize_from_str(
	const std::string& hdm_dump,
	const std::string& prj_path,
	const std::string& prj_name
){
	std::istringstream f(hdm_dump);
	return hdm_serialize_load_impl(f, prj_path, prj_name);
}

/*-----------------------------------------------------------------
 * export save & load to python
 *----------------------------------------------------------------*/
BOOST_PYTHON_FUNCTION_OVERLOADS(hdm_serialize_save_overl, hdm_serialize_save, 3, 4)
//BOOST_PYTHON_FUNCTION_OVERLOADS(hdm_serialize_load_overl, hdm_serialize_load, 2, 3)
BOOST_PYTHON_FUNCTION_OVERLOADS(hdm_serialize_to_str_overl, hdm_serialize_to_str, 3, 4)
//BOOST_PYTHON_FUNCTION_OVERLOADS(hdm_serialize_from_str_overl, hdm_serialize_from_str, 3, 4)

namespace python {

// helper that allows to manually set project path and name for serialization process
// invoke with empty strings to clear kernel table
void hdm_serialize_set_project_prop(const std::string& prj_path, const std::string& prj_name) {
	// save project path for serialization code
	kernel::idx_dt_ptr p_dt = BS_KERNEL.pert_idx_dt(hdm::bs_type());
	// clear kernel table to not confuse with further saves
	p_dt->clear< std::string >();

	// remember project path
	if(prj_path.size() > 0)
		p_dt->insert< std::string >(prj_path);
	// and project name
	if(prj_name.size() > 0)
		p_dt->insert< std::string >(prj_name);
}

// wstring version
void hdm_serialize_set_project_prop_w(const std::wstring& prj_path, const std::wstring& prj_name) {
	hdm_serialize_set_project_prop(wstr2str(prj_path), wstr2str(prj_name));
}

BS_API_PLUGIN void py_export_hdm_serialize() {
	bp::def("hdm_serialize_save", &hdm_serialize_save, hdm_serialize_save_overl());
	bp::def("hdm_serialize_load", &hdm_serialize_load);
    // register pvt to/from str serialization
	bp::def("serialize_to_str", &blue_sky::hdm_serialize_to_str, hdm_serialize_to_str_overl());
	bp::def("serialize_from_str", &blue_sky::hdm_serialize_from_str);

	// export only wide-string version
	//bp::def("hdm_serialize_set_project_prop", &hdm_serialize_set_project_prop);
	bp::def("hdm_serialize_set_project_prop", &hdm_serialize_set_project_prop_w);
}

} /* python */

} /* blue_sky */

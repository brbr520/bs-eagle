/**
 *       \file  res_file_id.h
 *      \brief  Identifiers
 *     \author  Nikonov Max
 *       \date  05.06.2008
 *  \copyright  This source code is released under the terms of 
 *              the BSD License. See LICENSE for more details.
 *       \todo  Obsolete, should be removed
 * */
#ifndef _RES_FILE_ID_H_
#define _RES_FILE_ID_H_


//! ids for classes
enum
{
  YS_ID_CATALOG = 0,
  YS_ID_INIT,
  YS_ID_CDATA,
  YS_ID_RSV_STATUS,
  YS_ID_GROUP_STATUS,
  YS_ID_WELL_STATUS,
  YS_ID_CONNECTION_STATUS,
  YS_ID_MAPS,
  YS_ID_PVT,
  YS_ID_SPOF,
  YS_ID_TIME_STEP,
  YS_ID_GROUP,
  YS_ID_WELL,
  YS_ID_CONNECTION,
  YS_ID_RATE,
  YS_ID_FIP_DATA,
  YS_ID_MESH,
  YS_ID_INITIAL_FIP_DATA,
  YS_ID_IMPES_PARAM,
  YS_ID_WPIMUL,
  YS_ID_FIX_PRESSURE,
  YS_ID_STREAM_LINE,
  YS_ID_FRACTURES,
  YS_ID_STREAM_LINE_PARAMS,
  YS_ID_W_TRAJECTORY,
  YS_ID_FI_PARAMS,
  YS_ID_SMART_ARRAY,
  YS_ID_POOL,
  YS_ID_COMP_IDATA,
  YS_ID_WELL_RESULTS,
  YS_ID_WDATA,
  YS_ID_CONN_DATA,
  YS_ID_SPFN,
  YS_ID_SOF2,
  YS_ID_SOF3,
  YS_ID_FDATA,
  YS_ID_FIP_RESULTS,
  YS_ID_GCTRL_GECON,
  YS_ID_GCTRL_SWITCH_TO_BHP
};


//! ids for impes parameters
enum
{
  YS_ID_IMPES_PARAM_DOUBLE_VARIABLES = YS_ID_IMPES_PARAM << 16
};

//! ids for maps data
enum
{
  YS_ID_MAPS_CURR_DT = YS_ID_MAPS << 16,
  YS_ID_MAPS_P,
  YS_ID_MAPS_SO,
  YS_ID_MAPS_SW,
  YS_ID_MAPS_TNK,
  YS_ID_MAPS_PLANE_VALUE,
  YS_ID_MAPS_MAIN_VARIABLE,
  YS_ID_MAPS_GAS_OIL_RATIO,
  YS_ID_MAPS_FLOW_RATES,
  YS_ID_MAPS_XCP
};


//! ids for initial fip data
enum
{
  YS_ID_INITIAL_FIP_DATA_OOIP = YS_ID_INITIAL_FIP_DATA << 16,
  YS_ID_INITIAL_FIP_DATA_OWIP,
  YS_ID_INITIAL_FIP_DATA_OGIP,
  YS_ID_INITIAL_FIP_DATA_DHPV
};

//! ids for mesh class
enum
{
  YS_ID_MESH_MAIN_PROP_UINT = YS_ID_MESH << 16,
  YS_ID_MESH_NODES,
  YS_ID_MESH_NODE_BOUNDARY,
  YS_ID_MESH_ORIGINAL_NODES,
  YS_ID_MESH_ORIGINAL_NODES_NUM,
  YS_ID_MESH_PLANES,
  YS_ID_MESH_PLANE_BOUNDARY,
  YS_ID_MESH_PLANE_ORIENTATION,
  YS_ID_MESH_PLANE_PLUS,
  YS_ID_MESH_PLANE_MINUS,

  YS_ID_MESH_ORIGINAL_PLANES,
  YS_ID_MESH_ORIGINAL_PLANES_NUM,
  YS_ID_MESH_ELEMENTS,
  YS_ID_MESH_ELEMENT_PLANES,
  YS_ID_MESH_ELEMENT_NEIGHBOUR,
  YS_ID_MESH_ELEMENT_BOUNDARY,
  YS_ID_MESH_ORIGINAL_ELEMENT_NUM,
  YS_ID_MESH_ORIGINAL_ELEMENTS,
  YS_ID_MESH_BOUNDARY_PLANES_ALL,
  YS_ID_MESH_BOUNDARY_PLANES,

  YS_ID_MESH_CONNECTION_PLUS,
  YS_ID_MESH_CONNECTION_MINUS,
  YS_ID_MESH_ELEMENT_CONNECTIONS_IND,
  YS_ID_MESH_ELEMENT_CONNECTIONS_PTR,
  YS_ID_MESH_CONNECTION_TYPE
};
//! ids for rsv status class
enum
{
  YS_ID_RSV_STATUS_GUI_VARS = YS_ID_RSV_STATUS << 16,
  YS_ID_RSV_STATUS_CURRENT_RATE,
  YS_ID_RSV_STATUS_TOTAL_RATE,
  YS_ID_RSV_STATUS_CURRENT_INITIAL_RATE,
  YS_ID_RSV_STATUS_TOTAL_INITIAL_RATE,
  YS_ID_RSV_STATUS_GROUP_STATUS,
  YS_ID_RSV_STATUS_WELL_STATUS,

  YS_ID_RSV_STATUS_FIP_REGIONS,
  YS_ID_RSV_STATUS_FIP_DATA,
  YS_ID_RSV_STATUS_CURRENT_DATE,
  YS_ID_RSV_STATUS_FIX_PRESSURE,
  YS_ID_RSV_STATUS_STREAM_LINE,
  YS_ID_RSV_STATUS_INT_PARAMS
};

//! ids for well status class
enum
{
  YS_ID_GROUP_STATUS_PARENT = YS_ID_GROUP_STATUS << 16,
  YS_ID_GROUP_STATUS_CURRENT_RATE,
  YS_ID_GROUP_STATUS_TOTAL_RATE,
  YS_ID_GROUP_STATUS_CURRENT_INITIAL_RATE,
  YS_ID_GROUP_STATUS_TOTAL_INITIAL_RATE,
  YS_ID_GROUP_STATUS_GUI_VARS,
  YS_ID_GROUP_STATUS_WELL_STATUS,
  YS_ID_GROUP_STATUS_DPARAMS,
  YS_ID_GROUP_STATUS_IPARAMS,
  YS_ID_GROUP_STATUS_BPARAMS

};
//! ids for well status class
enum
{
  YS_ID_WELL_STATUS_PARENT = YS_ID_WELL_STATUS << 16,
  YS_ID_WELL_STATUS_CURRENT_STATUS,
  YS_ID_WELL_STATUS_CURRENT_RATE,
  YS_ID_WELL_STATUS_TOTAL_RATE,
  YS_ID_WELL_STATUS_CURRENT_INITIAL_RATE,
  YS_ID_WELL_STATUS_TOTAL_INITIAL_RATE,
  YS_ID_WELL_STATUS_MAIN_PROP_DOUBLE,
  YS_ID_WELL_STATUS_GUI_VARS,
  YS_ID_WELL_STATUS_CONNECTION_STATUS
};

//! ids for connection status class
enum
{
  YS_ID_CONNECTION_STATUS_PARENT = YS_ID_CONNECTION_STATUS << 16,
  YS_ID_CONNECTION_STATUS_RATE,
  YS_ID_CONNECTION_STATUS_MAIN_PROP_DOUBLE,
  YS_ID_CONNECTION_STATUS_COMMULATIVE_RATE,
};

//! ids for fip_data class
enum
{
  YS_ID_FIP_DATA_MAIN_PROP_DOUBLE = YS_ID_FIP_DATA << 16
};

//! ids for rate class
enum
{
  YS_ID_CONNECTION_MAIN_PROP_DOUBLE = YS_ID_CONNECTION << 16,
  YS_ID_CONNECTION_MAIN_PROP_INT,
  YS_ID_CONNECTION_NAME,
  YS_ID_CONNECTION_GROUPNAME,
  YS_ID_CONNECTION_FLAG
};


//! ids for rate class
enum
{
  YS_ID_RATE_MAIN_PROP_DOUBLE = YS_ID_RATE << 16
};
//! ids for well class
enum
{
  YS_ID_WELL_MAIN_PROP_DOUBLE = YS_ID_WELL << 16,
  YS_ID_WELL_MAIN_PROP_INT,
  YS_ID_WELL_INITIAL_RATES,
  YS_ID_WELL_NAME,
  YS_ID_WELL_GROUPNAME
};
//! ids for group class
enum
{
  YS_ID_GROUP_COMPENSATION = YS_ID_GROUP << 16,
  YS_ID_GROUP_COMPENSATION_TYPE,
  YS_ID_GROUP_START_TIME,
  YS_ID_GROUP_NAME
};
//! ids for time_step class
enum
{
  YS_ID_TIME_STEP_MAIN_PROP_INT = YS_ID_TIME_STEP << 16,
  YS_ID_TIME_STEP_MAIN_PROP_DOUBLE,
  YS_ID_TIME_STEP_GROUP,
  YS_ID_TIME_STEP_WELL,
  YS_ID_TIME_STEP_CONNECTION,
  YS_ID_TIME_STEP_IMPES_PARAM,
  YS_ID_TIME_STEP_WPIMUL,
  YS_ID_TIME_STEP_FIX_PRESSURE,
  YS_ID_TIME_STEP_STREAM_LINE_PARAMS,
  YS_ID_TIME_STEP_FI_TS_PARAMS
};
//! ids for spof class
enum
{
  YS_ID_SPOF_TABLE_LEN = YS_ID_SPOF << 16,
  YS_ID_SPOF_SATURATION,
  YS_ID_SPOF_PHASE_PERM,
  YS_ID_SPOF_OIL_PERM,
  YS_ID_SPOF_P_CAP,
  YS_ID_SPOF_MAIN_PROP
};
//! ids for spfn class
enum
{
  YS_ID_SPFN_TABLE_LEN = YS_ID_SPFN << 16,
  YS_ID_SPFN_SATURATION,
  YS_ID_SPFN_PHASE_PERM,
  YS_ID_SPFN_P_CAP,
  YS_ID_SPFN_MAIN_PROP
};
//! ids for sof2 class
enum
{
  YS_ID_SOF2_TABLE_LEN = YS_ID_SOF2 << 16,
  YS_ID_SOF2_SATURATION,
  YS_ID_SOF2_OIL_PERM
};
//! ids for sof3 class
enum
{
  YS_ID_SOF3_TABLE_LEN = YS_ID_SOF3 << 16,
  YS_ID_SOF3_SATURATION,
  YS_ID_SOF3_OIL_PERM_IN_WATER,
  YS_ID_SOF3_OIL_PERM_IN_GAS
};
//! ids for pvt class
enum
{
  YS_ID_PVT_DENSITY = YS_ID_PVT << 16,
  YS_ID_PVT_TABLE_LEN_AND_PVTO_FLAG,
  YS_ID_PVT_PRESSURE,
  YS_ID_PVT_FVF,
  YS_ID_PVT_VISCOSITY,
  YS_ID_PVT_GPR,
  YS_ID_PVT_MOLAR_DENSITY,
  YS_ID_PVT_COMPRESS_AND_P_REF,
  YS_ID_PVT_DOUBLE_DATA,
  YS_ID_PVT_INFO
};

//! ids for INIT
enum
{
  YS_ID_INIT_TITLE = YS_ID_INIT << 16,
  YS_ID_INIT_MAIN_PROP,
  YS_ID_INIT_DX,
  YS_ID_INIT_DY,
  YS_ID_INIT_DZ,
  YS_ID_INIT_MULTX,
  YS_ID_INIT_MULTY,
  YS_ID_INIT_MULTZ,
  YS_ID_INIT_TOPS,
  YS_ID_INIT_PERMX,
  YS_ID_INIT_PERMY,
  YS_ID_INIT_PERMZ,
  YS_ID_INIT_PORO,
  YS_ID_INIT_NTG,
  YS_ID_INIT_SOIL,
  YS_ID_INIT_SWAT,
  YS_ID_INIT_PRESSURE,
  YS_ID_INIT_EQLNUM,
  YS_ID_INIT_FIPNUM,
  YS_ID_INIT_SATNUM,
  YS_ID_INIT_PVTNUM,
  YS_ID_INIT_BNDNUM,
  YS_ID_INIT_ROCK,
  YS_ID_INIT_PREF,
  YS_ID_INIT_PVTO,
  YS_ID_INIT_PVTW,
  YS_ID_INIT_PVTG,
  YS_ID_INIT_SWOF,
  YS_ID_INIT_SGOF,
  YS_ID_INIT_FIP,
  YS_ID_INIT_TIME_STEP,
  YS_ID_INIT_START_DATE,
  YS_ID_INIT_COORD,
  YS_ID_INIT_ZCORN,
  YS_ID_INIT_DEPTH,
  YS_ID_INIT_W_TRAJECTORY,
  YS_ID_INIT_FRACTURES,
  YS_ID_INIT_RS,
  YS_ID_INIT_PBUB,
  YS_ID_INIT_FI_PARAMS,
  YS_ID_INIT_INT_POOL,
  YS_ID_INIT_DOUBLE_POOL,
  YS_ID_INIT_COMP_IDATA,
  YS_ID_INIT_SWFN,
  YS_ID_INIT_SGFN,
  YS_ID_INIT_SOF2,
  YS_ID_INIT_SOF3
};

//! ids for wpimul class
enum
{
  YS_ID_WPIMUL_MAIN_PROP_INT = YS_ID_WPIMUL << 16,
  YS_ID_WPIMUL_MAIN_PROP_DOUBLE,
  YS_ID_WPIMUL_WELL_NAME
};

//! ids for stream_line class
enum
{
  YS_ID_STREAM_LINE_MAIN_PROP_INT = YS_ID_STREAM_LINE << 16,
  YS_ID_STREAM_LINE_WELL_NAME,
  YS_ID_STREAM_LINE_COORDS,
  YS_ID_STREAM_LINE_INDEXES
};
//! ids for stream_line_params class
enum
{
  YS_ID_STREAM_LINE_PARAMS_MAIN_PROP_INT = YS_ID_STREAM_LINE_PARAMS << 16,
  YS_ID_STREAM_LINE_PARAMS_MAIN_PROP_DOUBLE
};

//! ids for fracture_list class
enum
{
  YS_ID_FRACTURES_MAIN_PROP_DOUBLE = YS_ID_FRACTURES << 16,
  YS_ID_FRACTURES_MAIN_PROP_INT,
  YS_ID_FRACTURES_WELL_NAME
};

enum
{
  YS_ID_W_TRAJECTORY_WELL_NAME = YS_ID_W_TRAJECTORY << 16,
  YS_ID_W_TRAJECTORY_INT_PROP,
  YS_ID_W_TRAJECTORY_TRAJECT
};

//! enumerate IDs for full implicit properties
enum
{
  YS_ID_FI_PARAMS_DOUBLE = YS_ID_FI_PARAMS << 16,
  YS_ID_FI_PARAMS_INT,
  YS_ID_FI_PARAMS_BOOLEAN
};

//! enumerate IDs for #smart_array class
enum
{
  YS_ID_SMART_ARRAY_INT_PROP = YS_ID_SMART_ARRAY << 16,
  YS_ID_SMART_ARRAY_INIT_ARRAY,
  YS_ID_SMART_ARRAY_VALUES
};

//! IF |a - b| < DIFF_EPSILON => a == b
#define DIFF_EPSILON 1.0e-12

enum
{
  YS_COMPARE_RESERVOIR = 1,
  YS_COMPARE_GROUP,
  YS_COMPARE_WELL,
  YS_COMPARE_CONNECTION
  //  YS_COMPARE_RATES,
};

//! enumerate IDs for #array_pool class
//! DO NOT CHANGE IT, OR COMPATIBILITY WITH OLD RST-FILES WILL BE LOST !!!!!!!!
enum
{
  YS_ID_POOL_INT_PROP = YS_ID_POOL << 16,
  YS_ID_POOL_ARRAYS //
};


//! ids for COMP_IDATA
enum
{
  YS_ID_COMP_IDATA_MAIN_PROP = YS_ID_COMP_IDATA << 16,
  YS_ID_COMP_IDATA_NAMES,
  YS_ID_COMP_IDATA_EOS,
  YS_ID_COMP_IDATA_EOSS,
  YS_ID_COMP_IDATA_BIC,
  YS_ID_COMP_IDATA_BICS,
  YS_ID_COMP_IDATA_POOL
};
//! ids for well_results class
enum
{
  YS_ID_WELL_RESULTS_WELL_NAME = YS_ID_WELL_RESULTS << 16,
  YS_ID_WELL_RESULTS_WELL_DATA
};
//! ids for well_data class
enum
{
  YS_ID_WDATA_DATES = YS_ID_WDATA << 16,
  YS_ID_WDATA_DATES_LEN,
  YS_ID_WDATA_DPARAMS_LEN,
  YS_ID_WDATA_DPARAMS,
  YS_ID_WDATA_IPARAMS_LEN,
  YS_ID_WDATA_IPARAMS,
  YS_ID_WDATA_CONN_CELL,
  YS_ID_WDATA_CONN_DATA,
  YS_ID_WDATA_GROUP_NAME,
};

//! ids for fip_results class
enum
{
  YS_ID_FDATA_DATES = YS_ID_FDATA << 16,
  YS_ID_FDATA_DATES_LEN,
  YS_ID_FDATA_DPARAMS_LEN,
  YS_ID_FDATA_DPARAMS,
};

//! ids for fip_results class
enum
{
  YS_ID_FIP_RESULTS_FIP_REGNUM = YS_ID_FIP_RESULTS << 16,
  YS_ID_FIP_RESULTS_FIP_DATA
};

//! ids for connection_results class
enum
{
  YS_ID_CONN_DATA_DATES = YS_ID_CONN_DATA << 16,
  YS_ID_CONN_DATA_DATES_LEN,
  YS_ID_CONN_DATA_DPARAMS_LEN,
  YS_ID_CONN_DATA_DPARAMS,
  YS_ID_CONN_DATA_IPARAMS_LEN,
  YS_ID_CONN_DATA_IPARAMS,
  YS_ID_CONN_DATA_CELL
};

//! ids for gctrl_gecon class
enum
{
  YS_ID_GCTRL_GECON_GNAME = YS_ID_GCTRL_GECON << 16,
  YS_ID_GCTRL_GECON_DPARAMS,
  YS_ID_GCTRL_GECON_IPARAMS,
  YS_ID_GCTRL_GECON_BPARAMS
};

//! ids for gctrl_switch_to_bhp class
enum
{
  YS_ID_GCTRL_SWITCH_TO_BHP_GNAME = YS_ID_GCTRL_SWITCH_TO_BHP << 16,
  YS_ID_GCTRL_SWITCH_TO_BHP_DPARAMS
};

#endif //_RES_FILE_ID_H_


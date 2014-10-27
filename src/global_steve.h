/* This file is created by steve,
 * for the use to define global variables 
 * that added to this project
 */
#ifndef _GLOBAL_STEVE_H_
#define _GLOBAL_STEVE_H_

#include "compute_approximate.h"
#include "appro_stat_sim.h"

//#define STEVE_DEBUG 
#define STEVE_GLOBAL 
#define APPRO

//for mode selection
#define NO_APPRO                    0
#define APPRO_OPERANDS_COMP_ALL     1
#define APPRO_OPERANDS_COMP_SELECT  2

//for F TYPE define
#include "appro_op.def"
#define F32_TYPE 307
#define F64_TYPE 308
#define FF64_TYPE 309

namespace steve_glb_sp{
    //extern FILE* value_output_file;
    extern int f32_type, f64_type, ff64_type;
    extern unsigned appro_mode;
    extern appro_stat_sim glb_appro_stat;
}

extern gpu_appro_stat g_gpu_appro_stat;
#endif

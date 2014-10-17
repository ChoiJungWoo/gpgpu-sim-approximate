#ifndef _GLOBAL_STEVE_H_
#define _GLOBAL_STEVE_H_

#include "compute_approximate.h"
//#define STEVE_DEBUG 
#define STEVE_GLOBAL 
#define APPRO
/* This file is created by steve,
 * for the use to define global variables 
 * that added to this project
 */

#define APPRO_OPERANDS_COMP_ALL     0
#define APPRO_OPERANDS_COMP_SELECT  1

#include "appro_op.def"
#define F32_TYPE 307
#define F64_TYPE 308
#define FF64_TYPE 309

namespace steve_glb_sp{
    extern FILE* exec_output_file;
    extern FILE* value_output_file;
    extern unsigned long long glb_cycle[15];
    extern unsigned long glb_last_pc;
    extern unsigned int  glb_last_sid;
    extern unsigned long long glb_last_cycle[15];
    extern unsigned int  glb_last_uid;
    extern int f32_type, f64_type, ff64_type;
    extern unsigned appro_mode;

}

extern gpu_appro_stat g_gpu_appro_stat;
/*
extern appro_stat g_appro_output;
extern appro_stat g_appro_op_1;
extern appro_stat g_appro_op_2;
extern appro_stat g_appro_op_3;
extern appro_stat g_appro_op_array[3];
*/

#endif

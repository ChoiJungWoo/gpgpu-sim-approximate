#ifndef _COMPUTE_APPROXIMATE_H_
#define _COMPUTE_APPROXIMATE_H_

#include <stdio.h>
#include <fstream>
#include <iostream>

//using namespace std;

#define ADD    1
#define SUB    67
#define MAD24  30
#define MAD    31
#define MUL24  37
#define MUL    38
#define DIV    22

class appro_stat;
class gpu_appro_stat;

class gpu_appro_stat{
public:
   gpu_appro_stat();
   appro_stat *g_appro_output;
   appro_stat *g_appro_op_1;
   appro_stat *g_appro_op_2;
   appro_stat *g_appro_op_3;
   appro_stat *g_appro_op_array[3];

   void record_op(unsigned int op);
   void record_op_class(unsigned int op_class);
   void print_stat(FILE *fp);

   void compute_pred_output_f(unsigned int opcode, unsigned int lid);
   void compute_warp_pred_output_error_f();

   void re_init_values();
private:
   unsigned int histOpClass[11] ;
   unsigned int histOp[88] ;
   unsigned int f_op;

   long double pred_output_f[32] ;
   long long   pred_output_i[32] ;

   long long num_pred_output;
   long long num_warp_pred_output_f;

   long double sum_output_error_f;
   long double sum_output_nor_error_f;

   unsigned int lane_mask;

   unsigned int op_code;
};

//*****************************
//appro_stat
//*****************************
class appro_stat{
public:
   appro_stat(std::string class_name);

   double compute_r_f();
   double compute_r_i();
   
   void record_error_f();
   void record_error_i();

   void record_warp_error_f();
   void record_warp_error_i();

   void compute_pred_f();
   void compute_pred_i();

   void re_init_values();

   void compute_final();

   float compute_final_error();
   float compute_average_r();

   void set_ob_val(long double val, int lane_id);
   void set_ob_val(long long val, int lane_id);

   bool check_array_nan_f();

   void print_stat(FILE *fp);

   void print_warp_values_f(FILE *);
   void print_warp_values_i(FILE *);

   long double get_pred_values_f(unsigned int i);
   long long get_pred_values_i(unsigned int i);
   long double get_ob_values_f(unsigned int i);
   long long get_ob_values_i(unsigned int i);

private:
   std::string name;
   
   long double ob_values_f[32];
   long double pred_values_f[32];

   long long ob_values_i[32];
   long long pred_values_i[32];

   float coefficient_r;
   float average_r;
   float average_f_r;
   float average_i_r;

   long double ret_r;

   long double sum_r;
   long double sum_f_r;
   long double sum_i_r;

   long double final_error;
   long double final_normalized_error;

   long double final_f_error;
   long double final_nor_f_error;

   long double final_i_error;
   long double final_nor_i_error;

   long double final_avg_error;
   long double final_avg_error_f;
   long double final_avg_error_i;

   long double final_avg_nor_error;
   long double final_avg_pred_error;
   long double final_avg_pred_nor_error;
   long double final_avg_non_pred_error;
   long double final_avg_non_pred_nor_error;

   //quantitative
   long long num_is_perfect_pred;
   long long num_is_high_pred;
   long long num_comp_warps_r;

   long long num_values;
   long long num_float;
   long long num_int;

   long long num_preded_warps;
   long long num_warps_error;

   long long num_float_warps;
   long long num_int_warps;

   //errors
   long double sum_bias_sqr;
   long double sum_ob_values_sqr;
   long double sum_bias_sqr_f;
   long double sum_ob_values_sqr_f;
   long double sum_bias_sqr_i;
   long double sum_ob_values_sqr_i;

   long double sum_error_f;
   long double sum_nor_error_f;

   long double sum_error_i;
   long double sum_nor_error_i;

   long double sum_pred_error_f;
   long double sum_pred_error_i;

   long double sum_non_pred_error_f;
   long double sum_non_pred_error_i;

   long double sum_pred_nor_error_f;
   long double sum_pred_nor_error_i;

   long double sum_non_pred_nor_error_f;
   long double sum_non_pred_nor_error_i;
   //long double sum_comp_warp_error;
   
   //others
   unsigned int lane_set;

   FILE * r_dump_file;

};

#endif

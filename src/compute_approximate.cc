#include <math.h>
#include <assert.h>
#include"compute_approximate.h"
#define NO_LANE 32

//using namespace std;

appro_stat::appro_stat(std::string class_name){
   name = class_name;
   r_dump_file = fopen( (char *)name.append(".r_dump").c_str(), "w");

   ob_values_f[32] = {0};
   pred_values_f[32] = {0};
   
   ob_values_i[32] = {0};
   pred_values_i[32] = {0};
   
   coefficient_r = 0;
   average_r =0;
   average_f_r =0;
   average_i_r =0;
   
   ret_r =0;
   
   sum_r =0;
   sum_f_r=0;
   sum_i_r=0;
   
   final_error=0;
   final_normalized_error=0;
   
   final_f_error =0;
   final_nor_f_error=0;
   
   final_i_error =0;   
   final_nor_i_error=0;
   
   final_avg_nor_error =0;
   final_avg_error   =0;
   final_avg_error_f =0;
   final_avg_error_i =0;
   //
   num_is_perfect_pred = 0;
   num_is_high_pred = 0;
   num_comp_warps_r = 0;
   
   num_values = 0; 
   num_float =0;
   num_int =0;
   
   num_preded_warps = 0;
   num_warps_error  = 0;
   
   num_float_warps=0;
   num_int_warps=0;
   //
   sum_bias_sqr =0;
   sum_ob_values_sqr=0;
   sum_bias_sqr_f =0;
   sum_ob_values_sqr_f=0;
   sum_bias_sqr_i =0;
   sum_ob_values_sqr_i=0;
   //sum_comp_warp_error =0;
   sum_error_f =0;
   sum_nor_error_f=0;
   
   sum_error_i=0;
   sum_nor_error_i=0;
   
   sum_pred_error_f=0;
   sum_pred_error_i=0;
   
   sum_non_pred_error_f=0;
   sum_non_pred_error_i=0;
   
   sum_pred_nor_error_f=0;
   sum_pred_nor_error_i=0;
   
   sum_non_pred_nor_error_f=0;
   sum_non_pred_nor_error_i=0;
   
   lane_set = 0;
}

double appro_stat::compute_r_f( ) {
   if( lane_set != 0xffffffff ){
      lane_set = 0x0;
      return 0;
   }

   ret_r=0;
   long double sum_ob = 0, sum_pred =0, sum_ob_pred =0, sum_ob_sqr =0, sum_pred_sqr =0;

   for(int i = 0 ; i < NO_LANE ; i++ ){
      sum_ob      +=    ob_values_f[i];
      sum_pred    +=    pred_values_f[i];
      sum_ob_pred +=    ob_values_f[i]   * pred_values_f[i];
      sum_ob_sqr  +=    ob_values_f[i]   * ob_values_f[i];
      sum_pred_sqr +=   pred_values_f[i] * pred_values_f[i];
   }

   long double denominator = sqrt( NO_LANE * sum_ob_sqr - sum_ob * sum_ob ) 
                        * 
                        sqrt( NO_LANE * sum_pred_sqr - sum_pred * sum_pred ) ;
   long double numerator = NO_LANE * sum_ob_pred - sum_ob * sum_pred ;

   //if( isnan(numerator)){
      //printf("sum_ob_pred (%Le) sum_pred (%Le)\n", sum_ob_pred, sum_pred );
   //}

   if(denominator == 0 && numerator == 0){
      ret_r = 1.0;
      //print_warp_values_f(stdout);
   }
   else ret_r = numerator / denominator;

   if( isnan(ret_r) || isinf(ret_r))
      ret_r =0;

   if(ret_r ==1 || ret_r ==-1)
      num_is_perfect_pred++;
   if((ret_r >= 0.8 && ret_r <1) || (ret_r <= -0.8 && ret_r >-1))
      num_is_high_pred++;

   sum_r += ret_r;
   sum_f_r += ret_r;
   //printf("*** ret_r (%Le)\n", ret_r);

   //number of computed r
   num_comp_warps_r++;
   num_float_warps ++;

   //fprintf(r_dump_file,"f %Le\n",ret_r);
   //fflush(r_dump_file);
   return ret_r;
}

double appro_stat::compute_r_i( ) {
   if( lane_set != 0xffffffff){
      lane_set = 0x0;
      return 0;
   }

   ret_r = 0;
   long double sum_ob = 0, sum_pred =0, sum_ob_pred =0, sum_ob_sqr =0, sum_pred_sqr =0;

   for(int i =0 ; i < NO_LANE  ; i++ ){
      sum_ob      +=    ob_values_i  [i];
      sum_pred    +=    pred_values_i[i];
      sum_ob_pred +=    ob_values_i  [i] * pred_values_i[i];
      sum_ob_sqr  +=    ob_values_i  [i] * ob_values_i  [i];
      sum_pred_sqr +=   pred_values_i[i] * pred_values_i[i];
   }

   long double denominator =  sqrt( NO_LANE * sum_ob_sqr - sum_ob * sum_ob ) 
                        * 
                        sqrt( NO_LANE * sum_pred_sqr - sum_pred * sum_pred ) ;

   long double numerator = NO_LANE * sum_ob_pred - sum_ob * sum_pred ;

   if(denominator == 0 && numerator == 0){
      ret_r = 1.0;
      //print_warp_values_i(stdout);
   }
   else ret_r = numerator / denominator;

   if( isnan(ret_r) || isinf(ret_r) )
      ret_r = 0;

   if( ret_r == 1.0 || ret_r == -1.0 )
      num_is_perfect_pred++;

   if( (ret_r >= 0.8 && ret_r < 1.0 ) || (ret_r <= -0.8 && ret_r > -1.0 ) )
      num_is_high_pred++;

   sum_r += ret_r;
   sum_i_r += ret_r;

   //number of computed r
   num_comp_warps_r++;
   num_int_warps++;

   //fprintf(r_dump_file,"i %Le\n",ret_r);
   //fflush(r_dump_file);
   return ret_r;
}

void appro_stat::record_error_f(){
   if(lane_set != 0xffffffff){
      lane_set = 0x0;
      return;
   }

   long double sum_warp_bias_sqr =0;
   long double sum_warp_ob_values_sqr =0;

   for(int i=0; i<NO_LANE; i++){
      sum_warp_bias_sqr       += (long double)(ob_values_f[i] - pred_values_f[i])
                                 *(ob_values_f[i] - pred_values_f[i]);
      sum_warp_ob_values_sqr  += (long double)ob_values_f[i] * ob_values_f[i];
   }
   
   num_warps_error++;
}

void appro_stat::record_error_i(){
   if(lane_set != 0xffffffff){
      lane_set = 0x0;
      return;
   }

   long double sum_warp_bias_sqr =0;
   long double  sum_warp_ob_values_sqr=0;

   for(int i=0; i<NO_LANE; i++){
      sum_warp_bias_sqr       += (long double) (ob_values_i[i] - pred_values_i[i])
                                 *(ob_values_i[i] - pred_values_i[i]);
      sum_warp_ob_values_sqr  += (long double) ob_values_i[i] * ob_values_i[i];
   }

   num_warps_error++;
}

void appro_stat::record_warp_error_f(){
   if(lane_set != 0xffffffff){
      lane_set = 0x0;
      return;
   }

   long double sum_warp_bias_sqr =0;
   long double sum_warp_ob_values_sqr =0;

   for(int i=0; i<NO_LANE; i++){
      sum_warp_bias_sqr       += (ob_values_f[i] - pred_values_f[i])
                                 *(ob_values_f[i] - pred_values_f[i]);
      sum_warp_ob_values_sqr  += ob_values_f[i] * ob_values_f[i];
   }

   long double warp_error     = sqrt( sum_warp_bias_sqr / 32);
   long double warp_nor_error = 0;

   if(sum_warp_ob_values_sqr != 0)
      warp_nor_error = sqrt( sum_warp_bias_sqr / (32 * sum_warp_ob_values_sqr) );

   sum_error_f     += warp_error;
   sum_nor_error_f += warp_nor_error;

   if( isnan(warp_nor_error) )
      return;
   
   if(ret_r <= -0.8 || ret_r >= 0.8){
      sum_pred_error_f     += warp_error;
      sum_pred_nor_error_f += warp_nor_error;
   }
   else {
      sum_non_pred_error_f     += warp_error;
      sum_non_pred_nor_error_f += warp_nor_error;
   }
   
   num_warps_error++;
}

void appro_stat::record_warp_error_i(){
   if(lane_set != 0xffffffff){
      lane_set = 0x0;
      return;
   }

   long double sum_warp_bias_sqr =0;
   long double sum_warp_ob_values_sqr=0;

   for(int i=0; i<NO_LANE; i++){
      sum_warp_bias_sqr       += (long double)(ob_values_i[i] - pred_values_i[i])
                                 * (long double)(ob_values_i[i] - pred_values_i[i]);
      sum_warp_ob_values_sqr  += (long double)ob_values_i[i] * ob_values_i[i];
   }

   long double warp_error = 0;
   warp_error = sqrt( sum_warp_bias_sqr / 32);

   long double warp_nor_error=0;
   if(sum_warp_ob_values_sqr != 0)
      warp_nor_error = sqrt( sum_warp_bias_sqr / (32 * sum_warp_ob_values_sqr) );

   if(isnan(warp_error)){
      printf("*** warp_error_i NAN! sum_warp_bias_sqr (%Le)\n", sum_warp_bias_sqr);
      warp_error = 0;
   }

   if( isnan(warp_nor_error) )
      return;

   sum_error_i += warp_error;
   sum_nor_error_i += warp_nor_error;

   if(ret_r <= -0.8 || ret_r >= 0.8){
      sum_pred_error_i     += warp_error;
      sum_pred_nor_error_i += warp_nor_error;
   }
   else {
      sum_non_pred_error_i     += warp_error;
      sum_non_pred_nor_error_i += warp_nor_error;
   }

   num_warps_error++;
}

void appro_stat::set_ob_val(long double val, int lane_id){
   if(lane_id == 0)
      lane_set = 0;

   if(isnan(val)){
      return ;
   }

   if(isinf(val)){
      return ;
   }

   lane_set ^= (1<<lane_id);

   ob_values_f[lane_id] = val;

   if(lane_id == 31){
      compute_pred_f();
      if(!check_array_nan_f()){
         return;
      }
      compute_r_f();
      record_warp_error_f();
   }

   num_values ++;
   num_float ++;
}

void appro_stat::set_ob_val(long long val, int lane_id){
   if(lane_id == 0){
      lane_set = 0;
   }

   if(isnan(val)){
      return ;
   }

   if(isinf(val)){
      return ;
   }

   lane_set ^= (1<<lane_id);

   ob_values_i[lane_id] = val;

   if(lane_id == 31){
      compute_pred_i();
      compute_r_i();
      record_warp_error_i();
   }

   num_values ++;
   num_int++;
}

void appro_stat::re_init_values(){
   for(int i=0;i<NO_LANE; i++){
      ob_values_f[i] = 0;
      pred_values_f[i] = 0;
      ob_values_i[i] = 0;
      pred_values_i[i] = 0;
   }
}

void appro_stat::compute_pred_f(){
   pred_values_f[0] = ob_values_f[0];
   pred_values_f[31] = pred_values_f[NO_LANE-1];

   double dist = (ob_values_f[31]- ob_values_f[0] )/(float)(NO_LANE-1);

   for(int i=1; i < NO_LANE ; i++){
      pred_values_f[i] = pred_values_f[i-1] + dist;
   }

   num_preded_warps++;
} 

void appro_stat::compute_pred_i(){
   pred_values_i[0] = ob_values_i[0];
   pred_values_i[31] = pred_values_i[NO_LANE-1];

   double dist = (float)(ob_values_i[31] -ob_values_i[0] )/(float)(NO_LANE-1);

   for(int i=1; i < NO_LANE ; i++){
      pred_values_i[i] = pred_values_i[i-1] + dist;
   }

   num_preded_warps++;
} 

void appro_stat::compute_final(){
   printf("sum_error_f (%Le) sum_error_i (%Le) / num_warps_error(%lld)\n", sum_error_f , sum_error_i, num_warps_error);

   final_avg_error = (long double)( (sum_error_f + sum_error_i) / (num_warps_error));

   if(num_float_warps != 0)
      final_avg_error_f = (long double) sum_error_f / num_float_warps;
   
   if(num_int_warps != 0)
      final_avg_error_i = (long double) sum_error_i / num_int_warps;

   printf("sum_pred_nor_error_i+sum_pred_nor_error_f (%Le) / num_is_perfect_pred + num_is_high_pred (%lld)\n", sum_pred_nor_error_i+sum_pred_nor_error_f,num_is_perfect_pred + num_is_high_pred );

   if( num_comp_warps_r != 0)
      final_avg_nor_error = (long double) ( (  sum_pred_nor_error_i + sum_pred_nor_error_f + sum_non_pred_nor_error_i + sum_non_pred_nor_error_f ) / num_comp_warps_r );

   if( (num_is_perfect_pred+num_is_high_pred) != 0)
      final_avg_pred_nor_error = (long double) ( (  sum_pred_nor_error_i + sum_pred_nor_error_f) / (num_is_perfect_pred + num_is_high_pred) );

   if( (num_comp_warps_r - num_is_perfect_pred - num_is_high_pred) != 0)
      final_avg_non_pred_nor_error = (long double)( ( sum_non_pred_nor_error_i + sum_non_pred_nor_error_f) / ( num_comp_warps_r - num_is_perfect_pred - num_is_high_pred) );

   average_r = sum_r / num_comp_warps_r;

   if(num_float_warps!=0){
      average_f_r = (long double) sum_f_r / num_float_warps;
   }

   if(num_int_warps!=0){
      average_i_r = (long double) sum_i_r / num_int_warps;
   }

   printf("*** sum_r (%Le) num_comp_warps_r (%lld)\n", sum_r, num_comp_warps_r);
   //r_dump_file.close();
   //fclose(r_dump_file);
}
   
float appro_stat::compute_final_error(){
   return sqrt(sum_bias_sqr/(num_values * sum_ob_values_sqr));
}

float appro_stat::compute_average_r(){
   return sum_r/num_comp_warps_r;
}

bool appro_stat::check_array_nan_f(){
   for(int i =0; i< 32;i++){
      if( isnan(ob_values_f[i]) ){
         return false;
      }
      if( isnan(pred_values_f[i]) ){
         return false;
      }
      if( isinf(ob_values_f[i]) ){
         return false;
      }
      if( isinf(pred_values_f[i]) ){
         return false;
      }

      if( isnan(ob_values_i[i]) ){
         return false;
      }
      if( isnan(pred_values_i[i]) ){
         return false;
      }
      if( isinf(ob_values_i[i]) ){
         return false;
      }
      if( isinf(pred_values_i[i]) ){
         return false;
      }
   }
   return true;
}

void appro_stat::print_stat(FILE *fp){
   compute_final();

   fprintf(fp, "\n=====approximate stat:%s======\n",name.c_str());

   fprintf(fp, "average r        \t-----(%f) f(%f) i(%f)\n",                    
               average_r, average_f_r, average_i_r);

   fprintf(fp, "------------------------------\n");

   fprintf(fp, "avg error: \t----(%Le) = f(%Le) i(%Le)\n",
               final_avg_error, final_avg_error_f, final_avg_error_i);

   fprintf(fp, "avg pred/non-pred normalized error: \t----overall (%Le) pred(%Le) non-pred(%Le)\n",
               final_avg_nor_error, final_avg_pred_nor_error, final_avg_non_pred_nor_error);

   //fprintf(fp, "------------------------------\n");

   //fprintf(fp, "RMSE       \t-----(%Le = sqrt(%Le / %llu )\n",             
   //            final_error, sum_bias_sqr, num_values);
   //fprintf(fp, "NRMSE      \t-----(%Le = sqrt(%Le / (%lld * %Le) ) )\n",   
   //            final_normalized_error, sum_bias_sqr, num_values, sum_ob_values_sqr );
   //fprintf(fp, "------------------------------\n");

   //fprintf(fp, "float RMSE \t-----(%Le = sqrt(%Le / %lld )\n", 
   //            final_f_error, sum_bias_sqr_f, num_float);
   //fprintf(fp, "float NRMSE\t-----(%Le = sqrt(%Le / (%lld * %Le) ) )\n", 
   //            final_nor_f_error, sum_bias_sqr_f, num_float, sum_ob_values_sqr_f );
   //fprintf(fp, "------------------------------\n");

   //fprintf(fp, "int RMSE   \t-----(%Le = sqrt(%Le / %lld )\n", 
   //            final_i_error, sum_bias_sqr_i, num_int);
   //fprintf(fp, "int NRMSE  \t-----(%Le = sqrt(%Le / (%lld * %Le) ) )\n", 
   //            final_nor_i_error, sum_bias_sqr_i, num_int, sum_ob_values_sqr_i );
   fprintf(fp, "------------------------------\n");

   fprintf(fp, "number of perfect r\t-----(%llu/%llu = %f)\n", 
               num_is_perfect_pred, num_preded_warps, (float)num_is_perfect_pred/(float)num_preded_warps);

   fprintf(fp, "number of high r \t-----(%llu/%llu = %f)\n", 
               num_is_high_pred, num_preded_warps, (float)num_is_high_pred/num_preded_warps);

   fprintf(fp, "num_values       \t-----(%llu)\n", num_values);
   fprintf(fp, "num_comp_warp_r  \t-----(%llu = (f)%llu + (i)%llu)\n", num_comp_warps_r, num_float_warps, num_int_warps);
   fprintf(fp, "num_warps_error  \t-----(%llu)\n", num_warps_error);
   fprintf(fp, "num_preded_warps \t-----(%llu)\n", num_preded_warps);
   fflush(fp);
}


void appro_stat::print_warp_values_f(FILE *out_file){
   for(int i = 0; i < 32; i++){
     fprintf(out_file,
               "[%Le]",
               ob_values_f[i]);
   }
   fprintf(out_file, "\n");

   for(int i = 0; i < 32; i++){
     fprintf(out_file,
               "[%Le]",
               pred_values_f[i]);
   }
   fprintf(out_file, "\n");
}

void appro_stat::print_warp_values_i(FILE *out_file){
   for(int i = 0; i < 32; i++){
     fprintf(out_file,
               "[%lld]",
               ob_values_i[i]);
   }
   fprintf(out_file, "\n");

   for(int i = 0; i < 32; i++){
     fprintf(out_file,
               "[%lld]",
               pred_values_i[i]);
   }
   fprintf(out_file, "\n");
}

long double appro_stat::get_pred_values_f(unsigned int i){
   return pred_values_f[i];
}
long double appro_stat::get_ob_values_f(unsigned int i){
   return ob_values_f[i];
}

long long appro_stat::get_pred_values_i(unsigned int i){
   return pred_values_i[i];
}
long long appro_stat::get_ob_values_i(unsigned int i){
   return ob_values_i[i];
}

//***************************************************
//gpu_appro_stat
//***************************************************
gpu_appro_stat::gpu_appro_stat(){
   g_appro_output = new appro_stat("output");
   g_appro_op_1 = new appro_stat("op_1");
   g_appro_op_2 = new appro_stat("op_2");
   g_appro_op_3 = new appro_stat("op_3");
   g_appro_op_array[0] = g_appro_op_1;
   g_appro_op_array[1] = g_appro_op_2;
   g_appro_op_array[2] = g_appro_op_3;

   num_pred_output=0;
   num_warp_pred_output_f=0;
   f_op = 0;
}

void gpu_appro_stat::record_op(unsigned int op){
   histOp[op]++;
   f_op++;
}

void gpu_appro_stat::record_op_class(unsigned int op_class){
    histOpClass[op_class]++;  
    //tot num
    histOpClass[0]++;
}

void gpu_appro_stat::print_stat(FILE *fp){
   fprintf(fp, 
         "=====gpu_appro_stat: float =====\n");
   fprintf(fp,
         "tot num of float opcode (%u)\n",  histOpClass[0]);
   fprintf(fp,
         "num of flaot ALU opcode (%u) (%3f)\n", histOpClass[1], ((float)histOpClass[1])/histOpClass[0]);
   fprintf(fp, "\tADD (%3f) SUB(%3f) MAD(%f) MUL(%3f) DIV(%3f) = (%f)\n",
         (float)histOp[ADD]                  /histOpClass[1],
         (float)histOp[SUB]                  /histOpClass[1],
         (float)(histOp[MAD24]+histOp[MAD])  /histOpClass[1],
         (float)(histOp[MUL24]+histOp[MUL])  /histOpClass[1],
         (float)(histOp[DIV])                /histOpClass[1],
         (float)(histOp[ADD]+histOp[SUB]+histOp[MAD24]+histOp[MAD]+histOp[MUL24]+histOp[MUL]+histOp[DIV])        /histOpClass[1]
         );

   fprintf(fp,
         "num of CTR opcode (%u) (%3f)\n", histOpClass[3], ((float)histOpClass[3])/histOpClass[0]);
   fprintf(fp,
         "num of MEM opcode (%u) (%3f)\n", histOpClass[5], ((float)histOpClass[5])/histOpClass[0]);

   fprintf(fp,"----------------------\n");
   
   fprintf(fp,
         "float pred output errors f(%Le) = (%Le)/(%Le)\n",
         sum_output_error_f/num_warp_pred_output_f, sum_output_error_f, num_warp_pred_output_f );

   fprintf(fp,
         "float pred output normalized errors f(%Le) = (%Le)/(%Le)\n",
         sum_output_nor_error_f/num_warp_pred_output_f, sum_output_nor_error_f, num_warp_pred_output_f );
   fprintf(fp,
         "float num of ASMD op (%llu)/tot(%u) = (%3f)\n",
         num_pred_output, histOpClass[0], (float)num_pred_output/histOpClass[0]); 
   
   fflush(fp);

   g_appro_output->print_stat(fp);
   g_appro_op_array[0]->print_stat(fp);
   g_appro_op_array[1]->print_stat(fp);
   g_appro_op_array[2]->print_stat(fp);
}

void gpu_appro_stat::compute_pred_output_f(unsigned int opcode, unsigned int lid){
   record_op(opcode);

   if(lid == 0 )
      lane_mask = 0;
   lane_mask ^= 0x1<<lid; 

   op_code = opcode;

   long double op_1 = g_appro_op_1->get_pred_values_f(lid);
   long double op_2 = g_appro_op_2->get_pred_values_f(lid);
   long double op_3 = g_appro_op_3->get_pred_values_f(lid);

   switch (opcode){
      case ADD: 
         pred_output_f[lid] = op_1 + op_2;
         if( isnan(pred_output_f[lid]) ){
            printf("add: pred_output_f[%u] (%Le)\n", lid, pred_output_f[lid]);
         }
         if(lane_mask == 0xffffffff){
            compute_warp_pred_output_error_f();
            re_init_values();
         }
         num_pred_output ++;
      break;
      case SUB:
         pred_output_f[lid] = op_1 - op_2;
         if( isnan(pred_output_f[lid]) ){
            printf("sub: pred_output_f[%u] (%Le)\n", lid, pred_output_f[lid]);
         }
         if(lane_mask == 0xffffffff){
            compute_warp_pred_output_error_f();
            re_init_values();
         }
         num_pred_output ++;
         break;

      case MAD24:
      case MAD:
         pred_output_f[lid] = op_1 * op_2 + op_3;
         if( isnan(pred_output_f[lid]) ){
            printf("MAD: pred_output_f[%u] (%Le) (%Le)\n", lid, pred_output_f[lid], g_appro_output->get_ob_values_f(lid) );
         }
         if(lane_mask == 0xffffffff){
            compute_warp_pred_output_error_f();
            re_init_values();
         }
         num_pred_output ++;
         break;

      case MUL24:
      case MUL:
         pred_output_f[lid] = op_1 * op_2;
         if( isnan(pred_output_f[lid]) ){
            printf("mul: pred_output_f[%u] (%Le) (%Le)\n", lid, pred_output_f[lid], g_appro_output->get_ob_values_f(lid) );
         }
         if(lane_mask == 0xffffffff){
            compute_warp_pred_output_error_f();
            re_init_values();
         }
         num_pred_output ++;
         break;

      case DIV:
         pred_output_f[lid] = op_1 / op_2;
         if( isnan(pred_output_f[lid]) ){
            printf("div: pred_output_f[%u] (%Le)=(%Le)/(%Le) | (%Le) = (%Le)/(%Le)\n", lid, pred_output_f[lid],op_1, op_2, g_appro_output->get_ob_values_f(lid), g_appro_op_1->get_ob_values_f(lid), g_appro_op_2->get_ob_values_f(lid) );
            pred_output_f[lid] = 0;
         }
         if( isinf(pred_output_f[lid]) ){
            printf("div: pred_output_f[%u] (%Le)=(%Le)/(%Le) | (%Le) = (%Le)/(%Le)\n", lid, pred_output_f[lid],op_1, op_2, g_appro_output->get_ob_values_f(lid), g_appro_op_1->get_ob_values_f(lid), g_appro_op_2->get_ob_values_f(lid) );
            pred_output_f[lid] = 0;
         }
         if(lane_mask == 0xffffffff){
            compute_warp_pred_output_error_f();
            re_init_values();
         }
         num_pred_output ++;
         break;

      default: break;
   }
}

void gpu_appro_stat::compute_warp_pred_output_error_f(){
   long double sum_warp_bias_sqr =0;
   long double sum_warp_ob_values_sqr =0;

   for(int i=0; i<NO_LANE; i++){
      long double real_output = g_appro_output->get_ob_values_f(i);

      sum_warp_bias_sqr       += (real_output - pred_output_f[i])
                                 *(real_output - pred_output_f[i]);
      sum_warp_ob_values_sqr  += real_output * real_output;
   }

   long double warp_error     = sqrt( sum_warp_bias_sqr / 32);
   long double warp_nor_error = 0;

   if(sum_warp_ob_values_sqr != 0)
      warp_nor_error = sqrt( sum_warp_bias_sqr / (32 * sum_warp_ob_values_sqr) );


#ifdef STEVE_DEGUB
   if(warp_nor_error != 0.0){
      printf("%Le---------------------\n", warp_nor_error); 
      printf("op1\t");
      switch(op_code){
         case ADD: printf("(%u)+",op_code); break;
         case MUL24: printf("(%u)*(24)",op_code); break;
         case MUL: printf("(%u)*",op_code); break;
         case DIV: printf("(%u)/", op_code); break;
         default: printf("(%u)?", op_code);  break;
      }
      printf("\top2\t \tout\t \tpred\n");
      for(int i =0 ;i<2; i++)
         printf("[%Le] \t[%Le] \t[%Le] \t[%Le] \n", g_appro_op_1->get_ob_values_f(i), g_appro_op_2->get_ob_values_f(i), g_appro_output->get_ob_values_f(i), pred_output_f[i]);
      //printf("\n");
      /*
      printf("op2:");
      for(int i =0 ;i<32; i++)
         printf("[%Le] ", g_appro_op_2->get_ob_values_f(i));
      printf("\n");
      printf("output:");
      for(int i =0 ;i<32; i++)
         printf("[%Le] ", g_appro_output->get_ob_values_f(i));
      printf("\n");
      printf("pred:");
      for(int i =0 ;i<32; i++)
         printf("[%Le] ", pred_output_f[i]);
      printf("\n");
      */
      printf("---------------------\n");
   }
#endif

   if( isnan(warp_error) ){
      return;
   }

   if( isnan(warp_nor_error) ){
      return;
   }

   sum_output_error_f     += warp_error;
   sum_output_nor_error_f += warp_nor_error;


   num_warp_pred_output_f ++;
}

void gpu_appro_stat::re_init_values(){
   for(int i=0; i<NO_LANE;i++){
      pred_output_f[i]=0;
   }

   g_appro_output->re_init_values();
   for(int i =0;i <3;i++){
      g_appro_op_array[i]->re_init_values();
   }
}

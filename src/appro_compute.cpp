#include"abstract_hardware_model.h"
#include"global_steve.h"
#include"./cuda-sim/ptx_ir.h"
#include<math.h>

//steve appro
void core_t::appro_src_all_exe_f(warp_inst_t &inst, unsigned warpId){
    const unsigned warp_size_f = m_warp_size;
    const ptx_instruction *perWarp_pI[warp_size_f];
    int opcode_warp[warp_size_f];

    double src1_data[warp_size_f],
           src2_data[warp_size_f],
           src3_data[warp_size_f],
           dest_data[warp_size_f];

    operand_info dst_warp[warp_size_f];
    unsigned i_type[warp_size_f];
    //get_src
    get_src_t(inst, warpId, perWarp_pI, opcode_warp, i_type, dst_warp, src1_data, src2_data, src3_data);

    //compute the approximate src values
    if( get_dest_appro_src_exe_all_f( opcode_warp, src1_data, src2_data, src3_data, dest_data, warp_size_f ) == false )
        return ;

    //commit all
    appro_src_commit_all(inst, warpId, i_type, dest_data, dst_warp, perWarp_pI);

}

void core_t::commit_warp_dest(warp_inst_t &inst, unsigned warpId, unsigned i_type[], double dest_data[], operand_info dst_warp[], const ptx_instruction *perWarp_pI[] ){
    //commit the dest_data[] to the registers
    ptx_reg_t commit_data[m_warp_size];

    for ( unsigned t=0; t < m_warp_size; t++ ) {
        if( inst.active(t) ) {
            if(warpId==(unsigned (-1)))
                warpId = inst.warp_id();

            switch(i_type[t]){
                case F32_TYPE:
                    commit_data[t].f32 = dest_data[t]; break;
                case F64_TYPE:
                case FF64_TYPE:
                    commit_data[t].f64 = dest_data[t]; break;
                default: return;
                         break;
            }
        }
    }

    for ( unsigned t=0; t < m_warp_size; t++ ) {
        if( inst.active(t) ) {
            if(warpId==(unsigned (-1)))
                warpId = inst.warp_id();
            unsigned tid=m_warp_size*warpId+t;

            m_thread[tid]->set_operand_value(dst_warp[t], commit_data[t], i_type[t], m_thread[tid], perWarp_pI[t], 0, 0  );
            //the last two are overflow and carry, 0s for floating op
        }
    }
}

void core_t::get_appro_src(const double src1_data[], const double src2_data[], const double src3_data[], double appro_src1[], double appro_src2[], double appro_src3[] ){
    const unsigned warp_size = m_warp_size;
    double src1_step_diff = 0,
           src2_step_diff = 0,
           src3_step_diff = 0;

    src1_step_diff = (src1_data[warp_size-1] - src1_data[0])/(warp_size - 1);
    src2_step_diff = (src2_data[warp_size-1] - src2_data[0])/(warp_size - 1);
    src3_step_diff = (src3_data[warp_size-1] - src3_data[0])/(warp_size - 1);
    //approximate
    for(unsigned t = 0 ; t < warp_size ; t++){
        appro_src1[t] = src1_data[0] + src1_step_diff*t;
        appro_src2[t] = src2_data[0] + src2_step_diff*t;
        appro_src3[t] = src3_data[0] + src3_step_diff*t;
    }
}

//return the R value
bool core_t::get_dest_appro_src_exe_all_f( const int op_warp[],const double src1_data[], const double src2_data[], const double src3_data[], double dest_data[],  const unsigned warp_size){

    double appro_src1[warp_size],
           appro_src2[warp_size],
           appro_src3[warp_size];
    //get appro_src
    get_appro_src(src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3);

    //check R
    //check the corellational coefficient R, if R < 0.9, reject the approximate values
    if(check_R(op_warp[0], src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3) == false)
        return false;

    //approximate instruction
    for(unsigned i=0; i < warp_size; i++){
        switch(op_warp[i]){
            case ADD:
                dest_data[i] = appro_src1[i]+appro_src2[i];
                break;
            case SUB:
                dest_data[i] = appro_src1[i]-appro_src2[i];
                break;
            case MUL:
                dest_data[i] = appro_src1[i]*appro_src2[i];
                break;
            case MAD:
                dest_data[i] = appro_src1[i]*appro_src2[i] + appro_src3[i];
                break;
            case DIV:
                dest_data[i] = appro_src1[i]/appro_src2[i];
                break;
            default: return false; break;
        }
    }
    return true;
}

bool core_t::check_R(const int op, const double src1_data[], const double src2_data[], const double src3_data[], const double appro_src1[], const double appro_src2[], const double appro_src3[]){
    if(op == ADD || op == SUB || op == MUL || op == DIV || op == MAD){
        float src1_r = compute_R(src1_data, appro_src1);
        float src2_r = compute_R(src2_data, appro_src2);
        if( src1_r< 0.9 && src1_r > -0.9 )
            return false;
        if( src2_r< 0.9 && src2_r > -0.9 )
            return false;
        if(op == MAD){
            float src3_r = compute_R(src3_data, appro_src3);
            if( src3_r< 0.9 && src3_r > -0.9 )
                return false;
        }
        return true;
    }
    return false;
}

float core_t::compute_R(const double ob_values_f[], const double pred_values_f[]){

   float ret_r=0;
   double sum_ob = 0, sum_pred =0, sum_ob_pred =0, sum_ob_sqr =0, sum_pred_sqr =0;

   for(unsigned i = 0 ; i < m_warp_size ; i++ ){
      sum_ob      +=    ob_values_f[i];
      sum_pred    +=    pred_values_f[i];
      sum_ob_pred +=    ob_values_f[i]   * pred_values_f[i];
      sum_ob_sqr  +=    ob_values_f[i]   * ob_values_f[i];
      sum_pred_sqr +=   pred_values_f[i] * pred_values_f[i];
   }

   double denominator = sqrt( m_warp_size * sum_ob_sqr - sum_ob * sum_ob ) * sqrt( m_warp_size * sum_pred_sqr - sum_pred * sum_pred ) ;
   double numerator = m_warp_size * sum_ob_pred - sum_ob * sum_pred ;

   if(denominator == 0 && numerator == 0){
      ret_r = 1.0;
   }
   else ret_r = numerator / denominator;

   if( isnan(ret_r) || isinf(ret_r))
      ret_r =0;

   return ret_r;
}

void core_t::appro_src_sel_exe_appro_out_f(warp_inst_t &inst, unsigned warpId){
    const unsigned warp_size_f = m_warp_size;
    const ptx_instruction *perWarp_pI[warp_size_f];
    int opcode_warp[warp_size_f];

    double src1_data[warp_size_f],
           src2_data[warp_size_f],
           src3_data[warp_size_f],
           dest_data[warp_size_f];

    operand_info dst_warp[warp_size_f];
    unsigned i_type[warp_size_f];

    get_src_t(inst, warpId, perWarp_pI, opcode_warp, i_type, dst_warp, src1_data, src2_data, src3_data);

    //got src data, need R, then decide to conitnue or not
    double appro_src1[warp_size_f],
           appro_src2[warp_size_f],
           appro_src3[warp_size_f];
    get_appro_src(src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3);

    //if it is predictable
    const int opcode = opcode_warp[0];
    if( is_predictable_src( opcode, src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3) == false)
        return;

    //compute select lanes through ALU and get dest data
    get_dest_sel_exe(opcode_warp, src1_data, src2_data, src3_data, dest_data);
    
    //compute appro dest
    double dest_appro_data[warp_size_f];
    get_appro_dest_sel_exe(dest_data, dest_appro_data);
    
    //commit selected executed appro dest
   commit_warp_dest(inst, warpId, i_type, dest_appro_data, dst_warp, perWarp_pI);

   return ;
}

bool core_t::is_predictable_src(const int op, const double src1_data[], const double src2_data[], const double src3_data[], const double appro_src1[], const double appro_src2[], const double appro_src3[]){
    return check_R(op, src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3);
}

void core_t::get_src_t(const warp_inst_t &inst,
        unsigned warpId, const ptx_instruction *perWarp_pI[],
        int opcode_warp[], unsigned i_type[], operand_info dst_warp[],
        double src1_data[], double src2_data[], double src3_data[]
        ){

    const unsigned warp_size_f = m_warp_size;
    ptx_reg_t src1_t[warp_size_f],
              src2_t[warp_size_f],
              src3_t[warp_size_f],
              dest_t[warp_size_f];

    for ( unsigned t=0; t < m_warp_size; t++ ) {
        if( inst.active(t) ) {
            if(warpId==(unsigned (-1)))
                warpId = inst.warp_id();
            unsigned tid=m_warp_size*warpId+t;

            //get stored *pI
            perWarp_pI[t] = m_thread[tid]->appro_per_thread_info.get_pI();
            opcode_warp[t] = perWarp_pI[t]->get_opcode();

            if(opcode_warp[t] == ADD || opcode_warp[t] == SUB ||opcode_warp[t] == MUL ||opcode_warp[t] == DIV || opcode_warp[t] == MAD){
                i_type[t] = perWarp_pI[t]->get_type();
                dst_warp[t] = perWarp_pI[t]->dst();

                src1_t[t] = m_thread[tid]->get_operand_value(
                        perWarp_pI[t]->src1(),
                        dst_warp[t],
                        i_type[t],
                        m_thread[tid], 1
                        );

                switch(i_type[t]){
                    case F32_TYPE:
                        src1_data[t] = (double)src1_t[t].f32;
                        break;
                    case F64_TYPE:
                    case FF64_TYPE:
                        src1_data[t] = (double)src1_t[t].f64;
                        break;
                    default: return; //not floating op
                }

                src2_t[t] = m_thread[tid]->get_operand_value(
                        perWarp_pI[t]->src2(),
                        dst_warp[t],
                        i_type[t],
                        m_thread[tid], 1
                        );
                switch(i_type[t]){
                    case F32_TYPE:
                        src2_data[t] = (double)src2_t[t].f32;
                        break;
                    case F64_TYPE:
                    case FF64_TYPE:
                        src2_data[t] = (double)src2_t[t].f64;
                        break;
                    default: return; //not floating op
                }

                //MAD
                if(opcode_warp[t] == MAD){
                    src3_t[t] = m_thread[tid]->get_operand_value(
                            perWarp_pI[t]->src3(),
                            dst_warp[t],
                            i_type[t],
                            m_thread[tid], 1
                            );
                    switch(i_type[t]){
                        case F32_TYPE:
                            src3_data[t] = (double)src3_t[t].f32;
                            break;
                        case F64_TYPE:
                        case FF64_TYPE:
                            src3_data[t] = (double)src3_t[t].f64;
                            break;
                        default: return; //not floating op
                    }
                }
            }
            else
                return ;
            //not one of ASMD operations, don't commit
        }
    }//iterate over the threads of warp, and get the operands.
    return ;
}

void core_t::get_dest_sel_exe(const int op_warp[], const double src1_data[], const double src2_data[], const double src3_data[], double dest_data[]){

    const unsigned warp_size = m_warp_size;
    for(unsigned i=0; i < warp_size; i++){
        switch(op_warp[i]){
            case ADD:
                dest_data[i] = src1_data[i]+src2_data[i];
                break;
            case SUB:
                dest_data[i] = src1_data[i]-src2_data[i];
                break;
            case MUL:
                dest_data[i] = src1_data[i]*src2_data[i];
                break;
            case MAD:
                dest_data[i] =src1_data[i]*src2_data[i]+src3_data[i];
                break;
            case DIV:
                dest_data[i] = src1_data[i]/src2_data[i];
                break;
            default: return ; break;
        }
    }
    return;
}

void core_t::get_appro_dest_sel_exe(const double dest_data[], double appro_dest[]){
    //compute appro form head and tail
    const unsigned warp_size = m_warp_size;
    double dest_diff;
    dest_diff = (dest_data[warp_size-1] - dest_data[0])/(warp_size-1);
    for(unsigned i =0 ;i <warp_size; i++){
        appro_dest[i] = dest_data[0] + dest_diff * i;
    }

    return ;
}




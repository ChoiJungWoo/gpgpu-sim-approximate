#include"abstract_hardware_model.h"
#include"global_steve.h"
#include"./cuda-sim/ptx_ir.h"
#include<math.h>
#include<cmath>

//entry function
void core_t::appro_computing_entry(warp_inst_t &inst, unsigned warpId, unsigned mode, bool isSat){
    //steve appro
    if(isSat == false)
        return;

    steve_glb_sp::glb_appro_stat.one_warp_computing();

    switch(mode){
        case NO_APPRO:
            return;
            break;
        case APPRO_OPERANDS_COMP_ALL:
            appro_src_all_exe_f(inst, warpId);
            break;
        case APPRO_OPERANDS_COMP_SELECT:
            appro_src_sel_exe_appro_out_f(inst, warpId);
            break;
        default:
            assert(0);
            break;
    }
    //else, not staturate warp, skip
    return ;
}

//*****************************
//appro: v1
//*****************************
void core_t::appro_src_all_exe_f(warp_inst_t &inst, unsigned warpId){
    steve_glb_sp::glb_appro_stat.one_pre_appro_computing();

    const unsigned warp_size_f = m_warp_size;
    const ptx_instruction *perWarp_pI[warp_size_f];
    int opcode_warp[warp_size_f];

    double src1_data [m_warp_size],
           src2_data [m_warp_size],
           src3_data [m_warp_size],
           dest_data [m_warp_size];

    operand_info dst_warp[m_warp_size];
    unsigned i_type[warp_size_f];

    //get_src
    if( get_src_t(inst, warpId, perWarp_pI, opcode_warp, i_type, dst_warp,
                src1_data, src2_data, src3_data) == false){
        return;
    }

    //check if the opcodes are identical
    int opcode = checkOpcodes(opcode_warp);
    if(opcode == -1){
        return;
    }

    //compute the approximate src values
    if( get_dest_appro_src_exe_all_f( opcode_warp[0],
                src1_data, src2_data, src3_data, dest_data, warp_size_f ) == false ){
        return ;
    }

    //commit all
    commit_warp_dest(inst, warpId, i_type, dest_data, dst_warp, perWarp_pI);

    return;
}

//*****************************************************
//compute R for src values,
//select head lane and tail to execute instructions,
//approximate the output values
//*****************************************************
void core_t::appro_src_sel_exe_appro_out_f(warp_inst_t &inst, unsigned warpId){
    steve_glb_sp::glb_appro_stat.one_pre_appro_computing();

    const unsigned warp_size_f = m_warp_size;
    const ptx_instruction *perWarp_pI[warp_size_f];
    int opcode_warp[warp_size_f];

    double src1_data [m_warp_size],
           src2_data [m_warp_size],
           src3_data [m_warp_size],
           dest_data [m_warp_size];

    //got src data, need R, then decide to conitnue or not
    double appro_src1 [m_warp_size],
           appro_src2 [m_warp_size],
           appro_src3 [m_warp_size];

    operand_info dst_warp[m_warp_size];
    unsigned i_type[warp_size_f];

    double dest_appro_data [m_warp_size];

    if( get_src_t(inst, warpId, perWarp_pI, opcode_warp, i_type, dst_warp, src1_data, src2_data, src3_data) == false){
        return;
    }

    //check if all opcodes in a warp is identical, if not dont do approximate computing
    int opcode = opcode_warp[0];
    opcode = checkOpcodes(opcode_warp);
    if(opcode == -1){
        return;
    }

    get_appro_src(src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3);

    //if it is predictable
    if( is_predictable_src( opcode, src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3) == false){
        return;
    }

    //compute select lanes through ALU and get dest data
    get_dest_sel_exe(opcode_warp, src1_data, src2_data, src3_data, dest_data);

    //compute appro dest
    get_appro_dest_sel_exe(dest_data, dest_appro_data);

    //commit selected executed appro dest
    commit_warp_dest(inst, warpId, i_type, dest_appro_data, dst_warp, perWarp_pI);

    return ;
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
                default:
                    assert(0);
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

//******************
//
//******************
bool core_t::get_dest_appro_src_exe_all_f( const int op,const double src1_data[], const double src2_data[], const double src3_data[], double dest_data[],  const unsigned warp_size){

    double appro_src1[warp_size],
           appro_src2[warp_size],
           appro_src3[warp_size];

    //get appro_src
    get_appro_src(src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3);

    //check the corellational coefficient R,
    //if R < threshold, reject the approximate values
    if(check_R(op, src1_data, src2_data, src3_data,
                appro_src1, appro_src2, appro_src3) == false){
        return false;
    }

    //approximate instruction
    for(unsigned i=0; i < warp_size; i++){
        switch(op){
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
            default: return false;
                     break;
        }
    }
    return true;
}

bool core_t::check_R(const int op, const double src1_data[], const double src2_data[], const double src3_data[], const double appro_src1[], const double appro_src2[], const double appro_src3[]){

    float src1_r = compute_R(src1_data, appro_src1);
    float src2_r = compute_R(src2_data, appro_src2);
    bool isValid_R = true;

    if( std::abs(src1_r) <= steve_glb_sp::appro_R ){
        isValid_R = false;
        steve_glb_sp::glb_appro_stat.record_R(src1_r, 1, false);
    }else{
        steve_glb_sp::glb_appro_stat.record_R(src1_r, 1, true);
    }

    if( std::abs(src2_r) <= steve_glb_sp::appro_R ){
        isValid_R = false;
        steve_glb_sp::glb_appro_stat.record_R(src2_r, 2, false);
    }else{
        steve_glb_sp::glb_appro_stat.record_R(src2_r, 2, true);
    }

    if(op == MAD){
        float src3_r = compute_R(src3_data, appro_src3);
        if( std::abs(src3_r) <= steve_glb_sp::appro_R ){
            isValid_R = false;
            steve_glb_sp::glb_appro_stat.record_R(src3_r, 3, false);
        }else{
            steve_glb_sp::glb_appro_stat.record_R(src3_r, 3, true);
        }
    }

    if(isValid_R){
        steve_glb_sp::glb_appro_stat.one_appro_computing();
    }
    return isValid_R;
}

float core_t::compute_R(const double ob_values_f[], const double pred_values_f[]){

    float ret_r=0;
    long double sum_ob = 0, sum_pred =0, sum_ob_pred =0, sum_ob_sqr =0, sum_pred_sqr =0;

    for(unsigned i=0;i<m_warp_size;i++){
        if( isnan(ob_values_f[i]) || isinf(ob_values_f[i]) ){
            return 0.0f;
        }
    }

    for(unsigned i = 0 ; i < m_warp_size ; i++ ){
       sum_ob       +=   ob_values_f[i];
       sum_pred     +=   pred_values_f[i];
       sum_ob_pred  +=   ob_values_f[i]   * pred_values_f[i];
       sum_ob_sqr   +=   ob_values_f[i]   * ob_values_f[i];
       sum_pred_sqr +=   pred_values_f[i] * pred_values_f[i];
    }

    long double denominator = sqrt( m_warp_size * sum_ob_sqr - sum_ob * sum_ob ) * sqrt( m_warp_size * sum_pred_sqr - sum_pred * sum_pred ) ;
    long double numerator = m_warp_size * sum_ob_pred - sum_ob * sum_pred ;

    if( isnan(denominator) ){
        return 0.0f;
    }

    if(denominator == 0 && numerator == 0){
       ret_r = 1.0f;
    }
    else if( denominator == 0){
        ret_r = 1.0f;
    }
    else ret_r = (float)(numerator / denominator);

    if( isnan(ret_r) || isinf(ret_r))
       ret_r = 0.0f;

    return ret_r;
}


bool core_t::is_predictable_src(const int op,
        const double src1_data[], const double src2_data[], const double src3_data[],
        const double appro_src1[], const double appro_src2[], const double appro_src3[]){

    return check_R(op, src1_data, src2_data, src3_data, appro_src1, appro_src2, appro_src3);
}

bool core_t::get_src_t(const warp_inst_t &inst,
        unsigned warpId, const ptx_instruction *perWarp_pI[],
        int opcode_warp[], unsigned i_type[], operand_info dst_warp[],
        double src1_data[], double src2_data[], double src3_data[]
        ){

    ptx_reg_t *src1_t = new ptx_reg_t[m_warp_size],
              *src2_t = new ptx_reg_t[m_warp_size],
              *src3_t = new ptx_reg_t[m_warp_size];

    for ( unsigned t=0; t < m_warp_size; t++ ) {
        if( inst.active(t) ) {
            if(warpId==(unsigned (-1)))
                warpId = inst.warp_id();
            unsigned tid=m_warp_size*warpId+t;

            //get stored *pI
            perWarp_pI[t] = m_thread[tid]->appro_per_thread_info.get_pI();
            opcode_warp[t] = perWarp_pI[t]->get_opcode();

            if(opcode_warp[t] == ADD || opcode_warp[t] == SUB ||opcode_warp[t] == MUL
                    //||opcode_warp[t] == DIV
                    || opcode_warp[t] == MAD){

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
                        src1_data[t] = src1_t[t].f64;
                        break;
                    default:
                        return false; //not floating op
                        break;
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
                    default:
                        return false; //not floating op
                        break;
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
                        default:
                            return false; //not floating op
                            break;
                    }
                }
            }
            else
                return false;
            //not one of ASMD operations, don't return src values
        }
    }//iterate over the threads of warp, and get the operands.
    return true;
}

void core_t::get_dest_sel_exe(const int op_warp[], const double src1_data[], const double src2_data[], const double src3_data[], double dest_data[]){

    for(unsigned i=0; i < m_warp_size; i++){
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
            default:
                printf("opcode [%d]\n",op_warp[i]);
                //assert(0);
                return ;
                break;
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

int core_t::checkOpcodes(const int op_warp[]){
    for(unsigned i = 0; i < m_warp_size; i++){
        if(op_warp[i] != op_warp[0]){
            return -1;
        }
    }

    if(op_warp[0] == ADD || op_warp[0] == SUB ||
        op_warp[0] == MUL || op_warp[0] == MAD){
        return op_warp[0];
    }
    else{
        return -1;
    }

    return op_warp[0];
}

#include"appro_stat_sim.h"
#include<cmath>

appro_stat_sim::appro_stat_sim(){
    num_appro_computing = 0;
    num_warp_computing = 0;
}

void appro_stat_sim::print_num_appro_comp(FILE *out){
    fprintf(out, "=====================\n");

    fprintf(out, "\tnum of approximate computing: (%lu/%lu = %f)\n", num_appro_computing, num_warp_computing, (float)num_appro_computing/num_warp_computing);
    fprintf(out, "\tnum of pre-approximate computing: (%lu/%lu = %f)\n", num_pre_appro_computing, num_warp_computing, (float)num_pre_appro_computing/num_warp_computing);
    fprintf(out, "\taverage R(effective): sr1 (%f) src2(%f) src3(%f)\n", get_avg_src1_R(), get_avg_src2_R(), get_avg_src3_R());
    fprintf(out, "\taverage R(overall): sr1 (%f) src2(%f) src3(%f)\n", get_avg_src1_R_tot(), get_avg_src2_R_tot(), get_avg_src3_R_tot());
}

void appro_stat_sim::one_appro_computing(){
    num_appro_computing++;
}

void appro_stat_sim::one_warp_computing(){
    num_warp_computing++;
}

void appro_stat_sim::record_R(float R, const int src, const bool valid){
    R = std::abs(R);
    if(valid == true){
        switch(src){
            case (1):
                sum_src1_R+=R;
                num_src1_R++;
                break;
            case (2):
                sum_src2_R+=R;
                num_src2_R++;
                break;
            case (3):
                sum_src3_R+=R;
                num_src3_R++;
                break;
            default:
                break;
        }
    }
    else{
    switch(src){
        case (1):
            sum_src1_R_tot+=R;
            num_src1_R_tot++;
            break;
        case (2):
            sum_src2_R_tot+=R;
            num_src2_R_tot++;
            break;
        case (3):
            sum_src3_R_tot+=R;
            num_src3_R_tot++;
            break;
        default:
            break;
    }
    }

    return;
}

double appro_stat_sim::get_avg_src1_R(){
    return sum_src1_R/num_src1_R;
}
double appro_stat_sim::get_avg_src2_R(){
    return sum_src2_R/num_src2_R;
}
double appro_stat_sim::get_avg_src3_R(){
    return sum_src3_R/num_src3_R;
}

double appro_stat_sim::get_avg_src1_R_tot(){
    return sum_src1_R_tot/num_src1_R_tot;
}
double appro_stat_sim::get_avg_src2_R_tot(){
    return sum_src2_R_tot/num_src2_R_tot;
}
double appro_stat_sim::get_avg_src3_R_tot(){
    return sum_src3_R_tot/num_src3_R_tot;
}

void appro_stat_sim::one_pre_appro_computing(){
    num_pre_appro_computing++;
}


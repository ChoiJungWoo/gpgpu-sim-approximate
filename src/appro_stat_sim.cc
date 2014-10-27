#include"appro_stat_sim.h"

appro_stat_sim::appro_stat_sim(){
    sum_R = 0;
    avg_R = 0;
    sum_effctive_R = 0;
    avg_effctive_R = 0;
    num_appro_computing = 0;
}

void appro_stat_sim::one_appro_computing(){
    num_appro_computing++;
}

void appro_stat_sim::print_num_appro_comp(FILE *out){
    fprintf(out, "### num of approximate computing: (%lu/%lu = %f)\n", num_appro_computing, num_warp_computing, (float)num_appro_computing/num_warp_computing);
    fprintf(out, "### num of pre-approximate computing: (%lu/%lu = %f)\n", num_pre_appro_computing, num_warp_computing, (float)num_pre_appro_computing/num_warp_computing);
}

void appro_stat_sim::one_warp_computing(){
    num_warp_computing++;
}

void appro_stat_sim::record_R(float R){
    sum_R += R;
}

double appro_stat_sim::get_sum_R(){
    return sum_R;
}

double appro_stat_sim::get_avg_R(){
    return sum_R/num_appro_computing;
}

void appro_stat_sim::one_pre_appro_computing(){
    num_pre_appro_computing++;
}


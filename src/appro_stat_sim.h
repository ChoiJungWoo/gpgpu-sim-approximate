#ifndef __APPRO_STAT_SIM_H__
#define __APPRO_STAT_SIM_H__
#include<stdio.h>

class appro_stat_sim{
    public:
        appro_stat_sim();
        void one_warp_computing();
        void one_appro_computing();
        void one_pre_appro_computing();

        void record_R(float R, const int src, const bool valid);

        double get_avg_src1_R();
        double get_avg_src2_R();
        double get_avg_src3_R();
        double get_avg_src1_R_tot();
        double get_avg_src2_R_tot();
        double get_avg_src3_R_tot();
        //unsigned get_num_appro_computing();

        //void print_sum_R(FILE *out);
        //void print_avg_R(FILE *out);
        void print_num_appro_comp(FILE *out);

    private:
        double sum_src1_R;
        double sum_src2_R;
        double sum_src3_R;

        double num_src1_R;
        double num_src2_R;
        double num_src3_R;

        double sum_src1_R_tot;
        double sum_src2_R_tot;
        double sum_src3_R_tot;

        double num_src1_R_tot;
        double num_src2_R_tot;
        double num_src3_R_tot;

        unsigned long num_appro_computing;
        unsigned long num_pre_appro_computing;
        unsigned long num_warp_computing;
};

#endif

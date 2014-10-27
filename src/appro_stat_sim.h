#ifndef __APPRO_STAT_SIM_H__
#define __APPRO_STAT_SIM_H__
#include<stdio.h>

class appro_stat_sim{
    public:
        appro_stat_sim();
        void one_warp_computing();
        void one_appro_computing();
        void one_pre_appro_computing();

        void record_R(float R);
        double get_sum_R();
        double get_avg_R();
        //unsigned get_num_appro_computing();

        //void print_sum_R(FILE *out);
        //void print_avg_R(FILE *out);
        void print_num_appro_comp(FILE *out);

    private:
        double sum_R;
        double avg_R;
        double sum_effctive_R;
        double avg_effctive_R;
        unsigned long num_appro_computing;
        unsigned long num_pre_appro_computing;
        unsigned long num_warp_computing;
};

#endif

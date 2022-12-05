//
// Created by 王泽远 on 2022/5/20.
//

#ifndef PHY_SIM_PBD_H
#define PHY_SIM_PBD_H

class simulator
{
public:
    void simulate(){
        if(paused) return;
        for(int step = 0;step<num_sub_steps;++step){
            //for object in scene
            //  pre_solve(sdt)
            //for object in scene
            // solve(sdt)
            //for object in scene
            //post_solve(sdt)
        }
    };
private:
    double dt;
    unsigned int num_sub_steps;
    double sdt;
    bool paused;
};

#endif //PHY_SIM_PBD_H

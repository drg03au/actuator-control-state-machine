#include <stdio.h>

typedef enum{
    IDLE,
    RUN,
    FAULT
}State;

typedef struct{
    State state;
    double position_cmd;
    double position_meas;  //measured position
    int fault_sensor_missing;
    int fault_overcurrent;
}ActuatorSystem;

static const char* state_to_string(State s){
    switch (s){
        case IDLE: return "IDLE";
        case RUN: return "RUN";
        case FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}

static void inject_faults(ActuatorSystem *sys,int step){
    if(step == 200)
        sys->fault_sensor_missing = 1;
    if(step ==350)
        sys->fault_overcurrent = 1;
}

static void actuator_step(ActuatorSystem *sys){
    sys->position_meas += 0.08 * (sys->position_cmd -sys->position_meas);
}

static void system_update(ActuatorSystem *sys){
    switch (sys->state){
        case IDLE:
            if(!sys->fault_sensor_missing && !sys->fault_overcurrent){
                sys->state = RUN;
            }else{
                sys->state = FAULT;
            }
            break;

        case RUN:
            actuator_step(sys);

            if(sys->fault_sensor_missing || sys->fault_overcurrent){
                sys->state = FAULT;
            }
            break;

        case FAULT:
            
            sys->position_cmd = 0.0;

            //actuator returning to safe position
            actuator_step(sys);
            break;
    }
}


int main(void){
    ActuatorSystem sys = {0};
    sys.state = IDLE;
    sys.position_cmd = 1.0;
    sys.position_meas = 0.0;

    const int total_steps = 500;

    for(int step=0; step<total_steps; ++step){
        inject_faults(&sys, step);
        system_update(&sys);

        if(step%10==0){
            printf("step=%d, state=%s, cmd=%.2f, meas=%.2f, sensor_missing=%d, overcurrent=%d\n",
                step,
                state_to_string(sys.state),
                sys.position_cmd,
                sys.position_meas,
                sys.fault_sensor_missing,
                sys.fault_overcurrent);
        }
    }
    return 0;
}
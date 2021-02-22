#include "kernel_stats.h"

void set_early_stop_(int *number){
  early_stop = true;
  stop_in = *number;
}

double get_current_time() {
    struct timeval tp;
    struct timezone tzp;
    gettimeofday(&tp,&tzp);
    return ((double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

void print_timestep(uint8_t type, double collected_time) {
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  switch(type) {
    case PRINT_INIT:
      printf("[MO833] Init time,%i,%f\n", rank, collected_time - init_time);
      break;
    case PRINT_STATS:
      printf("[MO833] Paramount Iteration,%i,%i,%f,%f\n", rank, current_iteration, collected_time - init_time, pi);
      break;
    case PRINT_EXIT:
      printf("[MO833] Total time,%f\n", collected_time - init_time);
      break;
    case PRINT_AVG:
      printf("[MO833] PI avg,%i,%f,%d\n", rank, pi_sum/current_iteration, current_iteration);
      break;
    case PRINT_BETA:
      printf("[MO833] Beta,%i,%f\n", rank, (collected_time - end_time)/pi_sum);
  }
}

void init_timestep_() {
  current_iteration = 0;
  init_time = get_current_time();
}

void end_timestep_() {
  end_time = get_current_time();

  //print_timestep(PRINT_STATS, end_time);
  //pi_sum += end_time - begin_time;

  //my_exit();

  //if(early_stop && current_iteration == stop_in)
  //  MPI_Abort(MPI_COMM_WORLD, 0);
}

void begin_timestep_() {
  double old_begin_time = begin_time;
  begin_time = get_current_time();

  if(current_iteration == 0) {
    double current_time = get_current_time();
    print_timestep(PRINT_INIT, current_time);

    if(stop_in == 0)
      my_exit();
  }else {
    pi = end_time - old_begin_time;
    pi += begin_time - end_time;
    pi_sum += pi;
    print_timestep(PRINT_STATS, begin_time);
  }

  my_exit();

  current_iteration++;
}

int get_iteration_() {
  return current_iteration;
}

void exit_timestep_() {
  int rank;
  double current_time = get_current_time();

  if(current_iteration > 0) {
    print_timestep(PRINT_AVG, 0);
    print_timestep(PRINT_BETA, current_time);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if(rank == 0)
    print_timestep(PRINT_EXIT, current_time);
}

void my_exit() {
  if(early_stop && current_iteration == stop_in) {
    exit_timestep_();
    MPI_Abort(MPI_COMM_WORLD, 0);
  }
}

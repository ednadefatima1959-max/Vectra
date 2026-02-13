#include "rmr_hw_detect.h"
#include "rmr_math_fabric.h"
#include <stdio.h>

int main(void){
  RmR_HW_Info hw;
  RmR_MathFabricPlan plan;
  u32 points[RMR_MATH_POINTS];
  u32 domains[RMR_MATH_DOMAINS];
  u32 checksum = 0u;

  RmR_HW_Detect(&hw);
  RmR_MathFabric_AutodetectPlan(&hw, &plan);

  for (u32 i = 0; i < RMR_MATH_POINTS; ++i) {
    points[i] = ((i + 1u) * 0x11111111u) ^ plan.matrix_seed;
  }
  RmR_MathFabric_VectorMix(&plan, points, domains);

  for (u32 d = 0; d < RMR_MATH_DOMAINS; ++d) {
    checksum ^= domains[d] + (d << 8);
  }

  printf("math_fabric_selftest arch=%u lanes=%u stride=%u seed=%u checksum=%u\n",
         plan.arch_code,
         plan.lane_count,
         plan.pin_stride,
         plan.matrix_seed,
         checksum);

  return (checksum != 0u && plan.matrix[0][0] != 0u && plan.matrix[7][8] != 0u) ? 0 : 1;
}

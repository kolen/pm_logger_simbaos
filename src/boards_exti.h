// https://github.com/eerimoq/simba/commit/f6160d3ff947a3bef5a0b3878d80eef1b6a61eaa
// Not in current version

#ifndef exti_d1_dev

#ifdef BOARD_NODEMCU
#define exti_d1_dev exti_device[5]
#define exti_d2_dev exti_device[4]
#define exti_d3_dev exti_device[0]
#define exti_d4_dev exti_device[2]
#define exti_d5_dev exti_device[14]
#define exti_d6_dev exti_device[12]
#define exti_d7_dev exti_device[13]
#define exti_d8_dev exti_device[15]
#define exti_d9_dev exti_device[3]
#define exti_d10_dev exti_device[1]
#endif

#ifdef BOARD_WEMOS_D1_MINI
#define exti_d1_dev exti_device[5]
#define exti_d2_dev exti_device[4]
#define exti_d3_dev exti_device[0]
#define exti_d4_dev exti_device[2]
#define exti_d5_dev exti_device[14]
#define exti_d6_dev exti_device[12]
#define exti_d7_dev exti_device[13]
#define exti_d8_dev exti_device[15]
#endif

#if !defined(BOARD_NODEMCU) && !defined(BOARD_WEMOS_D1_MINI)
#error Board not supported
#endif

#endif

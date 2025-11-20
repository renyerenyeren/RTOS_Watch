//
// Created by redmiX on 2025/11/20.
//

#ifndef RTOS_PROJECT_MPU6050_DRIVER_H
#define RTOS_PROJECT_MPU6050_DRIVER_H

//******************************** Includes *********************************//

//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//

//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
typedef enum
{
    MPU6050_OK             = 0,          /*Operation completed successfully   */
    MPU6050_ERROR          = 1,          /*Run-time error without case matched*/
    MPU6050_ERRORTIMEOUT   = 2,          /*Operation failed with timeout      */
    MPU6050_ERRORRESOURCE  = 3,          /*Resource not available             */
    MPU6050_ERRORPARAMETER = 4,          /*Parameter error                    */
    MPU6050_ERRORNOMEMORY  = 5,          /*Out of memory                      */
    MPU6050_ERRORISR       = 6,          /*Not allowed in ISR context         */
    MPU6050_RESERVED       = 0x7FFFFFFF, /*Reserved                           */
    
}mpu6050_status_t;
//******************************** Typedefs *********************************//






#endif //RTOS_PROJECT_MPU6050_DRIVER_H
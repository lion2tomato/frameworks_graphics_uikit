/**
 * @file lvx_camera_controller.h
 *
 */

#ifndef LVX_CAMERA_CONTROLLER_H
#define LVX_CAMERA_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <lvgl/lvgl.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t obj;
    lv_obj_t* video;
    lv_obj_t* take_picture_btn;
    lv_obj_t* picture_btn;
    lv_obj_t* recorder_btn;
} lvx_camera_controller_t;

extern const lv_obj_class_t lvx_camera_controller_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t* lvx_camera_controller_create(lv_obj_t* parent);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVX_CAMERA_CONTROLLER_H*/

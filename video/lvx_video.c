/**
 * @file lvx_video.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvx_video.h"

#ifdef CONFIG_LVX_USE_VIDEO_ADAPTER

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lvx_video_class
#define LVX_VIDEO_DEFAULT_PERIOD (30)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lvx_video_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_video_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj);
static void lvx_video_event(const lv_obj_class_t* class_p, lv_event_t* e);
static void video_frame_task_cb(lv_timer_t* t);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lvx_video_class = {
    .constructor_cb = lvx_video_constructor,
    .destructor_cb = lvx_video_destructor,
    .event_cb = lvx_video_event,
    .instance_size = sizeof(lvx_video_t),
    .base_class = &lv_img_class
};

static lvx_video_vtable_t* g_video_default_vtable = NULL;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lvx_video_vtable_set_default(lvx_video_vtable_t* vtable)
{
    LV_ASSERT_NULL(vtable);

    g_video_default_vtable = vtable;
}

lvx_video_vtable_t* lvx_video_vtable_get_default(void)
{
    return g_video_default_vtable;
}

lv_obj_t* lvx_video_create(lv_obj_t* parent)
{
    lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);

    return obj;
}

void lvx_video_set_src(lv_obj_t* obj, const char* src)
{
    lvx_video_set_src_opt(obj, src, NULL);
}

void lvx_video_set_src_opt(lv_obj_t* obj, const char* src, const char* option)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    if ((video_obj->video_ctx = video_obj->vtable->video_adapter_open(video_obj->vtable, src, option)) != NULL) {
        video_obj->duration = video_obj->vtable->video_adapter_get_dur(video_obj->vtable, video_obj->video_ctx);
    }
}

int lvx_video_set_event_callback(lv_obj_t* obj, void* cookie, media_event_callback event_callback)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_set_event_callback(video_obj->vtable, video_obj->video_ctx, cookie, event_callback);
}

void lvx_video_set_timer_period(lv_obj_t* obj, uint32_t peroid)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    lv_timer_set_period(video_obj->timer, peroid);
}

void lvx_video_set_vtable(lv_obj_t* obj, lvx_video_vtable_t* vtable)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    LV_ASSERT_NULL(vtable);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    video_obj->vtable = vtable;
}

int lvx_video_start(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    int ret;

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    if ((ret = video_obj->vtable->video_adapter_start(video_obj->vtable, video_obj->video_ctx)) == 0) {
        lv_timer_resume(video_obj->timer);
        lv_timer_reset(video_obj->timer);
    }

    return ret;
}

int lvx_video_stop(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    int ret;

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    lv_img_cache_invalidate_src(&video_obj->img_dsc);

    if ((ret = video_obj->vtable->video_adapter_stop(video_obj->vtable, video_obj->video_ctx)) == 0) {
        lv_timer_pause(video_obj->timer);
    }

    return ret;
}

int lvx_video_seek(lv_obj_t* obj, int pos)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_seek(video_obj->vtable, video_obj->video_ctx, pos);
}

int lvx_video_pause(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    int ret;

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    if ((ret = video_obj->vtable->video_adapter_pause(video_obj->vtable, video_obj->video_ctx)) == 0) {
        lv_timer_pause(video_obj->timer);
    }

    return ret;
}

int lvx_video_resume(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    int ret;

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    if ((ret = video_obj->vtable->video_adapter_resume(video_obj->vtable, video_obj->video_ctx)) == 0) {
        lv_timer_resume(video_obj->timer);
    }

    return ret;
}

int lvx_video_get_dur(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_get_dur(video_obj->vtable, video_obj->video_ctx);
}

int lvx_video_set_loop(lv_obj_t* obj, int loop)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_loop(video_obj->vtable, video_obj->video_ctx, loop);
}

void lvx_video_set_poster(lv_obj_t* obj, const char* poster_path)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    lv_img_set_src(&video_obj->img.obj, poster_path);
}

bool lvx_video_is_playing(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    int pos = video_obj->vtable->video_adapter_get_player_state(video_obj->vtable, video_obj->video_ctx);

    return pos ? true : false;
}

int lvx_video_write_data(lv_obj_t* obj, void* data, size_t len)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->vtable->video_adapter_write_data(video_obj->vtable, video_obj->video_ctx, data, len);
}

lv_img_dsc_t* lvx_video_get_img_dsc(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return &video_obj->img_dsc;
}

lv_event_code_t lvx_video_get_custom_event_id(lv_obj_t* obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    return video_obj->custom_event_id;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lvx_video_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    video_obj->vtable = g_video_default_vtable;

    LV_ASSERT_NULL(video_obj->vtable);

    video_obj->timer = lv_timer_create(video_frame_task_cb, LVX_VIDEO_DEFAULT_PERIOD, obj);
    lv_timer_pause(video_obj->timer);

    video_obj->custom_event_id = lv_event_register_id();
}

static void lvx_video_destructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
    LV_UNUSED(class_p);

    lvx_video_t* video_obj = (lvx_video_t*)obj;

    lv_img_cache_invalidate_src(&video_obj->img_dsc);

    video_obj->vtable->video_adapter_close(video_obj->vtable, video_obj->video_ctx);

    lv_timer_del(video_obj->timer);
}

static void lvx_video_set_crop(lvx_video_t* video_obj)
{
    int crop_width = video_obj->img_dsc.header.w - (video_obj->crop_coords.x1 + video_obj->crop_coords.x2);
    int crop_height = video_obj->img_dsc.header.h - (video_obj->crop_coords.y1 + video_obj->crop_coords.y2);
    lv_obj_set_size(&video_obj->img.obj, crop_width, crop_height);
    lv_img_set_offset_x(&video_obj->img.obj, -video_obj->crop_coords.x1);
    lv_img_set_offset_y(&video_obj->img.obj, -video_obj->crop_coords.y1);
}

static void video_frame_task_cb(lv_timer_t* t)
{
    lv_obj_t* obj = t->user_data;
    lvx_video_t* video_obj = (lvx_video_t*)obj;
    int32_t last_frame_time = video_obj->cur_time;
    bool first_frame = video_obj->img_dsc.data == NULL ? true : false;

    if (video_obj->vtable->video_adapter_get_frame(video_obj->vtable, video_obj->video_ctx, video_obj) < 0) {
        return;
    }

    if (first_frame) {
        lv_img_set_src(&video_obj->img.obj, &video_obj->img_dsc);
        lvx_video_set_crop(video_obj);
        lv_event_send(obj, video_obj->custom_event_id, NULL);
    } else {
        lv_img_cache_invalidate_src(&video_obj->img_dsc);
        lv_obj_invalidate(obj);
    }

    if (video_obj->cur_time != last_frame_time) {
        lv_event_send((lv_obj_t*)video_obj, LV_EVENT_VALUE_CHANGED, NULL);
    }
}

static void lvx_video_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
    LV_UNUSED(class_p);

    lv_res_t res;

    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK)
        return;
}

#endif /* CONFIG_LVX_USE_VIDEO_ADAPTER */
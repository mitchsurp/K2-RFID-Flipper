#include "../k2_rfid_app.h"
#include "k2_scene.h"

static void k2_scene_emulate_worker_callback(K2WorkerEvent event, void* context) {
    K2RfidApp* app = context;
    if (event == K2WorkerEventEmulating) {
        view_dispatcher_send_custom_event(app->view_dispatcher, K2CustomEventEmulating);
    } else if (event == K2WorkerEventStopped) {
        view_dispatcher_send_custom_event(app->view_dispatcher, K2CustomEventStopped);
    }
}

void k2_scene_emulate_on_enter(void* context) {
    K2RfidApp* app = context;
    Popup* popup = app->popup;

    k2_rfid_app_sync_config(app);

    const K2Material* mat = k2_db_find_material_by_id(app->config.material_id);
    const char* mat_name = mat ? mat->name : "Custom";
    const char* col_name = k2_db_find_closest_color_name(app->config.color_hex);
    const char* weight = k2_db_find_weight_label_by_code(app->config.weight_code);

    FuriString* str = furi_string_alloc();
    furi_string_printf(
        str,
        "%s\n%s - %s\nHold against CFS slot\nPress Back to Stop",
        mat_name,
        col_name,
        weight);

    popup_reset(popup);
    popup_set_header(popup, "Emulating Spool", 64, 5, AlignCenter, AlignTop);
    popup_set_text(popup, furi_string_get_cstr(str), 64, 20, AlignCenter, AlignTop);
    furi_string_free(str);

    k2_worker_set_callback(app->worker, k2_scene_emulate_worker_callback, app);
    k2_worker_start_emulate(app->worker, &app->config);

    notification_message(app->notifications, &sequence_blink_yellow_10);
    view_dispatcher_switch_to_view(app->view_dispatcher, K2ViewPopup);
}

bool k2_scene_emulate_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void k2_scene_emulate_on_exit(void* context) {
    K2RfidApp* app = context;
    k2_worker_stop(app->worker);
    k2_worker_set_callback(app->worker, NULL, NULL);
    popup_reset(app->popup);
}

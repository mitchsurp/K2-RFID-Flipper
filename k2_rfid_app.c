#include "k2_rfid_app.h"
#include "scenes/k2_scene.h"
#include <furi_hal.h>

void k2_rfid_app_sync_config(K2RfidApp* app) {
    if (!app) return;

    /* Printer */
    const char* pr = k2_db_get_printer_name(app->printer_idx);
    strncpy(app->config.printer_model, pr ? pr : "K2", sizeof(app->config.printer_model) - 1);

    /* Material */
    const K2Material* mat = k2_db_get_material(app->material_idx);
    strncpy(app->config.material_id, mat ? mat->id : "01001", sizeof(app->config.material_id) - 1);

    /* Color */
    const K2ColorPreset* col = k2_db_get_color(app->color_idx);
    strncpy(app->config.color_hex, col ? col->hex : "FFFFFF", sizeof(app->config.color_hex) - 1);

    /* Weight */
    const K2WeightOption* w = k2_db_get_weight(app->weight_idx);
    strncpy(app->config.weight_code, w ? w->code : "0330", sizeof(app->config.weight_code) - 1);

    /* Serial */
    strncpy(app->config.serial, app->serial_str, sizeof(app->config.serial) - 1);

    /* Defaults */
    strncpy(app->config.batch, "A2", sizeof(app->config.batch) - 1);
    strncpy(app->config.vendor_id, "0276", sizeof(app->config.vendor_id) - 1);
    strncpy(app->config.date, "AB124", sizeof(app->config.date) - 1);
}

void k2_rfid_app_randomize_serial(K2RfidApp* app) {
    if (!app) return;
    uint32_t rand_val = furi_hal_random_get();
    uint32_t serial_num = (rand_val % 900000) + 100000;
    snprintf(app->serial_str, sizeof(app->serial_str), "%06lu", (unsigned long)serial_num);
    k2_rfid_app_sync_config(app);
}

static bool k2_rfid_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    K2RfidApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool k2_rfid_app_back_event_callback(void* context) {
    furi_assert(context);
    K2RfidApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void k2_rfid_app_tick_event_callback(void* context) {
    furi_assert(context);
    K2RfidApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static K2RfidApp* k2_rfid_app_alloc(void) {
    K2RfidApp* app = malloc(sizeof(K2RfidApp));
    memset(app, 0, sizeof(K2RfidApp));

    /* Initialize crypto and self test */
    if (!k2_crypto_self_test()) {
        FURI_LOG_E("K2App", "Crypto self-test failed!");
    }

    /* Initialize Spool defaults */
    app->printer_idx = 0;
    app->material_idx = 0;
    app->color_idx = 0;
    app->weight_idx = 0;
    strncpy(app->serial_str, "000001", sizeof(app->serial_str));
    k2_rfid_app_sync_config(app);

    /* Records */
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    /* View Dispatcher */
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&k2_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, k2_rfid_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, k2_rfid_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(app->view_dispatcher, k2_rfid_app_tick_event_callback, 100);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    /* Views */
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, K2ViewSubmenu, submenu_get_view(app->submenu));

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(app->view_dispatcher, K2ViewVariableItemList, variable_item_list_get_view(app->var_item_list));

    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, K2ViewPopup, popup_get_view(app->popup));

    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, K2ViewWidget, widget_get_view(app->widget));

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(app->view_dispatcher, K2ViewTextInput, text_input_get_view(app->text_input));

    /* NFC Worker */
    app->worker = k2_worker_alloc();

    return app;
}

static void k2_rfid_app_free(K2RfidApp* app) {
    if (!app) return;

    /* Free worker */
    k2_worker_free(app->worker);

    /* Remove and free views */
    view_dispatcher_remove_view(app->view_dispatcher, K2ViewSubmenu);
    submenu_free(app->submenu);

    view_dispatcher_remove_view(app->view_dispatcher, K2ViewVariableItemList);
    variable_item_list_free(app->var_item_list);

    view_dispatcher_remove_view(app->view_dispatcher, K2ViewPopup);
    popup_free(app->popup);

    view_dispatcher_remove_view(app->view_dispatcher, K2ViewWidget);
    widget_free(app->widget);

    view_dispatcher_remove_view(app->view_dispatcher, K2ViewTextInput);
    text_input_free(app->text_input);

    /* Free manager & dispatcher */
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    /* Close records */
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t k2_rfid_app(void* p) {
    UNUSED(p);
    K2RfidApp* app = k2_rfid_app_alloc();

    scene_manager_next_scene(app->scene_manager, K2SceneMainMenu);
    view_dispatcher_run(app->view_dispatcher);

    k2_rfid_app_free(app);
    return 0;
}

#include "../k2_rfid_app.h"
#include "k2_scene.h"

static void k2_scene_tag_info_btn_callback(GuiButtonType result, InputType type, void* context) {
    if (type != InputTypeShort) return;
    K2RfidApp* app = context;

    if (result == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, K2CustomEventTagInfoSave);
    } else if (result == GuiButtonTypeCenter) {
        view_dispatcher_send_custom_event(app->view_dispatcher, K2CustomEventTagInfoEmulate);
    } else if (result == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, K2CustomEventTagInfoEdit);
    }
}

void k2_scene_tag_info_on_enter(void* context) {
    K2RfidApp* app = context;
    Widget* widget = app->widget;

    widget_reset(widget);

    const K2SpoolInfo* info = &app->last_spool;
    const char* brand = info->material ? info->material->brand : "Unknown";
    const char* name = info->material ? info->material->name : "Custom Spool";
    const char* type = info->material ? info->material->type : "PLA";

    FuriString* str = furi_string_alloc();
    furi_string_printf(
        str,
        "%s %s\nType: %s | %s\nColor: #%s (%s)\nSize: %s\nUID: %02X%02X%02X%02X | SN: %s",
        brand,
        name,
        type,
        info->printer_model[0] ? info->printer_model : "K2",
        info->color_hex,
        info->color_name,
        info->weight_label,
        info->uid[0],
        info->uid[1],
        info->uid[2],
        info->uid[3],
        info->serial);

    widget_add_string_multiline_element(widget, 0, 0, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(str));
    furi_string_free(str);

    widget_add_button_element(widget, GuiButtonTypeLeft, "Save", k2_scene_tag_info_btn_callback, app);
    widget_add_button_element(widget, GuiButtonTypeCenter, "Emul", k2_scene_tag_info_btn_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "Edit", k2_scene_tag_info_btn_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, K2ViewWidget);
}

bool k2_scene_tag_info_on_event(void* context, SceneManagerEvent event) {
    K2RfidApp* app = context;
    bool consumed = false;

    if (event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if (event.event == K2CustomEventTagInfoSave) {
            /* Sync config to last_spool and save */
            strncpy(app->config.material_id, app->last_spool.material_id, sizeof(app->config.material_id));
            strncpy(app->config.color_hex, app->last_spool.color_hex, sizeof(app->config.color_hex));
            strncpy(app->config.weight_code, app->last_spool.length_code, sizeof(app->config.weight_code));
            strncpy(app->config.serial, app->last_spool.serial, sizeof(app->config.serial));
            strncpy(app->config.printer_model, app->last_spool.printer_model, sizeof(app->config.printer_model));
            scene_manager_next_scene(app->scene_manager, K2SceneSave);
        } else if (event.event == K2CustomEventTagInfoEmulate) {
            strncpy(app->config.material_id, app->last_spool.material_id, sizeof(app->config.material_id));
            strncpy(app->config.color_hex, app->last_spool.color_hex, sizeof(app->config.color_hex));
            strncpy(app->config.weight_code, app->last_spool.length_code, sizeof(app->config.weight_code));
            strncpy(app->config.serial, app->last_spool.serial, sizeof(app->config.serial));
            strncpy(app->config.printer_model, app->last_spool.printer_model, sizeof(app->config.printer_model));
            scene_manager_next_scene(app->scene_manager, K2SceneEmulate);
        } else if (event.event == K2CustomEventTagInfoEdit) {
            strncpy(app->config.material_id, app->last_spool.material_id, sizeof(app->config.material_id));
            strncpy(app->config.color_hex, app->last_spool.color_hex, sizeof(app->config.color_hex));
            strncpy(app->config.weight_code, app->last_spool.length_code, sizeof(app->config.weight_code));
            strncpy(app->config.serial, app->last_spool.serial, sizeof(app->config.serial));
            strncpy(app->config.printer_model, app->last_spool.printer_model, sizeof(app->config.printer_model));
            /* Find matching indices */
            for (size_t i = 0; i < k2_db_get_material_count(); i++) {
                if (strcmp(k2_db_get_material(i)->id, app->config.material_id) == 0) {
                    app->material_idx = i;
                    break;
                }
            }
            scene_manager_next_scene(app->scene_manager, K2SceneConfig);
        }
    }

    return consumed;
}

void k2_scene_tag_info_on_exit(void* context) {
    K2RfidApp* app = context;
    widget_reset(app->widget);
}

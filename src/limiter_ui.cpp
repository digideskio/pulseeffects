#include "limiter_ui.hpp"

LimiterUi::LimiterUi(BaseObjectType* cobject,
                     const Glib::RefPtr<Gtk::Builder>& refBuilder,
                     std::string settings_name)
    : Gtk::Grid(cobject), PluginUiBase(refBuilder, settings_name) {
    // loading glade widgets

    builder->get_widget("asc", asc);
    builder->get_widget("autovolume_enable", autovolume_enable);
    builder->get_widget("autovolume_controls", autovolume_controls);
    builder->get_widget("limiter_controls", limiter_controls);
    builder->get_widget("attenuation", attenuation);
    builder->get_widget("attenuation_label", attenuation_label);

    get_object("input_gain", input_gain);
    get_object("limit", limit);
    get_object("lookahead", lookahead);
    get_object("release", release);
    get_object("oversampling", oversampling);
    get_object("asc_level", asc_level);
    get_object("autovolume_window", autovolume_window);
    get_object("autovolume_target", autovolume_target);
    get_object("autovolume_tolerance", autovolume_tolerance);
    get_object("autovolume_threshold", autovolume_threshold);

    // gsettings bindings

    auto flag = Gio::SettingsBindFlags::SETTINGS_BIND_DEFAULT;
    auto flag_get = Gio::SettingsBindFlags::SETTINGS_BIND_GET;
    auto flag_invert_boolean =
        Gio::SettingsBindFlags::SETTINGS_BIND_INVERT_BOOLEAN;

    settings->bind("input-gain", input_gain, "value", flag);
    settings->bind("limit", limit, "value", flag);
    settings->bind("lookahead", lookahead, "value", flag);
    settings->bind("release", release, "value", flag);
    settings->bind("oversampling", oversampling, "value", flag);
    settings->bind("asc", asc, "active", flag);
    settings->bind("asc-level", asc_level, "value", flag);

    settings->bind("autovolume-state", autovolume_enable, "active", flag);
    settings->bind("autovolume-state", autovolume_controls, "sensitive",
                   flag_get);

    settings->bind("autovolume-state", limiter_controls, "sensitive",
                   flag_get | flag_invert_boolean);

    settings->bind("autovolume-window", autovolume_window, "value", flag);
    settings->bind("autovolume-target", autovolume_target, "value", flag);
    settings->bind("autovolume-tolerance", autovolume_tolerance, "value", flag);
    settings->bind("autovolume-threshold", autovolume_threshold, "value", flag);

    settings->signal_changed("autovolume-state").connect([&](auto key) {
        init_autovolume();
    });

    settings->signal_changed("autovolume-window").connect([&](auto key) {
        auto window = settings->get_double("autovolume-window");

        release->set_value(window);
    });

    settings->signal_changed("autovolume-target").connect([&](auto key) {
        auto target = settings->get_int("autovolume-target");
        auto tolerance = settings->get_int("autovolume-tolerance");

        limit->set_value(target + tolerance);
    });

    settings->signal_changed("autovolume-tolerance").connect([&](auto key) {
        auto target = settings->get_int("autovolume-target");
        auto tolerance = settings->get_int("autovolume-tolerance");

        limit->set_value(target + tolerance);
    });

    init_autovolume();
}

LimiterUi::~LimiterUi() {}

LimiterUi* LimiterUi::create(std::string settings_name) {
    auto builder = Gtk::Builder::create_from_resource(
        "/com/github/wwmm/pulseeffects/limiter.glade");

    LimiterUi* grid = nullptr;

    builder->get_widget_derived("widgets_grid", grid, settings_name);

    return grid;
}

void LimiterUi::init_autovolume() {
    auto enabled = settings->get_boolean("autovolume-state");

    if (enabled) {
        auto window = settings->get_double("autovolume-window");
        auto target = settings->get_int("autovolume-target");
        auto tolerance = settings->get_int("autovolume-tolerance");

        limit->set_value(target + tolerance);
        release->set_value(window);
        asc->set_active(true);
        asc_level->set_value(1.0);
        lookahead->set_value(10.0);  // 10 ms
    }
}

void LimiterUi::on_new_attenuation(double value) {
    attenuation->set_value(1 - value);

    attenuation_label->set_text(level_to_str(20 * log10(value)));
}
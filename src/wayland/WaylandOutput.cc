#include "WaylandOutput.hh"
#include <iostream>

const zxdg_output_v1_listener WaylandOutput::XDG_OUTPUT_LISTENER = {
    .logical_position = WaylandOutput::ON_XDG_OUT_POS,
    .logical_size     = WaylandOutput::ON_XDG_OUT_SIZE,
    .done             = WaylandOutput::ON_XDG_OUT_DONE_DUMMY,
    .name             = WaylandOutput::ON_XDG_OUT_STRING_DUMMY,
    .description      = WaylandOutput::ON_XDG_OUT_STRING_DUMMY,
};

const wl_output_listener WaylandOutput::WL_OUTPUT_LISTENER = {
    .geometry    = WaylandOutput::ON_WL_OUT_GEOMETRY,
    .mode        = WaylandOutput::ON_WL_OUT_MODE_DUMMY,
    .done        = WaylandOutput::ON_WL_OUT_DONE_DUMMY,
    .scale       = WaylandOutput::ON_WL_OUT_SCALE_DUMMY,
    .name        = WaylandOutput::ON_WL_OUT_NAME,
    .description = WaylandOutput::ON_WL_OUT_DESC_DUMMY,
};

WaylandOutput::WaylandOutput(WaylandManager *wm, uint32_t wl_name) {
    this->wm       = wm;
    this->wl_name  = wl_name;
    this->w_output = (wl_output *)
        wl_registry_bind(wm->registry, wl_name, &wl_output_interface, 4);
    this->x_output = zxdg_output_manager_v1_get_xdg_output(
        wm->output_manager,
        this->w_output
    );

    wl_output_add_listener(this->w_output, &WL_OUTPUT_LISTENER, this);
    zxdg_output_v1_add_listener(this->x_output, &XDG_OUTPUT_LISTENER, this);
}

WaylandOutput::~WaylandOutput() {
    zxdg_output_v1_destroy(x_output);
    wl_output_destroy(w_output);
}

bool WaylandOutput::valid() const {
    return name.has_value() && posValid && sizeValid;
}

const std::string &WaylandOutput::getName() const {
    if (!valid())
        throw std::logic_error("The compositor hasn't sent us the data yet.");

    return this->name.value();
}

Rect<int32_t> WaylandOutput::getBounds() const {
    if (!valid())
        throw std::logic_error("The compositor hasn't sent us the data yet.");

    return {x, y, w, h};
}

uint32_t WaylandOutput::getWlName() const { return this->wl_name; }

void WaylandOutput::notify_manager_if_valid() {
    if (wasValid) return; // prevent notifying the manager more than once
    if (!valid()) return;
    wasValid = true;
    this->wm->inputReady(this->wl_name);
}

void WaylandOutput::ON_XDG_OUT_POS(
    void *data,
    zxdg_output_v1 *,
    int32_t x,
    int32_t y
) {
    auto that = reinterpret_cast<WaylandOutput *>(data);

    that->x = x;
    that->y = y;

    that->posValid = true;
    that->notify_manager_if_valid();
}

void WaylandOutput::ON_XDG_OUT_SIZE(
    void *data,
    zxdg_output_v1 *,
    int32_t w,
    int32_t h
) {
    auto that = reinterpret_cast<WaylandOutput *>(data);

    that->w = w;
    that->h = h;

    that->sizeValid = true;
    that->notify_manager_if_valid();
}

void WaylandOutput::ON_WL_OUT_NAME(void *data, wl_output *, const char *name) {
    auto that = reinterpret_cast<WaylandOutput *>(data);

    that->name = std::string(name);
    that->notify_manager_if_valid();
}

void WaylandOutput::ON_WL_OUT_GEOMETRY(
    void *,
    wl_output *,
    int32_t,
    int32_t,
    int32_t,
    int32_t,
    int32_t,
    const char *,
    const char *,
    int32_t
) {}

void WaylandOutput::ON_WL_OUT_DONE_DUMMY(void *, wl_output *) {}

void WaylandOutput::ON_WL_OUT_MODE_DUMMY(
    void *,
    wl_output *,
    uint32_t,
    int32_t,
    int32_t,
    int32_t
) {}

void WaylandOutput::ON_WL_OUT_SCALE_DUMMY(void *, wl_output *, int32_t) {}

void WaylandOutput::ON_WL_OUT_DESC_DUMMY(void *, wl_output *, const char *) {}

void WaylandOutput::
    ON_XDG_OUT_STRING_DUMMY(void *, zxdg_output_v1 *, const char *) {}

void WaylandOutput::ON_XDG_OUT_DONE_DUMMY(void *, zxdg_output_v1 *) {}

#include "munin/solid_frame.hpp"
#include "munin/detail/unicode_glyphs.hpp"
#include "munin/compass_layout.hpp"
#include "munin/filled_box.hpp"
#include "munin/render_surface.hpp"
#include "munin/view.hpp"

namespace munin {
namespace {

constexpr terminalpp::glyph const default_corner_element          = '+';
constexpr terminalpp::glyph const default_horizontal_beam_element = '-';
constexpr terminalpp::glyph const default_vertical_beam_element   = '|';
    
// ==========================================================================
// TOP_LEFT_CORNER_GLYPH
// ==========================================================================
terminalpp::element select_top_left_corner_element(
    render_surface const &surface,
    terminalpp::attribute const &attr)
{
    return {surface.supports_unicode()
         ? detail::single_lined_rounded_top_left_corner
         : default_corner_element, attr};
}

// ==========================================================================
// TOP_RIGHT_CORNER_GLYPH
// ==========================================================================
terminalpp::element select_top_right_corner_element(
    render_surface const &surface,
    terminalpp::attribute const &attr)
{
    return {surface.supports_unicode()
         ? detail::single_lined_rounded_top_right_corner
         : default_corner_element, attr};
}

// ==========================================================================
// BOTTOM_LEFT_CORNER_GLYPH
// ==========================================================================
terminalpp::element select_bottom_left_corner_element(
    render_surface const &surface,
    terminalpp::attribute const &attr)
{
    return {surface.supports_unicode()
         ? detail::single_lined_rounded_bottom_left_corner
         : default_corner_element, attr};
}

// ==========================================================================
// BOTTOM_RIGHT_CORNER_GLYPH
// ==========================================================================
terminalpp::element select_bottom_right_corner_element(
    render_surface const &surface,
    terminalpp::attribute const &attr)
{
    return {surface.supports_unicode()
         ? detail::single_lined_rounded_bottom_right_corner
         : default_corner_element, attr};
}

// ==========================================================================
// HORIZONTAL_BEAM_GLYPH
// ==========================================================================
terminalpp::element select_horizontal_beam_element(
    render_surface const &surface,
    terminalpp::attribute const &attr)
{
    return {surface.supports_unicode()
         ? detail::single_lined_horizontal_beam
         : default_horizontal_beam_element, attr};
}

// ==========================================================================
// VERTICAL_BEAM_GLYPH
// ==========================================================================
terminalpp::element select_vertical_beam_element(
    render_surface const &surface,
    terminalpp::attribute const &attr)
{
    return {surface.supports_unicode()
         ? detail::single_lined_vertical_beam
         : default_vertical_beam_element, attr};
}

}

struct solid_frame::impl
{
    terminalpp::attribute lowlight_attribute;
    terminalpp::attribute highlight_attribute = {
        terminalpp::ansi::graphics::colour::cyan,
        terminalpp::colour(),
        terminalpp::ansi::graphics::intensity::bold};
    terminalpp::attribute *current_attribute = &lowlight_attribute;

    std::shared_ptr<munin::filled_box> top_left = munin::make_fill(
        [this](render_surface &surface)
        {
            return select_top_left_corner_element(surface, *current_attribute);
        });
            
    std::shared_ptr<munin::filled_box> top_centre = munin::make_fill(
        [this](render_surface &surface)
        {
            return select_horizontal_beam_element(surface, *current_attribute);
        });
        
    std::shared_ptr<munin::filled_box> top_right = munin::make_fill(
        [this](render_surface &surface)
        {
            return select_top_right_corner_element(surface, *current_attribute);
        });
        
    std::shared_ptr<munin::filled_box> centre_left = munin::make_fill(
        [this](render_surface &surface)
        {
            return select_vertical_beam_element(surface, *current_attribute);
        });
        
    std::shared_ptr<munin::filled_box> centre_right = munin::make_fill(
        [this](render_surface &surface)
        {
            return select_vertical_beam_element(surface, *current_attribute);
        });
        
    std::shared_ptr<munin::filled_box> bottom_left = munin::make_fill(
        [this](render_surface &surface)
        {
            return select_bottom_left_corner_element(surface, *current_attribute);
        });
        
    std::shared_ptr<munin::filled_box> bottom_centre = munin::make_fill(
        [this](render_surface &surface)
        {
            return select_horizontal_beam_element(surface, *current_attribute);
        });
        
    std::shared_ptr<munin::filled_box> bottom_right = munin::make_fill(
        [this](render_surface &surface)
        {
            return select_bottom_right_corner_element(surface, *current_attribute);
        });
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
solid_frame::solid_frame()
  : pimpl_(std::make_shared<impl>())
{
    auto north_beam = view(
        make_compass_layout(),
        pimpl_->top_left,   compass_layout::heading::west,
        pimpl_->top_centre, compass_layout::heading::centre,
        pimpl_->top_right,  compass_layout::heading::east);
    
    auto south_beam = view(
        make_compass_layout(),
        pimpl_->bottom_left, compass_layout::heading::west,
        pimpl_->bottom_centre, compass_layout::heading::centre,
        pimpl_->bottom_right, compass_layout::heading::east);
        
    auto west_beam = pimpl_->centre_left;
    auto east_beam = pimpl_->centre_right;
    
    set_layout(make_compass_layout());
    
    add_component(north_beam, compass_layout::heading::north);
    add_component(south_beam, compass_layout::heading::south);
    add_component(west_beam, compass_layout::heading::west);
    add_component(east_beam, compass_layout::heading::east);
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
solid_frame::~solid_frame()
{
}

// ==========================================================================
// SET_HIGHLIGHT_ATTRIBUTE
// ==========================================================================
void solid_frame::set_highlight_attribute(
    terminalpp::attribute const &highlight_attribute)
{
    pimpl_->highlight_attribute = highlight_attribute;
}

// ==========================================================================
// SET_LOWLIGHT_ATTRIBUTE
// ==========================================================================
void solid_frame::set_lowlight_attribute(
    terminalpp::attribute const &lowlight_attribute)
{
    pimpl_->lowlight_attribute = lowlight_attribute;
}

// ==========================================================================
// HIGHLIGHT_ON_FOCUS
// ==========================================================================
void solid_frame::highlight_on_focus(
    std::shared_ptr<component> const &associated_component)
{
    auto const evaluate_focus = 
        [this, wp = std::weak_ptr<component>(associated_component)]
        {
            std::shared_ptr<component> associated_component(wp.lock());

            if (associated_component)
            {
                pimpl_->current_attribute = associated_component->has_focus()
                                          ? &pimpl_->highlight_attribute
                                          : &pimpl_->lowlight_attribute;
            }
        };

    associated_component->on_focus_set.connect(evaluate_focus);
    associated_component->on_focus_lost.connect(evaluate_focus);
    evaluate_focus();
}

// ==========================================================================
// MAKE_SOLID_FRAME
// ==========================================================================
std::shared_ptr<solid_frame> make_solid_frame()
{
    return std::make_shared<solid_frame>();
}

// ==========================================================================
// MAKE_SOLID_FRAME
// ==========================================================================
std::shared_ptr<solid_frame> make_solid_frame(
    std::shared_ptr<component> const &associated_component)
{
    auto frame = make_solid_frame();
    frame->highlight_on_focus(associated_component);
    return frame;
}

}

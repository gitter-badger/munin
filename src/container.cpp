#include "munin/container.hpp"
#include "munin/layout.hpp"
#include "munin/null_layout.hpp"
#include "munin/render_surface.hpp"
#include "munin/detail/algorithm.hpp"
#include "munin/detail/json_adaptors.hpp"
#include <terminalpp/ansi/mouse.hpp>
#include <terminalpp/rectangle.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/scope_exit.hpp>
#include <vector>

namespace munin {

namespace {

using component_connections =
    std::vector<boost::signals2::connection>;

template <class ForwardRange>
auto find_first_focussed_component(const ForwardRange &rng)
{
    auto const &component_has_focus = 
        [](auto const &comp)
        {
            return comp->has_focus();
        };

    return boost::find_if(rng, component_has_focus);
}

template <class ForwardRange, class IncrementFunction>
auto increment_focus(
    const ForwardRange &rng,
    IncrementFunction &&increment)
{
    return boost::find_if(rng, std::forward<IncrementFunction>(increment));
}

template <class ForwardRange>
auto find_component_at_point(
    const ForwardRange &rng,
    terminalpp::point const &location)
{
    auto const &has_location_at_point =
        [&location](auto const &comp)
        {
            auto const &position = comp->get_position();
            auto const &size     = comp->get_size();

            // Check to see if the reported position is within the component's
            // bounds.
            return (location.x >= position.x
                 && location.x  < position.x + size.width
                 && location.y >= position.y
                 && location.y < position.y + size.height);
        };

    return boost::find_if(rng, has_location_at_point);
}

}

// ==========================================================================
// CONTAINER::IMPLEMENTATION STRUCTURE
// ==========================================================================
struct container::impl
{
    // ======================================================================
    // CONSTRUCTOR
    // ======================================================================
    impl(container &self)
      : self_(self)
    {
    }

    // ======================================================================
    // LAYOUT_CONTAINER
    // ======================================================================
    void layout_container()
    {
        (*layout_)(components_, hints_, bounds_.size);
    }

    // ======================================================================
    // GET_PREFERRED_SIZE
    // ======================================================================
    terminalpp::extent get_preferred_size() const
    {
        return layout_->get_preferred_size(components_, hints_);
    }

    // ======================================================================
    // DRAW_COMPONENTS
    // ======================================================================
    void draw_components(
        render_surface &surface, terminalpp::rectangle const &region) const
    {
        for (auto const &comp : components_)
        {
            draw_component(comp, surface, region);
        }
    }

    // ======================================================================
    // DRAW_COMPONENT
    // ======================================================================
    void draw_component(
        std::shared_ptr<component> const &comp,
        render_surface &surface,
        terminalpp::rectangle const &region) const
    {
        auto const component_region = terminalpp::rectangle {
            comp->get_position(),
            comp->get_size()
        };

        auto draw_region = detail::intersection(component_region, region);

        if (draw_region)
        {
            // The draw region is currently relative to this container's
            // origin.  It should be relative to the child's origin.
            draw_region->origin -= component_region.origin;

            // The canvas must have an offset applied to it so that the
            // inner component can pretend that it is being drawn with its
            // container being at position (0,0).
            surface.offset_by({
                component_region.origin.x,
                component_region.origin.y
            });

            // Ensure that the offset is unapplied before exit of this
            // function.
            BOOST_SCOPE_EXIT_ALL(&surface, &component_region)
            {
                surface.offset_by({
                    -component_region.origin.x,
                    -component_region.origin.y
                });
            };

            comp->draw(surface, draw_region.get());
        }
    }

    // ======================================================================
    // SUBCOMPONENT_REDRAW_HANDLER
    // ======================================================================
    void subcomponent_redraw_handler(
        std::weak_ptr<component>           weak_subcomponent,
        std::vector<terminalpp::rectangle> regions)
    {
        auto subcomponent = weak_subcomponent.lock();

        if (subcomponent != NULL)
        {
            // Each region is bound to the origin of the component in question.
            // It must be rebound to the origin of the container.  We do this
            // by offsetting the regions' origins by the origin of the
            // subcomponent within this container.
            auto origin = subcomponent->get_position();

            for (auto &rect : regions)
            {
                rect.origin.x += origin.x;
                rect.origin.y += origin.y;
            }

            // This new information must be passed up the component heirarchy.
            self_.on_redraw(regions);
        }
    }

    // ======================================================================
    // SUBCOMPONENT_FOCUS_SET_HANDLER
    // ======================================================================
    void subcomponent_focus_set_handler(
        std::weak_ptr<component> const &weak_comp)
    {
        if (!in_focus_operation_)
        {
            auto const &another_component_has_focus = 
                [orig = weak_comp.lock()](auto const &comp)
                {
                    return comp != orig && comp->has_focus();
                };

            auto comp = 
                boost::find_if(components_, another_component_has_focus);

            if (comp != components_.end())
            {
                in_focus_operation_ = true;

                BOOST_SCOPE_EXIT_ALL(this)
                {
                    in_focus_operation_ = false;
                };

                (*comp)->lose_focus();
            }
            else
            {
                has_focus_ = true;
                self_.on_focus_set();
            }
        }
    }

    // ======================================================================
    // SUBCOMPONENT_FOCUS_LOST_HANDLER
    // ======================================================================
    void subcomponent_focus_lost_handler()
    {
        if (!in_focus_operation_)
        {
            has_focus_ = false;
            self_.on_focus_lost();
        }
    }

    // ======================================================================
    // SUBCOMPONENT_CURSOR_STATE_CHANGE_HANDLER
    // ======================================================================
    void subcomponent_cursor_state_change_handler(
        std::weak_ptr<component> weak_subcomponent)
    {
        auto subcomponent = weak_subcomponent.lock();

        if (subcomponent && subcomponent->has_focus())
        {
            self_.on_cursor_state_changed();
        }
    }

    // ======================================================================
    // SUBCOMPONENT_CURSOR_POSITION_CHANGE_HANDLER
    // ======================================================================
    void subcomponent_cursor_position_change_handler(
        std::weak_ptr<component> weak_subcomponent)
    {
        auto subcomponent = weak_subcomponent.lock();

        if (subcomponent && subcomponent->has_focus())
        {
            self_.on_cursor_position_changed();
        }
    }

    // ======================================================================
    // DO_EVENT
    // ======================================================================
    void do_event(boost::any const &event)
    {
        // We split incoming events into two types:
        // * Common events (e.g. keypressed, etc.) are passed on to the
        //   subcomponent with focus.
        // * Mouse events are passed on to the subcomponent at the location
        //   of the event, and the co-ordinates of the event are passed on
        //   relative to the subcomponent's location.
        auto const *report = boost::any_cast<terminalpp::ansi::mouse::report>(&event);

        if (report == nullptr)
        {
            handle_common_event(event);
        }
        else
        {
            handle_mouse_event(*report);
        }
    }

    // ======================================================================
    // HANDLE_COMMON_EVENT
    // ======================================================================
    void handle_common_event(boost::any const &event)
    {
        auto comp = find_first_focussed_component(components_);

        if (comp != components_.end())
        {
            (*comp)->event(event);
        }
    }

    // ======================================================================
    // HANDLE_MOUSE_EVENT
    // ======================================================================
    void handle_mouse_event(terminalpp::ansi::mouse::report const &report)
    {
        auto const &comp = find_component_at_point(
            components_,
            terminalpp::point(report.x_position_, report.y_position_));

        if (comp != components_.end())
        {
            auto const &position = (*comp)->get_position();

            (*comp)->event(
                terminalpp::ansi::mouse::report {
                    report.button_,
                    report.x_position_ - position.x,
                    report.y_position_ - position.y
                });
        }
    }

    container                               &self_;
    terminalpp::rectangle                    bounds_;
    std::unique_ptr<munin::layout>           layout_ = make_null_layout();
    std::vector<std::shared_ptr<component>>  components_;
    std::vector<boost::any>                  hints_;
    std::vector<component_connections>       component_connections_;
    bool                                     has_focus_ = false;
    bool                                     in_focus_operation_ = false;
};

// ==========================================================================
// CONSTRUCTOR
// ==========================================================================
container::container()
{
    pimpl_ = std::make_shared<impl>(std::ref(*this));
}

// ==========================================================================
// DESTRUCTOR
// ==========================================================================
container::~container()
{
}

// ==========================================================================
// SET_LAYOUT
// ==========================================================================
void container::set_layout(std::unique_ptr<munin::layout> &&lyt)
{
    pimpl_->layout_ = lyt.get() == nullptr 
                    ? make_null_layout() 
                    : std::move(lyt);
    pimpl_->layout_container();
}

// ==========================================================================
// ADD_COMPONENT
// ==========================================================================
void container::add_component(
    std::shared_ptr<component> const &comp
  , boost::any                 const &layout_hint)
{
    component_connections cnx;

    cnx.push_back(comp->on_focus_set.connect(
        [this, wcomp = std::weak_ptr<component>(comp)]
        {
            pimpl_->subcomponent_focus_set_handler(wcomp);
        }));

    cnx.push_back(comp->on_focus_lost.connect(
        [this]
        {
            pimpl_->subcomponent_focus_lost_handler();
        }));

    cnx.push_back(comp->on_cursor_state_changed.connect(
        [this, wcomp = std::weak_ptr<component>(comp)]
        {
            pimpl_->subcomponent_cursor_state_change_handler(wcomp);
        }));

    cnx.push_back(comp->on_cursor_position_changed.connect(
        [this, wcomp = std::weak_ptr<component>(comp)]
        {
            pimpl_->subcomponent_cursor_position_change_handler(wcomp);
        }));

    cnx.push_back(comp->on_redraw.connect(
        [this, wcomp = std::weak_ptr<component>(comp)](
            auto const &redraw_regions)
        {
            pimpl_->subcomponent_redraw_handler(wcomp, redraw_regions);
        }));

    pimpl_->components_.push_back(comp);
    pimpl_->hints_.push_back(layout_hint);
    pimpl_->component_connections_.push_back(cnx);
    pimpl_->layout_container();
    on_preferred_size_changed();
}

// ==========================================================================
// REMOVE_COMPONENT
// ==========================================================================
void container::remove_component(std::shared_ptr<component> const &comp)
{
    for (auto index = 0; index < pimpl_->components_.size(); ++index)
    {
        if (pimpl_->components_[index] == comp)
        {
            pimpl_->components_.erase(pimpl_->components_.begin() + index);
            pimpl_->hints_.erase(pimpl_->hints_.begin() + index);

            std::for_each(
                pimpl_->component_connections_[index].begin(),
                pimpl_->component_connections_[index].end(),
                [](auto &cnx)
                {
                    cnx.disconnect();
                });

            pimpl_->component_connections_.erase(
                pimpl_->component_connections_.begin() + index);
        }
    }

    pimpl_->layout_container();
    on_preferred_size_changed();
}

// ==========================================================================
// DO_SET_POSITION
// ==========================================================================
void container::do_set_position(terminalpp::point const &position)
{
    pimpl_->bounds_.origin = position;
}

// ==========================================================================
// DO_GET_POSITION
// ==========================================================================
terminalpp::point container::do_get_position() const
{
    return pimpl_->bounds_.origin;
}

// ==========================================================================
// DO_SET_SIZE
// ==========================================================================
void container::do_set_size(terminalpp::extent const &size)
{
    pimpl_->bounds_.size = size;
    pimpl_->layout_container();
}

// ==========================================================================
// DO_GET_SIZE
// ==========================================================================
terminalpp::extent container::do_get_size() const
{
    return pimpl_->bounds_.size;
}

// ==========================================================================
// DO_GET_PREFERRED_SIZE
// ==========================================================================
terminalpp::extent container::do_get_preferred_size() const
{
    return pimpl_->get_preferred_size();
}

// ==========================================================================
// DO_HAS_FOCUS
// ==========================================================================
bool container::do_has_focus() const
{
    return pimpl_->has_focus_;
}

// ==========================================================================
// DO_SET_FOCUS
// ==========================================================================
void container::do_set_focus()
{
    pimpl_->in_focus_operation_ = true;

    BOOST_SCOPE_EXIT_ALL(this)
    {
        pimpl_->in_focus_operation_ = false;
    };

    if (!pimpl_->has_focus_)
    {
        auto const &set_component_focus = 
            [](auto const &comp)
            {
                comp->set_focus();
                return comp->has_focus();
            };

        auto const &focussed_component = 
            increment_focus(pimpl_->components_, set_component_focus);

        pimpl_->has_focus_ = focussed_component != pimpl_->components_.end();

        if (pimpl_->has_focus_)
        {
            on_focus_set();
            on_cursor_state_changed();
            on_cursor_position_changed();
        }
    }
}

// ==========================================================================
// DO_LOSE_FOCUS
// ==========================================================================
void container::do_lose_focus()
{
    pimpl_->in_focus_operation_ = true;

    BOOST_SCOPE_EXIT_ALL(this)
    {
        pimpl_->in_focus_operation_ = false;
    };

    
    auto focussed_component = 
        find_first_focussed_component(pimpl_->components_);

    if (focussed_component != pimpl_->components_.end())
    {
        (*focussed_component)->lose_focus();
        pimpl_->has_focus_ = false;
        on_focus_lost();
        on_cursor_state_changed();
        on_cursor_position_changed();
    }
}

// ==========================================================================
// DO_FOCUS_NEXT
// ==========================================================================
void container::do_focus_next()
{
    pimpl_->in_focus_operation_ = true;

    BOOST_SCOPE_EXIT_ALL(this)
    {
        pimpl_->in_focus_operation_ = false;
    };

    bool const had_focus = pimpl_->has_focus_;

    auto const &focus_next_component =
        [](auto const &comp)
        {
            comp->focus_next();
            return comp->has_focus();
        };

    auto const &first_focussed_component = 
        find_first_focussed_component(pimpl_->components_);
    
    // Since we are focussing the next component, we want to look at components
    // from the first focussed component if there was one, otherwise the
    // first subcomponent.
    auto const &increment_from =
        first_focussed_component == pimpl_->components_.cend()
      ? pimpl_->components_.cbegin()
      : first_focussed_component;

    auto const &next_focussed_component = 
        increment_focus(
            boost::make_iterator_range(
                increment_from, pimpl_->components_.cend()),
            focus_next_component);

    pimpl_->has_focus_ = next_focussed_component != pimpl_->components_.end();

    // Announce a change in focus if that changed.
    if (had_focus != pimpl_->has_focus_)
    {
        if (pimpl_->has_focus_)
        {
            on_focus_set();
        }
        else
        {
            on_focus_lost();
        }

        on_cursor_position_changed();
        on_cursor_state_changed();
    }

    // If we had focus continuously, but the focussed subcomponent changed,
    // then we also want to announce cursor changes, since even though the
    // position and state of the cursor in the individual subcomponents hasn't
    // changed (and therefore they have no reason to send such a signal),
    // we know that the cursor may have moved about due to focus.
    if (had_focus 
     && pimpl_->has_focus_ 
     && increment_from != next_focussed_component)
    {
        on_cursor_position_changed();
        on_cursor_state_changed();
    }
}

// ==========================================================================
// DO_FOCUS_PREVIOUS
// ==========================================================================
void container::do_focus_previous()
{
    using boost::adaptors::reversed;
    using std::cbegin;
    using std::cend;
    
    pimpl_->in_focus_operation_ = true;

    BOOST_SCOPE_EXIT_ALL(this)
    {
        pimpl_->in_focus_operation_ = false;
    };

    bool const had_focus = pimpl_->has_focus_;

    auto const &focus_previous_component =
        [](auto const &comp)
        {
            comp->focus_previous();
            return comp->has_focus();
        };

    auto const &reversed_components = pimpl_->components_ | reversed;

    auto const &first_focussed_component = 
        find_first_focussed_component(reversed_components);
    
    // Since we are focussing the next component, we want to look at components
    // from the first focussed component if there was one, otherwise the
    // first subcomponent.
    auto const &increment_from =
        first_focussed_component == cend(reversed_components)
      ? cbegin(reversed_components)
      : first_focussed_component;

    auto const &previous_focussed_component = 
        increment_focus(
            boost::make_iterator_range(
                increment_from, cend(reversed_components)),
            focus_previous_component);

    pimpl_->has_focus_ = 
        previous_focussed_component != cend(reversed_components);

    // Announce a change in focus if that changed.
    if (had_focus != pimpl_->has_focus_)
    {
        if (pimpl_->has_focus_)
        {
            on_focus_set();
        }
        else
        {
            on_focus_lost();
        }

        on_cursor_position_changed();
        on_cursor_state_changed();
    }

    // If we had focus continuously, but the focussed subcomponent changed,
    // then we also want to announce cursor changes, since even though the
    // position and state of the cursor in the individual subcomponents hasn't
    // changed (and therefore they have no reason to send such a signal),
    // we know that the cursor may have moved about due to focus.
    if (had_focus 
     && pimpl_->has_focus_ 
     && increment_from != previous_focussed_component)
    {
        on_cursor_position_changed();
        on_cursor_state_changed();
    }
}

// ==========================================================================
// DO_GET_CURSOR_STATE
// ==========================================================================
bool container::do_get_cursor_state() const
{
    auto comp = find_first_focussed_component(pimpl_->components_);

    return comp == pimpl_->components_.end()
         ? false
         : (*comp)->get_cursor_state();
}

// ==========================================================================
// DO_GET_CURSOR_POSITION
// ==========================================================================
terminalpp::point container::do_get_cursor_position() const
{
    auto comp = find_first_focussed_component(pimpl_->components_);

    return comp == pimpl_->components_.end()
         ? terminalpp::point{}
         : (*comp)->get_position() + (*comp)->get_cursor_position();
}

// ==========================================================================
// DO_SET_CURSOR_POSITION
// ==========================================================================
void container::do_set_cursor_position(terminalpp::point const &position)
{
    // Note: Setting the cursor position on a container doesn't really
    // make too much sense, but an implementation is required to fulfil the
    // component interface.  Our default implementation sets the relative
    // cursor position in the focussed component.
    auto comp = find_first_focussed_component(pimpl_->components_);

    if (comp != pimpl_->components_.end())
    {
        (*comp)->set_cursor_position(position - (*comp)->get_position());
    }
}

// ==========================================================================
// DO_DRAW
// ==========================================================================
void container::do_draw(
    render_surface &surface, terminalpp::rectangle const &region) const
{
    pimpl_->draw_components(surface, region);
}

// ==========================================================================
// DO_EVENT
// ==========================================================================
void container::do_event(boost::any const &event)
{
    pimpl_->do_event(event);
}

// ==========================================================================
// MAKE_CONTAINER
// ==========================================================================
std::shared_ptr<container> make_container()
{
    return std::make_shared<container>();
}

// ==========================================================================
// DO_TO_JSON
// ==========================================================================
nlohmann::json container::do_to_json() const
{
    nlohmann::json json = {
        { "type",            "container" },
        { "position",        detail::to_json(get_position()) },
        { "size",            detail::to_json(get_size()) },
        { "preferred_size",  detail::to_json(get_preferred_size()) },
        { "has_focus",       has_focus() },
        { "cursor_state",    get_cursor_state() },
        { "cursor_position", detail::to_json(get_cursor_position()) },
    };

    auto &subcomponents = json["subcomponents"];

    for (auto index = size_t{0}; index < pimpl_->components_.size(); ++index)
    {
        subcomponents[index] = pimpl_->components_[index]->to_json();
    }

    return json;
}

}


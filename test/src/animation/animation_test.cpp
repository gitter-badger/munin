#include <munin/basic_component.hpp>
#include <munin/render_surface.hpp>
#include <terminalpp/algorithm/for_each_in_region.hpp>
#include "mock/animation_timer.hpp"
#include <gtest/gtest.h>
#include <chrono>

using namespace std::literals;
using testing::Return;
using testing::_;

namespace {

constexpr auto frame_change_time = 3ms;
constexpr auto before_element = terminalpp::element('<');
constexpr auto after_element  = terminalpp::element('>');

class animated_component
  : public munin::basic_component
{
private:
    //* =====================================================================
    /// \brief Called by get_preferred_size().  Derived classes must override
    /// this function in order to get the size of the component in a custom
    /// manner.
    //* =====================================================================
    terminalpp::extent do_get_preferred_size() const override
    {
        return {3, 3};
    }

    //* =====================================================================
    /// \brief Called by draw().  Derived classes must override this function
    /// in order to draw onto the passed context.  A component must only draw
    /// the part of itself specified by the region.
    ///
    /// \param surface the surface on which the component should draw itself.
    /// \param region the region relative to this component's origin that
    /// should be drawn.
    //* =====================================================================
    void do_draw(
        munin::render_surface &surface,
        terminalpp::rectangle const &region) const override
    {
        auto const now = surface.now();

        if (!frame_0_time)
        {
            frame_0_time = now;
        }

        auto const fill = (now - *frame_0_time < frame_change_time)
                        ? before_element
                        : after_element;

        terminalpp::for_each_in_region(
            surface,
            region,
            [=](terminalpp::element &elem,
                terminalpp::coordinate_type column,
                terminalpp::coordinate_type row)
            {
                elem = fill;
            });
    }

    mutable boost::optional<std::chrono::steady_clock::time_point> frame_0_time;
};

class an_animated_component
  : public testing::Test
{
protected:
    an_animated_component()
    {
        component_->set_position({0, 0});
        component_->set_size({3, 3});
    }

    std::shared_ptr<animated_component> component_ = 
        std::make_shared<animated_component>();

    terminalpp::canvas canvas_{{3, 3}};
    mock_animation_timer animation_timer_;
    munin::render_surface surface_{canvas_, animation_timer_};
};

}

TEST_F(an_animated_component, draws_initial_frame_at_time_0)
{
    auto const time_now = std::chrono::steady_clock::now();
    ON_CALL(animation_timer_, now())
        .WillByDefault(Return(time_now));

    component_->draw(surface_, {{0, 0}, {3, 3}});

    ASSERT_EQ(before_element, surface_[0][0]);
    ASSERT_EQ(before_element, surface_[1][0]);
    ASSERT_EQ(before_element, surface_[2][0]);
    ASSERT_EQ(before_element, surface_[0][1]);
    ASSERT_EQ(before_element, surface_[1][1]);
    ASSERT_EQ(before_element, surface_[2][1]);
    ASSERT_EQ(before_element, surface_[0][2]);
    ASSERT_EQ(before_element, surface_[1][2]);
    ASSERT_EQ(before_element, surface_[2][2]);
}

TEST_F(an_animated_component, draws_initial_frame_before_change_time)
{
    auto const time_now = std::chrono::steady_clock::now();
    ON_CALL(animation_timer_, now())
        .WillByDefault(Return(time_now));

    component_->draw(surface_, {{0, 0}, {3, 3}});

    ON_CALL(animation_timer_, now())
        .WillByDefault(Return(time_now + (frame_change_time - 1ms)));

    component_->draw(surface_, {{0, 0}, {3, 3}});

    ASSERT_EQ(before_element, surface_[0][0]);
    ASSERT_EQ(before_element, surface_[1][0]);
    ASSERT_EQ(before_element, surface_[2][0]);
    ASSERT_EQ(before_element, surface_[0][1]);
    ASSERT_EQ(before_element, surface_[1][1]);
    ASSERT_EQ(before_element, surface_[2][1]);
    ASSERT_EQ(before_element, surface_[0][2]);
    ASSERT_EQ(before_element, surface_[1][2]);
    ASSERT_EQ(before_element, surface_[2][2]);
}

TEST_F(an_animated_component, draws_updated_frame_after_change_time)
{
    auto const time_now = std::chrono::steady_clock::now();
    ON_CALL(animation_timer_, now())
        .WillByDefault(Return(time_now));

    component_->draw(surface_, {{0, 0}, {3, 3}});

    ON_CALL(animation_timer_, now())
        .WillByDefault(Return(time_now + frame_change_time));

    component_->draw(surface_, {{0, 0}, {3, 3}});

    ASSERT_EQ(after_element, surface_[0][0]);
    ASSERT_EQ(after_element, surface_[1][0]);
    ASSERT_EQ(after_element, surface_[2][0]);
    ASSERT_EQ(after_element, surface_[0][1]);
    ASSERT_EQ(after_element, surface_[1][1]);
    ASSERT_EQ(after_element, surface_[2][1]);
    ASSERT_EQ(after_element, surface_[0][2]);
    ASSERT_EQ(after_element, surface_[1][2]);
    ASSERT_EQ(after_element, surface_[2][2]);
}

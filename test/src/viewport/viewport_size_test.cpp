#include "viewport_test.hpp"
#include <munin/render_surface.hpp>
#include <munin/viewport.hpp>
#include <terminalpp/canvas.hpp>
#include <gtest/gtest.h>

using testing::ValuesIn;
using testing::_;

namespace {
    
using viewport_size_test_data = std::tuple<
    terminalpp::extent,    // component's preferred size
    terminalpp::point,     // initial tracked component cursor position
    terminalpp::extent,    // changed viewport size
    std::initializer_list<terminalpp::point>, // Extra cursor movements
    terminalpp::point,     // expected viewport cursor position
    terminalpp::rectangle  // expected tracked draw area
>;

using viewport_size_test = 
    viewport_mock_test_with_data<viewport_size_test_data>;

}

TEST_P(viewport_size_test, viewports_track_size_changes)
{
    using std::get;
    using testing::Return;
    
    auto const &param = GetParam();
    auto const &tracked_component_preferred_size = get<0>(param);
    auto const &tracked_component_cursor_position = get<1>(param);
    auto const &changed_viewport_size = get<2>(param);
    auto const &extra_tracked_cursor_movements = get<3>(param);
    auto const &expected_viewport_cursor_position = get<4>(param);
    auto const &expected_tracked_draw_area = get<5>(param);
    
    // Set up the conditions of the tracked component
    ON_CALL(*tracked_component_, do_get_preferred_size())
        .WillByDefault(Return(tracked_component_preferred_size));
    tracked_component_->on_preferred_size_changed();
    tracked_component_->set_cursor_position(tracked_component_cursor_position);
    
    // Perform any extra cursor movements to set the viewport up.
    for (auto const &cursor_position : extra_tracked_cursor_movements)
    {
        tracked_component_->set_cursor_position(cursor_position);
    }
    
    // Change the size of the viewport.  This may cause the viewport to resize
    // and reposition the tracked component.
    viewport_->set_size(changed_viewport_size);

    // This may have changed the viewport's idea of where the cursor is.
    ASSERT_EQ(expected_viewport_cursor_position, viewport_->get_cursor_position());
    
    // Draw the viewport and check that the correct part of the tracked area is drawn.
    EXPECT_CALL(*tracked_component_, do_draw(_, expected_tracked_draw_area));

    terminalpp::canvas cvs{{5, 5}};
    munin::render_surface surface{cvs};
    surface.offset_by({1, 1});
    viewport_->draw(surface, {{}, viewport_->get_size()});
}

INSTANTIATE_TEST_CASE_P(
    changing_viewport_size,
    viewport_size_test,
    ValuesIn({
        // Base case: everything stays the same.
        viewport_size_test_data{
            {6, 6}, {1, 1}, {3, 3}, {}, {1, 1}, {{0, 0}, {3, 3}}
        },
        
        // Extending the viewport eastward when at the origin does not
        // move the cursor, but does expand the draw area.
        viewport_size_test_data{
            {6, 6}, {1, 1}, {6, 3}, {}, {1, 1}, {{0, 0}, {6, 3}}
        },
        
        // Extending the viewport southward when at the origin does not
        // move the cursor, but does expand the draw area.
        viewport_size_test_data{
            {6, 6}, {1, 1}, {3, 6}, {}, {1, 1}, {{0, 0}, {3, 6}}
        },
        
        // Shrinking the viewport so that the cursor is still in view does
        // not move the cursor, but does shrink the draw area.
        viewport_size_test_data{
            {6, 6}, {1, 1}, {2, 2}, {}, {1, 1}, {{0, 0}, {2, 2}}
        },
        
        // If the viewport has been scrolled to the right, then shrinking
        // the viewport but leaving the cursor in sight will not change the
        // origin of the draw area, but just shrink it.
        viewport_size_test_data{
            {6, 6}, {5, 0}, {2, 3}, {{4, 0}}, {1, 0}, {{3, 0}, {2, 3}}
        },

        // Similarly for scrolling off the bottom and back.
        viewport_size_test_data{
            {6, 6}, {0, 5}, {3, 2}, {{0, 4}}, {0, 1}, {{0, 3}, {3, 2}}
        },
        
        // However, if the viewport has been scrolled to the right, and 
        // shrinking the viewport would move the cursor out of sight, then
        // the viewport will instead draw from a position that contains the
        // cursor.
        viewport_size_test_data{
            {6, 6}, {5, 0}, {2, 3}, {}, {1, 0}, {{4, 0}, {2, 3}}
        },

        // Similarly for scrolling off the bottom and then resizing so that
        // the cursor is no longer visible.
        viewport_size_test_data{
            {6, 6}, {0, 5}, {3, 2}, {}, {0, 1}, {{0, 4}, {3, 2}}
        },
    })
);
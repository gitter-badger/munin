#include "text_area_test.hpp"
#include <munin/render_surface.hpp>

using testing::ValuesIn;

namespace {

using cursor_position_test_data = std::tuple<
    terminalpp::extent, // text area size
    terminalpp::string, // pre-inserted text
    munin::text_area::text_index, // initial caret index
    terminalpp::point, // set cursor position
    terminalpp::point, // expected resultant cursor position
    munin::text_area::text_index // expected caret position
>;

class cursor_caret_relationship 
  : public testing::TestWithParam<cursor_position_test_data>
{
protected:
    munin::text_area text_area_;
};

}

TEST_P(cursor_caret_relationship, cursor_and_caret_move_together)
{
    using std::get;

    auto const &param = GetParam();
    auto const &text_area_size = get<0>(param);
    auto const &text_area_text = get<1>(param);
    auto const &initial_caret_index = get<2>(param);
    auto const &set_cursor_position = get<3>(param);
    auto const &expected_cursor_position = get<4>(param);
    auto const &expected_caret_position = get<5>(param);

    text_area_.set_size(text_area_size);
    text_area_.insert_text(text_area_text);
    // text_area_.set_caret_position(initial_caret_index); // @TODO

    bool cursor_position_changed = false;
    text_area_.on_cursor_position_changed.connect(
        [&cursor_position_changed]
        { 
            cursor_position_changed = true; 
        });

    bool caret_position_changed = false;
    text_area_.on_caret_position_changed.connect(
        [&caret_position_changed]
        {
            caret_position_changed = true;
        });

    text_area_.set_cursor_position(set_cursor_position);

    ASSERT_TRUE(cursor_position_changed);
    ASSERT_EQ(expected_cursor_position, text_area_.get_cursor_position());

    ASSERT_TRUE(caret_position_changed);
    ASSERT_EQ(expected_caret_position, text_area_.get_caret_position());
}

INSTANTIATE_TEST_CASE_P(
    moving_the_cursor_in_an_empty_text_area_always_moves_to_home,
    cursor_caret_relationship,
    ValuesIn({
        cursor_position_test_data{
            {4, 2},
            "",
            0,
            {0, 0},
            {0, 0},
            0
        },
        cursor_position_test_data{
            {4, 2},
            "",
            0,
            {1, 0},
            {0, 0},
            0
        },
        cursor_position_test_data{
            {4, 2},
            "",
            0,
            {0, 1},
            {0, 0},
            0
        }
    })
);

/*
INSTANTIATE_TEST_CASE_P(
    moving_the_cursor_to_the_beginning_of_a_text_area_moves_the_caret_with_it,
    cursor_caret_relationship,
    ValuesIn({
        cursor_position_test_data{
            {4, 2},
            "ab"
        }
    })
);
*/

namespace {

class a_text_area_with_text_inserted : public a_text_area
{
public:
    a_text_area_with_text_inserted()
    {
        text_area_.insert_text("ab");
    }
};

}
TEST_F(a_text_area_with_text_inserted, can_have_text_inserted_at_the_front)
{
    text_area_.set_size({4, 2});

    bool redraw_requested = false;
    std::vector<terminalpp::rectangle> redraw_regions;
    
    text_area_.on_redraw.connect(
        [&](auto const &regions)
        {
            redraw_requested = true;
            redraw_regions = regions;
        });

    text_area_.insert_text("c", 0);
    
    fill_canvas({5, 3});
    munin::render_surface surface{canvas_};
    
    for (auto const &region : redraw_regions)
    {
        text_area_.draw(surface, region);
    }

    ASSERT_EQ(terminalpp::element{'c'}, canvas_[0][0]);
    ASSERT_EQ(terminalpp::element{'a'}, canvas_[1][0]);
    ASSERT_EQ(terminalpp::element{'b'}, canvas_[2][0]);
    
    verify_oob_is_untouched();
}

TEST_F(a_text_area_with_text_inserted, can_have_text_inserted_in_the_middle)
{
    text_area_.set_size({4, 2});

    bool redraw_requested = false;
    std::vector<terminalpp::rectangle> redraw_regions;
    
    text_area_.on_redraw.connect(
        [&](auto const &regions)
        {
            redraw_requested = true;
            redraw_regions = regions;
        });

    text_area_.insert_text("c", 1);
    
    fill_canvas({5, 3});
    munin::render_surface surface{canvas_};
    
    for (auto const &region : redraw_regions)
    {
        text_area_.draw(surface, region);
    }

    ASSERT_EQ(terminalpp::element{'a'}, canvas_[0][0]);
    ASSERT_EQ(terminalpp::element{'c'}, canvas_[1][0]);
    ASSERT_EQ(terminalpp::element{'b'}, canvas_[2][0]);
    
    verify_oob_is_untouched();
}

TEST_F(a_text_area_with_text_inserted, can_have_text_inserted_that_pushes_existing_text_into_the_next_line)
{
    text_area_.set_size({4, 2});

    bool redraw_requested = false;
    std::vector<terminalpp::rectangle> redraw_regions;
    
    text_area_.on_redraw.connect(
        [&](auto const &regions)
        {
            redraw_requested = true;
            redraw_regions = regions;
        });

    text_area_.insert_text("defg", 1);
    
    fill_canvas({5, 3});
    munin::render_surface surface{canvas_};
    
    for (auto const &region : redraw_regions)
    {
        text_area_.draw(surface, region);
    }

    ASSERT_EQ(terminalpp::element{'a'}, canvas_[0][0]);
    ASSERT_EQ(terminalpp::element{'d'}, canvas_[1][0]);
    ASSERT_EQ(terminalpp::element{'e'}, canvas_[2][0]);
    ASSERT_EQ(terminalpp::element{'f'}, canvas_[3][0]);
    ASSERT_EQ(terminalpp::element{'g'}, canvas_[0][1]);
    ASSERT_EQ(terminalpp::element{'b'}, canvas_[1][1]);
    ASSERT_EQ(terminalpp::element{' '}, canvas_[2][1]);
    ASSERT_EQ(terminalpp::element{' '}, canvas_[3][1]);
    
    verify_oob_is_untouched();
}

/*
TEST_F(a_text_area_with_text_inserted, advances_the_caret_when_inserting_text_before_the_caret)
{
    text_area_.set_cursor_position({0, 0});

    bool caret_position_changed = false;
    text_area_.on_caret_position_changed.connect(
        [&caret_position_changed]()
        {
            caret_position_changed = true;
        });

    text_area_.insert_text("de", 2);

    ASSERT_FALSE(caret_position_changed);
    ASSERT_EQ(2, text_area_.get_caret_position());
}

TEST_F(a_text_area_with_text_inserted, does_not_advance_the_caret_when_inserting_text_after_the_caret)
{
    text_area_.set_cursor_position({1, 0});//!!

    bool caret_position_changed = false;
    text_area_.on_caret_position_changed.connect(
        [&caret_position_changed]()
        {
            caret_position_changed = true;
        });

    text_area_.insert_text("de", 2);

    ASSERT_FALSE(caret_position_changed);
    ASSERT_EQ(1, text_area_.get_caret_position());
}
*/

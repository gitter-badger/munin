#include "fill_canvas.hpp"
#include <munin/status_bar.hpp>
#include <munin/render_context.hpp>
#include <gtest/gtest.h>

TEST(a_status_bar, is_a_component)
{
    std::shared_ptr<munin::component> comp = munin::make_status_bar();
}

namespace {

class a_new_status_bar : public testing::Test
{
protected:
    munin::status_bar status_bar_;
};

}

TEST_F(a_new_status_bar, has_a_zero_preferred_size)
{
    ASSERT_EQ(terminalpp::extent{}, status_bar_.get_preferred_size());
}

TEST_F(a_new_status_bar, draws_itself_as_blank)
{
    terminalpp::canvas canvas{{5, 3}};
    fill_canvas(canvas, 'x');

    munin::render_surface surface{canvas};
    munin::render_context context{surface, munin::default_animation_timer};
    context.offset_by({1, 1});

    status_bar_.set_size({3, 1});
    status_bar_.draw(context, {{}, status_bar_.get_size()});

    ASSERT_EQ(terminalpp::element('x'), canvas[0][0]);
    ASSERT_EQ(terminalpp::element('x'), canvas[1][0]);
    ASSERT_EQ(terminalpp::element('x'), canvas[2][0]);
    ASSERT_EQ(terminalpp::element('x'), canvas[3][0]);
    ASSERT_EQ(terminalpp::element('x'), canvas[4][0]);
    ASSERT_EQ(terminalpp::element('x'), canvas[0][1]);
    ASSERT_EQ(terminalpp::element(' '), canvas[1][1]);
    ASSERT_EQ(terminalpp::element(' '), canvas[2][1]);
    ASSERT_EQ(terminalpp::element(' '), canvas[3][1]);
    ASSERT_EQ(terminalpp::element('x'), canvas[4][1]);
    ASSERT_EQ(terminalpp::element('x'), canvas[0][2]);
    ASSERT_EQ(terminalpp::element('x'), canvas[1][2]);
    ASSERT_EQ(terminalpp::element('x'), canvas[2][2]);
    ASSERT_EQ(terminalpp::element('x'), canvas[3][2]);
    ASSERT_EQ(terminalpp::element('x'), canvas[4][2]);
}
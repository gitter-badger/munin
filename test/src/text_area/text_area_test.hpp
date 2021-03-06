#include <munin/text_area.hpp>
#include <terminalpp/canvas.hpp>
#include <gtest/gtest.h>

class a_text_area : public testing::Test
{
protected:
    static terminalpp::element const fill;
    
    munin::text_area   text_area_;
    terminalpp::canvas canvas_{{0, 0}};
    
    void fill_canvas(terminalpp::extent canvas_size);
    void verify_oob_is_untouched();
};

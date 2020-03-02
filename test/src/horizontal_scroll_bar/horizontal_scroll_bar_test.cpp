#include <munin/horizontal_scroll_bar.hpp>
#include <gtest/gtest.h>

namespace {
    
class a_new_horizontal_scroll_bar : public testing::Test
{
protected:
    std::shared_ptr<munin::horizontal_scroll_bar> scroll_bar_{
        munin::make_horizontal_scroll_bar()
    };
};

}

TEST_F(a_new_horizontal_scroll_bar, is_a_component)
{
    std::shared_ptr<munin::component> comp = scroll_bar_;
}

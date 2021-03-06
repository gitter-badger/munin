#pragma once

#include "munin/export.hpp"
#include <terminalpp/canvas.hpp>
#include <terminalpp/extent.hpp>
#include <terminalpp/terminal.hpp>
#include <nlohmann/json.hpp>
#include <boost/any.hpp>
#include <boost/signals2/signal.hpp>
#include <memory>

namespace munin {

class component;

//* =========================================================================
/// \brief An object that represents a top-level window.
//* =========================================================================
class MUNIN_EXPORT window
{
public :
    //* =====================================================================
    /// \brief Constructor
    /// \param content A component that this window displays.  May not be
    ///        null.
    //* =====================================================================
    explicit window(std::shared_ptr<component> content);
    
    //* =====================================================================
    /// \brief Destructor
    //* =====================================================================
    ~window();
    
    //* =====================================================================
    /// \brief Send an event to the window.  This will be passed straight to
    /// the content.
    //* =====================================================================
    void event(boost::any const &ev);

    //* =====================================================================
    /// \brief Returns a string that represents the change in state of the
    /// window since the last repaint.
    //* =====================================================================
    std::string repaint(
        terminalpp::canvas &cvs, terminalpp::terminal &term);

    //* =====================================================================
    /// \brief Returns a JSON representation of the current state of the
    /// window and its content.
    //* =====================================================================
    nlohmann::json to_json() const;
    
    //* =====================================================================
    /// \fn on_repaint_request
    /// \brief Connect to this signal in order to receive notifications that
    /// the content of the window has been changed and required repainting.
    //* =====================================================================
    boost::signals2::signal
    <
        void ()
    > on_repaint_request;
    
private :
    class impl;
    std::unique_ptr<impl> pimpl_;
};

}

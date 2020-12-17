#include "EventListener.h"


namespace RPI {

namespace UI {

using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/

EventListener::EventListener(const std::shared_ptr<RPI::Network::TcpBase> tcp_client)
    : client_ptr{tcp_client}
    , manager{}
    , keyboardId{manager.CreateDevice<gainput::InputDeviceKeyboard>()}
    , input_dev_map{manager, "RPI UI Input Map"}
{
    if(setupListener() != ReturnCodes::Success) {
        cerr << "Error: Failed to setup gainput event listener" << endl;
    }
}

EventListener::~EventListener() {
    // stub
}

/********************************************* Getters/Setters *********************************************/


/********************************************* Helper Functions ********************************************/

ReturnCodes EventListener::setupListener() {
    // create & setup devices
    // display size only really needed for mouse inputs
    manager.SetDisplaySize(Constants::UI::DISPLAY_WIDTH, Constants::UI::DISPLAY_HEIGHT);

    // create button mappings
    input_dev_map.MapBool(
        static_cast<int>(ButtonCodes::ButtonConfirm),
        keyboardId,
        gainput::KeyReturn
    );

    return ReturnCodes::Success;
}


}; // end of UI namespace

}; // end of RPI namespace


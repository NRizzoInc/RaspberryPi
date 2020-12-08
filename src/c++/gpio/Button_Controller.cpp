#include "Button_Controller.h"

using std::cout;
using std::cerr;
using std::endl;


namespace gpio {
namespace Button {

/********************************************** Constructors **********************************************/
ButtonController::ButtonController()
    : color_to_btns ({
            {"red",     2}, // gpio2/pin3
            {"yellow",  3},
            {"green",   4},
            {"blue",    17}
        })
{

}

ButtonController::~ButtonController() {
    //stub
}

/********************************************* Public Helpers *********************************************/
std::vector<std::string> ButtonController::getBtnColorList() {
    return Helpers::getMapKeys(color_to_btns);
}


/********************************************* Helper Functions ********************************************/


}; // end of Button namespace

}; // end of gpio namespace

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants {

    // Defines different possible returns rather than just success/fail
    enum class ReturnCodes {
        Success,
        Error,
        TryAgain,
    };

}; // end of constants namespace

#endif
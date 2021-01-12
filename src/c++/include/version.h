#ifndef RPI_VERSION_H
#define RPI_VERSION_H

// Standard Includes
#include <iostream>
#include <string>

// Our Includes

// 3rd Party Includes


namespace RPI::Version {


extern const std::string GIT_SHA1;
extern const std::string GIT_BRANCH;
extern const std::string GIT_COMMIT_SUBJECT;
extern const std::string GIT_DATE;
extern const std::string GIT_DESCRIBE;

}; // end of RPI::Version namespace


#endif

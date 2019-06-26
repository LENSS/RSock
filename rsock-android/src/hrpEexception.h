//
// Created by Chen on 7/18/17.
//

#ifndef HRP_EXCEPTION_H
#define HRP_EXCEPTION_H

#include <exception>
#include <string>

namespace hrp {
    class hrpException: public std::exception {
    public:
        explicit hrpException(const std::string& rson) : _msg(rson) {};

        virtual const char* what() const throw() { return _msg.c_str(); }
        ~hrpException() throw() {}
    private:
        std::string _msg;
    };
}

#endif //HRP_EXCEPTION_H

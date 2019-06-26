//
// Created by Chen on 7/28/17.
//

#include "hrpNode.h"


namespace hrp {
    hrpNode sock2node(const sockaddr_in &sa) {
        char str[256];
        inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
        hrpNode n(str);
        return n;
    }
}
#pragma once

#include "cluon-complete.hpp"

class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    virtual void setup(cluon::OD4Session& od4) = 0;
};

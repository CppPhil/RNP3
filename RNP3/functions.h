#pragma once
#include "Utility.h"
#include "Types.h"
#include <cstddef>
#include <memory>
#include <QTcpSocket>

namespace func {
    std::unique_ptr<utils::Message> makeMessage(QTcpSocket &socket);
} // END of namespace func

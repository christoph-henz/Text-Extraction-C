#pragma once

#include "Entity.h"
#include <optional>
#include <string>

namespace Core {

struct IRepository {
    virtual ~IRepository() = default;
    virtual std::optional<Document> GetDocument(const std::string &id) = 0;
};

} // namespace Core

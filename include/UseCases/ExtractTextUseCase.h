#pragma once

#include "../Core/Entity.h"
#include "../Core/IRepository.h"
#include <memory>
#include <string>

namespace UseCases {

class ExtractTextUseCase {
public:
    explicit ExtractTextUseCase(std::shared_ptr<Core::IRepository> repo);
    std::string Execute(const std::string &path);

private:
    std::shared_ptr<Core::IRepository> repo_;
};

} // namespace UseCases

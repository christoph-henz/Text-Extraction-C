#pragma once

#include "../ExtractTextUseCase.h"
#include <sstream>

namespace UseCases {

inline ExtractTextUseCase::ExtractTextUseCase(std::shared_ptr<Core::IRepository> repo)
    : repo_(repo)
{
}

inline std::string ExtractTextUseCase::Execute(const std::string &path)
{
    // Skeleton: return dummy extracted text
    std::ostringstream ss;
    ss << "Extracted text (skeleton) from: " << path;
    return ss.str();
}

} // namespace UseCases

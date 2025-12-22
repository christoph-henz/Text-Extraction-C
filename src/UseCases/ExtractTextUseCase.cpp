#include "../../include/UseCases/ExtractTextUseCase.h"

namespace UseCases {

ExtractTextUseCase::ExtractTextUseCase(std::shared_ptr<Core::IRepository> repo)
    : repo_(repo)
{
}

std::string ExtractTextUseCase::Execute(const std::string &path)
{
    // Stub implementation: attempt to fetch document, return mock result
    auto doc = repo_->GetDocument(path);
    if (doc) {
        return "Extracted from " + path + ": " + doc->content;
    }
    return "Failed to extract from " + path;
}

} // namespace UseCases

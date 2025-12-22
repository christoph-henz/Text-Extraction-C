#pragma once

#include <memory>
#include <string>

namespace UseCases { class ExtractTextUseCase; }

namespace Presentation::ViewModel {

class MainViewModel {
public:
    explicit MainViewModel(std::shared_ptr<UseCases::ExtractTextUseCase> usecase);
    void StartExtraction(const std::string &path);
    std::string GetStatus() const;
    std::string GetLastResult() const;

private:
    std::shared_ptr<UseCases::ExtractTextUseCase> usecase_;
    std::string status_ = "Idle";
    std::string lastResult_;
};

} // namespace Presentation::ViewModel

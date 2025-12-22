#include "../../include/Presentation/ViewModel/MainViewModel.h"
#include "../../include/UseCases/ExtractTextUseCase.h"
#include <iostream>

using namespace Presentation::ViewModel;

MainViewModel::MainViewModel(std::shared_ptr<UseCases::ExtractTextUseCase> usecase)
    : usecase_(usecase)
{
}

void MainViewModel::StartExtraction(const std::string &path)
{
    status_ = "Running";
    // synchronously call use case for skeleton
    std::string result = usecase_->Execute(path);
    lastResult_ = result;
    status_ = "Finished";
}

std::string MainViewModel::GetStatus() const { return status_; }

std::string MainViewModel::GetLastResult() const { return lastResult_; }

#include "../include/Presentation/ViewModel/MainViewModel.h"
#include "../include/Presentation/View/MainView.hpp"
#include "../include/UseCases/ExtractTextUseCase.h"
#include <iostream>
#include <memory>

// Simple stub repository implementation (infrastructure placeholder)
struct StubRepo : Core::IRepository {
    std::optional<Core::Document> GetDocument(const std::string &id) override {
        Core::Document d; d.id = id; d.content = "stub content for " + id;
        return d;
    }
};

int main()
{
    auto repo = std::make_shared<StubRepo>();
    auto usecase = std::make_shared<UseCases::ExtractTextUseCase>(repo);
    Presentation::ViewModel::MainViewModel vm(usecase);

    RunGui(vm);

    return 0;
}

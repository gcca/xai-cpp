#include <iostream>

#include "xai.hpp"

int main(int argc, char *argv[]) {

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <apikey>" << std::endl;
    return EXIT_FAILURE;
  }

  const char *apikey{argv[1]};

  auto client = xai::Client::Make(apikey);
  auto messages = xai::Messages::Make("grok-beta");

  while (true) {
    std::cout << ">>> ";

    std::string line;
    std::getline(std::cin, line);

    if (line == "\\q") {
      break;
    } else if (line == "\\m") {
      client->ListModels()->Traverse([](const xai::ModelList::Model &model) {
        std::cout << model.id << std::endl;
      });
    } else if (line == "\\l") {
      client->ListLanguageModels()->Traverse(
          [](const xai::LanguageModelList::LanguageModel &model) {
            std::cout << model.id << std::endl;
          });
    } else {
      messages->Add(line);
      auto choices = client->ChatCompletion(messages);
      messages->Pick(choices);

      std::cout << "... " << choices->first() << std::endl;
    }
  }

  std::cout << "end" << std::endl;

  return EXIT_SUCCESS;
}

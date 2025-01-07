#pragma once

#include <functional>
#include <memory>
#include <string_view>

#define XAI_PROTO(T)                                                           \
public:                                                                        \
  virtual ~T();                                                                \
                                                                               \
protected:                                                                     \
  T(const T &) = delete;                                                       \
  T(const T &&) = delete;                                                      \
  T &operator=(const T &) = delete;                                            \
  T &operator=(const T &&) = delete;                                           \
  T() = default;

namespace xai {

class Choices {
  XAI_PROTO(Choices)
public:
  virtual std::string_view first() = 0;
};

class Messages {
  XAI_PROTO(Messages)
public:
  virtual void Add(const std::string &line) = 0;
  virtual void Pick(const std::unique_ptr<Choices> &choices) = 0;

  static std::unique_ptr<Messages> Make(const char *model);
};

class ModelList {
  XAI_PROTO(ModelList)
public:
  struct Model {
    std::string_view id;
  };

  virtual void Traverse(const std::function<void(const Model &)> &call) = 0;
};

class LanguageModelList {
  XAI_PROTO(LanguageModelList)
public:
  struct LanguageModel {
    std::string_view id;
  };

  virtual void
  Traverse(const std::function<void(const LanguageModel &)> &call) = 0;
};

class Client {
  XAI_PROTO(Client)
public:
  [[nodiscard]]
  virtual std::unique_ptr<Choices>
  ChatCompletion(const std::unique_ptr<Messages> &messages) = 0;

  [[nodiscard]]
  virtual std::unique_ptr<ModelList> ListModels() = 0;

  [[nodiscard]]
  virtual std::unique_ptr<LanguageModelList> ListLanguageModels() = 0;

  [[nodiscard]]
  static std::unique_ptr<Client> Make(const char *apikey);

  [[nodiscard]]
  static std::unique_ptr<Client> Make(const char *apikey, const char *host);
};

} // namespace xai

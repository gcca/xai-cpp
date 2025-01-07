#include "xai.hpp"

#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>

#include "dev.hpp"

namespace {

class xAIChoices : public xai::Choices {
public:
  explicit xAIChoices(boost::json::object &&object)
      : object_{std::move(object)} {}

  std::string_view first() final {
    return object_["choices"]
        .as_array()[0]
        .as_object()["message"]
        .as_object()["content"]
        .as_string();
  }

  boost::json::object object_;
};

class xAIMessages final : public xai::Messages {
public:
  explicit xAIMessages(const char *model) : model_{model} {}

  void Add(const std::string &line) final {
    messages_.emplace_back(
        boost::json::object{{"role", "user"}, {"content", line}});
  }

  void Pick(const std::unique_ptr<xai::Choices> &choices) final {
    messages_.emplace_back(boost::json::object{{"role", "assistant"},
                                               {"content", choices->first()}});
  }

  boost::json::array messages_;

  const char *model_;
};

class xAIModelList : public xai::ModelList {
public:
  explicit xAIModelList(boost::json::object &&object)
      : object_{std::move(object)} {}

  void Traverse(const std::function<void(const Model &)> &call) final {
    boost::json::array models = object_["data"].as_array();
    for (auto &model : models) {
      call(Model{model.as_object()["id"].as_string()});
    }
  }

private:
  boost::json::object object_;
};

class xAILanguageModelList : public xai::LanguageModelList {
public:
  explicit xAILanguageModelList(boost::json::object &&object)
      : object_{std::move(object)} {}

  void Traverse(const std::function<void(const LanguageModel &)> &call) final {
    boost::json::array models = object_["models"].as_array();
    for (auto &model : models) {
      call(LanguageModel{model.as_object()["id"].as_string()});
    }
  }

private:
  boost::json::object object_;
};

struct Server {
  static constexpr int version = 11;
  static constexpr const char *port = "443", *user_agent = "xaicpp/0.1",
                              *content_type = "application/json";
};

static constexpr const char *default_host = "api.x.ai";

class xAIClient final : public xai::Client {
public:
  explicit xAIClient(const char *apikey, const char *host = default_host)
      : io_context_{}, ssl_context_{boost::asio::ssl::context::tlsv12_client},
        stream_{io_context_, ssl_context_}, host_{host} {
    dev::load_certs(ssl_context_);
    ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer);

    if (!SSL_set_tlsext_host_name(stream_.native_handle(), host_.c_str())) {
      boost::beast::error_code ec{static_cast<int>(::ERR_get_error()),
                                  boost::asio::error::get_ssl_category()};
      throw boost::beast::system_error{ec};
    }

    boost::asio::ip::tcp::resolver resolver(io_context_);
    const boost::asio::ip::tcp::resolver::results_type results =
        resolver.resolve(host_, Server::port);

    boost::beast::get_lowest_layer(stream_).connect(results);

    stream_.handshake(boost::asio::ssl::stream_base::client);

    authorization_.reserve(135);
    authorization_.assign("Bearer ", 7);
    authorization_.append(apikey);
  }

  ~xAIClient() final = default;

  std::unique_ptr<xai::Choices>
  ChatCompletion(const std::unique_ptr<xai::Messages> &messages) final {
    boost::json::object obj{
        {"model", static_cast<xAIMessages *>(messages.get())->model_},
        {"stream", false},
        {"temperature", 0},
        {"messages",
         static_cast<const xAIMessages *>(messages.get())->messages_},
    };

    boost::beast::http::request<boost::beast::http::string_body> request{
        boost::beast::http::verb::post, "/v1/chat/completions",
        Server::version};
    SetUp(request);
    request.body() = boost::json::serialize(obj);
    request.prepare_payload();

    boost::beast::http::response<boost::beast::http::dynamic_body> response =
        Do(request);

    boost::json::value value = boost::json::parse(
        boost::beast::buffers_to_string(response.body().data()));

    return std::make_unique<xAIChoices>(std::move(value.as_object()));
  }

  std::unique_ptr<xai::ModelList> ListModels() final {
    boost::beast::http::request<boost::beast::http::string_body> request{
        boost::beast::http::verb::get, "/v1/models", Server::version};
    SetUp(request);

    boost::beast::http::response<boost::beast::http::dynamic_body> response =
        Do(request);

    boost::json::value value = boost::json::parse(
        boost::beast::buffers_to_string(response.body().data()));

    return std::make_unique<xAIModelList>(std::move(value.as_object()));
  }

  std::unique_ptr<xai::LanguageModelList> ListLanguageModels() final {
    boost::beast::http::request<boost::beast::http::string_body> request{
        boost::beast::http::verb::get, "/v1/language-models", Server::version};
    SetUp(request);

    boost::beast::http::response<boost::beast::http::dynamic_body> response =
        Do(request);

    boost::json::value value = boost::json::parse(
        boost::beast::buffers_to_string(response.body().data()));

    return std::make_unique<xAILanguageModelList>(std::move(value.as_object()));
  }

private:
  boost::asio::io_context io_context_;
  boost::asio::ssl::context ssl_context_;
  boost::asio::ssl::stream<boost::beast::tcp_stream> stream_;
  std::string host_;
  std::string authorization_;

  inline void
  SetUp(boost::beast::http::request<boost::beast::http::string_body> &request) {
    request.set(boost::beast::http::field::host, host_);
    request.set(boost::beast::http::field::content_type, Server::content_type);
    request.set(boost::beast::http::field::authorization, authorization_);
    request.set(boost::beast::http::field::user_agent, Server::user_agent);
  }

  inline boost::beast::http::response<boost::beast::http::dynamic_body>
  Do(boost::beast::http::request<boost::beast::http::string_body> &request) {
    boost::beast::http::write(stream_, request);

    boost::beast::flat_buffer buffer;

    boost::beast::http::response<boost::beast::http::dynamic_body> response;

    boost::beast::http::read(stream_, buffer, response);

    return response;
  }
};

} // namespace

namespace xai {

Choices::~Choices() = default;
Messages::~Messages() = default;
ModelList::~ModelList() = default;
LanguageModelList::~LanguageModelList() = default;
Client::~Client() = default;

std::unique_ptr<Client> Client::Make(const char *apikey) {
  return std::make_unique<xAIClient>(apikey);
}

std::unique_ptr<Client> Client::Make(const char *apikey, const char *host) {
  return std::make_unique<xAIClient>(apikey, host);
}

std::unique_ptr<Messages> Messages::Make(const char *model) {
  return std::make_unique<xAIMessages>(model);
}

} // namespace xai

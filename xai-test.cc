#include <gtest/gtest.h>

#include "test.hpp"
#include "xai.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>

static void ServerRun() {
  try {
    auto const address = boost::asio::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(std::atoi("443"));

    boost::asio::io_context ioc{1};

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12};

    test::load_certs(ctx);

    boost::asio::ip::tcp::acceptor acceptor{ioc, {address, port}};

    boost::asio::ip::tcp::socket socket{ioc};

    acceptor.accept(socket);

    boost::beast::error_code ec, ec2;

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket &> stream{socket,
                                                                    ctx};

    ec2 = stream.handshake(boost::asio::ssl::stream_base::server, ec);
    if (ec) {
      std::cerr << "handshake: " << ec.message() << std::endl;
      return;
    }

    boost::beast::flat_buffer buffer;

    boost::beast::http::request<boost::beast::http::string_body> req;
    boost::beast::http::read(stream, buffer, req, ec);
    if (ec == boost::beast::http::error::end_of_stream) {
      std::cerr << "end of stream" << std::endl;
      return;
    }
    if (ec) {
      std::cerr << "read: " << ec.message() << std::endl;
      return;
    }

    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::ok, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "{\"choices\":[{\"message\":{\"content\":\"foo content\"}}]}";
    res.prepare_payload();

    boost::beast::http::write(stream, res, ec);

    if (ec) {
      std::cerr << "write: " << ec.message() << std::endl;
      return;
    }

    ec2 = stream.shutdown(ec);
    if (ec) {
      std::cerr << "shutdown: " << ec.message() << std::endl;
      return;
    }

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

TEST(XaiTest, Connect) {
  std::thread server{ServerRun};

  auto client = xai::Client::Make("foo_key");
  auto messages = xai::Messages::Make("test");

  messages->AddU("hello");

  auto choices = client->ChatCompletion(messages);

  server.join();

  EXPECT_EQ(choices->first(), "foo content");
}

#include <boost/asio/ssl.hpp>

namespace dev {

void load_certs(boost::asio::ssl::context &);
void load_certs(boost::asio::ssl::context &, boost::system::error_code &);

} // namespace dev

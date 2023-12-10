#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using namespace boost;


asio::awaitable<void> echo(tcp::socket sock){
    try{
        std::vector<char> buf(1024);
        while (true){
            size_t n=co_await sock.async_read_some(asio::buffer(buf),asio::use_awaitable);
            co_await async_write(sock,asio::buffer(buf,n),asio::use_awaitable);
        }
    }
    catch(const std::exception& e){
        BOOST_LOG_TRIVIAL(info) << "【SERVER】:Error..." << e.what();
    }
}

asio::awaitable<void> listener(){
    auto executor=co_await asio::this_coro::executor;
    tcp::acceptor acceptor(executor,{tcp::v4(),5555});
    while (true){
        auto sock=co_await acceptor.async_accept(asio::use_awaitable);
        BOOST_LOG_TRIVIAL(info) << "【SERVER】:Accept..." << sock.remote_endpoint().address();
        asio::co_spawn(executor,echo(std::move(sock)),asio::detached);
    }
} 

// int main(){
//     try{
//          asio::io_context ctx(1);
//          asio::signal_set signals(ctx,SIGINT,SIGTERM);
//          signals.async_wait([&](auto,auto){ctx.stop();});
//          asio::co_spawn(ctx,listener(),asio::detached);
//          BOOST_LOG_TRIVIAL(info) << "SERVER】:Start...";
//          ctx.run();
//     }
//     catch(const std::exception& e){
//         BOOST_LOG_TRIVIAL(info) << "【SERVER】:Error..." << e.what();
//     }
// }
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <set>
#include <deque>

using boost::asio::ip::tcp;
using namespace boost;

class Participant{
public:
    virtual void Deliver(std::string)=0;
    virtual ~Participant(){};
};
using Participant_ptr=std::shared_ptr<Participant>;

class Room{
public:
    void Join(Participant_ptr participant){
        _participants.insert(participant);
        for(auto msg: _distr_msgs)
            participant->Deliver(msg);
    }
    void Deliver(const std::string& msg){
        _distr_msgs.push_back(msg);
        while(_distr_msgs.size()>MAX_COMMUNICATION_SIZE)
            _distr_msgs.pop_front();
        for(auto participant: _participants)
            participant->Deliver(msg);
    }
    void Leave(Participant_ptr participant){
        _participants.erase(participant);
    }
private:
    std::set<Participant_ptr> _participants;
    std::deque<std::string> _distr_msgs;
    enum{MAX_COMMUNICATION_SIZE=100};
};

class Session:public Participant,public std::enable_shared_from_this<Session>{
public:
    Session(tcp::socket sock,Room& room):_sock(std::move(sock)),_room(room),_timer(_sock.get_executor()){
        _timer.expires_at(std::chrono::steady_clock::time_point::max());
        BOOST_LOG_TRIVIAL(info) << "SERVER】:Accept..." << _sock.remote_endpoint().address();
    }
    void Start(){
        _room.Join(shared_from_this());
        // share to keep alive in single coroutine
        asio::co_spawn(_sock.get_executor(),[self=shared_from_this()](){return self->Reader();},asio::detached);
        asio::co_spawn(_sock.get_executor(),[self=shared_from_this()](){return self->Writer();},asio::detached);
    }
    void Deliver(std::string msg){
        _write_msgs.push_back(msg);
        _timer.cancel_one();
    }
private:
    asio::awaitable<void> Reader(){
        try{
            for(std::string readStr;;){
                std::size_t size=co_await asio::async_read_until(_sock,asio::dynamic_buffer(readStr,1024),"\n",asio::use_awaitable);
                BOOST_LOG_TRIVIAL(info) << "SERVER】:Send..." << readStr.substr(0,size);
                _room.Deliver(readStr.substr(0,size));
                readStr.erase(0,size);
            }
        }
        catch(const std::exception& e){
            stop();
            BOOST_LOG_TRIVIAL(error) << "【server】:Error..." << e.what();
        }
    }
    asio::awaitable<void> Writer(){
        try{
            while(true){
                if(_write_msgs.empty()){
                    boost::system::error_code ec;
                    co_await _timer.async_wait(asio::redirect_error(asio::use_awaitable,ec)); 
                }else{
                    co_await asio::async_write(_sock,asio::buffer(_write_msgs.front()),asio::use_awaitable);
                    _write_msgs.pop_front();
                }
            }
        }
        catch(const std::exception& e){
            stop();
            BOOST_LOG_TRIVIAL(error) << "【server】:Error..." << e.what();
        }
    }
    void stop(){
        _room.Leave(shared_from_this());
        _sock.close();
        _timer.cancel();
    }
    tcp::socket _sock;
    std::deque<std::string> _write_msgs{};
    Room& _room;
    asio::steady_timer _timer;
};

asio::awaitable<void> listener(){
    auto exector=co_await asio::this_coro::executor;
    tcp::acceptor acceptor(exector,{tcp::v4(),5555});
    Room room;
    while(true){
        std::make_shared<Session>(co_await acceptor.async_accept(asio::use_awaitable),room)->Start();
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
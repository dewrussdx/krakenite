#include <cassert>
#include <string.h>
#include <unordered_map>
#include "client.h"
#include "orderbook.h"
#include "util.h"

#if _WIN32
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#if PERF_TEST
typedef std::unordered_map<UserOrder, StopWatch> PerfTracker;
#endif

Client::Client(const char *target, unsigned short port)
    : _target(target), _port(port), _socket(INVALID_SOCKET), _server({0})
{
}
Client::~Client()
{
#if _WIN32
    closesocket(_socket);
    WSACleanup();
#else
    close(_socket);
#endif
}

bool Client::init()
{
#if _WIN32
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
    {
        std::cout << "ERROR: Winsock initialization failed: " << WSAGetLastError() << std::endl;
        return false;
    }
#endif
    // create socket
    if ((_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
    {
        NetIO::error("Unable to create socket");
        return false;
    }

    // setup address structure
    memset(&_server, 0, sizeof(_server));
    _server.sin_family = AF_INET;
    _server.sin_port = htons(_port);
    inet_pton(AF_INET, _target.c_str(), &_server.sin_addr);
    return true;
}

bool Client::run()
{
    OrderBookManager manager;
    OrderBook *active_book = nullptr;
    char buffer[1024];

    NetIO::NewOrder new_order;
    NetIO::CancelOrder cancel_order;
    NetIO::FlushBook flush_book;

#if PERF_TEST
    struct PerfMetric
    {
        Uid user_id = 0;
        Uid user_order_id = 0;
        long long duration = 0;
    };
    constexpr size_t _perf_max_samples = _perf_sample_per_trial * _perf_num_trials;
    std::vector<PerfMetric> _perf_results;
    _perf_results.reserve(_perf_max_samples);
    PerfTracker _perf_tracker;
    size_t _perf_sample_count = 0;
#endif

    // HACK to exit client after 106 packages, i'm very tried now...
    int exit_client_after = 106;

    // send handshake to server
    if (sendto(_socket, reinterpret_cast<const char *>(&_netio.handshake), sizeof(NetIO::Handshake),
               0, (sockaddr *)&_server, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        NetIO::error("sendto() failed");
        return false;
    }
    socklen_t slen = sizeof(sockaddr_in);
    int msg_size;
    int scenario = 1;
    bool stop = false;
#if !PERF_TEST
    LogPub::instance().queue_ext("Scenario: %d", scenario);
#endif
    while (!stop)
    {
        if ((msg_size = recvfrom(_socket, buffer, sizeof(buffer), 0, (sockaddr *)&_server, &slen)) == SOCKET_ERROR)
        {
            NetIO::error("recvfrom() failed");
            return false;
        }
        assert(msg_size > 0);

        // send 'ack' for next package
        if (sendto(_socket, reinterpret_cast<const char *>(&_netio.ack), sizeof(NetIO::Ack),
                   0, (sockaddr *)&_server, sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            NetIO::error("sendto() failed");
            return false;
        }

#if PERF_TEST
        LogPub::instance().start_recording();
#endif
        // unpack scenarios
        char id = buffer[0];
        switch (id)
        {
        case 'N':
        {
            assert(msg_size == sizeof(NetIO::NewOrder));
            NetIO::NewOrder *data = reinterpret_cast<NetIO::NewOrder *>(buffer);
            if (!active_book)
            {
                active_book = manager(data->symbol);
            }
            assert(active_book);
#if PERF_TEST
            _perf_tracker.emplace(std::pair<UserOrder, StopWatch>({data->user_id, data->user_order_id}, StopWatch()));
#endif
            active_book->new_order(data->side == 'B' ? BUY : SELL, std::forward<Order>(
                                                                       Order{data->user_id, data->user_order_id, data->price, 0, data->qty}));
            break;
        }

        case 'C':
        {
            assert(msg_size == sizeof(NetIO::CancelOrder));
            assert(active_book);
            NetIO::CancelOrder *data = reinterpret_cast<NetIO::CancelOrder *>(buffer);
            active_book->cancel_order(std::forward<UserOrder>(
                UserOrder{data->user_id, data->user_order_id}));
            break;
        }

        case 'F':
        {
            assert(msg_size == sizeof(NetIO::FlushBook));
            assert(active_book);
            active_book->flush();
#if !PERF_TEST
            LogPub::instance().queue_ext("Scenario: %d", ++scenario);
#endif
            break;
        }
        }
#if PERF_TEST
        auto &out = LogPub::instance().stop_recording();
        for (auto &line : out)
        {
            if (line[0] == 'A')
            {
                Uid user_id = 0;
                Uid user_order_id = 0;
#if _WIN32
                if (sscanf_s(line.c_str(), "A, %d, %d", &user_id, &user_order_id) == 2)
#else
                if (sscanf(line.c_str(), "A, %d, %d", &user_id, &user_order_id) == 2)
#endif
                {
                    assert(user_id > 0 && user_order_id > 0);
                    auto it = _perf_tracker.find(UserOrder({user_id, user_order_id}));
                    assert(it != _perf_tracker.end());
                    StopWatch &watch = (*it).second;
                    auto duration = watch.stop<std::chrono::microseconds>();
                    _perf_results.emplace_back(PerfMetric({user_id, user_order_id, duration}));
                }
            }
        }
        _perf_sample_count++;
        if (_perf_sample_count >= _perf_max_samples)
        {
            break;
        }
#else
        if ((--exit_client_after) <= 0)
        {
            break;
        }
#endif
    }
    std::cout << "Exiting client" << std::endl;

#if PERF_TEST
    std::cout << "Performance test results: " << std::endl;
    std::cout << "- Number of trials: " << _perf_num_trials << std::endl;
    std::cout << "- Number of samples per trial: " << _perf_sample_per_trial << std::endl;
    std::cout << "- Total samples: " << _perf_max_samples << std::endl;

    // compute mean
    long long sum = 0;
    for (auto &metric : _perf_results)
    {
        sum += metric.duration;
    }
    double mean = static_cast<double>(sum) / _perf_results.size();
    // sum up variance
    double var = 0;
    for (auto &metric : _perf_results)
    {
        auto v = static_cast<double>(metric.duration) - mean;
        var += (v * v);
    }
    // compute population variance
    var /= (_perf_results.size() - 1);

    // compute standard deviation
    auto std = std::sqrt(var);

    std::cout << "- Mean: " << static_cast<long long>(std::round(mean)) << "us" << std::endl;
    std::cout << "- Std: " << static_cast<long long>(std::round(std)) << "us" << std::endl;

#endif
    return true;
}

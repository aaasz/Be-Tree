// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
#include "network/transport.hpp"

namespace network {

TransportReceiver::~TransportReceiver(){}

//Timeout::Timeout(Transport *transport, uint64_t ms, timer_callback_t cb)
//    : transport(transport), ms(ms), cb(cb)
//{
//    timerId = 0;
//}
//
//Timeout::~Timeout()
//{
//    Stop();
//}
//
//void
//Timeout::SetTimeout(uint64_t ms)
//{
//    ASSERT(!Active());
//    this->ms = ms;
//}
//
//uint64_t
//Timeout::Start()
//{
//    return this->Reset();
//}
//
//
//uint64_t
//Timeout::Reset()
//{
//    Stop();
//    
//    timerId = transport->Timer(ms, [this]() {
//            timerId = 0;
//            Reset();
//            cb();
//        });
//    
//    return ms;
//}
//
//void
//Timeout::Stop()
//{
//    if (timerId > 0) {
//        transport->CancelTimer(timerId);
//        timerId = 0;
//    }
//}
//
//bool
//Timeout::Active() const
//{
//    return (timerId != 0);
//}

} // namespace network

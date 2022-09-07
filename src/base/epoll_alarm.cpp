//
// Created by Chengke Wong on 2019/10/4.
//

#include "epoll_alarm.h"

EpollAlarm::EpollAlarm(EpollServer *epoll_server,
                       std::unique_ptr<Delegate> delegate)
    : Alarm(std::move(delegate)), server_(epoll_server) {}

void EpollAlarm::setImpl() { token_ = server_->registerAlarm(this); }

void EpollAlarm::cancelImpl() { server_->unregisterAlarm(token_); }

EpollAlarm::~EpollAlarm() { cancel(); }

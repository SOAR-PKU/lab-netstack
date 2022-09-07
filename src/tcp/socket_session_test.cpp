/**
 * @file socket_session_test.cpp
 * @brief This testsuit tests some TCP scenarios illustrated in RFC 793.
 * DO NOT MODIFY THIS FILE!
 *
 */

#include "../util/mock_alarm_factory.h"
#include "../util/mock_ip_layer.h"
#include "socket_session.h"
#include "gtest/gtest.h"

class SocketSessionTest : public ::testing::Test {
protected:
  SocketSessionTest()
      : time_machine_(std::make_unique<MockAlarmFactory>()),
        ip_layer_(std::make_unique<MockIPLayer>()),
        rand_generator_(std::make_unique<RandGenerator>(20191106)),
        seg_factory_(std::make_unique<SegmentFactory>(tester_, subject_)),
        session_(std::make_unique<SocketSession>(
            ip_layer_.get(), time_machine_.get(), subject_, tester_,
            rand_generator_.get())) {}

  ~SocketSessionTest() override = default;

  void SetUp() override {}

  void TearDown() override {}

  inline std::deque<std::unique_ptr<Segment>> &queue() {
    return ip_layer_->queue();
  }

  bool hasSegment() { return !ip_layer_->queue().empty(); }

  std::unique_ptr<Segment> popSegment() {
    EXPECT_FALSE(ip_layer_->queue().empty());
    std::unique_ptr<Segment> rv = std::move(ip_layer_->queue().front());
    queue().pop_front();
    return rv;
  }

  void segmentDropAll() { ip_layer_->queue().clear(); }

  void expectState(ConnectionState state) {
    EXPECT_EQ(state, session_->state());
  }

  template <class... T> void reply(T &&...args) {
    std::unique_ptr<Segment> seg =
        seg_factory_->createSegment(std::forward<T>(args)...);
    DLOG(INFO) << "<- " << seg->toString();
    session_->onSegmentArrival(std::move(seg));
  }

private:
  SocketAddress subject_ = {"192.168.0.0", 10086};
  SocketAddress tester_ = {"192.168.0.1", 65535};

  std::unique_ptr<MockIPLayer> ip_layer_;
  std::unique_ptr<RandGenerator> rand_generator_;

protected:
  std::unique_ptr<MockAlarmFactory> time_machine_;
  std::unique_ptr<SegmentFactory> seg_factory_;
  std::unique_ptr<SocketSession> session_;
};

/** NOTE: all figures are excerpted from RFC 793 **/

/*
      TCP A                                                TCP B

  1.  CLOSED                                               LISTEN

  2.  SYN-SENT    --> <SEQ=100><CTL=SYN>               --> SYN-RECEIVED

  3.  ESTABLISHED <-- <SEQ=300><ACK=101><CTL=SYN,ACK>  <-- SYN-RECEIVED

  4.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK>       --> ESTABLISHED

  5.  ESTABLISHED --> <SEQ=101><ACK=301><CTL=ACK><DATA> --> ESTABLISHED

          Basic 3-Way Handshake for Connection Synchronization

                                Figure 7.
 */

TEST_F(SocketSessionTest, ClientBasic3WayHandshake) {
  session_->open();
  expectState(ConnectionState::SYN_SENT);
  std::unique_ptr<Segment> seg;
  seg = popSegment();
  EXPECT_TRUE(seg->isSYN());
  SequenceNumber syn = seg->sequence_;
  reply(666, syn + 1, Segment::SYN | Segment::ACK);

  expectState(ConnectionState::ESTABLISHED);
  seg = popSegment();
  EXPECT_TRUE(seg->isACK());
  EXPECT_EQ(seg->sequence_, syn + 1);
  EXPECT_EQ(seg->acknowledgment_, 667);

  time_machine_->elapse(TimeBase::Delta::fromMilliseconds(3600));
  if (!hasSegment()) {
    return;
  }
  seg = popSegment();
  EXPECT_FALSE(seg->isRST());
  EXPECT_FALSE(seg->isFIN());
}

TEST_F(SocketSessionTest, ServerBasic3WayHandshake) {
  std::unique_ptr<Segment> seg = seg_factory_->createSegment(300, Segment::SYN);
  session_->open(std::move(seg));
  seg = popSegment();
  EXPECT_TRUE(seg->isSYN());
  EXPECT_TRUE(seg->isACK());
  EXPECT_EQ(seg->acknowledgment_, 301);
  EXPECT_FALSE(hasSegment());
}

/*
      TCP A                                            TCP B

  1.  CLOSED                                           CLOSED

  2.  SYN-SENT     --> <SEQ=100><CTL=SYN>              ...

  3.  SYN-RECEIVED <-- <SEQ=300><CTL=SYN>              <-- SYN-SENT

  4.               ... <SEQ=100><CTL=SYN>              --> SYN-RECEIVED

  5.  SYN-RECEIVED --> <SEQ=100><ACK=301><CTL=SYN,ACK> ...

  6.  ESTABLISHED  <-- <SEQ=300><ACK=101><CTL=SYN,ACK> <-- SYN-RECEIVED

  7.               ... <SEQ=101><ACK=301><CTL=ACK>     --> ESTABLISHED

                Simultaneous Connection Synchronization

                               Figure 8.
 */

TEST_F(SocketSessionTest, SimultaneousSyncA) {
  session_->open();
  std::unique_ptr<Segment> seg;
  seg = popSegment();
  SequenceNumber seq100 = seg->sequence_;
  reply(300, Segment::SYN);
  expectState(ConnectionState::SYN_RECEIVED);
  reply(300, seq100 + 1, Segment::SYN | Segment::ACK);
  expectState(ConnectionState::ESTABLISHED);
}

TEST_F(SocketSessionTest, ClientSendsDataTest1) {
  session_->open();
  session_->send((char *)"I Love U.", 9);
  std::unique_ptr<Segment> seg;
  seg = popSegment();
  SequenceNumber syn = seg->sequence_;

  segmentDropAll();
  reply(300, syn + 1, Segment::SYN | Segment::ACK);

  while (true) {
    seg = popSegment();
    if (seg->data_length_ > 0 && seg->data_[7] == 'U') {
      break;
    }
  }
}

TEST_F(SocketSessionTest, SendBulkData) {
  session_->open();
  SequenceNumber seq_100 = popSegment()->sequence_;
  segmentDropAll();
  char data[2000];
  for (int i = 0; i < 2000; i++) {
    data[i] = (char)i;
  }
  // We expect that the send buffer is larger than 2000 bytes.
  EXPECT_EQ(session_->send(data, 2000), 2000);

  reply(300, seq_100 + 1, Segment::SYN | Segment::ACK);
  segmentDropAll();
  time_machine_->elapse(TimeBase::Delta::fromSeconds(1));

  std::unique_ptr<Segment> seg = popSegment();
  EXPECT_EQ(seg->data_length_, 1460);
  for (int i = 0; i < 1460; i++) {
    EXPECT_EQ(seg->data_[i], (char)i);
  }
  reply(301, seq_100 + 1 + 1460, Segment::ACK);
  seg = popSegment();
  EXPECT_EQ(seg->data_length_, 2000 - 1460);
  for (int i = 1460; i < 2000; i++) {
    EXPECT_EQ(seg->data_[i - 1460], (char)i);
  }
}

TEST_F(SocketSessionTest, ServerReceiveDataTest1) {
  session_->open(seg_factory_->createSegment(300, Segment::SYN));
  std::unique_ptr<Segment> seg;
  seg = popSegment();
  SequenceNumber syn = seg->sequence_;
  reply(301, syn + 1, Segment::ACK, (char *)"Hello", 5);
  char buf[32];
  int nread = session_->receive(buf, 2);
  EXPECT_EQ(nread, 2);
  EXPECT_EQ(buf[1], 'e');

  seg = popSegment();
  EXPECT_TRUE(seg->isACK());
  EXPECT_EQ(seg->acknowledgment_, 301 + 5);
}

TEST_F(SocketSessionTest, ClientSendsDataTest2) {
  session_->open();

  std::unique_ptr<Segment> seg;
  seg = popSegment();
  SequenceNumber syn = seg->sequence_;
  reply(300, syn + 1, Segment::SYN | Segment::ACK);

  time_machine_->elapse(TimeBase::Delta::fromSeconds(3));
  session_->send((char *)"I Love U.", 9);

  while (true) {
    seg = popSegment();
    if (seg->data_length_ > 0 && seg->data_[7] == 'U') {
      break;
    }
  }
}

TEST_F(SocketSessionTest, ClientEchoTest) {
  session_->open();
  std::unique_ptr<Segment> seg;
  seg = popSegment();
  SequenceNumber syn = seg->sequence_;
  reply(300, syn + 1, Segment::SYN | Segment::ACK);
  session_->send((char *)"I Love U.", 9);
  segmentDropAll();
  reply(301, syn + 1 + 9, Segment::ACK);
  EXPECT_TRUE(!hasSegment());
  reply(301, syn + 1 + 9, Segment::ACK, (char *)"Love you too", 12);
  seg = popSegment();
  EXPECT_EQ(seg->sequence_, syn + 1 + 9);
  EXPECT_EQ(seg->acknowledgment_, 301 + 12);
}

TEST_F(SocketSessionTest, ServerEchoTest) {
  session_->open(seg_factory_->createSegment(300, Segment::SYN));
  std::unique_ptr<Segment> seg;
  seg = popSegment();
  SequenceNumber syn = seg->sequence_;
  reply(301, syn + 1, Segment::ACK, (char *)"Hello", 5);
  segmentDropAll();
  session_->send((char *)"stuvwx", 6);
  time_machine_->elapse(TimeBase::Delta::fromSeconds(3));
  while (hasSegment()) {
    seg = popSegment();
    EXPECT_FALSE(seg->isSYN());
    if (seg->data_length_ > 0 && seg->data_[5] == 'x') {
      return;
    }
  }
  EXPECT_TRUE(false);
}

/*

      TCP A                                                TCP B

  1.  ESTABLISHED                                          ESTABLISHED

  2.  (Close)
      FIN-WAIT-1  --> <SEQ=100><ACK=300><CTL=FIN,ACK>  --> CLOSE-WAIT

  3.  FIN-WAIT-2  <-- <SEQ=300><ACK=101><CTL=ACK>      <-- CLOSE-WAIT

  4.                                                       (Close)
      TIME-WAIT   <-- <SEQ=300><ACK=101><CTL=FIN,ACK>  <-- LAST-ACK

  5.  TIME-WAIT   --> <SEQ=101><ACK=301><CTL=ACK>      --> CLOSED

  6.  (2 MSL)
      CLOSED

                         Normal Close Sequence

                               Figure 13.

 */

TEST_F(SocketSessionTest, NormalCloseA) {
  session_->open();
  SequenceNumber seq_99 = popSegment()->sequence_;
  reply(299, seq_99 + 1, Segment::SYN | Segment::ACK);
  session_->close();
  expectState(ConnectionState::FIN_WAIT_1);
  segmentDropAll();
  reply(300, seq_99 + 2, Segment::ACK | Segment::FIN);
  auto seg = popSegment();
  expectState(ConnectionState::TIME_WAIT);
  time_machine_->elapse(kTwoMSL);
  expectState(ConnectionState::CLOSED);
}

TEST_F(SocketSessionTest, NormalCloseB) {
  session_->open();
  SequenceNumber seq_299 = popSegment()->sequence_;
  reply(99, seq_299 + 1, Segment::SYN | Segment::ACK);

  reply(100, seq_299 + 1, Segment::ACK | Segment::FIN);
  expectState(ConnectionState::CLOSE_WAIT);
  session_->close();
  expectState(ConnectionState::LAST_ACK);
  reply(101, seq_299 + 2, Segment::ACK);
  expectState(ConnectionState::CLOSED);
}

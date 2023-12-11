#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>
#include<iostream>
using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_cnt_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmit_cnt_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if(!queued_segments_.empty()){
    if(!timer_.is_running()){
      timer_.start();
    }
    auto msg = queued_segments_.front();
    queued_segments_.pop();
    return msg;
  }
  return {};
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  size_t cur_window_size = window_size_ == 0 ? 1 : window_size_;
  while(outstanding_cnt_ < cur_window_size){
    TCPSenderMessage msg;
    if(!syn_){
      syn_ = msg.SYN = true;
      outstanding_cnt_ += 1;
    }
    msg.seqno = Wrap32::wrap(next_seqno_, isn_);

    auto const payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, cur_window_size - outstanding_cnt_);
    read(outbound_stream, payload_size, msg.payload);
    outstanding_cnt_ += msg.payload.size();

    if(!fin_ && outbound_stream.is_finished() && outstanding_cnt_ < cur_window_size){
      fin_ = msg.FIN = true;
      outstanding_cnt_ += 1;
    }

    if(msg.sequence_length() == 0)
      break;

    next_seqno_ += msg.sequence_length();
    queued_segments_.push( msg );
    outstanding_segments_.push(msg);

    if(msg.FIN)
      break;

  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  auto seqno = Wrap32::wrap(next_seqno_, isn_);
  return {seqno, false, {}, false};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  window_size_ = msg.window_size;
  if(msg.ackno.has_value()){
    auto ackno = msg.ackno.value().unwrap(isn_, next_seqno_);
    if(ackno > next_seqno_)
      return;
    acked_seqno_ = ackno;

    while(!outstanding_segments_.empty()){
      auto &front_msg = outstanding_segments_.front();
      if(front_msg.seqno.unwrap(isn_, next_seqno_) + front_msg.sequence_length() <= acked_seqno_){
        outstanding_cnt_ -= front_msg.sequence_length();
        outstanding_segments_.pop();
        retransmit_cnt_ = 0;
        timer_.reset_RTO();
        if(!outstanding_segments_.empty()){
          timer_.start();
        }
      }else break;
    }

    if(outstanding_segments_.empty()){
      timer_.stop();
    }
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  timer_.tick(ms_since_last_tick);
  if(timer_.is_expired()){
    queued_segments_.push(outstanding_segments_.front());
    if(window_size_ != 0){
      ++retransmit_cnt_;  
      timer_.double_RTO();
    }
    timer_.start();
  }
  
}

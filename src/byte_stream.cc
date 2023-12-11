#include <stdexcept>

#include "byte_stream.hh"
#include<iostream>
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  uint64_t len = data.length();
  if (len == 0 || available_capacity() == 0)
    return;
  uint64_t n = min(len, capacity_ - total_buffered);
  buffer.push_back(std::move(data.substr(0, n)));
  
  buffer_view.emplace_back(buffer.back().c_str(), n);
  total_pushed += n;
  total_buffered += n;

}

void Writer::close()
{
  // Your code here.
  is_close = true;
}

void Writer::set_error()
{
  // Your code here.
  is_error = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return is_close;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - total_buffered;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_pushed;
}

string_view Reader::peek() const
{
  // Your code here.
  if (total_buffered == 0)
    return {};
  return buffer_view.front();
}

bool Reader::is_finished() const
{
  // Your code here.
  return is_close && total_buffered == 0;
}

bool Reader::has_error() const
{
  // Your code here.
  return is_error;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  auto pop_len = min(total_buffered, len);
  
  while(pop_len > 0){
    auto sz = buffer_view.front().size();
    if(sz > pop_len){  
      buffer_view.front().remove_prefix(pop_len);
      total_poped += pop_len;
      total_buffered -= pop_len;
      return;
    }
  
    buffer_view.pop_front();
    total_poped += sz;
    total_buffered -= sz;
    pop_len -= sz;
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return total_buffered;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return total_poped;
}

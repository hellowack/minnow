#include "reassembler.hh"
#include<iostream>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // cout<<"——enter insert——"<<endl;
  // Your code here.
  if (data.empty()){
    if(is_last_substring){
      output.close();
    }
    return;
  }

  if(output.available_capacity() == 0){
    return;
  }

  auto const last_index = first_index + data.size();
  auto const first_unacceptable_index = first_unassembled_index + output.available_capacity();
  //bytes lie beyond the unassembled range
  if(first_index >= first_unacceptable_index || last_index <= first_unassembled_index){
    return;
  }
  //bytes overlap the range [first_unassembled_index, first_unacceptable_index]
  if(last_index > first_unacceptable_index){
    // cout<<last_index<<" "<<first_unacceptable_index<<endl;
    data = data.substr(0, data.size() - (last_index - first_unacceptable_index));
    // cout<<"cut last data:"<<data<<endl;
    is_last_substring = false;
  }

  //bytes between the range [first_unassembled_index, first_unacceptable_index] into buffer
  if(first_index > first_unassembled_index){
    insert_into_buffer(first_index, std::move(data), is_last_substring);
    return;
  }

  if(first_index < first_unassembled_index){
    // cout<<last_index<<" "<<first_unacceptable_index<<endl;
    data = data.substr(first_unassembled_index - first_index);
    // cout<<"cut first data:"<<data<<endl;
  }

  first_unassembled_index += data.size();
  // cout<<"data:"<<data<<endl;
  output.push(std::move(data));
  // cout<<"capacity:"<<output.available_capacity()<<endl;
  if(is_last_substring){
    output.close();
  }

  if(!buffer.empty() && buffer.begin()->first <= first_unassembled_index){
    pop_from_buffer(output);
  }
}

void Reassembler::insert_into_buffer( const uint64_t first_index, std::string&& data, const bool is_last_substring )
{
  // cout<<"——enter insert_into_buffer——"<<endl;
  // cout<<"first_index:"<<first_index<<endl;
  // cout<<"data:"<<data<<endl;
  auto begin_index = first_index;
  const auto end_index = first_index + data.size();

  for ( auto it = buffer.begin(); it != buffer.end() && begin_index < end_index; ) {
    if ( it->first <= begin_index ) {
      begin_index = max( begin_index, it->first + it->second.size() );
      ++it;
      continue;
    }

    if ( begin_index == first_index && end_index <= it->first ) {
      buffer_size += data.size();
      buffer.emplace( it, first_index, std::move( data ) );
      return;
    }

    const auto right_index = min( it->first, end_index );
    const auto len = right_index - begin_index;

    buffer.emplace( it, begin_index, data.substr( begin_index - first_index, len ) );
    buffer_size += len;
    begin_index = right_index;
  }

  if ( begin_index < end_index ) {
    //cout<<"data:"<<data.substr(0, end_index - begin_index )<<endl;
    buffer_size += end_index - begin_index;
    buffer.emplace_back( begin_index, data.substr(begin_index - first_index, end_index - begin_index ) );
  }

  if ( is_last_substring ) {
    has_last = true;
  }
}

void Reassembler::pop_from_buffer( Writer& output )
{
  // cout<<"——pop_from_buffer——"<<endl;
  for ( auto it = buffer.begin(); it != buffer.end(); ) {
    if ( it->first > first_unassembled_index ) {
      // cout<<"it->first > first_unassembled_index"<<endl;
      break;
    }
    // it->first <= first_unassembled_index
    const auto end = it->first + it->second.size();
    // cout<<"it:"<<it->first<<" "<<it->second<<endl;
    // cout<<"end:"<<end<<endl;
    if ( end <= first_unassembled_index ) {
      buffer_size -= it->second.size();
      // cout<<"end <= first_unassembled_index"<<endl;
    } else {
      auto data = std::move( it->second );
      buffer_size -= data.size();
      if ( it->first <= first_unassembled_index ) {
        data = data.substr( first_unassembled_index - it->first);
        // cout<<"data:"<<data<<endl;
      }
      first_unassembled_index += data.size();
      output.push( std::move( data ) );
    }
    it = buffer.erase( it );
  }

  if ( buffer.empty() && has_last ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return buffer_size;
}

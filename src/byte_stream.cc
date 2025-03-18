#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  uint64_t now_capacity {available_capacity()};
  if(is_closed() or available_capacity()==0 or data.empty()){
    return;
  } 
  if(data.size()>available_capacity()){
    data.resize(now_capacity);
  }
  total_buffered_ +=data.size();
  total_pushed_ +=data.size();

  stream_.emplace(std::move(data));
}

void Writer::close()
{
  closed_ = true;
}

bool Writer::is_closed() const
{
  return closed_; // Your code here.
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - total_buffered_; 
}

uint64_t Writer::bytes_pushed() const
{
  return total_pushed_; 
}

string_view Reader::peek() const
{
  return stream_.empty()? string_view{} : string_view{stream_.front()}.substr(removed_); 
}

void Reader::pop( uint64_t len )
{
  total_buffered_ -= len;
  total_poped_ += len;
  while(len != 0){
    const uint64_t size {stream_.front().size() - removed_};
    if(len < size){
      removed_ +=len;
      break;
    }
    stream_.pop();
    len -=size;
    removed_ = 0;
  }
}

bool Reader::is_finished() const
{
  return closed_ and total_buffered_ == 0; // Your code here.
}

uint64_t Reader::bytes_buffered() const
{
  return total_buffered_; 
}

uint64_t Reader::bytes_popped() const
{
  return total_poped_; 
}

